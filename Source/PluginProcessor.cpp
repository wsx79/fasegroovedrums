// Fase Groove Drums - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Localisation.h"

#include <complex>

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
    choice (ParamIDs::partLength, "Part length", { "1/2 bar", "1 bar" }, 0);
    for (int i = 0; i < fgd::numSegments; ++i)
        choice (ParamIDs::segment (i), "Part " + String (i + 1), { "A", "B", "C", "D" }, i);
    group ("groove", "Groove");

    // Feel
    unitFloat (ParamIDs::swing, "Swing", 0.0f, 1.0f, fgd::genreSwing (0));
    add (std::make_unique<AudioParameterInt> (ParameterID { ParamIDs::velocity, 1 }, "Velocity", 1, 127, 100));
    unitFloat (ParamIDs::humanize, "Humanize", 0.0f, 1.0f, 0.15f);
    add (std::make_unique<AudioParameterInt> (ParameterID { ParamIDs::humanizeSeed, 1 }, "Feel seed", 0, 100000, 0));
    unitFloat (ParamIDs::accents, "Accents", 0.0f, 1.0f, 0.6f);
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
        add (std::make_unique<AudioParameterFloat> (ParameterID { ParamIDs::laneVolume (l), 1 }, name + " volume",
                                                    NormalisableRange<float> (-48.0f, 12.0f, 0.1f, 2.0f), 0.0f,
                                                    AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int)
                                                        { return v <= -47.9f ? String ("-inf") : String (v, 1) + " dB"; })));
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
    for (auto& lane : userVelocity)
        for (auto& v : lane)
            v = 0;
    for (auto& f : laneFlash)
        f = 0.0f;
    resetSong();
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
    s.grooveBars = juce::roundToInt (raw (ParamIDs::partLength)) == 1 ? 4 : 2;
    s.swing = raw (ParamIDs::swing);
    s.velocity = juce::roundToInt (raw (ParamIDs::velocity));
    s.humanize = raw (ParamIDs::humanize);
    s.humanizeSeed = (std::uint32_t) juce::roundToInt (raw (ParamIDs::humanizeSeed));
    s.accentGroove = raw (ParamIDs::accents);
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
        {
            s.user[(size_t) l][(size_t) i] = userGrid[(size_t) l][(size_t) i].load();
            s.userVelocity[(size_t) l][(size_t) i] = (unsigned char) juce::jlimit (0, 127, userVelocity[(size_t) l][(size_t) i].load());
        }
    }
    for (int i = 0; i < fgd::numSegments; ++i)
        s.segmentOrder[(size_t) i] = segmentAt (i);
    return s;
}

//==============================================================================
fgd::DrumPattern FaseGrooveDrumsProcessor::displayedPattern() const
{
    const auto settings = currentSettings();
    return fgd::applySegmentOrder (fgd::GrooveEngine::basePattern (settings), settings.segmentOrder);
}

int FaseGrooveDrumsProcessor::segmentAt (int position) const
{
    return juce::jlimit (0, fgd::numSegments - 1, juce::roundToInt (raw (ParamIDs::segment (juce::jlimit (0, fgd::numSegments - 1, position)))));
}

void FaseGrooveDrumsProcessor::setSegmentAt (int position, int part)
{
    setParameterFromUI (ParamIDs::segment (juce::jlimit (0, fgd::numSegments - 1, position)), (float) juce::jlimit (0, fgd::numSegments - 1, part));
}

int FaseGrooveDrumsProcessor::baseStep (int displayedStep) const
{
    const int size = steps() / fgd::numSegments;
    const int step = juce::jlimit (0, steps() - 1, displayedStep);
    return segmentAt (step / size) * size + step % size;
}

char FaseGrooveDrumsProcessor::stepSymbol (int lane, int step) const
{
    if (! juce::isPositiveAndBelow (lane, fgd::numLanes) || ! juce::isPositiveAndBelow (step, steps()))
        return '.';
    if (juce::roundToInt (raw (ParamIDs::pattern)) == fgd::userPatternIndex())
        return userGrid[(size_t) lane][(size_t) baseStep (step)].load();
    return displayedPattern().lanes[(size_t) lane][(size_t) step];
}

void FaseGrooveDrumsProcessor::setUserPattern (const fgd::DrumPattern& source)
{
    auto p = source;
    fgd::fitPattern (p, gridSteps);   // a two-bar groove fills the four bars twice
    for (int l = 0; l < fgd::numLanes; ++l)
        for (int i = 0; i < gridSteps; ++i)
        {
            userGrid[(size_t) l][(size_t) i] = p.lanes[(size_t) l][(size_t) i];
            userVelocity[(size_t) l][(size_t) i] = 0;
        }
    setParameterFromUI (ParamIDs::pattern, (float) fgd::userPatternIndex());
}

bool FaseGrooveDrumsProcessor::makeUserPattern()
{
    if (juce::roundToInt (raw (ParamIDs::pattern)) == fgd::userPatternIndex())
        return true;
    // The groove as written (the A-D order stays as it is, on top of it).
    setUserPattern (fgd::GrooveEngine::basePattern (currentSettings()));
    return false;
}

void FaseGrooveDrumsProcessor::editStep (int lane, int step, char symbol)
{
    if (! juce::isPositiveAndBelow (lane, fgd::numLanes) || ! juce::isPositiveAndBelow (step, steps()))
        return;

    // The first edit of a preset (or random beat) copies it into the user pattern.
    makeUserPattern();
    const int at = baseStep (step);
    userGrid[(size_t) lane][(size_t) at] = fgd::isHit (symbol) ? symbol : '.';
    if (! fgd::isHit (symbol))
        userVelocity[(size_t) lane][(size_t) at] = 0;
}

