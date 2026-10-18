// Fase Groove Drums - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Plug-in level tests (they need JUCE, unlike EngineTests): project state, grid editing,
// styles, exported MIDI files, DAW transport and the guide kit.

#include "../Source/PluginProcessor.h"
#include "../Source/Localisation.h"
#include "../Source/PluginEditor.h"

#include <cstdio>
#include <set>

namespace
{
    int failures = 0;

    #define CHECK(cond) \
        do { if (! (cond)) { ++failures; std::printf ("FAILED %s:%d  %s\n", __FILE__, __LINE__, #cond); } } while (false)

    struct PlayHead : juce::AudioPlayHead
    {
        double ppq = 0.0, bpm = 124.0;
        bool playing = true;
        juce::Optional<PositionInfo> getPosition() const override
        {
            PositionInfo p;
            p.setBpm (bpm);
            p.setPpqPosition (ppq);
            p.setIsPlaying (playing);
            return p;
        }
    };

    float value (FaseGrooveDrumsProcessor& p, const juce::String& id)
    {
        return p.apvts.getRawParameterValue (id)->load();
    }

    int countNoteOns (const juce::MidiFile& file, int note = -1, int channel = -1)
    {
        int count = 0;
        for (int t = 0; t < file.getNumTracks(); ++t)
            for (const auto* e : *file.getTrack (t))
                if (e->message.isNoteOn() && (note < 0 || e->message.getNoteNumber() == note)
                    && (channel < 0 || e->message.getChannel() == channel))
                    ++count;
        return count;
    }

    //==============================================================================
    void testStateRoundTrip()
    {
        FaseGrooveDrumsProcessor a;
        a.setParameterFromUI (ParamIDs::swing, 0.4f);
        a.setParameterFromUI (ParamIDs::kit, 2.0f);
        a.setParameterFromUI (ParamIDs::note (0), 24.0f);
        a.editStep (1, 3, 'f');   // turns the groove into the user pattern
        a.editorScale = 1.3f;

        juce::MemoryBlock data;
        a.getStateInformation (data);

        FaseGrooveDrumsProcessor b;
        b.setStateInformation (data.getData(), (int) data.getSize());
        CHECK (std::abs (value (b, ParamIDs::swing) - 0.4f) < 1.0e-4f);
        CHECK (juce::roundToInt (value (b, ParamIDs::kit)) == 2);
        CHECK (b.laneNote (0) == 24);
        CHECK (juce::roundToInt (value (b, ParamIDs::pattern)) == fgd::userPatternIndex());
        CHECK (b.stepSymbol (1, 3) == 'f');
        CHECK (std::abs (b.editorScale - 1.3f) < 1.0e-4f);

        // The grid of the user pattern is the groove that was showing, plus the edit.
        for (int l = 0; l < fgd::numLanes; ++l)
            for (int s = 0; s < 32; ++s)
                CHECK (a.stepSymbol (l, s) == b.stepSymbol (l, s));

        // Garbage doesn't crash or change anything.
        const char junk[] = "not a state";
        b.setStateInformation (junk, (int) sizeof (junk));
        CHECK (b.stepSymbol (1, 3) == 'f');
    }

    void testGridEditing()
    {
        FaseGrooveDrumsProcessor p;
        const auto before = p.displayedPattern();
        CHECK (p.stepSymbol (0, 0) == before.lanes[0][0]);

        p.editStep (0, 1, 'X');
        CHECK (juce::roundToInt (value (p, ParamIDs::pattern)) == fgd::userPatternIndex());
        CHECK (p.stepSymbol (0, 1) == 'X');
        CHECK (p.stepSymbol (0, 0) == before.lanes[0][0]);   // the rest of the groove stays

        p.editStep (0, 1, '?');   // unknown symbols are rests
        CHECK (p.stepSymbol (0, 1) == '.');

        p.clearLane (0);
        for (int s = 0; s < 32; ++s)
            CHECK (p.stepSymbol (0, s) == '.');

        p.clearGrid();
        for (int l = 0; l < fgd::numLanes; ++l)
            for (int s = 0; s < 32; ++s)
                CHECK (p.stepSymbol (l, s) == '.');

        // NEW BEAT: the random beat with a new seed.
        const auto seed = value (p, ParamIDs::riffSeed);
        p.rollNewBeat();
        CHECK (juce::roundToInt (value (p, ParamIDs::pattern)) == fgd::randomPatternIndex());
        CHECK (std::abs (value (p, ParamIDs::riffSeed) - seed) > 0.5f);
    }

    void testStyles()
    {
        FaseGrooveDrumsProcessor p;
        const auto names = FaseGrooveDrumsProcessor::styleNames();
        CHECK (names.size() == (int) fgd::grooveCategories().size());

        for (int i = 0; i < names.size(); ++i)
        {
            p.applyStyle (i);
            const auto& groove = fgd::drumPatterns()[(size_t) juce::roundToInt (value (p, ParamIDs::pattern))];
            CHECK (groove.category == fgd::grooveCategories()[(size_t) i]);   // a groove of its genre
            CHECK (std::abs (value (p, ParamIDs::swing) - fgd::genreSwing (i)) < 1.0e-3f);
        }

        p.resetToDefaults();
        CHECK (juce::roundToInt (value (p, ParamIDs::pattern)) == 0);
        CHECK (juce::roundToInt (value (p, ParamIDs::midiChannel)) == 10);
    }

    void testMidiExport()
    {
        FaseGrooveDrumsProcessor p;
        p.setParameterFromUI (ParamIDs::tempoSync, 0.0f);
        p.setParameterFromUI (ParamIDs::tempo, 122.0f);
        p.setParameterFromUI (ParamIDs::fills, 0.0f);
        p.setParameterFromUI (ParamIDs::variation, 0.0f);

        const auto clip = p.createMidiClip (4, FaseGrooveDrumsProcessor::ClipPart::all);
        CHECK (clip.getNumTracks() == 1);
        CHECK (countNoteOns (clip, 36, 10) == 16);   // four on the floor, on the drum channel
        CHECK (countNoteOns (clip, -1, 1) == 0);

        // The tempo of the file.
        juce::MidiMessageSequence tempos;
        clip.findAllTempoEvents (tempos);
        CHECK (tempos.getNumEvents() == 1);
        if (tempos.getNumEvents() == 1)
            CHECK (std::abs (60.0 / tempos.getEventPointer (0)->message.getTempoSecondsPerQuarterNote() - 122.0) < 0.01);

        // One track per drum: a conductor track plus one per drum that plays.
        const auto tracks = p.createMidiClip (4, FaseGrooveDrumsProcessor::ClipPart::tracks);
        CHECK (tracks.getNumTracks() > 3);
        CHECK (countNoteOns (tracks) == countNoteOns (clip));

        // Another channel, a remapped kick.
        p.setParameterFromUI (ParamIDs::midiChannel, 1.0f);
        p.setParameterFromUI (ParamIDs::note (0), 48.0f);
        const auto remapped = p.createMidiClip (4, FaseGrooveDrumsProcessor::ClipPart::all);
        CHECK (countNoteOns (remapped, 48, 1) == 16);
        CHECK (countNoteOns (remapped, 36) == 0);

        // A muted drum is left out.
        p.setLaneMuted (0, true);
        CHECK (countNoteOns (p.createMidiClip (4, FaseGrooveDrumsProcessor::ClipPart::all), 48) == 0);
        p.setLaneMuted (0, false);

        // The whole song: 60 bars with markers.
        const auto song = p.createMidiClip (FaseGrooveDrumsProcessor::songArrangement, FaseGrooveDrumsProcessor::ClipPart::all);
        std::set<juce::String> markers;
        for (const auto* e : *song.getTrack (0))
            if (e->message.isTextMetaEvent() && e->message.getMetaEventType() == 6)
                markers.insert (e->message.getTextFromTextMetaEvent());
        CHECK (markers.count ("Intro") == 1 && markers.count ("Drop") == 1 && markers.count ("Outro") == 1);
        CHECK (std::abs (song.getLastTimestamp() - 60.0 * 4.0 * 960.0) < 1.0);

        // The file on disk, with a readable name.
        const auto file = p.writeMidiClip (0, FaseGrooveDrumsProcessor::ClipPart::all);
        CHECK (file.existsAsFile());
        CHECK (file.getFileName().startsWith ("Drums "));
        CHECK (file.getFileName().contains ("122bpm"));
        file.deleteFile();
    }

    void testTransport()
    {
        FaseGrooveDrumsProcessor p;
        PlayHead head;
        p.setPlayHead (&head);
        p.setParameterFromUI (ParamIDs::fills, 0.0f);
        p.setParameterFromUI (ParamIDs::variation, 0.0f);
        p.setParameterFromUI (ParamIDs::humanize, 0.0f);
        p.setRateAndBufferSizeDetails (48000.0, 512);
        p.prepareToPlay (48000.0, 512);

        juce::AudioBuffer<float> buffer (2, 512);
        int kicks = 0;
        bool sound = false;
        const double beatsPerBlock = 512.0 / (48000.0 * 60.0 / head.bpm);
        const int blocks = (int) std::ceil (8.0 / beatsPerBlock);   // two bars
        for (int b = 0; b < blocks; ++b)
        {
            head.ppq = b * beatsPerBlock;
            juce::MidiBuffer midi;
            p.processBlock (buffer, midi);
            for (const auto m : midi)
                if (m.getMessage().isNoteOn() && m.getMessage().getNoteNumber() == 36 && head.ppq + m.samplePosition / (48000.0 * 60.0 / head.bpm) < 7.99)
                    ++kicks;
            sound = sound || buffer.getMagnitude (0, 512) > 0.01f;
        }
        CHECK (kicks == 8);
        CHECK (sound);   // the guide kit plays

        // Stop: everything is released, nothing new starts.
        head.playing = false;
        juce::MidiBuffer midi;
        p.processBlock (buffer, midi);
        for (const auto m : midi)
            CHECK (! m.getMessage().isNoteOn());

        // The sequencer off: the DAW plays, the groove doesn't.
        head.playing = true;
        p.setParameterFromUI (ParamIDs::sequencer, 0.0f);
        for (int b = 0; b < 40; ++b)
        {
            juce::MidiBuffer m2;
            head.ppq = 100.0 + b * beatsPerBlock;
            p.processBlock (buffer, m2);
            for (const auto m : m2)
                CHECK (! m.getMessage().isNoteOn());
        }

        // ...but PLAY still previews it (following the DAW's position while it plays).
        p.previewPlay();
        int notes = 0;
        for (int b = 0; b < 40; ++b)
        {
            juce::MidiBuffer m2;
            head.ppq = 200.0 + b * beatsPerBlock;
            p.processBlock (buffer, m2);
            for (const auto m : m2)
                notes += m.getMessage().isNoteOn() ? 1 : 0;
        }
        CHECK (notes > 0);
        p.setPlayHead (nullptr);
    }

    void testV020()
    {
        auto temp = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("fgd-tests");
        temp.deleteRecursively();
        temp.createDirectory();

        FaseGrooveDrumsProcessor p;

        // A-D parts: the grid shows the order you hear; an edit lands in the part shown.
        p.editStep (0, 2, 'X');
        p.setSegmentAt (1, 0);   // A A C D
        CHECK (p.stepSymbol (0, 10) == 'X');
        p.editStep (0, 11, 'g');   // shown in the second half bar = part A
        CHECK (p.stepSymbol (0, 3) == 'g');
        p.setSegmentAt (1, 1);

        // Dice: one drum, one part; locked parts stay.
        const auto before = p.displayedPattern();
        p.setSegmentLocked (0, true);
        for (int i = 0; i < 5; ++i)
            p.randomizeLane ((int) fgd::Lane::conga);
        for (int s = 0; s < 8; ++s)
            CHECK (p.stepSymbol ((int) fgd::Lane::conga, s) == before.lanes[(size_t) fgd::Lane::conga][(size_t) s]);
        p.randomizeSegment (0);
        for (int l = 0; l < fgd::numLanes; ++l)
            for (int s = 0; s < 8; ++s)
                CHECK (p.stepSymbol (l, s) == before.lanes[(size_t) l][(size_t) s] || (l == (int) fgd::Lane::kick && (s == 2 || s == 3)));
        p.setSegmentLocked (0, false);
        p.totalRandom();   // never crashes, always a groove
        CHECK (p.displayedPattern().length() == p.steps());

        // Velocity: by hand, shapes, back to automatic; it plays.
        p.clearGrid();
        for (int s = 0; s < 32; s += 2)
            p.editStep ((int) fgd::Lane::closedHat, s, 'x');
        p.applyVelocityShape ((int) fgd::Lane::closedHat, 0);
        CHECK (p.stepVelocity ((int) fgd::Lane::closedHat, 2) > p.stepVelocity ((int) fgd::Lane::closedHat, 0));
        p.setStepVelocity ((int) fgd::Lane::closedHat, 0, 33);
        CHECK (p.effectiveVelocity ((int) fgd::Lane::closedHat, 0) == 33);
        p.setParameterFromUI (ParamIDs::humanize, 0.0f);
        p.setParameterFromUI (ParamIDs::velocity, 100.0f);
        const auto clip = p.createMidiClip (1, FaseGrooveDrumsProcessor::ClipPart::all);
        bool found = false;
        for (const auto* e : *clip.getTrack (0))
            if (e->message.isNoteOn() && e->message.getNoteNumber() == 42 && e->message.getTimeStamp() < 1.0)
                found = e->message.getVelocity() == 33;
        CHECK (found);
        p.resetStepVelocities ((int) fgd::Lane::closedHat);
        CHECK (! p.hasStepVelocities());

        // Undo snapshots bring everything back.
        const auto snap = p.snapshot();
        p.editStep (5, 5, 'X');
        p.setSongBars (2, 0);
        p.restoreSnapshot (snap);
        CHECK (p.stepSymbol (5, 5) == '.');
        CHECK (p.songArrangementParts()[2].bars == 8);

        // The song: bars, own variation, own fill; saved with the project.
        p.setSongBars (0, 4);
        p.regenerateSongPart (1);
        p.setSongFill (1, (int) fgd::FillStyle::tomRun);
        juce::MemoryBlock data;
        p.getStateInformation (data);
        FaseGrooveDrumsProcessor q;
        q.setStateInformation (data.getData(), (int) data.getSize());
        CHECK (q.songArrangementParts()[0].bars == 4);
        CHECK (q.songArrangementParts()[1].variation == p.songArrangementParts()[1].variation);
        CHECK (q.songArrangementParts()[1].endFill == (int) fgd::FillStyle::tomRun);
        CHECK (q.clipLengthInBars (FaseGrooveDrumsProcessor::songArrangement) == 56);
        CHECK (q.saveSongSections (temp.getChildFile ("sections"), FaseGrooveDrumsProcessor::ClipPart::all) == 6);

        // Your own sample: loaded, played by the guide kit, saved with the project.
        auto wav = temp.getChildFile ("kick.wav");
        {
            juce::WavAudioFormat format;
            std::unique_ptr<juce::AudioFormatWriter> writer (format.createWriterFor (new juce::FileOutputStream (wav), 44100.0, 1, 16, {}, 0));
            juce::AudioBuffer<float> b (1, 4410);
            for (int i = 0; i < 4410; ++i)
                b.setSample (0, i, 0.8f * std::sin ((float) i * 0.05f));
            writer->writeFromAudioSampleBuffer (b, 0, 4410);
        }
        CHECK (p.loadSample (0, wav));
        CHECK (p.samplePath (0) == wav.getFullPathName());
        CHECK (! p.loadSample (0, temp.getChildFile ("missing.wav")));
        juce::MemoryBlock withSample;
        p.getStateInformation (withSample);
        FaseGrooveDrumsProcessor r;
        r.setStateInformation (withSample.getData(), (int) withSample.getSize());
        CHECK (r.samplePath (0) == wav.getFullPathName());
        r.setRateAndBufferSizeDetails (44100.0, 256);
        r.prepareToPlay (44100.0, 256);
        juce::AudioBuffer<float> audio (2, 256);
        juce::MidiBuffer in;
        in.addEvent (juce::MidiMessage::noteOn (10, 36, (juce::uint8) 120), 0);
        r.processBlock (audio, in);
        CHECK (audio.getMagnitude (0, 256) > 0.1f);
        r.clearSample (0);
        CHECK (r.samplePath (0).isEmpty());

        // Presets and saved grooves.
        p.setParameterFromUI (ParamIDs::swing, 0.33f);
        const auto presetFile = FaseGrooveDrumsProcessor::presetFolder().getChildFile ("fgd test preset.fgdpreset");
        CHECK (p.savePreset ("fgd test preset"));
        p.setParameterFromUI (ParamIDs::swing, 0.0f);
        CHECK (p.loadPreset (presetFile));
        CHECK (std::abs (value (p, ParamIDs::swing) - 0.33f) < 1.0e-3f);
        presetFile.deleteFile();

        p.editStep (2, 7, 'f');
        const auto shown = p.displayedPattern();
        const auto grooveFile = FaseGrooveDrumsProcessor::grooveFolder().getChildFile ("fgd test groove.fgdgroove");
        CHECK (p.saveGroove ("fgd test groove"));
        p.clearGrid();
        CHECK (p.loadGroove (grooveFile));
        CHECK (p.displayedPattern().lanes == shown.lanes);
        grooveFile.deleteFile();

        // The drums of a MIDI file go into the grid.
        juce::MidiMessageSequence track;
        for (int beat = 0; beat < 4; ++beat)
        {
            track.addEvent (juce::MidiMessage::noteOn (10, 36, (juce::uint8) 127), beat * 480.0);
            track.addEvent (juce::MidiMessage::noteOff (10, 36), beat * 480.0 + 100.0);
            track.addEvent (juce::MidiMessage::noteOn (10, 42, (juce::uint8) 40), beat * 480.0 + 240.0);
            track.addEvent (juce::MidiMessage::noteOff (10, 42), beat * 480.0 + 300.0);
        }
        track.addEvent (juce::MidiMessage::noteOn (1, 60, (juce::uint8) 100), 0.0);   // not a drum
        juce::MidiFile midi;
        midi.setTicksPerQuarterNote (480);
        midi.addTrack (track);
        auto midFile = temp.getChildFile ("beat.mid");
        {
            juce::FileOutputStream out (midFile);
            midi.writeTo (out);
        }
        CHECK (p.importMidiGroove (midFile));
        CHECK (p.stepSymbol (0, 0) == 'X' && p.stepSymbol (0, 4) == 'X' && p.stepSymbol (0, 20) == 'X');   // one bar: twice
        CHECK (p.stepSymbol ((int) fgd::Lane::closedHat, 2) == 'g');
        CHECK (p.stepVelocity ((int) fgd::Lane::closedHat, 2) == 40);
        CHECK (p.stepSymbol (0, 1) == '.');

        // Parts of one bar: a 4-bar groove, the grid (and the user pattern) four bars long.
        FaseGrooveDrumsProcessor four;
        four.setParameterFromUI (ParamIDs::partLength, 1.0f);
        CHECK (four.steps() == 64);
        CHECK (four.displayedPattern().length() == 64);
        four.editStep (1, 60, 'X');   // bar 4
        CHECK (four.stepSymbol (1, 60) == 'X');
        CHECK (four.stepSymbol (1, 28) != 'X' || four.displayedPattern().lanes[1][28] == 'X');
        four.setSegmentAt (3, 0);     // A A' ... the last bar plays the first
        CHECK (four.stepSymbol (0, 48) == four.stepSymbol (0, 0));
        const auto clip4 = four.createMidiClip (0, FaseGrooveDrumsProcessor::ClipPart::all);
        CHECK (four.clipLengthInBars (0) % 4 == 0);
        juce::ignoreUnused (clip4);

        temp.deleteRecursively();
    }
}

namespace
{
    /** --screenshots <folder>: the editor and its views as PNG files (to check the layout). */
    void saveScreenshots (const juce::File& folder)
    {
        folder.createDirectory();
        FaseGrooveDrumsProcessor p;
        p.editStep (4, 3, 'x');
        std::unique_ptr<juce::AudioProcessorEditor> editor (p.createEditor());
        editor->setSize (1180, 860);

        auto save = [&] (const juce::String& name)
        {
            const auto image = editor->createComponentSnapshot (editor->getLocalBounds(), true, 1.0f);
            juce::PNGImageFormat png;
            juce::FileOutputStream out (folder.getChildFile (name + ".png"));
            out.setPosition (0);
            out.truncate();
            png.writeImageToStream (image, out);
        };
        std::function<juce::Button* (juce::Component&, const juce::String&)> findButton = [&] (juce::Component& c, const juce::String& text) -> juce::Button*
        {
            for (auto* child : c.getChildren())
            {
                if (auto* b = dynamic_cast<juce::Button*> (child); b != nullptr && b->getButtonText() == text)
                    return b;
                if (auto* found = findButton (*child, text))
                    return found;
            }
            return nullptr;
        };
        auto click = [&] (const juce::String& text) { if (auto* b = findButton (*editor, tr (text)); b != nullptr && b->onClick) b->onClick(); };

        save ("1-main");
        click ("VELOCITY");
        p.applyVelocityShape ((int) fgd::Lane::closedHat, 0);
        save ("2-velocity");
        click ("MIXER");
        save ("2b-mixer");
        click ("STEP GRID");
        p.setParameterFromUI (ParamIDs::pattern, 5.0f);
        std::function<juce::Button* (juce::Component&)> grooveButton = [&] (juce::Component& c) -> juce::Button*
        {
            for (auto* child : c.getChildren())
            {
                if (auto* b = dynamic_cast<juce::Button*> (child); b != nullptr && b->getTooltip().startsWith ("The groove: click"))
                    return b;
                if (auto* found = grooveButton (*child))
                    return found;
            }
            return nullptr;
        };
        if (auto* b = grooveButton (*editor)) b->onClick();
        save ("2c-browser");
        if (auto* b = findButton (*editor, tr ("CLOSE"))) b->onClick();
        click ("FULL SONG...");
        save ("3-song");
        click ("CLOSE");
        click ("OPTIONS");
        save ("4-about");
        std::function<void (juce::Component&)> closeOverlays = [&] (juce::Component& c)
        {
            for (auto* child : c.getChildren())
            {
                if (dynamic_cast<SongOverlay*> (child) != nullptr || dynamic_cast<AboutPanel*> (child) != nullptr
                    || dynamic_cast<GrooveBrowser*> (child) != nullptr)
                    child->setVisible (false);
                closeOverlays (*child);
            }
        };
        closeOverlays (*editor);
        p.setParameterFromUI (ParamIDs::partLength, 1.0f);
        p.setParameterFromUI (ParamIDs::pattern, 2.0f);
        if (auto* e = dynamic_cast<juce::Timer*> (editor.get())) e->timerCallback();
        save ("5-four-bars");
    }
}

namespace
{
    void testTranslations()
    {
        const auto before = Localisation::current();
        Localisation::setCurrent (Localisation::Language::italian);
        CHECK (tr ("CLOSE") == "CHIUDI");
        CHECK (tr ("Kick") == "Kick");                            // technical terms stay in English
        CHECK (tr ("Every 4 bars") == "Ogni 4 bar");
        CHECK (tr ("Some groove name") == "Some groove name");   // no translation: as it is

        // A bound text follows the language at once.
        juce::TextButton b;
        Localisation::bindText (b, "CLOSE");
        CHECK (b.getButtonText() == "CHIUDI");
        Localisation::setCurrent (Localisation::Language::german);
        CHECK (b.getButtonText() == "SCHLIESSEN");
        Localisation::setCurrent (Localisation::Language::english);
        CHECK (b.getButtonText() == "CLOSE");

        // Every language has a text for every entry.
        for (int lang = 1; lang < Localisation::numLanguages; ++lang)
        {
            Localisation::setCurrent ((Localisation::Language) lang);
            for (const auto* key : { "Groove feel & song", "Whole song", "CLOSE", "Song arrangement" })
                CHECK (tr (key).isNotEmpty() && tr (key) != key);
            for (const auto* key : { "Kick", "Snare roll", "Bar", "Fill", "Swing", "Velocity", "Build-up" })
                CHECK (tr (key) == key);
        }
        Localisation::setCurrent (before);
    }
}

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI juce;

    if (argc > 3 && juce::String (argv[1]) == "--screenshots")
        Localisation::setCurrent ((Localisation::Language) juce::String (argv[3]).getIntValue());
    if (argc > 2 && juce::String (argv[1]) == "--screenshots")
    {
        saveScreenshots (juce::File::getCurrentWorkingDirectory().getChildFile (argv[2]));
        return 0;
    }

    testStateRoundTrip();
    testGridEditing();
    testStyles();
    testMidiExport();
    testTransport();
    testV020();
    testTranslations();

    if (failures == 0)
        std::printf ("All plug-in tests passed.\n");
    return failures == 0 ? 0 : 1;
}
