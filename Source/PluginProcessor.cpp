// Fase Groove Drums - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    // Choice lists. Items are only ever appended, so saved projects keep their values.
    const int fillBars[] { 0, 1, 2, 4, 8 };
    const int variationBars[] { 0, 4, 8 };

    juce::String percent (float v, int) { return juce::String (juce::roundToInt (v * 100.0f)) + "%"; }

    juce::StringArray patternNames()
    {
        juce::StringArray names;
        for (const auto& p : fgd::drumPatterns())
            names.add (p.name);
        names.add ("Random beat");
        names.add ("User pattern");
        return names;
    }

    template <typename Fn>
    juce::StringArray namesFrom (int count, Fn&& name)
    {
        juce::StringArray names;
        for (int i = 0; i < count; ++i)
            names.add (name (i));
        return names;
    }

    struct Style
    {
        const char* name;
        const char* groove;
        float humanize;
        int fills;       // index in fillBars
        fgd::FillStyle fillStyle;
        int kit;
        int variation;   // index in variationBars
    };

    // One per genre, in grooveCategories() order: the genre's swing comes from fgd::genreSwing.
    const Style styles[] {
        { "Piano House",       "Piano House Classic",  0.15f, 4, fgd::FillStyle::snareRoll, 0, 1 },
        { "Soulful",           "Soulful Sunday",       0.30f, 4, fgd::FillStyle::clapBuild, 2, 1 },
        { "Deep",              "Deep Rim Shot",        0.25f, 0, fgd::FillStyle::hatLift,   1, 2 },
        { "Gospel",            "Gospel Tambourine",    0.30f, 3, fgd::FillStyle::snareRoll, 2, 0 },
        { "Jackin'",           "Chicago 909",          0.20f, 4, fgd::FillStyle::hatLift,   0, 1 },
        { "Jackin' Piano",     "Jackin' Piano Bounce", 0.20f, 4, fgd::FillStyle::mix,       0, 1 },
        { "Garage & UK",       "Two-Step",             0.25f, 4, fgd::FillStyle::dropOut,   1, 1 },
        { "Afro & Latin",      "Afro House",           0.30f, 4, fgd::FillStyle::congaRun,  0, 2 },
        { "Funky & Disco",     "Disco Classic",        0.30f, 3, fgd::FillStyle::tomRun,    2, 0 },
        { "'90s Piano House",  "90s Italo",            0.10f, 4, fgd::FillStyle::snareRoll, 0, 1 },
        { "Neo Soul & Broken", "Neo Soul Pocket",      0.40f, 0, fgd::FillStyle::snareRoll, 3, 2 },
        { "Chill & Pads",      "Chill House",          0.30f, 0, fgd::FillStyle::hatLift,   3, 2 },
        { "Dance Stabs",       "Dance Floor",          0.05f, 4, fgd::FillStyle::snareRoll, 0, 1 },
    };

    int indexOfGroove (const char* name)
    {
        const auto& presets = fgd::drumPatterns();
        for (size_t i = 0; i < presets.size(); ++i)
            if (presets[i].name == name)
                return (int) i;
        return 0;
    }
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout FaseGrooveDrumsProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;

    // Parameters are grouped (Groove, Feel, Song, Tempo & MIDI, Sound, Kit) so the DAW's
    // automation list is easy to browse. Groups don't change the parameter IDs.
    std::vector<std::unique_ptr<RangedAudioParameter>> current;
    auto add = [&current] (auto param) { current.push_back (std::move (param)); };
    auto group = [&layout, &current] (const String& id, const String& name)
    {
        auto g = std::make_unique<AudioProcessorParameterGroup> (id, name, " | ");
        for (auto& p : current)
            g->addChild (std::move (p));
        current.clear();
        layout.add (std::move (g));
    };
    auto choice = [&add] (const String& id, const String& name, const StringArray& items, int def)
    {
        add (std::make_unique<AudioParameterChoice> (ParameterID { id, 1 }, name, items, def));
    };
    auto unitFloat = [&add] (const String& id, const String& name, float lo, float hi, float def)
    {
        add (std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, name, NormalisableRange<float> (lo, hi),
                                                    def, AudioParameterFloatAttributes().withStringFromValueFunction (percent)));
    };
    auto toggle = [&add] (const String& id, const String& name, bool def)
    {
        add (std::make_unique<AudioParameterBool> (ParameterID { id, 1 }, name, def));
    };

    // Groove
    choice (ParamIDs::pattern, "Groove", patternNames(), 0);
    choice (ParamIDs::genre, "New beat genre", namesFrom ((int) fgd::grooveCategories().size(),
                                                          [] (int i) { return String (fgd::grooveCategories()[(size_t) i]); }), 0);
    add (std::make_unique<AudioParameterInt> (ParameterID { ParamIDs::riffSeed, 1 }, "Beat seed", 1, 100000, 1));
    unitFloat (ParamIDs::density, "Density", 0.0f, 1.0f, 0.5f);
    group ("groove", "Groove");

    // Feel
    unitFloat (ParamIDs::swing, "Swing", 0.0f, 1.0f, fgd::genreSwing (0));
    add (std::make_unique<AudioParameterInt> (ParameterID { ParamIDs::velocity, 1 }, "Velocity", 1, 127, 100));
    unitFloat (ParamIDs::humanize, "Humanize", 0.0f, 1.0f, 0.15f);
    add (std::make_unique<AudioParameterInt> (ParameterID { ParamIDs::humanizeSeed, 1 }, "Feel seed", 0, 100000, 0));
    group ("feel", "Feel");

    // Song
    choice (ParamIDs::section, "Section", namesFrom (fgd::numSections, fgd::sectionName), 0);
    choice (ParamIDs::fills, "Fills", { "Off", "Every bar", "Every 2 bars", "Every 4 bars", "Every 8 bars" }, 4);
    choice (ParamIDs::fillStyle, "Fill style", namesFrom (fgd::numFillStyles, fgd::fillStyleName), 0);
    toggle (ParamIDs::crash, "Crash after fills", true);
    choice (ParamIDs::variation, "Variation", { "Off", "Every 4 bars", "Every 8 bars" }, 1);
    group ("song", "Song");

    // Tempo & MIDI
    toggle (ParamIDs::sequencer, "Sequencer", true);
    toggle (ParamIDs::tempoSync, "Sync to DAW", true);
    add (std::make_unique<AudioParameterFloat> (ParameterID { ParamIDs::tempo, 1 }, "Tempo", NormalisableRange<float> (60.0f, 180.0f, 0.1f), 124.0f,
                                                AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int)
                                                    { return String (v, 1) + " BPM"; })));
    add (std::make_unique<AudioParameterInt> (ParameterID { ParamIDs::midiChannel, 1 }, "MIDI channel", 1, 16, 10));
    group ("tempoMidi", "Tempo & MIDI");

    // Sound (the guide kit)
    toggle (ParamIDs::sound, "Guide sound", true);
    choice (ParamIDs::kit, "Kit", namesFrom (GuideDrums::numKits, GuideDrums::kitName), 0);
    unitFloat (ParamIDs::tone, "Tone", 0.0f, 1.0f, 0.85f);
    unitFloat (ParamIDs::reverb, "Reverb", 0.0f, 1.0f, 0.12f);
    add (std::make_unique<AudioParameterFloat> (ParameterID { ParamIDs::volume, 1 }, "Volume", NormalisableRange<float> (-36.0f, 6.0f, 0.1f), -3.0f,
                                                AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int)
                                                    { return String (v, 1) + " dB"; })));
    group ("sound", "Sound");

    // Kit: per drum, mute, MIDI note and level
    for (int l = 0; l < fgd::numLanes; ++l)
    {
        const String name (fgd::laneName (l));
        toggle (ParamIDs::mute (l), name + " mute", false);
        add (std::make_unique<AudioParameterInt> (ParameterID { ParamIDs::note (l), 1 }, name + " note", 0, 127, fgd::defaultNote (l)));
        unitFloat (ParamIDs::level (l), name + " level", 0.0f, 1.5f, 1.0f);
    }
    group ("kit", "Kit");

    return layout;
}