void FaseGrooveDrumsProcessor::clearLane (int lane)
{
    if (! juce::isPositiveAndBelow (lane, fgd::numLanes))
        return;
    makeUserPattern();
    for (int i = 0; i < gridSteps; ++i)
    {
        userGrid[(size_t) lane][(size_t) i] = '.';
        userVelocity[(size_t) lane][(size_t) i] = 0;
    }
}

void FaseGrooveDrumsProcessor::clearGrid()
{
    for (int l = 0; l < fgd::numLanes; ++l)
        for (int i = 0; i < gridSteps; ++i)
        {
            userGrid[(size_t) l][(size_t) i] = '.';
            userVelocity[(size_t) l][(size_t) i] = 0;
        }
    setParameterFromUI (ParamIDs::pattern, (float) fgd::userPatternIndex());
}

void FaseGrooveDrumsProcessor::randomizeSegment (int part)
{
    if (! juce::isPositiveAndBelow (part, fgd::numSegments) || isSegmentLocked (part))
        return;
    makeUserPattern();
    const auto settings = currentSettings();
    auto fresh = fgd::makeRandomPattern (freshSeed(), settings.density, settings.randomGenre);
    fgd::fitPattern (fresh, steps());
    const int size = steps() / fgd::numSegments;
    for (int l = 0; l < fgd::numLanes; ++l)
        for (int i = part * size; i < (part + 1) * size; ++i)
        {
            userGrid[(size_t) l][(size_t) i] = fresh.lanes[(size_t) l][(size_t) i];
            userVelocity[(size_t) l][(size_t) i] = 0;
        }
}

void FaseGrooveDrumsProcessor::randomizeLane (int lane)
{
    if (! juce::isPositiveAndBelow (lane, fgd::numLanes))
        return;
    makeUserPattern();
    const auto settings = currentSettings();
    const auto line = fgd::makeRandomLane ((fgd::Lane) lane, freshSeed(), settings.density, settings.randomGenre);
    const int size = steps() / fgd::numSegments;
    for (int i = 0; i < steps(); ++i)
        if (! isSegmentLocked (i / size))
        {
            userGrid[(size_t) lane][(size_t) i] = line[(size_t) (i % (int) line.size())];
            userVelocity[(size_t) lane][(size_t) i] = 0;
        }
}

int FaseGrooveDrumsProcessor::stepVelocity (int lane, int step) const
{
    if (! juce::isPositiveAndBelow (lane, fgd::numLanes) || ! juce::isPositiveAndBelow (step, steps())
        || juce::roundToInt (raw (ParamIDs::pattern)) != fgd::userPatternIndex())
        return 0;
    return userVelocity[(size_t) lane][(size_t) baseStep (step)].load();
}

void FaseGrooveDrumsProcessor::setStepVelocity (int lane, int step, int level)
{
    if (! juce::isPositiveAndBelow (lane, fgd::numLanes) || ! juce::isPositiveAndBelow (step, steps()))
        return;
    makeUserPattern();
    userVelocity[(size_t) lane][(size_t) baseStep (step)] = juce::jlimit (0, 127, level);
}

void FaseGrooveDrumsProcessor::resetStepVelocities (int lane)
{
    for (int l = 0; l < fgd::numLanes; ++l)
        if (lane < 0 || l == lane)
            for (auto& v : userVelocity[(size_t) l])
                v = 0;
}

int FaseGrooveDrumsProcessor::effectiveVelocity (int lane, int step) const
{
    if (const int set = stepVelocity (lane, step); set > 0)
        return set;
    const char symbol = stepSymbol (lane, step);
    const double level = symbol == 'X' ? 1.0 : symbol == 'f' ? 0.9 : symbol == 'g' ? 0.42 : 0.8;
    return juce::jlimit (1, 127, juce::roundToInt (100.0 * level));
}

juce::StringArray FaseGrooveDrumsProcessor::velocityShapeNames()
{
    return { "Offbeat push", "On the beat", "Shuffle 16ths", "Crescendo", "Humanize", "Flat" };
}

void FaseGrooveDrumsProcessor::applyVelocityShape (int lane, int shape)
{
    if (! juce::isPositiveAndBelow (lane, fgd::numLanes))
        return;
    makeUserPattern();
    auto& r = juce::Random::getSystemRandom();

    for (int step = 0; step < steps(); ++step)
    {
        const char symbol = stepSymbol (lane, step);
        if (! fgd::isHit (symbol))
            continue;
        const int inBeat = step % 4;
        int v = 90;
        switch (shape)
        {
            case 0: v = inBeat == 2 ? 112 : inBeat == 0 ? 80 : inBeat == 3 ? 66 : 56; break;   // the "ts" on the offbeat
            case 1: v = inBeat == 0 ? 115 : inBeat == 2 ? 84 : 58; break;               // on the beat
            case 2: v = step % 2 == 0 ? 100 : 66; break;                                 // loud-soft 16ths
            case 3: v = 60 + (step % 16) * 55 / 15; break;                               // a crescendo over each bar
            case 4: v = juce::jlimit (1, 127, effectiveVelocity (lane, step) + r.nextInt ({ -14, 15 })); break;
            case 5:
            default: v = 96; break;
        }
        if (symbol == 'g')
            v = juce::jmin (v, 55);
        setStepVelocity (lane, step, juce::jlimit (1, 127, v));
    }
}

