// Fase Groove Drums - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Localisation.h"

#include <complex>
#include <map>

namespace
{
    // Choice lists. Items are only ever appended, so saved projects keep their values.
    const int fillBars[] { 0, 1, 2, 4, 8 };
    const int variationBars[] { 0, 4, 8 };

    juce::String percent (float v, int) { return juce::String (juce::roundToInt (v * 100.0f)) + "%"; }
    int swingPercent (float v) { return juce::roundToInt (50.0f + v * 50.0f / 3.0f); }

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
    // The swing, shown as drum machines show it: 50% straight, 67% a triplet shuffle, 75% the heaviest.
    add (std::make_unique<AudioParameterFloat> (ParameterID { ParamIDs::swing, 1 }, "Swing", NormalisableRange<float> (0.0f, 1.5f), fgd::genreSwing (0),
                                                AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int)
                                                    { return String (swingPercent (v)) + "%"; })));
    add (std::make_unique<AudioParameterInt> (ParameterID { ParamIDs::velocity, 1 }, "Velocity", 1, 127, 100));
    unitFloat (ParamIDs::humanize, "Humanize", 0.0f, 1.0f, 0.3f);
    add (std::make_unique<AudioParameterInt> (ParameterID { ParamIDs::humanizeSeed, 1 }, "Feel seed", 0, 100000, 0));
    unitFloat (ParamIDs::accents, "Accents", 0.0f, 1.0f, 0.85f);
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
        add (std::make_unique<AudioParameterFloat> (ParameterID { ParamIDs::push (l), 1 }, name + " push",
                                                    NormalisableRange<float> (-40.0f, 40.0f, 0.5f), 0.0f,
                                                    AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int)
                                                        { return std::abs (v) < 0.25f ? String ("0 ms") : (v < 0.0f ? "-" : "+") + String (std::abs (v), 1) + " ms"; })));
        unitFloat (ParamIDs::laneSwing (l), name + " swing", 0.0f, 2.0f, 1.0f);
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
    for (auto* grid : { &userVelocity, &userChance, &userRoll })
        for (auto& lane : *grid)
            for (auto& v : lane)
                v = 0;
    for (auto& lane : customFill)
        for (auto& c : lane)
            c = '.';
    for (int i = 0; i < 16; ++i)
    {
        templateTiming[(size_t) i] = 0.0f;
        templateLevel[(size_t) i] = 1.0f;
    }
    for (auto& f : laneFlash)
        f = 0.0f;
    resetSong();
    linkTimer.startTimer (150);
}

FaseGrooveDrumsProcessor::~FaseGrooveDrumsProcessor()
{
    linkTimer.stopTimer();
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
        s.laneShift[(size_t) l] = raw (ParamIDs::push (l));
        s.laneSwing[(size_t) l] = raw (ParamIDs::laneSwing (l));
        for (int i = 0; i < fgd::userPatternLength; ++i)
        {
            s.user[(size_t) l][(size_t) i] = userGrid[(size_t) l][(size_t) i].load();
            s.userVelocity[(size_t) l][(size_t) i] = (unsigned char) juce::jlimit (0, 127, userVelocity[(size_t) l][(size_t) i].load());
            s.userChance[(size_t) l][(size_t) i] = (unsigned char) juce::jlimit (0, 100, userChance[(size_t) l][(size_t) i].load());
            s.userRoll[(size_t) l][(size_t) i] = (unsigned char) juce::jlimit (0, 4, userRoll[(size_t) l][(size_t) i].load());
        }
        for (int i = 0; i < 16; ++i)
            s.customFill[(size_t) l][(size_t) i] = customFill[(size_t) l][(size_t) i].load();
    }
    s.tempo = lastBpm.load();
    s.grooveTemplate.active = templateActive.load();
    for (int i = 0; i < 16; ++i)
    {
        s.grooveTemplate.timing[(size_t) i] = templateTiming[(size_t) i].load();
        s.grooveTemplate.level[(size_t) i] = templateLevel[(size_t) i].load();
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
            resetStepExtras (l, i);
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
    {
        userVelocity[(size_t) lane][(size_t) at] = 0;
        resetStepExtras (lane, at);
    }
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
        resetStepExtras (lane, i);
    }
}