FaseGrooveDrumsProcessor::FaseGrooveDrumsProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "FaseGrooveDrums", createLayout())
{
    for (auto& lane : userGrid)
        for (auto& c : lane)
            c = '.';
    for (auto& f : laneFlash)
        f = 0.0f;
}

bool FaseGrooveDrumsProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

void FaseGrooveDrumsProcessor::prepareToPlay (double sampleRate, int)
{
    drums.prepare (sampleRate);
    reverbFx.setSampleRate (sampleRate);
    reverbFx.reset();
    outputGain.reset (sampleRate, 0.05);
    outputGain.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (raw (ParamIDs::volume)));
    events.reserve (1024);
}

juce::AudioProcessorEditor* FaseGrooveDrumsProcessor::createEditor()
{
    return new FaseGrooveDrumsEditor (*this);
}

float FaseGrooveDrumsProcessor::raw (const juce::String& id) const
{
    if (auto* v = apvts.getRawParameterValue (id))
        return v->load();
    return 0.0f;
}

void FaseGrooveDrumsProcessor::setParameterFromUI (const juce::String& id, float plainValue)
{
    if (auto* p = apvts.getParameter (id))
    {
        p->beginChangeGesture();
        p->setValueNotifyingHost (p->convertTo0to1 (plainValue));
        p->endChangeGesture();
    }
}