void FaseGrooveDrumsProcessor::randomizeVelocities (int lane)
{
    auto& r = juce::Random::getSystemRandom();
    applyVelocityShape (lane, r.nextInt (4));
    for (int step = 0; step < steps(); ++step)
        if (const int v = stepVelocity (lane, step); v > 0)
            setStepVelocity (lane, step, juce::jlimit (1, 127, v + r.nextInt ({ -9, 10 })));
}

bool FaseGrooveDrumsProcessor::hasStepVelocities() const
{
    if (juce::roundToInt (raw (ParamIDs::pattern)) != fgd::userPatternIndex())
        return false;
    for (const auto& lane : userVelocity)
        for (const auto& v : lane)
            if (v.load() > 0)
                return true;
    return false;
}

bool FaseGrooveDrumsProcessor::isAudioFile (const juce::File& file)
{
    return file.hasFileExtension ("wav;aif;aiff;flac;mp3;ogg");
}

bool FaseGrooveDrumsProcessor::loadSample (int lane, const juce::File& file)
{
    if (! juce::isPositiveAndBelow (lane, fgd::numLanes) || ! file.existsAsFile())
        return false;

    juce::AudioFormatManager formats;
    formats.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader (formats.createReaderFor (file));
    if (reader == nullptr || reader->lengthInSamples <= 0 || reader->sampleRate <= 0.0)
        return false;

    // Up to 10 seconds (a crash or a long 808 fits), in stereo at most.
    auto sample = std::make_shared<DrumSample>();
    const int length = (int) juce::jmin<juce::int64> (reader->lengthInSamples, (juce::int64) (reader->sampleRate * 10.0));
    sample->audio.setSize (juce::jlimit (1, 2, (int) reader->numChannels), length);
    reader->read (&sample->audio, 0, length, 0, true, sample->audio.getNumChannels() > 1);
    sample->sampleRate = reader->sampleRate;
    sample->path = file.getFullPathName();

    std::atomic_store (&samples[(size_t) lane], std::shared_ptr<const DrumSample> (std::move (sample)));
    return true;
}

void FaseGrooveDrumsProcessor::clearSample (int lane)
{
    if (juce::isPositiveAndBelow (lane, fgd::numLanes))
        std::atomic_store (&samples[(size_t) lane], std::shared_ptr<const DrumSample>());
}

juce::String FaseGrooveDrumsProcessor::samplePath (int lane) const
{
    if (! juce::isPositiveAndBelow (lane, fgd::numLanes))
        return {};
    const auto s = std::atomic_load (&samples[(size_t) lane]);
    return s != nullptr ? s->path : juce::String();
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

    // Locked parts stay: the new beat goes around them, in the user pattern.
    if (std::any_of (segmentLocked.begin(), segmentLocked.end(), [] (bool b) { return b; }))
    {
        for (int part = 0; part < fgd::numSegments; ++part)
            randomizeSegment (part);
        return;
    }

    setParameterFromUI (ParamIDs::riffSeed, (float) seed);
    setParameterFromUI (ParamIDs::pattern, (float) fgd::randomPatternIndex());
}

