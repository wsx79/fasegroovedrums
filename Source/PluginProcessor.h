// Fase Groove Drums - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include "GuideDrums.h"
#include "engine/GrooveEngine.h"

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include <array>
#include <atomic>

namespace ParamIDs
{
    inline constexpr const char* pattern     = "pattern";
    inline constexpr const char* swing       = "swing";
    inline constexpr const char* velocity    = "velocity";
    inline constexpr const char* humanize    = "humanize";
    inline constexpr const char* humanizeSeed = "humanizeSeed";
    inline constexpr const char* fills       = "fills";
    inline constexpr const char* fillStyle   = "fillStyle";
    inline constexpr const char* crash       = "crash";         // a crash after every fill
    inline constexpr const char* variation   = "variation";
    inline constexpr const char* section     = "section";
    inline constexpr const char* tempoSync   = "tempoSync";
    inline constexpr const char* tempo       = "tempo";
    inline constexpr const char* sequencer   = "sequencer";     // off: the groove doesn't play with the DAW
    inline constexpr const char* riffSeed    = "riffSeed";
    inline constexpr const char* density     = "density";
    inline constexpr const char* genre       = "genre";         // the genre NEW BEAT writes in
    inline constexpr const char* midiChannel = "midiChannel";
    inline constexpr const char* sound       = "sound";         // the guide kit on / off
    inline constexpr const char* kit         = "kit";
    inline constexpr const char* tone        = "tone";
    inline constexpr const char* reverb      = "reverb";
    inline constexpr const char* volume      = "volume";

    inline juce::String mute (int lane)  { return "mute" + juce::String (lane + 1); }
    inline juce::String note (int lane)  { return "note" + juce::String (lane + 1); }
    inline juce::String level (int lane) { return "level" + juce::String (lane + 1); }
}

class FaseGrooveDrumsProcessor : public juce::AudioProcessor
{
public:
    FaseGrooveDrumsProcessor();
    ~FaseGrooveDrumsProcessor() override = default;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override  { return true; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    /** Current parameter values as engine settings (safe from any thread). */
    fgd::DrumSettings currentSettings() const;

    /** Sets a parameter from the UI, notifying the host. */
    void setParameterFromUI (const juce::String& id, float plainValue);
    float raw (const juce::String& id) const;

    //==============================================================================
    // The grid (message thread). Steps are as displayed (two bars); edits go into the user
    // pattern, which then becomes the groove.
    static constexpr int gridSteps = fgd::userPatternLength;
    char stepSymbol (int lane, int step) const;
    void editStep (int lane, int step, char symbol);
    void clearLane (int lane);
    void clearGrid();
    /** The groove shown on the grid (before the song section is applied). */
    fgd::DrumPattern displayedPattern() const;

    bool isLaneMuted (int lane) const { return raw (ParamIDs::mute (lane)) > 0.5f; }
    void setLaneMuted (int lane, bool muted) { setParameterFromUI (ParamIDs::mute (lane), muted ? 1.0f : 0.0f); }
    int laneNote (int lane) const { return juce::roundToInt (raw (ParamIDs::note (lane))); }

    /** NEW BEAT: a brand new random beat in the chosen genre. */
    void rollNewBeat();
    /** NEW FEEL: another humanization (repeatable: it is saved with the project). */
    void newFeel();

    /** One-click setups by genre: a groove of that genre, its swing, feel, fills and kit. */
    static juce::StringArray styleNames();
    void applyStyle (int index);
    /** Every setting back to its default. */
    void resetToDefaults();

    //==============================================================================
    enum class ClipPart { all = 0, tracks };   // tracks: one MIDI file, one track per drum
    /** Pass as `bars` to export the whole song: intro, main, break, build-up, drop, outro. */
    static constexpr int songArrangement = -1;

    /** Renders the groove as a standard MIDI file. bars == 0 means "one full loop". */
    juce::MidiFile createMidiClip (int bars, ClipPart part) const;
    /** Writes the clip into Documents/Fase Groove Drums/MIDI clips and returns the file. */
    juce::File writeMidiClip (int bars, ClipPart part) const;
    juce::String clipFileName (int bars, ClipPart part) const;
    int clipLengthInBars (int bars) const;

    /** Documents/Fase Groove Drums: MIDI clips, easy to find and back up. */
    static juce::File userFolder();
    static juce::File clipFolder();

    // Editor preferences, saved with the project.
    float editorScale = 1.0f;

    juce::AudioProcessorValueTreeState apvts;

    //==============================================================================
    std::atomic<bool> previewOn { false };   // play the groove without the host transport
    std::atomic<bool> previewFromStart { true }, previewRewind { false };
    void previewPlay()    { previewOn = true; }
    void previewPause()   { previewOn = false; previewFromStart = false; }
    void previewStop()    { previewOn = false; previewFromStart = true; }
    void previewToStart() { previewFromStart = true; previewRewind = true; }
    void previewToggle()  { previewOn.load() ? previewPause() : previewPlay(); }

    std::atomic<double> lastBpm { 120.0 };   // tempo in use (DAW or manual)
    std::atomic<double> hostBpm { 120.0 };   // what the DAW reports
    /** The tempo exported files use: the manual BPM, or the DAW's (as last reported). */
    double exportBpm() const;

    // Read by the editor for display.
    std::atomic<int> uiStep { -1 };       // current 16th within the pattern, -1 = stopped
    std::atomic<double> uiPpq { -1.0 };   // playing position in beats, -1 = stopped
    std::array<std::atomic<float>, fgd::numLanes> laneFlash {};   // last velocity per lane (for the grid's lights)

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

private:
    void setUserPattern (const fgd::DrumPattern&);
    static std::uint32_t freshSeed();

    // The user pattern, readable from the audio thread without locks.
    std::array<std::array<std::atomic<char>, fgd::userPatternLength>, fgd::numLanes> userGrid;

    juce::MidiFile midiFromEvents (const std::vector<fgd::NoteEvent>&, double beats,
                                   const std::vector<fgd::GrooveEngine::Marker>*, ClipPart, const juce::String& name) const;

    fgd::GrooveEngine engine;
    std::vector<fgd::NoteEvent> events;
    juce::MidiBuffer outMidi;

    GuideDrums drums;
    juce::Reverb reverbFx;
    juce::SmoothedValue<float> outputGain;

    // MIDI clock (the standalone app has no DAW: a DAW can send it its clock).
    juce::int64 samplesProcessed = 0, lastClockSample = -1;
    double clockInterval = 0.0, clockPpq = 0.0;
    bool clockRunning = false;

    double internalPpq = 0.0;
    double expectedNextPpq = 0.0;
    bool wasRunning = false;
    bool soundWasOn = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FaseGrooveDrumsProcessor)
};