//==============================================================================
fgd::DrumSettings FaseGrooveDrumsProcessor::currentSettings() const
{
    fgd::DrumSettings s;
    s.pattern = juce::roundToInt (raw (ParamIDs::pattern));
    s.swing = raw (ParamIDs::swing);
    s.velocity = juce::roundToInt (raw (ParamIDs::velocity));
    s.humanize = raw (ParamIDs::humanize);
    s.humanizeSeed = (std::uint32_t) juce::roundToInt (raw (ParamIDs::humanizeSeed));
    s.fillEveryBars = fillBars[juce::jlimit (0, 4, juce::roundToInt (raw (ParamIDs::fills)))];
    s.fillStyle = (fgd::FillStyle) juce::jlimit (0, fgd::numFillStyles - 1, juce::roundToInt (raw (ParamIDs::fillStyle)));
    s.crashAfterFill = raw (ParamIDs::crash) > 0.5f;
    s.variationEvery = variationBars[juce::jlimit (0, 2, juce::roundToInt (raw (ParamIDs::variation)))];
    s.section = (fgd::Section) juce::jlimit (0, fgd::numSections - 1, juce::roundToInt (raw (ParamIDs::section)));
    s.riffSeed = (std::uint32_t) juce::roundToInt (raw (ParamIDs::riffSeed));
    s.density = raw (ParamIDs::density);
    s.randomGenre = juce::roundToInt (raw (ParamIDs::genre));

    for (int l = 0; l < fgd::numLanes; ++l)
    {
        s.notes[(size_t) l] = juce::roundToInt (raw (ParamIDs::note (l)));
        s.levels[(size_t) l] = raw (ParamIDs::level (l));
        s.muted[(size_t) l] = raw (ParamIDs::mute (l)) > 0.5f;
        for (int i = 0; i < fgd::userPatternLength; ++i)
            s.user[(size_t) l][(size_t) i] = userGrid[(size_t) l][(size_t) i].load();
    }
    return s;
}

//==============================================================================
fgd::DrumPattern FaseGrooveDrumsProcessor::displayedPattern() const
{
    auto p = fgd::GrooveEngine::basePattern (currentSettings());
    fgd::normalisePattern (p, gridSteps);   // a one-bar groove shows twice
    return p;
}

char FaseGrooveDrumsProcessor::stepSymbol (int lane, int step) const
{
    if (! juce::isPositiveAndBelow (lane, fgd::numLanes) || ! juce::isPositiveAndBelow (step, gridSteps))
        return '.';
    if (juce::roundToInt (raw (ParamIDs::pattern)) == fgd::userPatternIndex())
        return userGrid[(size_t) lane][(size_t) step].load();
    return displayedPattern().lanes[(size_t) lane][(size_t) step];
}

void FaseGrooveDrumsProcessor::setUserPattern (const fgd::DrumPattern& source)
{
    auto p = source;
    fgd::normalisePattern (p, gridSteps);
    for (int l = 0; l < fgd::numLanes; ++l)
        for (int i = 0; i < gridSteps; ++i)
            userGrid[(size_t) l][(size_t) i] = p.lanes[(size_t) l][(size_t) i];
    setParameterFromUI (ParamIDs::pattern, (float) fgd::userPatternIndex());
}