void FaseGrooveDrumsProcessor::totalRandom()
{
    auto& r = juce::Random::getSystemRandom();
    const int style = r.nextInt (styleNames().size());
    applyStyle (style);

    // Another groove of that genre, or a brand new beat in it.
    if (r.nextBool())
        rollNewBeat();
    else
    {
        juce::Array<int> grooves;
        for (size_t i = 0; i < fgd::drumPatterns().size(); ++i)
            if (fgd::drumPatterns()[i].category == fgd::grooveCategories()[(size_t) style])
                grooves.add ((int) i);
        if (! grooves.isEmpty())
            setParameterFromUI (ParamIDs::pattern, (float) grooves[r.nextInt (grooves.size())]);
    }

    setParameterFromUI (ParamIDs::density, 0.25f + 0.6f * r.nextFloat());
    setParameterFromUI (ParamIDs::swing, juce::jlimit (0.0f, 0.75f, fgd::genreSwing (style) + (r.nextFloat() - 0.5f) * 0.12f));
    setParameterFromUI (ParamIDs::humanize, 0.05f + 0.35f * r.nextFloat());
    setParameterFromUI (ParamIDs::fillStyle, (float) r.nextInt (fgd::numFillStyles));
    if (r.nextInt (3) == 0)
        setParameterFromUI (ParamIDs::kit, (float) r.nextInt (GuideDrums::numKits));
    newFeel();
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
    resetStepVelocities();
    segmentLocked.fill (false);
    resetSong();
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
        const int n = fgd::GrooveEngine::patternLength (settings);
        uiStep = (int) (((step % n) + n) % n);
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

    // A pad played by hand.
    if (const int request = auditionRequest.exchange (-1); request >= 0)
    {
        const int lane = request / 128, note = settings.notes[(size_t) juce::jlimit (0, fgd::numLanes - 1, lane)];
        outMidi.addEvent (juce::MidiMessage::noteOn (channel, note, (juce::uint8) (request % 128)), 0);
        outMidi.addEvent (juce::MidiMessage::noteOff (channel, note), juce::jmax (0, numSamples - 1));
        laneFlash[(size_t) juce::jlimit (0, fgd::numLanes - 1, lane)] = (float) (request % 128) / 127.0f;
    }

    // ---- The guide kit ----
    const bool soundOn = raw (ParamIDs::sound) > 0.5f;
    if (soundWasOn && ! soundOn)
        drums.allNotesOff();
    soundWasOn = soundOn;

    if (soundOn)
    {
        std::array<float, fgd::numLanes> gains {};
        for (int l = 0; l < fgd::numLanes; ++l)
        {
            const float db = raw (ParamIDs::laneVolume (l));
            gains[(size_t) l] = db <= -47.9f ? 0.0f : juce::Decibels::decibelsToGain (db);
        }
        drums.setKit (juce::roundToInt (raw (ParamIDs::kit)));
        drums.setTone (raw (ParamIDs::tone));
        DrumSamples current;
        for (int l = 0; l < fgd::numLanes; ++l)
            current[(size_t) l] = std::atomic_load (&samples[(size_t) l]);
        drums.render (buffer, outMidi, settings.notes, gains, current);

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
        for (const auto& p : songArrangementParts())
            total += juce::jmax (0, p.bars);
        return juce::jmax (1, total);
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
        const auto songEvents = fgd::GrooveEngine::renderArrangement (settings, songArrangementParts(), &markers);
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
juce::ValueTree FaseGrooveDrumsProcessor::stateTree() const
{
    auto state = const_cast<juce::AudioProcessorValueTreeState&> (apvts).copyState();   // copyState() locks, it changes nothing

    juce::ValueTree grid ("Grid");
    for (int l = 0; l < fgd::numLanes; ++l)
    {
        juce::String line, levels;
        for (int i = 0; i < gridSteps; ++i)
        {
            line << juce::String::charToString ((juce::juce_wchar) (unsigned char) userGrid[(size_t) l][(size_t) i].load());
            levels << juce::String::toHexString (juce::jlimit (0, 127, userVelocity[(size_t) l][(size_t) i].load())).paddedLeft ('0', 2);
        }
        grid.setProperty ("lane" + juce::String (l + 1), line, nullptr);
        grid.setProperty ("velocity" + juce::String (l + 1), levels, nullptr);
    }
    juce::String locks;
    for (bool b : segmentLocked)
        locks << (b ? "1" : "0");
    grid.setProperty ("locks", locks, nullptr);

    juce::ValueTree song ("Song");
    for (int i = 0; i < numSongParts; ++i)
    {
        juce::ValueTree part ("Part");
        part.setProperty ("bars", songBars[(size_t) i].load(), nullptr);
        part.setProperty ("variation", (juce::int64) songVariation[(size_t) i].load(), nullptr);
        part.setProperty ("fill", songFill[(size_t) i].load(), nullptr);
        song.appendChild (part, nullptr);
    }

    juce::ValueTree sounds ("Samples");
    for (int l = 0; l < fgd::numLanes; ++l)
        if (const auto path = samplePath (l); path.isNotEmpty())
            sounds.setProperty ("lane" + juce::String (l + 1), path, nullptr);

    for (const auto* name : { "Grid", "Song", "Samples" })
        state.removeChild (state.getChildWithName (name), nullptr);
    state.appendChild (grid, nullptr);
    state.appendChild (song, nullptr);
    state.appendChild (sounds, nullptr);
    return state;
}

void FaseGrooveDrumsProcessor::applyStateTree (const juce::ValueTree& source)
{
    if (! source.hasType (apvts.state.getType()))
        return;

    auto state = source.createCopy();
    const auto grid = state.getChildWithName ("Grid");
    for (int l = 0; l < fgd::numLanes; ++l)
    {
        const auto line = grid.getProperty ("lane" + juce::String (l + 1)).toString();
        const auto levels = grid.getProperty ("velocity" + juce::String (l + 1)).toString();
        for (int i = 0; i < gridSteps; ++i)
        {
            const char c = i < line.length() ? (char) line[i] : '.';
            userGrid[(size_t) l][(size_t) i] = fgd::isHit (c) ? c : '.';
            userVelocity[(size_t) l][(size_t) i] = juce::jlimit (0, 127, levels.substring (i * 2, i * 2 + 2).getHexValue32());
        }
    }
    const auto locks = grid.getProperty ("locks").toString();
    for (int i = 0; i < fgd::numSegments; ++i)
        segmentLocked[(size_t) i] = locks.substring (i, i + 1) == "1";

    resetSong();
    const auto song = state.getChildWithName ("Song");
    for (int i = 0; i < numSongParts && i < song.getNumChildren(); ++i)
    {
        const auto part = song.getChild (i);
        songBars[(size_t) i] = juce::jlimit (0, 64, (int) part.getProperty ("bars", songBars[(size_t) i].load()));
        songVariation[(size_t) i] = (std::uint32_t) (juce::int64) part.getProperty ("variation", 0);
        songFill[(size_t) i] = juce::jlimit (-2, fgd::numFillStyles - 1, (int) part.getProperty ("fill", -1));
    }

    // Your own samples: loaded again from their files (only when they changed).
    const auto sounds = state.getChildWithName ("Samples");
    for (int l = 0; l < fgd::numLanes; ++l)
    {
        const auto path = sounds.getProperty ("lane" + juce::String (l + 1)).toString();
        if (path == samplePath (l))
            continue;
        if (path.isEmpty() || ! loadSample (l, juce::File (path)))
            clearSample (l);
    }

    for (const auto* name : { "Grid", "Song", "Samples" })
        state.removeChild (state.getChildWithName (name), nullptr);
    state.removeProperty ("editorScale", nullptr);
    apvts.replaceState (state);
}

void FaseGrooveDrumsProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = stateTree();
    state.setProperty ("editorScale", editorScale, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void FaseGrooveDrumsProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);
    if (xml == nullptr || ! xml->hasTagName (apvts.state.getType()))
        return;

    const auto state = juce::ValueTree::fromXml (*xml);
    editorScale = juce::jlimit (0.6f, 2.0f, (float) state.getProperty ("editorScale", 1.0f));
    applyStateTree (state);
}

juce::String FaseGrooveDrumsProcessor::snapshot() const
{
    return stateTree().toXmlString();
}

void FaseGrooveDrumsProcessor::restoreSnapshot (const juce::String& text)
{
    if (auto xml = juce::parseXML (text))
        applyStateTree (juce::ValueTree::fromXml (*xml));
}

//==============================================================================
std::vector<fgd::ArrangementPart> FaseGrooveDrumsProcessor::songArrangementParts() const
{
    std::vector<fgd::ArrangementPart> parts;
    for (int i = 0; i < numSongParts; ++i)
        parts.push_back ({ fgd::defaultArrangement()[(size_t) i].section, songBars[(size_t) i].load(),
                           songVariation[(size_t) i].load(), songFill[(size_t) i].load() });
    return parts;
}

void FaseGrooveDrumsProcessor::setSongBars (int part, int bars)
{
    if (juce::isPositiveAndBelow (part, numSongParts))
        songBars[(size_t) part] = juce::jlimit (0, 64, bars);
}

void FaseGrooveDrumsProcessor::setSongFill (int part, int fill)
{
    if (juce::isPositiveAndBelow (part, numSongParts))
        songFill[(size_t) part] = juce::jlimit (-2, fgd::numFillStyles - 1, fill);
}

void FaseGrooveDrumsProcessor::regenerateSongPart (int part)
{
    if (juce::isPositiveAndBelow (part, numSongParts))
        songVariation[(size_t) part] = freshSeed();
}

void FaseGrooveDrumsProcessor::resetSongPart (int part)
{
    if (juce::isPositiveAndBelow (part, numSongParts))
        songVariation[(size_t) part] = 0;
}

void FaseGrooveDrumsProcessor::resetSong()
{
    for (int i = 0; i < numSongParts; ++i)
    {
        songBars[(size_t) i] = fgd::defaultArrangement()[(size_t) i].bars;
        songVariation[(size_t) i] = 0;
        songFill[(size_t) i] = -1;
    }
}

juce::String FaseGrooveDrumsProcessor::sectionClipName (int partIndex)
{
    const auto section = (int) fgd::defaultArrangement()[(size_t) juce::jlimit (0, numSongParts - 1, partIndex)].section;
    return section == (int) fgd::Section::main ? juce::String ("Main groove") : juce::String (fgd::sectionName (section));
}

juce::File FaseGrooveDrumsProcessor::writeSongSectionClip (int partIndex, ClipPart part) const
{
    const auto parts = songArrangementParts();
    if (! juce::isPositiveAndBelow (partIndex, (int) parts.size()) || parts[(size_t) partIndex].bars <= 0)
        return {};

    auto folder = clipFolder();
    if (! folder.createDirectory())
        folder = juce::File::getSpecialLocation (juce::File::tempDirectory);

    const auto& section = parts[(size_t) partIndex];
    std::vector<fgd::GrooveEngine::Marker> markers;
    const auto sectionEvents = fgd::GrooveEngine::renderArrangement (currentSettings(), { section }, &markers);
    const auto name = "Drums " + sectionClipName (partIndex) + " - "
                    + juce::String (fgd::GrooveEngine::basePattern (currentSettings()).name)
                    + " " + juce::String (juce::roundToInt (exportBpm())) + "bpm" + (part == ClipPart::tracks ? " (tracks)" : "");
    auto file = folder.getChildFile (juce::File::createLegalFileName (name.replace ("'", "")) + ".mid");
    file.deleteFile();

    juce::FileOutputStream stream (file);
    if (! stream.openedOk())
        return {};
    midiFromEvents (sectionEvents, section.bars * 4.0, &markers, part, name).writeTo (stream);
    return file;
}

int FaseGrooveDrumsProcessor::saveSongSections (const juce::File& folder, ClipPart part) const
{
    if (! folder.createDirectory())
        return 0;
    int written = 0;
    for (int i = 0; i < numSongParts; ++i)
        if (const auto file = writeSongSectionClip (i, part); file.existsAsFile())
        {
            const auto target = folder.getChildFile (juce::String (i + 1).paddedLeft ('0', 2) + " " + file.getFileName());
            target.deleteFile();
            if (file.copyFileTo (target))
                ++written;
        }
    return written;
}

//==============================================================================
juce::File FaseGrooveDrumsProcessor::presetFolder()  { return userFolder().getChildFile ("Presets"); }
juce::File FaseGrooveDrumsProcessor::grooveFolder()  { return userFolder().getChildFile ("Grooves"); }

juce::Array<juce::File> FaseGrooveDrumsProcessor::presetFiles()
{
    auto files = presetFolder().findChildFiles (juce::File::findFiles, false, "*.fgdpreset");
    files.sort();
    return files;
}

juce::Array<juce::File> FaseGrooveDrumsProcessor::grooveFiles()
{
    auto files = grooveFolder().findChildFiles (juce::File::findFiles, false, "*.fgdgroove");
    files.sort();
    return files;
}

bool FaseGrooveDrumsProcessor::savePreset (const juce::String& name)
{
    const auto clean = juce::File::createLegalFileName (name.trim());
    if (clean.isEmpty() || ! presetFolder().createDirectory())
        return false;
    return presetFolder().getChildFile (clean + ".fgdpreset").replaceWithText (snapshot());
}

bool FaseGrooveDrumsProcessor::loadPreset (const juce::File& file)
{
    auto xml = juce::parseXML (file);
    if (xml == nullptr || ! xml->hasTagName (apvts.state.getType()))
        return false;
    applyStateTree (juce::ValueTree::fromXml (*xml));
    return true;
}

bool FaseGrooveDrumsProcessor::saveGroove (const juce::String& name)
{
    const auto clean = juce::File::createLegalFileName (name.trim());
    if (clean.isEmpty() || ! grooveFolder().createDirectory())
        return false;

    // The grid as you hear it (in the A-D order), with its velocities.
    const auto p = displayedPattern();
    juce::XmlElement xml ("FaseGrooveDrumsGroove");
    xml.setAttribute ("name", clean);
    for (int l = 0; l < fgd::numLanes; ++l)
    {
        juce::String levels;
        for (int i = 0; i < steps(); ++i)
            levels << juce::String::toHexString (stepVelocity (l, i)).paddedLeft ('0', 2);
        auto* lane = xml.createNewChildElement ("Lane");
        lane->setAttribute ("drum", fgd::laneName (l));
        lane->setAttribute ("steps", juce::String (p.lanes[(size_t) l]));
        lane->setAttribute ("velocity", levels);
    }
    return xml.writeTo (grooveFolder().getChildFile (clean + ".fgdgroove"));
}

bool FaseGrooveDrumsProcessor::loadGroove (const juce::File& file)
{
    auto xml = juce::parseXML (file);
    if (xml == nullptr || ! xml->hasTagName ("FaseGrooveDrumsGroove"))
        return false;

    int l = 0;
    for (auto* lane : xml->getChildWithTagNameIterator ("Lane"))
    {
        if (l >= fgd::numLanes)
            break;
        const auto steps = lane->getStringAttribute ("steps");
        const auto levels = lane->getStringAttribute ("velocity");
        const int length = juce::jmax (1, steps.length());
        for (int i = 0; i < gridSteps; ++i)   // a two-bar groove fills the four bars twice
        {
            const int k = i % length;
            const char c = k < steps.length() ? (char) steps[k] : '.';
            userGrid[(size_t) l][(size_t) i] = fgd::isHit (c) ? c : '.';
            userVelocity[(size_t) l][(size_t) i] = juce::jlimit (0, 127, levels.substring (k * 2, k * 2 + 2).getHexValue32());
        }
        ++l;
    }
    for (int i = 0; i < fgd::numSegments; ++i)
        setSegmentAt (i, i);
    setParameterFromUI (ParamIDs::pattern, (float) fgd::userPatternIndex());
    return true;
}

namespace
{
    /** The first `loopSteps` of a groove (and their levels) repeated until `length`. */
    template <typename Levels>
    void repeatLoop (fgd::DrumPattern& p, Levels& levels, int loopSteps, int length)
    {
        loopSteps = juce::jlimit (16, length, loopSteps);
        for (int l = 0; l < fgd::numLanes; ++l)
            for (int i = loopSteps; i < length; ++i)
            {
                p.lanes[(size_t) l][(size_t) i] = p.lanes[(size_t) l][(size_t) (i % loopSteps)];
                levels[(size_t) l][(size_t) i] = levels[(size_t) l][(size_t) (i % loopSteps)];
            }
    }

    /** Zero-phase filtering (forwards, then backwards), twice: a steep band without delay. */
    void filterBand (std::vector<float>& y, double sr, double low, double high)
    {
        auto run = [&y] (juce::IIRCoefficients c)
        {
            for (int pass = 0; pass < 2; ++pass)
            {
                juce::IIRFilter f;
                f.setCoefficients (c);
                f.processSamples (y.data(), (int) y.size());
                std::reverse (y.begin(), y.end());
            }
        };
        for (int order = 0; order < 2; ++order)
        {
            if (low > 0.0)  run (juce::IIRCoefficients::makeHighPass (sr, low));
            if (high > 0.0) run (juce::IIRCoefficients::makeLowPass (sr, high));
        }
    }

    struct AudioHit { double time; float level, ring, body; };

    /** Where the sound of a band starts again (a fast rise in level), with how loud it is, how
        long it rings (open hats) and how much body it has 35-60 ms later (claps and snares do,
        a kick's click doesn't). */
    std::vector<AudioHit> findHits (const std::vector<float>& y, double sr, double minGap)
    {
        const int hop = juce::jmax (1, (int) (sr * 0.002));
        const int n = (int) y.size() / hop;
        std::vector<float> e ((size_t) n), db ((size_t) n), flux ((size_t) n, 0.0f);
        for (int i = 0; i < n; ++i)
        {
            double sum = 0.0;
            for (int k = 0; k < hop; ++k)
                sum += (double) y[(size_t) (i * hop + k)] * y[(size_t) (i * hop + k)];
            e[(size_t) i] = (float) std::sqrt (sum / hop + 1.0e-12);
            db[(size_t) i] = 20.0f * std::log10 (e[(size_t) i] + 1.0e-7f);
        }
        for (int i = 4; i < n; ++i)
            flux[(size_t) i] = juce::jmax (0.0f, db[(size_t) i] - db[(size_t) (i - 4)]);

        const int gap = juce::jmax (1, (int) (minGap * sr / hop));
        const int w = (int) (0.025 * sr / hop), k100 = (int) (0.1 * sr / hop), a = (int) (0.035 * sr / hop), b = (int) (0.06 * sr / hop);
        std::vector<AudioHit> hits;
        int last = -gap;
        for (int i = 1; i + 1 < n; ++i)
        {
            if (flux[(size_t) i] < 6.0f || flux[(size_t) i] < flux[(size_t) (i - 1)] || flux[(size_t) i] < flux[(size_t) (i + 1)])
                continue;
            float peak = 0.0f;
            for (int j = i; j < juce::jmin (n, i + w); ++j)
                peak = juce::jmax (peak, e[(size_t) j]);
            float body = 0.0f;
            for (int j = i + a; j < juce::jmin (n, i + b); ++j)
                body += e[(size_t) j];
            body /= (float) juce::jmax (1, juce::jmin (n, i + b) - (i + a)) * (peak + 1.0e-9f);
            const AudioHit hit { (double) i * hop / sr, peak, e[(size_t) juce::jmin (n - 1, i + k100)] / (peak + 1.0e-9f), body };
            if (i - last < gap)
            {
                if (! hits.empty() && hit.level > hits.back().level)
                    hits.back() = hit, last = i;   // the stronger of two hits too close together
                continue;
            }
            hits.push_back (hit);
            last = i;
        }
        return hits;
    }
}

juce::String FaseGrooveDrumsProcessor::importAudioGroove (const juce::File& file)
{
    juce::AudioFormatManager formats;
    formats.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader (formats.createReaderFor (file));
    if (reader == nullptr || reader->lengthInSamples <= 0 || reader->sampleRate <= 0.0)
        return {};

    const double sr = reader->sampleRate;
    const int length = (int) juce::jmin<juce::int64> (reader->lengthInSamples, (juce::int64) (sr * 40.0));
    juce::AudioBuffer<float> audio ((int) juce::jlimit (1u, 2u, reader->numChannels), length);
    reader->read (&audio, 0, length, 0, true, audio.getNumChannels() > 1);
    std::vector<float> mono ((size_t) length);
    for (int i = 0; i < length; ++i)
        mono[(size_t) i] = audio.getNumChannels() > 1 ? 0.5f * (audio.getSample (0, i) + audio.getSample (1, i)) : audio.getSample (0, i);

    // The tempo: a loop is a whole number of bars. The tempo in use first, else the one nearest
    // 124 BPM that makes it 1, 2, 4 or 8 bars.
    const double seconds = length / sr;
    double bpm = exportBpm();
    double bars = seconds / (4.0 * 60.0 / bpm);
    if (std::abs (bars - std::round (bars)) > 0.04 || std::round (bars) < 1.0)
    {
        double best = 1.0e9;
        for (int b : { 1, 2, 4, 8 })
            if (const double t = b * 4.0 * 60.0 / seconds; t >= 70.0 && t <= 180.0 && std::abs (t - 124.0) < best)
            {
                best = std::abs (t - 124.0);
                bpm = t;
            }
        bars = seconds / (4.0 * 60.0 / bpm);
    }
    const int wholeBars = juce::jmax (1, (int) std::round (bars));
    const double T = seconds / (wholeBars * 16.0);   // seconds per 16th

    auto bandOf = [&] (double low, double high) { auto y = mono; filterBand (y, sr, low, high); return y; };
    auto kicks = findHits (bandOf (0.0, 110.0), sr, 0.1);
    auto mids = findHits (bandOf (1000.0, 4000.0), sr, 0.045);
    auto highs = findHits (bandOf (7500.0, 0.0), sr, 0.045);
    if (kicks.empty() && mids.empty() && highs.empty())
        return {};

    // Where the loop sits against the grid: the strong kicks (or else the hats).
    double offset = 0.0;
    {
        const auto& ref = kicks.empty() ? highs : kicks;
        float top = 0.0f;
        for (const auto& h : ref) top = juce::jmax (top, h.level);
        std::complex<double> sum;
        for (const auto& h : ref)
            if (h.level > 0.5f * top)
                sum += std::polar (1.0, juce::MathConstants<double>::twoPi * std::fmod (h.time / T, 1.0));
        if (std::abs (sum) > 0.0)
            offset = std::arg (sum) / juce::MathConstants<double>::twoPi * T;
    }

    fgd::DrumPattern p;
    for (auto& line : p.lanes)
        line.assign ((size_t) gridSteps, '.');
    std::array<std::array<int, gridSteps>, fgd::numLanes> levels {};
    std::vector<double> oddDelays, evenDelays;

    auto place = [&] (const std::vector<AudioHit>& hits, std::function<int (const AudioHit&)> laneFor, float minBody, bool measureSwing)
    {
        float top = 0.0f;
        for (const auto& h : hits)
            if (h.body >= minBody) top = juce::jmax (top, h.level);
        for (const auto& h : hits)
        {
            if (h.body < minBody || top <= 0.0f)
                continue;
            const double pos = (h.time - offset) / T;
            const int step = (int) std::lround (pos);
            if (step < 0 || step >= steps())
                continue;   // the first two (or four) bars
            const float r = h.level / top;
            if (r < 0.12f)
                continue;
            const int lane = laneFor (h);
            const int v = juce::jlimit (1, 127, (int) std::lround (35.0 + 92.0 * r));
            if (v <= levels[(size_t) lane][(size_t) step])
                continue;
            levels[(size_t) lane][(size_t) step] = v;
            p.lanes[(size_t) lane][(size_t) step] = r > 0.75f ? 'X' : r > 0.38f ? 'x' : 'g';
            if (measureSwing && r > 0.38f)
                (step % 2 == 1 ? oddDelays : evenDelays).push_back (pos - step);
        }
    };
    place (kicks, [] (const AudioHit&) { return (int) fgd::Lane::kick; }, 0.0f, false);
    place (mids, [] (const AudioHit&) { return (int) fgd::Lane::clap; }, 0.28f, false);
    place (highs, [] (const AudioHit& h) { return (int) (h.ring > 0.4f ? fgd::Lane::openHat : fgd::Lane::closedHat); }, 0.0f, true);

    // A loop shorter than the groove plays again.
    repeatLoop (p, levels, juce::jmin (wholeBars * 16, steps()), gridSteps);

    setUserPattern (p);
    for (int l = 0; l < fgd::numLanes; ++l)
        for (int i = 0; i < gridSteps; ++i)
            userVelocity[(size_t) l][(size_t) i] = levels[(size_t) l][(size_t) i];
    for (int i = 0; i < fgd::numSegments; ++i)
        setSegmentAt (i, i);

    // The swing: how late the odd 16ths are, as the Swing control measures it.
    juce::String report = juce::String (wholeBars) + " " + tr (wholeBars == 1 ? "bar" : "bars") + ", " + juce::String (bpm, 1) + " BPM";
    if (oddDelays.size() >= 2)
    {
        auto median = [] (std::vector<double> v) { std::sort (v.begin(), v.end()); return v.empty() ? 0.0 : v[v.size() / 2]; };
        const double swingAmount = juce::jlimit (0.0, 1.0, (median (oddDelays) - median (evenDelays)) * 3.0);
        setParameterFromUI (ParamIDs::swing, (float) swingAmount);
        report << ", " << tr ("Swing").toLowerCase() << " " << juce::roundToInt (swingAmount * 100.0) << "%";
    }
    return report;
}

bool FaseGrooveDrumsProcessor::importMidiGroove (const juce::File& file)
{
    juce::FileInputStream in (file);
    juce::MidiFile midi;
    if (! in.openedOk() || ! midi.readFrom (in))
        return false;
    const int ticks = midi.getTimeFormat();
    if (ticks <= 0)
        return false;

    struct Hit { double beat; int lane; int velocity; };
    std::vector<Hit> hits;
    bool anyChannel10 = false;
    for (int t = 0; t < midi.getNumTracks(); ++t)
        for (const auto* e : *midi.getTrack (t))
            anyChannel10 = anyChannel10 || (e->message.isNoteOn() && e->message.getChannel() == 10);

    const auto settings = currentSettings();
    for (int t = 0; t < midi.getNumTracks(); ++t)
        for (const auto* e : *midi.getTrack (t))
        {
            const auto& m = e->message;
            if (! m.isNoteOn() || (anyChannel10 && m.getChannel() != 10))
                continue;
            // The lanes' own notes first, then the General MIDI drum map.
            int lane = -1;
            for (int l = 0; l < fgd::numLanes && lane < 0; ++l)
                if (settings.notes[(size_t) l] == m.getNoteNumber())
                    lane = l;
            if (lane < 0)
                lane = fgd::laneForGeneralMidiNote (m.getNoteNumber());
            if (lane >= 0)
                hits.push_back ({ m.getTimeStamp() / ticks, lane, m.getVelocity() });
        }
    if (hits.empty())
        return false;

    // Two bars from the first bar that plays, on the 16th grid.
    double first = hits.front().beat;
    for (const auto& h : hits)
        first = juce::jmin (first, h.beat);
    const double start = std::floor (first / 4.0 + 1.0e-6) * 4.0;
    double last = first;
    for (const auto& h : hits)
        last = juce::jmax (last, h.beat);

    fgd::DrumPattern p;
    for (auto& line : p.lanes)
        line.assign ((size_t) gridSteps, '.');
    std::array<std::array<int, gridSteps>, fgd::numLanes> levels {};
    for (const auto& h : hits)
    {
        const int step = (int) std::lround ((h.beat - start) / fgd::stepLength);
        if (! juce::isPositiveAndBelow (step, steps()))
            continue;
        if (h.velocity > levels[(size_t) h.lane][(size_t) step])
        {
            levels[(size_t) h.lane][(size_t) step] = h.velocity;
            p.lanes[(size_t) h.lane][(size_t) step] = h.velocity >= 110 ? 'X' : h.velocity <= 50 ? 'g' : 'x';
        }
    }
    // A loop shorter than the groove plays again (whole bars).
    repeatLoop (p, levels, juce::jmin ((int) std::ceil ((last - start) / 4.0 + 1.0e-6) * 16, steps()), gridSteps);

    setUserPattern (p);
    for (int l = 0; l < fgd::numLanes; ++l)
        for (int i = 0; i < gridSteps; ++i)
            userVelocity[(size_t) l][(size_t) i] = levels[(size_t) l][(size_t) i];
    for (int i = 0; i < fgd::numSegments; ++i)
        setSegmentAt (i, i);
    return true;
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FaseGrooveDrumsProcessor();
}
