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
    inline constexpr const char* accents     = "accents";       // the accent groove of hats, shaker, ride, tambourine
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

    inline juce::String segment (int index) { return "segment" + juce::String (index + 1); }
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
    /** The groove shown on the grid: as you hear it in the A-D order (before the song section). */
    fgd::DrumPattern displayedPattern() const;

    // A B C D: the four half bars of the groove, played in any order; each can get new random
    // content (its die) or be locked so the dice leave it alone.
    int segmentAt (int position) const;            // the part (0..3) played at a position
    void setSegmentAt (int position, int part);
    bool isSegmentLocked (int part) const          { return segmentLocked[(size_t) juce::jlimit (0, 3, part)]; }
    void setSegmentLocked (int part, bool locked)  { segmentLocked[(size_t) juce::jlimit (0, 3, part)] = locked; }
    void randomizeSegment (int part);
    /** A new line for one drum only (in the NEW BEAT genre); locked parts stay. */
    void randomizeLane (int lane);

    /** Velocity set by hand on a displayed step (1..127, 100 = the Velocity control), 0 = automatic. */
    int stepVelocity (int lane, int step) const;
    void setStepVelocity (int lane, int step, int level);
    void resetStepVelocities (int lane = -1);   // one drum, or all (-1)
    bool hasStepVelocities() const;
    /** The velocity a displayed step plays at (set by hand, or automatic from its symbol). */
    int effectiveVelocity (int lane, int step) const;

    /** Velocity shapes for one drum (what makes hats groove): the levels of its hits are set by hand. */
    static juce::StringArray velocityShapeNames();
    void applyVelocityShape (int lane, int shape);
    /** The velocity dice: a random shape with a little life on every hit. */
    void randomizeVelocities (int lane);

    bool isLaneMuted (int lane) const { return raw (ParamIDs::mute (lane)) > 0.5f; }
    void setLaneMuted (int lane, bool muted) { setParameterFromUI (ParamIDs::mute (lane), muted ? 1.0f : 0.0f); }
    int laneNote (int lane) const { return juce::roundToInt (raw (ParamIDs::note (lane))); }

    // Your own sounds: a sample (WAV, AIFF, FLAC, MP3...) in place of a drum's built-in sound.
    static bool isAudioFile (const juce::File&);
    bool loadSample (int lane, const juce::File&);
    void clearSample (int lane);
    juce::String samplePath (int lane) const;   // empty: the built-in sound

    /** NEW BEAT: a brand new random beat in the chosen genre. */
    void rollNewBeat();
    /** NEW FEEL: another humanization (repeatable: it is saved with the project). */
    void newFeel();

    /** TOTAL RANDOM: a random style, then a new beat, swing, feel, fills and kit; always musical. */
    void totalRandom();

    /** One-click setups by genre: a groove of that genre, its swing, feel, fills and kit. */
    static juce::StringArray styleNames();
    void applyStyle (int index);
    /** Every setting back to its default. */
    void resetToDefaults();

    //==============================================================================
    enum class ClipPart { all = 0, tracks };   // tracks: one MIDI file, one track per drum
    static juce::String sectionClipName (int partIndex);
    /** Pass as `bars` to export the whole song: intro, main, break, build-up, drop, outro. */
    static constexpr int songArrangement = -1;

    /** Renders the groove as a standard MIDI file. bars == 0 means "one full loop". */
    juce::MidiFile createMidiClip (int bars, ClipPart part) const;
    /** Writes the clip into Documents/Fase Groove Drums/MIDI clips and returns the file. */
    juce::File writeMidiClip (int bars, ClipPart part) const;
    juce::String clipFileName (int bars, ClipPart part) const;
    int clipLengthInBars (int bars) const;

    // Song arrangement: Intro, Main, Break, Build-up, Drop, Outro (the order of fgd::defaultArrangement).
    static constexpr int numSongParts = 6;
    std::vector<fgd::ArrangementPart> songArrangementParts() const;
    void setSongBars (int part, int bars);
    void setSongFill (int part, int fill);   // -1 as the Song panel, -2 none, else a fill style (last bar of the section)
    void regenerateSongPart (int part);      // this section plays its own version of the groove
    void resetSongPart (int part);           // back to the groove as it is
    void resetSong();
    /** One section of the song as a clip, named after it (empty file if the section is left out). */
    juce::File writeSongSectionClip (int partIndex, ClipPart part) const;
    /** Writes every section of the song as its own .mid file; returns how many were written. */
    int saveSongSections (const juce::File& folder, ClipPart) const;

    /** Documents/Fase Groove Drums: MIDI clips, presets and grooves, easy to find and back up. */
    static juce::File userFolder();
    static juce::File clipFolder();
    static juce::File presetFolder();
    static juce::File grooveFolder();
    static juce::Array<juce::File> presetFiles();
    static juce::Array<juce::File> grooveFiles();
    bool savePreset (const juce::String& name);
    bool loadPreset (const juce::File&);
    /** MY GROOVES: the grid (with its velocities) as a .fgdgroove file. */
    bool saveGroove (const juce::String& name);
    bool loadGroove (const juce::File&);

    /** Reads the drums of a MIDI file (channel 10, or the General MIDI drum notes) into the grid:
        two bars, quantized to 16ths, with their velocities. Returns false when no drums were found. */
    bool importMidiGroove (const juce::File&);

    /** Turns an audio drum loop into the grid: kick, clap / snare, closed and open hats, with their
        velocities, on two bars of 16ths (the loop's length gives the tempo). Returns a short report
        ("2 bars at 125 BPM, swing 30%"), empty when nothing could be read. */
    juce::String importAudioGroove (const juce::File&);

    // Undo / redo: snapshots of everything but the window size.
    juce::String snapshot() const;
    void restoreSnapshot (const juce::String&);

    // Editor preferences, saved with the project.
    float editorScale = 1.0f;

    juce::AudioProcessorValueTreeState apvts;

    //==============================================================================
    /** Plays one hit of a drum now (the pads), through the guide kit and the MIDI out. */
    void audition (int lane, int velocity = 110)   { auditionRequest = juce::jlimit (0, fgd::numLanes - 1, lane) * 128 + juce::jlimit (1, 127, velocity); }

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
    std::array<std::array<std::atomic<int>, fgd::userPatternLength>, fgd::numLanes> userVelocity;   // 0 = automatic
    int baseStep (int displayedStep) const;   // the step of the user pattern shown at a displayed step
    /** Makes the groove showing the user pattern (as written: before the A-D order); true if it already was. */
    bool makeUserPattern();
    std::array<bool, fgd::numSegments> segmentLocked {};
    std::array<std::atomic<int>, numSongParts> songBars;                 // per song section (0 = left out)
    std::array<std::atomic<std::uint32_t>, numSongParts> songVariation;  // 0 = the groove as it is
    std::array<std::atomic<int>, numSongParts> songFill;                 // see setSongFill
    juce::ValueTree stateTree() const;
    void applyStateTree (const juce::ValueTree&);

    juce::MidiFile midiFromEvents (const std::vector<fgd::NoteEvent>&, double beats,
                                   const std::vector<fgd::GrooveEngine::Marker>*, ClipPart, const juce::String& name) const;

    fgd::GrooveEngine engine;
    std::vector<fgd::NoteEvent> events;
    juce::MidiBuffer outMidi;
    std::atomic<int> auditionRequest { -1 };   // lane * 128 + velocity

    GuideDrums drums;
    DrumSamples samples;   // read and swapped with std::atomic_load / std::atomic_store
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