void FaseGrooveDrumsProcessor::clearGrid()
{
    for (int l = 0; l < fgd::numLanes; ++l)
        for (int i = 0; i < gridSteps; ++i)
        {
            userGrid[(size_t) l][(size_t) i] = '.';
            userVelocity[(size_t) l][(size_t) i] = 0;
            resetStepExtras (l, i);
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
            resetStepExtras (l, i);
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
            resetStepExtras (lane, i);
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

int FaseGrooveDrumsProcessor::stepChance (int lane, int step) const
{
    if (! juce::isPositiveAndBelow (lane, fgd::numLanes) || ! juce::isPositiveAndBelow (step, steps())
        || juce::roundToInt (raw (ParamIDs::pattern)) != fgd::userPatternIndex())
        return 100;
    const int c = userChance[(size_t) lane][(size_t) baseStep (step)].load();
    return c <= 0 || c >= 100 ? 100 : c;
}

void FaseGrooveDrumsProcessor::setStepChance (int lane, int step, int percent)
{
    if (! juce::isPositiveAndBelow (lane, fgd::numLanes) || ! juce::isPositiveAndBelow (step, steps()))
        return;
    makeUserPattern();
    userChance[(size_t) lane][(size_t) baseStep (step)] = percent >= 100 ? 0 : juce::jlimit (1, 99, percent);
}

int FaseGrooveDrumsProcessor::stepRoll (int lane, int step) const
{
    if (! juce::isPositiveAndBelow (lane, fgd::numLanes) || ! juce::isPositiveAndBelow (step, steps())
        || juce::roundToInt (raw (ParamIDs::pattern)) != fgd::userPatternIndex())
        return 1;
    return juce::jmax (1, userRoll[(size_t) lane][(size_t) baseStep (step)].load());
}

void FaseGrooveDrumsProcessor::setStepRoll (int lane, int step, int hits)
{
    if (! juce::isPositiveAndBelow (lane, fgd::numLanes) || lane == (int) fgd::Lane::kick || ! juce::isPositiveAndBelow (step, steps()))
        return;
    makeUserPattern();
    userRoll[(size_t) lane][(size_t) baseStep (step)] = hits <= 1 ? 0 : juce::jmin (4, hits);
}

char FaseGrooveDrumsProcessor::customFillSymbol (int lane, int step) const
{
    if (! juce::isPositiveAndBelow (lane, fgd::numLanes) || ! juce::isPositiveAndBelow (step, 16))
        return '.';
    return customFill[(size_t) lane][(size_t) step].load();
}

void FaseGrooveDrumsProcessor::editCustomFill (int lane, int step, char symbol)
{
    if (! juce::isPositiveAndBelow (lane, fgd::numLanes) || ! juce::isPositiveAndBelow (step, 16))
        return;
    customFill[(size_t) lane][(size_t) step] = fgd::isHit (symbol) ? symbol : '.';
    if (juce::roundToInt (raw (ParamIDs::fillStyle)) != (int) fgd::FillStyle::custom)
        setParameterFromUI (ParamIDs::fillStyle, (float) (int) fgd::FillStyle::custom);
}

void FaseGrooveDrumsProcessor::clearCustomFill()
{
    for (auto& lane : customFill)
        for (auto& c : lane)
            c = '.';
}

bool FaseGrooveDrumsProcessor::hasCustomFill() const
{
    for (const auto& lane : customFill)
        for (const auto& c : lane)
            if (fgd::isHit (c.load()))
                return true;
    return false;
}

void FaseGrooveDrumsProcessor::customFillFromStyle (int style)
{
    if (! juce::isPositiveAndBelow (style, fgd::numFillStyles) || style == (int) fgd::FillStyle::custom)
        return;

    // What the style plays on an empty groove: the fill alone.
    fgd::DrumSettings s;
    s.pattern = fgd::userPatternIndex();
    s.swing = 0.0f;
    s.humanize = 0.0f;
    s.accentGroove = 0.0f;
    s.crashAfterFill = false;
    s.fillEveryBars = 1;
    s.fillStyle = (fgd::FillStyle) style;
    if (style == (int) fgd::FillStyle::mix)
        s.fillStyle = fgd::GrooveEngine::effectiveFillStyle (fgd::FillStyle::mix, 0);

    clearCustomFill();
    for (const auto& e : fgd::GrooveEngine::renderClip (s, 4.0))
    {
        if (! e.isNoteOn())
            continue;
        const int step = juce::roundToInt (e.ppq / fgd::stepLength);
        if (! juce::isPositiveAndBelow (step, 16) || ! juce::isPositiveAndBelow (e.lane, fgd::numLanes))
            continue;
        auto& c = customFill[(size_t) e.lane][(size_t) step];
        if (c.load() == '.')
            c = e.velocity >= 100 ? 'X' : e.velocity <= 50 ? 'g' : 'x';
    }
    setParameterFromUI (ParamIDs::fillStyle, (float) (int) fgd::FillStyle::custom);
}

juce::StringArray FaseGrooveDrumsProcessor::timingFeelNames()
{
    return { "On the grid", "Laid-back snare & clap", "Pushed hats", "Jackin' (claps ahead, hats swing)",
             "Drunk hats (heavy swing on the hats only)", "Straight percussion" };
}

void FaseGrooveDrumsProcessor::applyTimingFeel (int feel)
{
    // push (ms) and share of the swing per drum; the kick always stays on the grid.
    for (int l = 0; l < fgd::numLanes; ++l)
    {
        const auto lane = (fgd::Lane) l;
        const bool hats = lane == fgd::Lane::closedHat || lane == fgd::Lane::openHat || lane == fgd::Lane::shaker || lane == fgd::Lane::tambourine;
        const bool backbeat = lane == fgd::Lane::snare || lane == fgd::Lane::clap || lane == fgd::Lane::rim;
        const bool percussion = lane == fgd::Lane::conga || lane == fgd::Lane::cowbell || lane == fgd::Lane::tom;
        float push = 0.0f, share = 1.0f;
        switch (feel)
        {
            case 1: push = backbeat ? 12.0f : 0.0f; break;
            case 2: push = hats ? -6.0f : 0.0f; break;
            case 3: push = backbeat ? -6.0f : hats ? 3.0f : 0.0f; share = hats ? 1.2f : backbeat ? 0.6f : 1.0f; break;
            case 4: share = hats ? 1.5f : 0.4f; break;
            case 5: share = percussion || lane == fgd::Lane::shaker ? 0.0f : 1.0f; break;
            case 0:
            default: break;
        }
        setParameterFromUI (ParamIDs::push (l), lane == fgd::Lane::kick ? 0.0f : push);
        setParameterFromUI (ParamIDs::laneSwing (l), share);
    }
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

namespace
{
    // The words drum sample names use: 3 = the drum's own word, 2 = a common word, 1 = an
    // abbreviation (only as a word of its own: "ch" but not "chord").
    struct DrumWord { const char* word; fgd::Lane lane; int strength; };
    const DrumWord drumWords[] {
        { "kick", fgd::Lane::kick, 3 }, { "kik", fgd::Lane::kick, 2 }, { "bassdrum", fgd::Lane::kick, 3 }, { "bd", fgd::Lane::kick, 1 }, { "kck", fgd::Lane::kick, 1 },
        { "snare", fgd::Lane::snare, 3 }, { "snr", fgd::Lane::snare, 1 }, { "sd", fgd::Lane::snare, 1 }, { "sn", fgd::Lane::snare, 1 },
        { "clap", fgd::Lane::clap, 3 }, { "handclap", fgd::Lane::clap, 3 }, { "cp", fgd::Lane::clap, 1 }, { "clp", fgd::Lane::clap, 1 },
        { "rimshot", fgd::Lane::rim, 3 }, { "rim", fgd::Lane::rim, 3 }, { "rims", fgd::Lane::rim, 3 }, { "sidestick", fgd::Lane::rim, 3 }, { "stick", fgd::Lane::rim, 2 }, { "rs", fgd::Lane::rim, 1 },
        { "closedhat", fgd::Lane::closedHat, 3 }, { "hihat", fgd::Lane::closedHat, 3 }, { "hat", fgd::Lane::closedHat, 2 }, { "hats", fgd::Lane::closedHat, 2 },
        { "closed", fgd::Lane::closedHat, 2 }, { "ch", fgd::Lane::closedHat, 1 }, { "chh", fgd::Lane::closedHat, 1 }, { "hhc", fgd::Lane::closedHat, 1 }, { "hh", fgd::Lane::closedHat, 1 },
        { "openhat", fgd::Lane::openHat, 3 }, { "open", fgd::Lane::openHat, 2 }, { "oh", fgd::Lane::openHat, 1 }, { "ohh", fgd::Lane::openHat, 1 }, { "hho", fgd::Lane::openHat, 1 },
        { "ride", fgd::Lane::ride, 3 }, { "rd", fgd::Lane::ride, 1 },
        { "crash", fgd::Lane::crash, 3 }, { "cymbal", fgd::Lane::crash, 3 }, { "splash", fgd::Lane::crash, 3 }, { "china", fgd::Lane::crash, 2 }, { "cym", fgd::Lane::crash, 2 }, { "cr", fgd::Lane::crash, 1 },
        { "shaker", fgd::Lane::shaker, 3 }, { "maraca", fgd::Lane::shaker, 3 }, { "cabasa", fgd::Lane::shaker, 3 }, { "shake", fgd::Lane::shaker, 2 }, { "shk", fgd::Lane::shaker, 1 },
        { "tambourine", fgd::Lane::tambourine, 3 }, { "tamborine", fgd::Lane::tambourine, 3 }, { "tamb", fgd::Lane::tambourine, 3 }, { "tmb", fgd::Lane::tambourine, 1 },
        { "cowbell", fgd::Lane::cowbell, 3 }, { "agogo", fgd::Lane::cowbell, 3 }, { "bell", fgd::Lane::cowbell, 2 }, { "cb", fgd::Lane::cowbell, 1 },
        { "conga", fgd::Lane::conga, 3 }, { "bongo", fgd::Lane::conga, 3 }, { "timbale", fgd::Lane::conga, 3 }, { "djembe", fgd::Lane::conga, 3 }, { "perc", fgd::Lane::conga, 2 }, { "cga", fgd::Lane::conga, 1 },
        { "tom", fgd::Lane::tom, 3 }, { "toms", fgd::Lane::tom, 3 }, { "floortom", fgd::Lane::tom, 3 }, { "tm", fgd::Lane::tom, 1 },
    };

    juce::StringArray wordsOf (const juce::String& text)
    {
        // "AB_XY_Clap_001", "KickDrum 02", "Hi-Hat closed": lower-case words, digits dropped,
        // camelCase split.
        juce::String spaced;
        for (int i = 0; i < text.length(); ++i)
        {
            const auto c = text[i];
            if (i > 0 && juce::CharacterFunctions::isUpperCase (c) && juce::CharacterFunctions::isLowerCase (text[i - 1]))
                spaced << ' ';
            spaced << (juce::CharacterFunctions::isLetter (c) ? juce::String::charToString (c) : juce::String (" "));
        }
        juce::StringArray words;
        words.addTokens (spaced.toLowerCase(), " ", {});
        words.removeEmptyStrings();
        return words;
    }
}

bool FaseGrooveDrumsProcessor::isNotADrum (const juce::String& name)
{
    static const char* const others[] { "guitar", "gtr", "bass", "bassline", "sub", "vocal", "vocals", "vox", "voice", "acapella",
                                        "synth", "pad", "pads", "piano", "keys", "organ", "rhodes", "chord", "chords", "stab", "stabs",
                                        "string", "strings", "brass", "horn", "lead", "pluck", "arp", "melody", "fx", "sfx", "riser",
                                        "sweep", "impact", "uplifter", "downlifter", "noise", "atmo", "ambience", "texture", "loop",
                                        "loops", "bpm", "song", "mix", "master", "stem" };
    // ...and words that are something else unless the name also says a drum ("Sub Kick", "Bass Drum").
    static const char* const unlessDrum[] { "bass", "sub", "fx", "sfx", "stab", "stabs", "noise", "impact" };
    const auto words = wordsOf (name);
    for (const auto* w : others)
        if (words.contains (w) && juce::String (w) != "bass" && juce::String (w) != "sub" && juce::String (w) != "fx"
            && juce::String (w) != "sfx" && juce::String (w) != "stab" && juce::String (w) != "stabs" && juce::String (w) != "noise"
            && juce::String (w) != "impact")
            return true;
    for (const auto* w : unlessDrum)
        if (words.contains (w) && laneFromWords (name) < 0 && ! words.joinIntoString ({}).contains ("bassdrum"))
            return true;
    return false;
}

int FaseGrooveDrumsProcessor::laneFromWords (const juce::String& text, const juce::StringArray& ignore)
{
    const auto words = wordsOf (text);
    if (words.isEmpty())
        return -1;
    const auto squashed = words.joinIntoString ({});   // "hi hat" -> "hihat"

    std::array<double, fgd::numLanes> score {};
    for (const auto& w : drumWords)
    {
        const juce::String word (w.word);
        for (int i = 0; i < words.size(); ++i)
        {
            const auto& token = words[i];
            const bool match = w.strength == 1 ? token == word
                                               : token == word || (word.length() > 3 && token.contains (word));
            if (! match || (w.strength == 1 && ignore.contains (token)))
                continue;
            // Later words weigh a little more: a pack's codes come first, the drum last.
            score[(size_t) w.lane] = std::max (score[(size_t) w.lane], w.strength * (1.0 + 0.3 * i / (double) words.size()));
        }
        if (w.strength == 3 && word.length() > 4 && squashed.contains (word))
            score[(size_t) w.lane] = std::max (score[(size_t) w.lane], 3.0);
    }

    // An open hat is a hat that says open.
    if (score[(size_t) fgd::Lane::openHat] > 0.0 && score[(size_t) fgd::Lane::closedHat] > 0.0)
        score[(size_t) fgd::Lane::openHat] += score[(size_t) fgd::Lane::closedHat];
    if (score[(size_t) fgd::Lane::openHat] > 0.0 && score[(size_t) fgd::Lane::openHat] < 2.0 && score[(size_t) fgd::Lane::closedHat] == 0.0
        && ! words.contains ("oh") && ! words.contains ("ohh") && ! words.contains ("hho"))
        score[(size_t) fgd::Lane::openHat] = 0.0;   // "open" alone isn't a hat

    int best = -1;
    for (int l = 0; l < fgd::numLanes; ++l)
        if (score[(size_t) l] > 0.0 && (best < 0 || score[(size_t) l] > score[(size_t) best]))
            best = l;
    return best;
}

int FaseGrooveDrumsProcessor::laneFromSound (const juce::File& file)
{
    juce::AudioFormatManager formats;
    formats.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader (formats.createReaderFor (file));
    if (reader == nullptr || reader->lengthInSamples <= 0 || reader->sampleRate <= 0.0)
        return -1;

    const double sr = reader->sampleRate;
    const int length = (int) juce::jmin<juce::int64> (reader->lengthInSamples, (juce::int64) (sr * 2.0));
    juce::AudioBuffer<float> audio ((int) juce::jlimit (1u, 2u, reader->numChannels), length);
    reader->read (&audio, 0, length, 0, true, audio.getNumChannels() > 1);
    std::vector<float> y ((size_t) length);
    for (int i = 0; i < length; ++i)
        y[(size_t) i] = audio.getNumChannels() > 1 ? 0.5f * (audio.getSample (0, i) + audio.getSample (1, i)) : audio.getSample (0, i);

    // How long it rings (to 40 dB under its peak), how fast it starts, and where its energy is
    // in the first 80 ms: sub, low-mid, mid, high-mid, highs.
    int peakAt = 0;
    float peak = 0.0f;
    for (int i = 0; i < length; ++i)
        if (std::abs (y[(size_t) i]) > peak) { peak = std::abs (y[(size_t) i]); peakAt = i; }
    if (peak < 1.0e-4f)
        return -1;
    int end = length - 1;
    while (end > peakAt && std::abs (y[(size_t) end]) < peak * 0.01f)
        --end;
    const double decay = (end - peakAt) / sr, attack = peakAt / sr;

    const int window = juce::jmin (length, (int) (0.08 * sr));
    auto bandEnergy = [&] (double lo, double hi)
    {
        std::vector<float> b (y.begin(), y.begin() + window);
        for (int order = 0; order < 2; ++order)
        {
            juce::IIRFilter f;
            if (lo > 0.0) { f.setCoefficients (juce::IIRCoefficients::makeHighPass (sr, lo)); f.processSamples (b.data(), window); f.reset(); }
            if (hi > 0.0) { f.setCoefficients (juce::IIRCoefficients::makeLowPass (sr, hi)); f.processSamples (b.data(), window); }
        }
        double sum = 0.0;
        for (float v : b) sum += (double) v * v;
        return sum;
    };
    const double sub = bandEnergy (0.0, 150.0), lowMid = bandEnergy (150.0, 400.0), mid = bandEnergy (400.0, 1500.0),
                 highMid = bandEnergy (1500.0, 6000.0), high = bandEnergy (6000.0, 0.0);
    const double total = sub + lowMid + mid + highMid + high + 1.0e-12;
    int crossings = 0;
    for (int i = 1; i < window; ++i)
        crossings += (y[(size_t) i - 1] < 0.0f) != (y[(size_t) i] < 0.0f);
    const double noisy = crossings / (window / sr) / 10000.0;   // ~1 for noise-like hits

    if (sub / total > 0.45)                       return (int) fgd::Lane::kick;
    if (high / total > 0.45)
    {
        if (decay > 0.9)                          return (int) fgd::Lane::crash;
        if (decay > 0.25)                         return (int) fgd::Lane::openHat;
        return attack > 0.012 ? (int) fgd::Lane::shaker : (int) fgd::Lane::closedHat;
    }
    if ((highMid + high) / total > 0.5 && noisy > 0.35)
        return lowMid / total > 0.1 ? (int) fgd::Lane::snare : (int) fgd::Lane::clap;
    if ((sub + lowMid) / total > 0.5)             return (int) fgd::Lane::tom;
    if (mid / total > 0.35)                       return decay < 0.06 ? (int) fgd::Lane::rim : (int) fgd::Lane::conga;
    if (highMid / total > 0.35)                   return noisy > 0.35 ? (int) fgd::Lane::clap : (int) fgd::Lane::cowbell;
    return -1;
}

namespace
{
    juce::File sampleTagFile()
    {
        return FaseGrooveDrumsProcessor::userFolder().getChildFile ("Sample tags.xml");
    }
}

void FaseGrooveDrumsProcessor::tagSample (const juce::File& file, int lane)
{
    auto xml = juce::parseXML (sampleTagFile());
    if (xml == nullptr || ! xml->hasTagName ("SampleTags"))
        xml = std::make_unique<juce::XmlElement> ("SampleTags");
    for (auto* e : xml->getChildIterator())
        if (e->getStringAttribute ("path") == file.getFullPathName())
        {
            xml->removeChildElement (e, true);
            break;
        }
    auto* e = xml->createNewChildElement ("Sample");
    e->setAttribute ("path", file.getFullPathName());
    e->setAttribute ("drum", fgd::laneName (juce::jlimit (0, fgd::numLanes - 1, lane)));
    sampleTagFile().getParentDirectory().createDirectory();
    xml->writeTo (sampleTagFile());
}

std::vector<FaseGrooveDrumsProcessor::SampleInfo> FaseGrooveDrumsProcessor::scanSampleFolder (const juce::File& folder, int maxFiles,
                                                                                         std::function<void (float)> progress)
{
    std::vector<SampleInfo> found;
    if (! folder.isDirectory())
        return found;

    auto files = folder.findChildFiles (juce::File::findFiles, true, "*.wav;*.aif;*.aiff;*.flac;*.mp3;*.ogg");
    files.sort();
    if (files.size() > maxFiles)
        files.removeRange (maxFiles, files.size());

    // A pack's codes ("AB_XY_..."): the short words most of its files start with don't count.
    juce::StringArray ignore;
    if (files.size() >= 4)
    {
        std::map<juce::String, int> count;
        for (const auto& file : files)
        {
            auto words = wordsOf (file.getFileNameWithoutExtension());
            words.removeDuplicates (false);
            for (const auto& w : words)
                if (w.length() <= 3)
                    ++count[w];
        }
        for (const auto& [word, n] : count)
            if (n * 2 > files.size())
                ignore.add (word);
    }

    // The drums you set by hand.
    std::map<juce::String, int> tags;
    if (auto xml = juce::parseXML (sampleTagFile()))
        for (auto* e : xml->getChildIterator())
            for (int l = 0; l < fgd::numLanes; ++l)
                if (e->getStringAttribute ("drum") == fgd::laneName (l))
                    tags[e->getStringAttribute ("path")] = l;

    int done = 0;
    for (const auto& file : files)
    {
        if (progress != nullptr)
            progress ((float) done++ / (float) juce::jmax (1, files.size()));
        SampleInfo info;
        info.file = file;
        if (const auto tag = tags.find (file.getFullPathName()); tag != tags.end())
        {
            info.lane = tag->second;
            info.source = SampleInfo::Source::manual;
            found.push_back (info);
            continue;
        }

        // Not a drum hit (a guitar, a vocal, a loop...) by its name or any of its folders: left out.
        bool other = isNotADrum (file.getFileNameWithoutExtension());
        for (auto dir = file.getParentDirectory(); ! other && dir.isAChildOf (folder); dir = dir.getParentDirectory())
            other = isNotADrum (dir.getFileName());
        if (other)
            continue;

        // Its name first, then the folders it sits in (nearest first, the folder chosen last).
        if ((info.lane = laneFromWords (file.getFileNameWithoutExtension(), ignore)) >= 0)
            info.source = SampleInfo::Source::name;
        else
            for (auto dir = file.getParentDirectory(); info.lane < 0 && (dir == folder || dir.isAChildOf (folder)); dir = dir.getParentDirectory())
            {
                if ((info.lane = laneFromWords (dir.getFileName(), ignore)) >= 0)
                    info.source = SampleInfo::Source::folder;
                if (dir == folder)
                    break;
            }
        // Nothing says it: only a guess from its sound, for you to confirm.
        if (info.lane < 0 && file.getSize() < 8 * 1024 * 1024)
        {
            info.guess = laneFromSound (file);
            info.source = info.guess >= 0 ? SampleInfo::Source::sound : SampleInfo::Source::none;
        }
        found.push_back (info);
    }
    return found;
}

juce::StringArray FaseGrooveDrumsProcessor::loadKitFolder (const juce::File& folder)
{
    // For each drum, the first sample that says it by name, else by folder, else by sound.
    juce::StringArray loaded;
    const auto found = scanSampleFolder (folder);
    for (int l = 0; l < fgd::numLanes; ++l)
        for (auto source : { SampleInfo::Source::manual, SampleInfo::Source::name, SampleInfo::Source::folder })
        {
            const auto it = std::find_if (found.begin(), found.end(), [l, source] (const SampleInfo& i) { return i.lane == l && i.source == source; });
            if (it != found.end() && loadSample (l, it->file))
            {
                loaded.add (fgd::laneName (l));
                break;
            }
        }
    sampleFolder = folder;
    return loaded;
}

void FaseGrooveDrumsProcessor::previewSampleFile (const juce::File& file)
{
    juce::AudioFormatManager formats;
    formats.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader (formats.createReaderFor (file));
    if (reader == nullptr || reader->lengthInSamples <= 0 || reader->sampleRate <= 0.0)
        return;
    auto sample = std::make_shared<DrumSample>();
    const int length = (int) juce::jmin<juce::int64> (reader->lengthInSamples, (juce::int64) (reader->sampleRate * 4.0));
    sample->audio.setSize (juce::jlimit (1, 2, (int) reader->numChannels), length);
    reader->read (&sample->audio, 0, length, 0, true, sample->audio.getNumChannels() > 1);
    sample->sampleRate = reader->sampleRate;
    sample->path = file.getFullPathName();
    previewPosition = -1.0;
    std::atomic_store (&previewSample, std::shared_ptr<const DrumSample> (std::move (sample)));
    previewPosition = 0.0;
}

void FaseGrooveDrumsProcessor::clearAllSamples()
{
    for (int l = 0; l < fgd::numLanes; ++l)
        clearSample (l);
}

std::uint32_t FaseGrooveDrumsProcessor::freshSeed()
{
    return (std::uint32_t) juce::Random::getSystemRandom().nextInt ({ 1, 100000 });
}

void FaseGrooveDrumsProcessor::selectGroove (int pattern)
{
    setParameterFromUI (ParamIDs::pattern, (float) pattern);
    const auto& presets = fgd::drumPatterns();
    if (juce::isPositiveAndBelow (pattern, (int) presets.size()))
        if (const int genre = fgd::categoryIndex (presets[(size_t) pattern].category); genre >= 0)
        {
            setParameterFromUI (ParamIDs::genre, (float) genre);
            setParameterFromUI (ParamIDs::swing, fgd::genreSwing (genre));
        }
}

void FaseGrooveDrumsProcessor::rollNewBeat()
{
    setParameterFromUI (ParamIDs::swing, fgd::genreSwing (juce::roundToInt (raw (ParamIDs::genre))));

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
    setParameterFromUI (ParamIDs::swing, juce::jlimit (0.0f, 1.0f, fgd::genreSwing (style) + (r.nextFloat() - 0.5f) * 0.12f));
    setParameterFromUI (ParamIDs::humanize, 0.05f + 0.35f * r.nextFloat());
    setParameterFromUI (ParamIDs::fillStyle, (float) r.nextInt ((int) fgd::FillStyle::custom));   // not the custom fill: it may be empty
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
    for (auto* grid : { &userChance, &userRoll })
        for (auto& lane : *grid)
            for (auto& v : lane)
                v = 0;
    clearCustomFill();
    clearGrooveTemplate();
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
        // Linked: the shared clock, so the linked plug-ins play the same beat at the same time.
        // Else a free-running clock: from the start, or where PAUSE left it; REWIND jumps back.
        if (const double anchor = linkAnchor.load(); linkOn.load() && preview && anchor >= 0.0)
        {
            startPpq = juce::jmax (0.0, (juce::Time::getMillisecondCounterHiRes() - anchor) * bpm / 60000.0);
            previewRewind = false;
        }
        else
        {
            startPpq = wasRunning || ! previewFromStart.load() ? internalPpq : 0.0;
            if (previewRewind.exchange (false))
                startPpq = 0.0;
        }
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

    // A sample auditioned in the sample browser, on top of everything.
    if (double pos = previewPosition.load(); pos >= 0.0)
        if (const auto sample = std::atomic_load (&previewSample); sample != nullptr)
        {
            const double step = sample->sampleRate / sampleRate;
            const int length = sample->audio.getNumSamples();
            for (int i = 0; i < numSamples && pos < length - 1; ++i, pos += step)
                for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                {
                    const int src = juce::jmin (ch, sample->audio.getNumChannels() - 1);
                    const int at = (int) pos;
                    const float frac = (float) (pos - at);
                    const float v = sample->audio.getSample (src, at) * (1.0f - frac) + sample->audio.getSample (src, at + 1) * frac;
                    buffer.addSample (ch, i, 0.8f * v);
                }
            previewPosition = pos < length - 1 ? pos : -1.0;
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
void FaseGrooveDrumsProcessor::renderGuideAudio (const std::vector<fgd::NoteEvent>& clipEvents, double beats,
                                                 juce::AudioBuffer<float>& out, double sampleRate, int onlyLane) const
{
    const double samplesPerBeat = sampleRate * 60.0 / exportBpm();
    const int total = juce::jmax (1, (int) std::ceil (beats * samplesPerBeat));
    out.setSize (2, total);
    out.clear();

    GuideDrums kit;
    kit.prepare (sampleRate);
    kit.setKit (juce::roundToInt (raw (ParamIDs::kit)));
    kit.setTone (raw (ParamIDs::tone));
    const auto settings = currentSettings();
    std::array<float, fgd::numLanes> gains {};
    DrumSamples current;
    for (int l = 0; l < fgd::numLanes; ++l)
    {
        const float db = raw (ParamIDs::laneVolume (l));
        gains[(size_t) l] = (onlyLane >= 0 && l != onlyLane) || db <= -47.9f ? 0.0f : juce::Decibels::decibelsToGain (db);
        current[(size_t) l] = std::atomic_load (&samples[(size_t) l]);
    }

    juce::Reverb reverb;
    reverb.setSampleRate (sampleRate);
    const float amount = raw (ParamIDs::reverb);
    juce::Reverb::Parameters rp;
    rp.roomSize = 0.45f;
    rp.damping = 0.5f;
    rp.width = 1.0f;
    rp.wetLevel = amount * 0.4f;
    rp.dryLevel = 1.0f - amount * 0.25f;
    reverb.setParameters (rp);
    const float gain = juce::Decibels::decibelsToGain (raw (ParamIDs::volume));

    constexpr int renderBlock = 256;
    juce::AudioBuffer<float> block (2, renderBlock);
    size_t next = 0;
    for (int start = 0; start < total; start += renderBlock)
    {
        const int n = juce::jmin (renderBlock, total - start);
        juce::MidiBuffer midi;
        while (next < clipEvents.size() && clipEvents[next].ppq * samplesPerBeat < start + n)
        {
            const auto& e = clipEvents[next++];
            const int at = juce::jlimit (0, n - 1, (int) std::floor (e.ppq * samplesPerBeat) - start);
            if (onlyLane >= 0 && e.lane != onlyLane)
                continue;
            midi.addEvent (e.isNoteOn() ? juce::MidiMessage::noteOn (1, e.note, (juce::uint8) e.velocity) : juce::MidiMessage::noteOff (1, e.note), at);
        }
        block.setSize (2, n, false, false, true);
        block.clear();
        kit.render (block, midi, settings.notes, gains, current);
        if (amount > 0.001f)
            reverb.processStereo (block.getWritePointer (0), block.getWritePointer (1), n);
        for (int ch = 0; ch < 2; ++ch)
            out.addFrom (ch, start, block, ch, 0, n, gain);
    }
}

juce::String FaseGrooveDrumsProcessor::audioFileName (AudioExport mode, int bars) const
{
    const auto midiName = clipFileName (mode == AudioExport::song ? songArrangement : bars, ClipPart::all);
    return midiName.upToLastOccurrenceOf (".mid", false, false) + (mode == AudioExport::stems ? " (stems)" : ".wav");
}

juce::Array<juce::File> FaseGrooveDrumsProcessor::writeAudio (AudioExport mode, int bars, const juce::File& target) const
{
    const double sampleRate = getSampleRate() >= 44100.0 ? getSampleRate() : 44100.0;
    const auto settings = currentSettings();
    juce::Array<juce::File> written;

    auto write = [&] (const juce::File& file, const juce::AudioBuffer<float>& audio)
    {
        file.deleteFile();
        juce::WavAudioFormat wav;
        std::unique_ptr<juce::OutputStream> stream (file.createOutputStream());
        if (stream == nullptr)
            return;
        const auto options = juce::AudioFormatWriterOptions().withSampleRate (sampleRate).withNumChannels (2).withBitsPerSample (24);
        if (auto writer = wav.createWriterFor (stream, options))
        {
            writer->writeFromAudioSampleBuffer (audio, 0, audio.getNumSamples());
            written.add (file);
        }
    };

    if (mode == AudioExport::song)
    {
        // The whole song, with two seconds for the last sounds to ring out.
        const double beats = clipLengthInBars (songArrangement) * 4.0;
        const auto songEvents = fgd::GrooveEngine::renderArrangement (settings, songArrangementParts());
        juce::AudioBuffer<float> audio;
        renderGuideAudio (songEvents, beats + exportBpm() / 30.0, audio, sampleRate, -1);
        write (target, audio);
        return written;
    }

    // A clip that loops seamlessly: rendered twice, the second time (with the tails of the first
    // ringing into it) is the one kept.
    const double beats = clipLengthInBars (bars) * 4.0;
    auto twice = fgd::GrooveEngine::renderClip (settings, beats * 2.0);
    auto loopOf = [&] (int lane)
    {
        juce::AudioBuffer<float> both, audio;
        renderGuideAudio (twice, beats * 2.0, both, sampleRate, lane);
        const int half = both.getNumSamples() / 2;
        audio.setSize (2, half);
        for (int ch = 0; ch < 2; ++ch)
            audio.copyFrom (ch, 0, both, ch, both.getNumSamples() - half, half);
        return audio;
    };

    if (mode == AudioExport::clip)
    {
        write (target, loopOf (-1));
        return written;
    }

    // Stems: one file per drum that plays.
    if (! target.createDirectory())
        return written;
    for (int l = 0; l < fgd::numLanes; ++l)
        if (std::any_of (twice.begin(), twice.end(), [l] (const fgd::NoteEvent& e) { return e.lane == l && e.isNoteOn(); }))
            write (target.getChildFile (juce::String (l + 1).paddedLeft ('0', 2) + " " + fgd::laneName (l) + ".wav"), loopOf (l));
    return written;
}

//==============================================================================
juce::File FaseGrooveDrumsProcessor::linkFile()
{
    // Shared by every Fase Groove plug-in on this computer.
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
#if JUCE_MAC
        .getChildFile ("Application Support")
#endif
        .getChildFile ("Fase Groove").getChildFile ("Link.xml");
}

void FaseGrooveDrumsProcessor::setLinked (bool on)
{
    if (on == linkOn.load())
        return;
    linkOn = on;
    linkSeenText = {};
    linkShared = {};
    linkAnchor = -1.0;
}

int FaseGrooveDrumsProcessor::currentGenre() const
{
    // The groove's own genre (a preset), else the genre the dice write in.
    const int pattern = juce::roundToInt (raw (ParamIDs::pattern));
    const auto& presets = fgd::drumPatterns();
    if (juce::isPositiveAndBelow (pattern, (int) presets.size()))
        if (const int g = fgd::categoryIndex (presets[(size_t) pattern].category); g >= 0)
            return g;
    return juce::roundToInt (raw (ParamIDs::genre));
}

FaseGrooveDrumsProcessor::LinkState FaseGrooveDrumsProcessor::localLinkState() const
{
    LinkState s;
    s.genre = currentGenre();
    s.swing = raw (ParamIDs::swing);
    s.bpm = raw (ParamIDs::tempo);
    s.sync = raw (ParamIDs::tempoSync) > 0.5f;
    s.playing = previewOn.load();
    s.paused = ! s.playing && ! previewFromStart.load();
    s.anchor = s.playing ? linkAnchor.load() : -1.0;
    return s;
}

void FaseGrooveDrumsProcessor::previewPlay()
{
    // Linked: the clock starts now (or where PAUSE left it), and everyone plays from it.
    if (linkOn.load())
        linkAnchor = juce::Time::getMillisecondCounterHiRes() - (previewFromStart.load() ? 0.0 : internalPpq * 60000.0 / lastBpm.load());
    previewOn = true;
    updateLink();
}

void FaseGrooveDrumsProcessor::previewPause()
{
    previewOn = false;
    previewFromStart = false;
    updateLink();
}

void FaseGrooveDrumsProcessor::previewStop()
{
    previewOn = false;
    previewFromStart = true;
    linkAnchor = -1.0;
    updateLink();
}

void FaseGrooveDrumsProcessor::previewToStart()
{
    previewFromStart = true;
    previewRewind = true;
    if (linkOn.load() && previewOn.load())
        linkAnchor = juce::Time::getMillisecondCounterHiRes();
    updateLink();
}

void FaseGrooveDrumsProcessor::updateLink()
{
    if (! linkOn.load())
        return;

    const auto file = linkFile();
    const auto me = juce::String::toHexString ((juce::pointer_sized_int) this);

    // Someone else changed it: take what changed.
    if (file.existsAsFile())
        if (const auto text = file.loadFileAsString(); text != linkSeenText)
        {
            linkSeenText = text;
            if (auto xml = juce::parseXML (text); xml != nullptr && xml->getStringAttribute ("instance") != me)
            {
                // Their genre: the first groove of it (unless this one already is), and the dice's genre.
                const int theirs = fgd::categoryIndex (xml->getStringAttribute ("genre").toStdString());
                if (theirs >= 0 && theirs != currentGenre())
                {
                    const int pattern = juce::roundToInt (raw (ParamIDs::pattern));
                    const auto& presets = fgd::drumPatterns();
                    if (juce::isPositiveAndBelow (pattern, (int) presets.size()))
                        for (size_t i = 0; i < presets.size(); ++i)
                            if (presets[i].category == fgd::grooveCategories()[(size_t) theirs])
                            {
                                setParameterFromUI (ParamIDs::pattern, (float) i);
                                break;
                            }
                    setParameterFromUI (ParamIDs::genre, (float) theirs);
                }
                if (xml->hasAttribute ("swing"))
                    setParameterFromUI (ParamIDs::swing, juce::jlimit (0.0f, 1.5f, (float) xml->getDoubleAttribute ("swing")));
                if (xml->hasAttribute ("bpm"))
                    setParameterFromUI (ParamIDs::tempo, juce::jlimit (60.0f, 180.0f, (float) xml->getDoubleAttribute ("bpm")));
                if (xml->hasAttribute ("sync"))
                    setParameterFromUI (ParamIDs::tempoSync, xml->getBoolAttribute ("sync") ? 1.0f : 0.0f);
                if (xml->hasAttribute ("playing"))
                {
                    const double anchor = xml->getDoubleAttribute ("anchor", -1.0);
                    if (xml->getBoolAttribute ("playing") && anchor >= 0.0)
                    {
                        linkAnchor = anchor;
                        previewOn = true;
                    }
                    else
                    {
                        previewOn = false;
                        previewFromStart = ! xml->getBoolAttribute ("paused");
                        linkAnchor = -1.0;
                    }
                }
                linkShared = localLinkState();
                return;
            }
        }

    // We changed it: tell the others. A new tempo while playing keeps the position on the clock.
    auto now = localLinkState();
    if (now.playing && now.anchor >= 0.0 && linkShared.bpm > 0.0f && std::abs (now.bpm - linkShared.bpm) >= 0.01f && ! now.sync)
    {
        const double t = juce::Time::getMillisecondCounterHiRes();
        const double beat = (t - now.anchor) * linkShared.bpm / 60000.0;
        linkAnchor = now.anchor = t - beat * 60000.0 / now.bpm;
    }
    if (now.sameAs (linkShared))
        return;

    juce::XmlElement xml ("FaseGrooveLink");
    xml.setAttribute ("from", JucePlugin_Name);
    xml.setAttribute ("instance", me);
    if (now.genre >= 0)
        xml.setAttribute ("genre", fgd::grooveCategories()[(size_t) juce::jlimit (0, (int) fgd::grooveCategories().size() - 1, now.genre)]);
    xml.setAttribute ("swing", now.swing);   // 0 straight, 1 a triplet shuffle (as Fase Groove Piano)
    xml.setAttribute ("bpm", now.bpm);
    xml.setAttribute ("sync", now.sync);
    xml.setAttribute ("playing", now.playing);
    xml.setAttribute ("paused", now.paused);
    xml.setAttribute ("anchor", juce::String (now.anchor, 3));
    file.getParentDirectory().createDirectory();
    if (xml.writeTo (file))
        linkSeenText = file.loadFileAsString();
    linkShared = now;
}

//==============================================================================
juce::ValueTree FaseGrooveDrumsProcessor::stateTree() const
{
    auto state = const_cast<juce::AudioProcessorValueTreeState&> (apvts).copyState();   // copyState() locks, it changes nothing

    juce::ValueTree grid ("Grid");
    for (int l = 0; l < fgd::numLanes; ++l)
    {
        juce::String line, levels, chances, rolls;
        bool extras = false;
        for (int i = 0; i < gridSteps; ++i)
        {
            line << juce::String::charToString ((juce::juce_wchar) (unsigned char) userGrid[(size_t) l][(size_t) i].load());
            levels << juce::String::toHexString (juce::jlimit (0, 127, userVelocity[(size_t) l][(size_t) i].load())).paddedLeft ('0', 2);
            chances << juce::String::toHexString (juce::jlimit (0, 100, userChance[(size_t) l][(size_t) i].load())).paddedLeft ('0', 2);
            rolls << juce::String (juce::jlimit (0, 4, userRoll[(size_t) l][(size_t) i].load()));
            extras = extras || userChance[(size_t) l][(size_t) i].load() > 0 || userRoll[(size_t) l][(size_t) i].load() > 1;
        }
        grid.setProperty ("lane" + juce::String (l + 1), line, nullptr);
        grid.setProperty ("velocity" + juce::String (l + 1), levels, nullptr);
        if (extras)
        {
            grid.setProperty ("chance" + juce::String (l + 1), chances, nullptr);
            grid.setProperty ("roll" + juce::String (l + 1), rolls, nullptr);
        }
    }

    juce::ValueTree fill ("Fill");
    for (int l = 0; l < fgd::numLanes; ++l)
    {
        juce::String line;
        for (int i = 0; i < 16; ++i)
            line << juce::String::charToString ((juce::juce_wchar) (unsigned char) customFill[(size_t) l][(size_t) i].load());
        fill.setProperty ("lane" + juce::String (l + 1), line, nullptr);
    }

    juce::ValueTree groove ("Template");
    if (templateName.isNotEmpty())
    {
        juce::String timing, level;
        for (int i = 0; i < 16; ++i)
        {
            timing << juce::String (templateTiming[(size_t) i].load(), 4) << " ";
            level << juce::String (templateLevel[(size_t) i].load(), 4) << " ";
        }
        groove.setProperty ("name", templateName, nullptr);
        groove.setProperty ("active", templateActive.load(), nullptr);
        groove.setProperty ("timing", timing.trim(), nullptr);
        groove.setProperty ("level", level.trim(), nullptr);
    }
    juce::ValueTree link ("Link");
    link.setProperty ("on", linkOn.load(), nullptr);
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
    if (sampleFolder != juce::File())
        sounds.setProperty ("folder", sampleFolder.getFullPathName(), nullptr);
    if (! sampleFolders.isEmpty())
        sounds.setProperty ("folders", sampleFolders.joinIntoString ("\n"), nullptr);
    for (int l = 0; l < fgd::numLanes; ++l)
        if (const auto path = samplePath (l); path.isNotEmpty())
            sounds.setProperty ("lane" + juce::String (l + 1), path, nullptr);

    for (const auto* name : { "Grid", "Song", "Samples", "Fill", "Template", "Link" })
        state.removeChild (state.getChildWithName (name), nullptr);
    for (auto& child : { grid, song, sounds, fill, groove, link })
        state.appendChild (child, nullptr);
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
        const auto chances = grid.getProperty ("chance" + juce::String (l + 1)).toString();
        const auto rolls = grid.getProperty ("roll" + juce::String (l + 1)).toString();
        for (int i = 0; i < gridSteps; ++i)
        {
            const char c = i < line.length() ? (char) line[i] : '.';
            userGrid[(size_t) l][(size_t) i] = fgd::isHit (c) ? c : '.';
            userVelocity[(size_t) l][(size_t) i] = juce::jlimit (0, 127, levels.substring (i * 2, i * 2 + 2).getHexValue32());
            userChance[(size_t) l][(size_t) i] = juce::jlimit (0, 100, chances.substring (i * 2, i * 2 + 2).getHexValue32());
            userRoll[(size_t) l][(size_t) i] = juce::jlimit (0, 4, rolls.substring (i, i + 1).getIntValue());
        }
    }

    const auto fill = state.getChildWithName ("Fill");
    for (int l = 0; l < fgd::numLanes; ++l)
    {
        const auto line = fill.getProperty ("lane" + juce::String (l + 1)).toString();
        for (int i = 0; i < 16; ++i)
        {
            const char c = i < line.length() ? (char) line[i] : '.';
            customFill[(size_t) l][(size_t) i] = fgd::isHit (c) ? c : '.';
        }
    }

    const auto groove = state.getChildWithName ("Template");
    {
        std::array<float, 16> timing {}, level {};
        level.fill (1.0f);
        const auto t = juce::StringArray::fromTokens (groove.getProperty ("timing").toString(), " ", {});
        const auto v = juce::StringArray::fromTokens (groove.getProperty ("level").toString(), " ", {});
        for (int i = 0; i < 16; ++i)
        {
            if (i < t.size()) timing[(size_t) i] = juce::jlimit (-0.5f, 0.5f, t[i].getFloatValue());
            if (i < v.size()) level[(size_t) i] = juce::jlimit (0.2f, 1.6f, v[i].getFloatValue());
        }
        setTemplate (groove.getProperty ("name").toString(), timing, level);
        templateActive = templateName.isNotEmpty() && (bool) groove.getProperty ("active", true);
    }
    if (const auto link = state.getChildWithName ("Link"); link.isValid())
        setLinked ((bool) link.getProperty ("on", false));
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
    if (const auto folder = sounds.getProperty ("folder").toString(); folder.isNotEmpty())
        sampleFolder = juce::File (folder);
    sampleFolders = juce::StringArray::fromLines (sounds.getProperty ("folders").toString());
    sampleFolders.removeEmptyStrings();
    for (int l = 0; l < fgd::numLanes; ++l)
    {
        const auto path = sounds.getProperty ("lane" + juce::String (l + 1)).toString();
        if (path == samplePath (l))
            continue;
        if (path.isEmpty() || ! loadSample (l, juce::File (path)))
            clearSample (l);
    }

    for (const auto* name : { "Grid", "Song", "Samples", "Fill", "Template", "Link" })
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
    editorScale = (float) state.getProperty ("editorScale", 0.0f);
    if (editorScale > 0.0f)
        editorScale = juce::jlimit (0.6f, 2.0f, editorScale);
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
        juce::String levels, chances, rolls;
        for (int i = 0; i < steps(); ++i)
        {
            levels << juce::String::toHexString (stepVelocity (l, i)).paddedLeft ('0', 2);
            chances << juce::String::toHexString (stepChance (l, i) >= 100 ? 0 : stepChance (l, i)).paddedLeft ('0', 2);
            rolls << juce::String (stepRoll (l, i) > 1 ? stepRoll (l, i) : 0);
        }
        auto* lane = xml.createNewChildElement ("Lane");
        lane->setAttribute ("drum", fgd::laneName (l));
        lane->setAttribute ("steps", juce::String (p.lanes[(size_t) l]));
        lane->setAttribute ("velocity", levels);
        lane->setAttribute ("chance", chances);
        lane->setAttribute ("roll", rolls);
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
        const auto chances = lane->getStringAttribute ("chance");
        const auto rolls = lane->getStringAttribute ("roll");
        const int length = juce::jmax (1, steps.length());
        for (int i = 0; i < gridSteps; ++i)   // a two-bar groove fills the four bars twice
        {
            const int k = i % length;
            const char c = k < steps.length() ? (char) steps[k] : '.';
            userGrid[(size_t) l][(size_t) i] = fgd::isHit (c) ? c : '.';
            userVelocity[(size_t) l][(size_t) i] = juce::jlimit (0, 127, levels.substring (k * 2, k * 2 + 2).getHexValue32());
            userChance[(size_t) l][(size_t) i] = juce::jlimit (0, 100, chances.substring (k * 2, k * 2 + 2).getHexValue32());
            userRoll[(size_t) l][(size_t) i] = l == (int) fgd::Lane::kick ? 0 : juce::jlimit (0, 4, rolls.substring (k, k + 1).getIntValue());
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

struct FaseGrooveDrumsProcessor::LoopAnalysis
{
    fgd::DrumPattern pattern;
    std::array<std::array<int, gridSteps>, fgd::numLanes> levels {};
    double bpm = 120.0;
    int bars = 1;
    struct Hit { double position; int lane; float level; };   // position in 16ths from the loop's grid
    std::vector<Hit> hits;
    std::vector<double> oddDelays, evenDelays;
};

std::unique_ptr<FaseGrooveDrumsProcessor::LoopAnalysis> FaseGrooveDrumsProcessor::analyseLoop (const juce::File& file) const
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

    auto result = std::make_unique<LoopAnalysis>();
    result->bpm = bpm;
    result->bars = wholeBars;
    auto& p = result->pattern;
    for (auto& line : p.lanes)
        line.assign ((size_t) gridSteps, '.');
    auto& levels = result->levels;

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
            const float r = h.level / top;
            if (r < 0.12f)
                continue;
            const int lane = laneFor (h);
            if (pos > -0.5)
                result->hits.push_back ({ pos, lane, r });
            const int step = (int) std::lround (pos);
            if (step < 0 || step >= steps())
                continue;   // the first two (or four) bars
            const int v = juce::jlimit (1, 127, (int) std::lround (35.0 + 92.0 * r));
            if (v <= levels[(size_t) lane][(size_t) step])
                continue;
            levels[(size_t) lane][(size_t) step] = v;
            p.lanes[(size_t) lane][(size_t) step] = r > 0.75f ? 'X' : r > 0.38f ? 'x' : 'g';
            if (measureSwing && r > 0.38f)
                (step % 2 == 1 ? result->oddDelays : result->evenDelays).push_back (pos - step);
        }
    };
    place (kicks, [] (const AudioHit&) { return (int) fgd::Lane::kick; }, 0.0f, false);
    place (mids, [] (const AudioHit&) { return (int) fgd::Lane::clap; }, 0.28f, false);
    place (highs, [] (const AudioHit& h) { return (int) (h.ring > 0.4f ? fgd::Lane::openHat : fgd::Lane::closedHat); }, 0.0f, true);

    // A loop shorter than the groove plays again.
    repeatLoop (p, levels, juce::jmin (wholeBars * 16, steps()), gridSteps);
    return result;
}

juce::String FaseGrooveDrumsProcessor::importAudioGroove (const juce::File& file)
{
    const auto loop = analyseLoop (file);
    if (loop == nullptr)
        return {};

    setUserPattern (loop->pattern);
    for (int l = 0; l < fgd::numLanes; ++l)
        for (int i = 0; i < gridSteps; ++i)
            userVelocity[(size_t) l][(size_t) i] = loop->levels[(size_t) l][(size_t) i];
    for (int i = 0; i < fgd::numSegments; ++i)
        setSegmentAt (i, i);

    // The swing: how late the odd 16ths are, as the Swing control measures it.
    juce::String report = juce::String (loop->bars) + " " + tr (loop->bars == 1 ? "bar" : "bars") + ", " + juce::String (loop->bpm, 1) + " BPM";
    if (loop->oddDelays.size() >= 2)
    {
        auto median = [] (std::vector<double> v) { std::sort (v.begin(), v.end()); return v.empty() ? 0.0 : v[v.size() / 2]; };
        const double swingAmount = juce::jlimit (0.0, 1.5, (median (loop->oddDelays) - median (loop->evenDelays)) * 3.0);
        setParameterFromUI (ParamIDs::swing, (float) swingAmount);
        report << ", " << tr ("Swing").toLowerCase() << " " << swingPercent ((float) swingAmount) << "%";
    }
    return report;
}

namespace
{
    /** A groove template from hits on a 16th grid (position in 16ths, level 0..1): the median
        timing and the average level of each 16th of the bar. */
    bool templateFrom (const std::vector<std::pair<double, float>>& hits, std::array<float, 16>& timing, std::array<float, 16>& level)
    {
        std::array<std::vector<double>, 16> offsets;
        std::array<std::vector<double>, 16> levels;
        for (const auto& [position, l] : hits)
        {
            const auto step = (long long) std::llround (position);
            const int inBar = (int) (((step % 16) + 16) % 16);
            offsets[(size_t) inBar].push_back (position - (double) step);
            levels[(size_t) inBar].push_back (l);
        }
        auto median = [] (std::vector<double> v) { std::sort (v.begin(), v.end()); return v[v.size() / 2]; };
        auto mean = [] (const std::vector<double>& v) { double sum = 0.0; for (double x : v) sum += x; return sum / (double) v.size(); };

        // The 16ths the loop doesn't play take the timing of their kind (even / odd).
        std::vector<double> even, odd;
        int filled = 0;
        double top = 0.0;
        for (int i = 0; i < 16; ++i)
            if (! offsets[(size_t) i].empty())
            {
                ++filled;
                (i % 2 == 0 ? even : odd).push_back (median (offsets[(size_t) i]));
                top = std::max (top, mean (levels[(size_t) i]));
            }
        if (filled < 3 || top <= 0.0)
            return false;
        const double evenTiming = even.empty() ? 0.0 : median (even);
        const double oddTiming = odd.empty() ? 0.0 : median (odd);
        for (int i = 0; i < 16; ++i)
        {
            // Relative to the downbeats: a loop that is late everywhere is just late.
            const double t = offsets[(size_t) i].empty() ? (i % 2 == 0 ? evenTiming : oddTiming) : median (offsets[(size_t) i]);
            timing[(size_t) i] = (float) juce::jlimit (-0.5, 0.5, t - evenTiming);
            level[(size_t) i] = levels[(size_t) i].empty() ? 1.0f : (float) juce::jlimit (0.4, 1.2, 0.45 + 0.75 * mean (levels[(size_t) i]) / top);
        }
        return true;
    }
}

void FaseGrooveDrumsProcessor::setTemplate (const juce::String& name, const std::array<float, 16>& timing, const std::array<float, 16>& level)
{
    templateName = name;
    for (int i = 0; i < 16; ++i)
    {
        templateTiming[(size_t) i] = timing[(size_t) i];
        templateLevel[(size_t) i] = level[(size_t) i];
    }
}

void FaseGrooveDrumsProcessor::clearGrooveTemplate()
{
    templateActive = false;
    std::array<float, 16> timing {}, level {};
    level.fill (1.0f);
    setTemplate ({}, timing, level);
}

juce::String FaseGrooveDrumsProcessor::grooveTemplateFromAudio (const juce::File& file)
{
    const auto loop = analyseLoop (file);
    if (loop == nullptr)
        return {};

    // The hands make the groove: hats, claps and snares (the kick is the grid).
    std::vector<std::pair<double, float>> hits;
    for (const auto& h : loop->hits)
        if (h.lane != (int) fgd::Lane::kick && h.level > 0.2f)
            hits.push_back ({ h.position, h.level });
    std::array<float, 16> timing {}, level {};
    if (! templateFrom (hits, timing, level))
        return {};
    setTemplate (file.getFileNameWithoutExtension(), timing, level);
    templateActive = true;
    return file.getFileNameWithoutExtension() + " (" + juce::String (loop->bpm, 1) + " BPM)";
}

juce::String FaseGrooveDrumsProcessor::grooveTemplateFromMidi (const juce::File& file)
{
    juce::FileInputStream in (file);
    juce::MidiFile midi;
    if (! in.openedOk() || ! midi.readFrom (in) || midi.getTimeFormat() <= 0)
        return {};
    const double ticks = midi.getTimeFormat();

    std::vector<std::pair<double, float>> hits;
    for (int t = 0; t < midi.getNumTracks(); ++t)
        for (const auto* e : *midi.getTrack (t))
            if (e->message.isNoteOn() && fgd::laneForGeneralMidiNote (e->message.getNoteNumber()) != (int) fgd::Lane::kick)
                hits.push_back ({ e->message.getTimeStamp() / ticks / fgd::stepLength, e->message.getFloatVelocity() });
    std::array<float, 16> timing {}, level {};
    if (! templateFrom (hits, timing, level))
        return {};
    setTemplate (file.getFileNameWithoutExtension(), timing, level);
    templateActive = true;
    return file.getFileNameWithoutExtension();
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
