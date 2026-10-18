// Fase Groove Drums - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Plug-in level tests (they need JUCE, unlike EngineTests): project state, grid editing,
// styles, exported MIDI files, DAW transport and the guide kit.

#include "../Source/PluginProcessor.h"

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
            for (int s = 0; s < FaseGrooveDrumsProcessor::gridSteps; ++s)
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
        for (int s = 0; s < FaseGrooveDrumsProcessor::gridSteps; ++s)
            CHECK (p.stepSymbol (0, s) == '.');

        p.clearGrid();
        for (int l = 0; l < fgd::numLanes; ++l)
            for (int s = 0; s < FaseGrooveDrumsProcessor::gridSteps; ++s)
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
}

int main()
{
    juce::ScopedJuceInitialiser_GUI juce;

    testStateRoundTrip();
    testGridEditing();
    testStyles();
    testMidiExport();
    testTransport();

    if (failures == 0)
        std::printf ("All plug-in tests passed.\n");
    return failures == 0 ? 0 : 1;
}