void FaseGrooveDrumsProcessor::editStep (int lane, int step, char symbol)
{
    if (! juce::isPositiveAndBelow (lane, fgd::numLanes) || ! juce::isPositiveAndBelow (step, gridSteps))
        return;

    // The first edit of a preset (or random beat) copies it into the user pattern.
    if (juce::roundToInt (raw (ParamIDs::pattern)) != fgd::userPatternIndex())
        setUserPattern (displayedPattern());

    userGrid[(size_t) lane][(size_t) step] = fgd::isHit (symbol) ? symbol : '.';
}

void FaseGrooveDrumsProcessor::clearLane (int lane)
{
    if (! juce::isPositiveAndBelow (lane, fgd::numLanes))
        return;
    if (juce::roundToInt (raw (ParamIDs::pattern)) != fgd::userPatternIndex())
        setUserPattern (displayedPattern());
    for (auto& c : userGrid[(size_t) lane])
        c = '.';
}

void FaseGrooveDrumsProcessor::clearGrid()
{
    for (auto& lane : userGrid)
        for (auto& c : lane)
            c = '.';
    setParameterFromUI (ParamIDs::pattern, (float) fgd::userPatternIndex());
}

std::uint32_t FaseGrooveDrumsProcessor::freshSeed()
{
    return (std::uint32_t) juce::Random::getSystemRandom().nextInt ({ 1, 100000 });
}

void FaseGrooveDrumsProcessor::rollNewBeat()
{
    auto seed = freshSeed();
    if (seed == (std::uint32_t) juce::roundToInt (raw (ParamIDs::riffSeed)))
        seed = seed % 99999 + 1;
    setParameterFromUI (ParamIDs::riffSeed, (float) seed);
    setParameterFromUI (ParamIDs::pattern, (float) fgd::randomPatternIndex());
}

void FaseGrooveDrumsProcessor::newFeel()
{
    setParameterFromUI (ParamIDs::humanizeSeed, (float) freshSeed());
}

juce::StringArray FaseGrooveDrumsProcessor::styleNames()
{
    juce::StringArray names;
    for (const auto& s : styles)
        names.add (s.name);
    return names;
}

void FaseGrooveDrumsProcessor::applyStyle (int index)
{
    if (! juce::isPositiveAndBelow (index, (int) std::size (styles)))
        return;

    const auto& s = styles[index];
    setParameterFromUI (ParamIDs::pattern, (float) indexOfGroove (s.groove));
    setParameterFromUI (ParamIDs::genre, (float) index);
    setParameterFromUI (ParamIDs::swing, fgd::genreSwing (index));
    setParameterFromUI (ParamIDs::humanize, s.humanize);
    setParameterFromUI (ParamIDs::fills, (float) s.fills);
    setParameterFromUI (ParamIDs::fillStyle, (float) (int) s.fillStyle);
    setParameterFromUI (ParamIDs::variation, (float) s.variation);
    setParameterFromUI (ParamIDs::kit, (float) s.kit);
}

void FaseGrooveDrumsProcessor::resetToDefaults()
{
    for (auto* p : getParameters())
        if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (p))
        {
            ranged->beginChangeGesture();
            ranged->setValueNotifyingHost (ranged->getDefaultValue());
            ranged->endChangeGesture();
        }
    for (auto& lane : userGrid)
        for (auto& c : lane)
            c = '.';
}

//==============================================================================
void FaseGrooveDrumsProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const double sampleRate = getSampleRate() > 0.0 ? getSampleRate() : 44100.0;
    buffer.clear();

    auto settings = currentSettings();
    const bool sequencerOn = raw (ParamIDs::sequencer) > 0.5f;
    const int channel = juce::jlimit (1, 16, juce::roundToInt (raw (ParamIDs::midiChannel)));

    // ---- MIDI in: the clock drives the standalone app; notes pass through (and play the kit) ----
    outMidi.clear();
    const double clockPpqAtStart = clockPpq;
    for (const auto meta : midi)
    {
        const auto m = meta.getMessage();
        if (m.isMidiClock())
        {
            const auto at = samplesProcessed + meta.samplePosition;
            if (lastClockSample >= 0)
                if (const double gap = (double) (at - lastClockSample); gap > 0.0 && gap < sampleRate * 0.25)
                    clockInterval = clockInterval <= 0.0 ? gap : clockInterval * 0.92 + gap * 0.08;
            lastClockSample = at;
            if (clockRunning)
                clockPpq += 1.0 / 24.0;
            continue;
        }
        if (m.isMidiStart())           { clockRunning = true; clockPpq = 0.0; continue; }
        if (m.isMidiContinue())        { clockRunning = true; continue; }
        if (m.isMidiStop())            { clockRunning = false; continue; }
        if (m.isSongPositionPointer()) { clockPpq = m.getSongPositionPointerMidiBeat() / 4.0; continue; }

        outMidi.addEvent (m, meta.samplePosition);
    }

    // ---- Timing ----
    double dawBpm = 120.0, hostPpq = 0.0;
    bool hostPlaying = false, haveHostPpq = false;
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
        {
            if (auto b = pos->getBpm())          dawBpm = *b;
            if (auto p = pos->getPpqPosition()) { hostPpq = *p; haveHostPpq = true; }
            hostPlaying = pos->getIsPlaying();
        }

    const bool clockAlive = lastClockSample >= 0 && clockInterval > 0.0
                         && (double) (samplesProcessed - lastClockSample) < sampleRate * 0.5;
    if (! haveHostPpq && clockAlive)
    {
        dawBpm = juce::roundToInt (600.0 * sampleRate / (24.0 * clockInterval)) / 10.0;
        hostPlaying = clockRunning;
        hostPpq = clockPpqAtStart;
        haveHostPpq = clockRunning;
    }

    // Synced: the DAW's tempo and position. Unsynced: our own BPM and clock,
    // still started / stopped by the DAW's play button.
    const bool tempoSynced = raw (ParamIDs::tempoSync) > 0.5f;
    hostBpm = juce::jlimit (20.0, 400.0, dawBpm);
    const double bpm = tempoSynced ? hostBpm.load() : juce::jlimit (20.0, 400.0, (double) raw (ParamIDs::tempo));
    lastBpm = bpm;
    const double samplesPerBeat = sampleRate * 60.0 / bpm;
    const double blockBeats = numSamples / samplesPerBeat;

    const bool followHost = tempoSynced && hostPlaying && haveHostPpq;
    const bool preview = previewOn.load();
    const bool running = (sequencerOn && hostPlaying) || preview;

    double startPpq = 0.0;
    if (followHost)
        startPpq = hostPpq;
    else if (running)
    {
        // Free-running clock: from the start, or where PAUSE left it; REWIND jumps back.
        startPpq = wasRunning || ! previewFromStart.load() ? internalPpq : 0.0;
        if (previewRewind.exchange (false))
            startPpq = 0.0;
        if (! wasRunning)
            previewFromStart = true;
    }
    else
        previewRewind = false;

    events.clear();
    if (running)
    {
        if (! wasRunning || std::abs (startPpq - expectedNextPpq) > 0.05)
            engine.allNotesOff (startPpq, events);   // start, loop or jump

        engine.render (startPpq, startPpq + blockBeats, settings, events);
        expectedNextPpq = internalPpq = startPpq + blockBeats;

        const auto step = (long long) std::floor (startPpq / fgd::stepLength);
        uiStep = (int) (((step % gridSteps) + gridSteps) % gridSteps);
        uiPpq = startPpq;
    }
    else
    {
        if (engine.anyNoteSounding())
            engine.allNotesOff (startPpq, events);
        uiStep = -1;
        uiPpq = -1.0;
    }
    wasRunning = running;

    for (const auto& e : events)
    {
        const int pos = juce::jlimit (0, numSamples - 1, (int) std::floor ((e.ppq - startPpq) * samplesPerBeat));
        if (e.isNoteOn())
        {
            outMidi.addEvent (juce::MidiMessage::noteOn (channel, e.note, (juce::uint8) e.velocity), pos);
            laneFlash[(size_t) juce::jlimit (0, fgd::numLanes - 1, e.lane)] = (float) e.velocity / 127.0f;
        }
        else
            outMidi.addEvent (juce::MidiMessage::noteOff (channel, e.note), pos);
    }

    // ---- The guide kit ----
    const bool soundOn = raw (ParamIDs::sound) > 0.5f;
    if (soundWasOn && ! soundOn)
        drums.allNotesOff();
    soundWasOn = soundOn;

    if (soundOn)
    {
        static const std::array<float, fgd::numLanes> unity = [] { std::array<float, fgd::numLanes> a {}; a.fill (1.0f); return a; }();
        drums.setKit (juce::roundToInt (raw (ParamIDs::kit)));
        drums.setTone (raw (ParamIDs::tone));
        drums.render (buffer, outMidi, settings.notes, unity);

        const float amount = raw (ParamIDs::reverb);
        if (amount > 0.001f)
        {
            juce::Reverb::Parameters rp;
            rp.roomSize = 0.45f;
            rp.damping = 0.5f;
            rp.width = 1.0f;
            rp.wetLevel = amount * 0.4f;
            rp.dryLevel = 1.0f - amount * 0.25f;
            reverbFx.setParameters (rp);
            if (buffer.getNumChannels() > 1)
                reverbFx.processStereo (buffer.getWritePointer (0), buffer.getWritePointer (1), numSamples);
            else
                reverbFx.processMono (buffer.getWritePointer (0), numSamples);
        }

        outputGain.setTargetValue (juce::Decibels::decibelsToGain (raw (ParamIDs::volume)));
        outputGain.applyGain (buffer, numSamples);
    }

    samplesProcessed += numSamples;
    midi.swapWith (outMidi);
}

//==============================================================================
double FaseGrooveDrumsProcessor::exportBpm() const
{
    return raw (ParamIDs::tempoSync) > 0.5f ? hostBpm.load() : (double) raw (ParamIDs::tempo);
}

int FaseGrooveDrumsProcessor::clipLengthInBars (int bars) const
{
    if (bars == songArrangement)
    {
        int total = 0;
        for (const auto& p : fgd::defaultArrangement())
            total += p.bars;
        return total;
    }
    if (bars > 0)
        return bars;
    return juce::jmax (1, fgd::GrooveEngine::loopLengthInSteps (currentSettings()) / 16);
}

juce::MidiFile FaseGrooveDrumsProcessor::midiFromEvents (const std::vector<fgd::NoteEvent>& clipEvents, double beats,
                                                         const std::vector<fgd::GrooveEngine::Marker>* markers,
                                                         ClipPart part, const juce::String& name) const
{
    constexpr int ticksPerQuarter = 960;
    const int channel = juce::jlimit (1, 16, juce::roundToInt (raw (ParamIDs::midiChannel)));

    // Track 1 (or the conductor track of a multitrack file): name, tempo, metre and the markers.
    juce::MidiMessageSequence header;
    header.addEvent (juce::MidiMessage::textMetaEvent (3, name), 0.0);
    header.addEvent (juce::MidiMessage::tempoMetaEvent (juce::roundToInt (60000000.0 / exportBpm())), 0.0);
    header.addEvent (juce::MidiMessage::timeSignatureMetaEvent (4, 4), 0.0);
    if (markers != nullptr)
        for (const auto& m : *markers)
            header.addEvent (juce::MidiMessage::textMetaEvent (6, m.name), m.ppq * ticksPerQuarter);

    auto notesOf = [&] (juce::MidiMessageSequence& track, int lane)
    {
        for (const auto& e : clipEvents)
        {
            if (lane >= 0 && e.lane != lane)
                continue;
            const double tick = std::round (juce::jlimit (0.0, beats, e.ppq) * ticksPerQuarter);
            track.addEvent (e.isNoteOn() ? juce::MidiMessage::noteOn (channel, e.note, (juce::uint8) e.velocity)
                                         : juce::MidiMessage::noteOff (channel, e.note),
                            tick);
        }
        // Every clip as long as asked: many DAWs cut a clip after its last event, so the end
        // gets a silent one (pitch bend in the middle: no change).
        const double end = beats * ticksPerQuarter;
        track.addEvent (juce::MidiMessage::pitchWheel (channel, 8192), end);
        track.addEvent (juce::MidiMessage::endOfTrack(), end);
        track.updateMatchedPairs();
    };

    juce::MidiFile file;
    file.setTicksPerQuarterNote (ticksPerQuarter);

    if (part == ClipPart::tracks)
    {
        // One file, one track per drum: the DAW puts kick, snare, hats... on separate tracks.
        header.addEvent (juce::MidiMessage::endOfTrack(), beats * ticksPerQuarter);
        file.addTrack (header);
        for (int l = 0; l < fgd::numLanes; ++l)
        {
            if (std::none_of (clipEvents.begin(), clipEvents.end(), [l] (const fgd::NoteEvent& e) { return e.lane == l && e.isNoteOn(); }))
                continue;
            juce::MidiMessageSequence track;
            track.addEvent (juce::MidiMessage::textMetaEvent (3, fgd::laneName (l)), 0.0);
            notesOf (track, l);
            file.addTrack (track);
        }
    }
    else
    {
        notesOf (header, -1);
        file.addTrack (header);
    }
    return file;
}

juce::MidiFile FaseGrooveDrumsProcessor::createMidiClip (int bars, ClipPart part) const
{
    const auto settings = currentSettings();
    const double beats = clipLengthInBars (bars) * 4.0;
    const auto name = clipFileName (bars, part).upToLastOccurrenceOf (".mid", false, false);

    if (bars == songArrangement)
    {
        std::vector<fgd::GrooveEngine::Marker> markers;
        const auto songEvents = fgd::GrooveEngine::renderArrangement (settings, fgd::defaultArrangement(), &markers);
        return midiFromEvents (songEvents, beats, &markers, part, name);
    }

    return midiFromEvents (fgd::GrooveEngine::renderClip (settings, beats), beats, nullptr, part, name);
}

juce::String FaseGrooveDrumsProcessor::clipFileName (int bars, ClipPart part) const
{
    const auto settings = currentSettings();
    const auto groove = fgd::GrooveEngine::basePattern (settings).name;
    juce::String name = "Drums " + juce::String (groove);
    if (settings.section != fgd::Section::main && bars != songArrangement)
        name << " " << fgd::sectionName ((int) settings.section);
    name << " " << juce::roundToInt (exportBpm()) << "bpm";
    name << (bars == songArrangement ? juce::String (" Song") : " " + juce::String (clipLengthInBars (bars)) + " bars");
    if (part == ClipPart::tracks)
        name << " (tracks)";
    return juce::File::createLegalFileName (name.replace ("'", "")) + ".mid";
}

juce::File FaseGrooveDrumsProcessor::userFolder()
{
    return juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("Fase Groove Drums");
}

juce::File FaseGrooveDrumsProcessor::clipFolder()
{
    return userFolder().getChildFile ("MIDI clips");
}

juce::File FaseGrooveDrumsProcessor::writeMidiClip (int bars, ClipPart part) const
{
    auto folder = clipFolder();
    if (! folder.createDirectory())
        folder = juce::File::getSpecialLocation (juce::File::tempDirectory);

    auto file = folder.getChildFile (clipFileName (bars, part));
    file.deleteFile();

    juce::FileOutputStream stream (file);
    if (! stream.openedOk())
        return {};

    createMidiClip (bars, part).writeTo (stream);
    return file;
}

//==============================================================================
void FaseGrooveDrumsProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();

    juce::ValueTree grid ("Grid");
    for (int l = 0; l < fgd::numLanes; ++l)
    {
        juce::String line;
        for (const auto& c : userGrid[(size_t) l])
            line << juce::String::charToString ((juce::juce_wchar) (unsigned char) c.load());
        grid.setProperty ("lane" + juce::String (l + 1), line, nullptr);
    }
    state.removeChild (state.getChildWithName ("Grid"), nullptr);
    state.appendChild (grid, nullptr);
    state.setProperty ("editorScale", editorScale, nullptr);

    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void FaseGrooveDrumsProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);
    if (xml == nullptr || ! xml->hasTagName (apvts.state.getType()))
        return;

    auto state = juce::ValueTree::fromXml (*xml);
    const auto grid = state.getChildWithName ("Grid");
    for (int l = 0; l < fgd::numLanes; ++l)
    {
        const auto line = grid.getProperty ("lane" + juce::String (l + 1)).toString();
        for (int i = 0; i < fgd::userPatternLength; ++i)
        {
            const char c = i < line.length() ? (char) line[i] : '.';
            userGrid[(size_t) l][(size_t) i] = fgd::isHit (c) ? c : '.';
        }
    }
    editorScale = juce::jlimit (0.6f, 2.0f, (float) state.getProperty ("editorScale", 1.0f));

    state.removeChild (grid, nullptr);
    apvts.replaceState (state);
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FaseGrooveDrumsProcessor();
}
