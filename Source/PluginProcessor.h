// Fase Groove Drums - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include "LinkFile.h"
#include "RawParameters.h"
#include "RetireBin.h"

#include "GuideDrums.h"
#include "engine/GrooveEngine.h"

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include <array>
#include <atomic>
#include <functional>

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
    inline constexpr const char* partLength  = "partLength";    // the parts A-D: half a bar (2-bar groove) or 1 bar (4-bar groove)
    inline constexpr const char* genre       = "genre";         // the genre NEW BEAT writes in
    inline constexpr const char* midiChannel = "midiChannel";
    inline constexpr const char* sound       = "sound";         // the guide kit on / off
    inline constexpr const char* kit         = "kit";
    inline constexpr const char* tone        = "tone";
    inline constexpr const char* reverb      = "reverb";
    inline constexpr const char* volume      = "volume";

    inline constexpr const char* genreFeel   = "genreFeel";     // each genre's own pocket (push and swing per drum)
    inline constexpr const char* bassSpace   = "bassSpace";     // kick pick-ups only at the end of four bars

    inline juce::String segment (int index) { return "segment" + juce::String (index + 1); }
    inline juce::String mute (int lane)  { return "mute" + juce::String (lane + 1); }
    inline juce::String note (int lane)  { return "note" + juce::String (lane + 1); }
    inline juce::String level (int lane) { return "level" + juce::String (lane + 1); }
    inline juce::String laneVolume (int lane) { return "volume" + juce::String (lane + 1); }   // the guide sound's volume of a drum, in dB
    inline juce::String push (int lane) { return "push" + juce::String (lane + 1); }             // ms ahead (< 0) or behind (> 0) the beat
    inline juce::String laneSwing (int lane) { return "laneSwing" + juce::String (lane + 1); }   // the share of the swing a drum takes
}

class FaseGrooveDrumsProcessor : public juce::AudioProcessor
{
public:
    FaseGrooveDrumsProcessor();
    ~FaseGrooveDrumsProcessor() override;

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
    /** A parameter's current value, by its ID (a cached lookup: no String built, safe on any thread). */
    float raw (juce::StringRef id) const { return rawParams (id); }

    //==============================================================================
    // The grid (message thread). Steps are as displayed (two bars); edits go into the user
    // pattern, which then becomes the groove.
    static constexpr int gridSteps = fgd::userPatternLength;   // the most steps a groove can have (4 bars)
    /** The steps of the groove now: 32 (parts of half a bar) or 64 (parts of one bar). */
    int steps() const { return juce::roundToInt (raw (ParamIDs::partLength)) == 1 ? 64 : 32; }
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

    /** The chance a displayed step plays (100 = always) and its roll (1 = one hit, 2-4 quick hits). */
    int stepChance (int lane, int step) const;
    void setStepChance (int lane, int step, int percent);
    int stepRoll (int lane, int step) const;
    void setStepRoll (int lane, int step, int hits);

    // The fill drawn by hand (Fill style: Custom): one bar of 16ths.
    char customFillSymbol (int lane, int step) const;
    void editCustomFill (int lane, int step, char symbol);
    void clearCustomFill();
    /** Starts the custom fill from what a fill style plays (and makes Custom the fill style). */
    void customFillFromStyle (int style);
    bool hasCustomFill() const;

    // The feel of each drum (MIXER): its push / pull and its share of the swing.
    static juce::StringArray timingFeelNames();
    void applyTimingFeel (int feel);

    // Groove templates: the timing and accents of every 16th of the bar, taken from an audio
    // loop or a MIDI file and applied to any groove. Returns a short report, empty on failure.
    juce::String grooveTemplateFromAudio (const juce::File&);
    juce::String grooveTemplateFromMidi (const juce::File&);
    void clearGrooveTemplate();
    bool hasGrooveTemplate() const { return templateActive.load(); }
    juce::String grooveTemplateName() const { return templateName; }
    void setGrooveTemplateActive (bool on) { templateActive = on && templateName.isNotEmpty(); }

    /** A whole kit at once: the samples of a folder, matched to the drums by their names
        (kick / bd, snare / sd, clap, rim, closed hat / ch, open hat / oh...). Returns the drums loaded. */
    juce::StringArray loadKitFolder (const juce::File&);
    void clearAllSamples();

    /** A sample found in a folder, and its drum: set by you, or said by its file name or its
        folder's name (Kicks, Hi Hats...). Samples whose name or folder says something else
        (guitar, vocal, loop...) are left out; the others get only a guess from their sound. */
    struct SampleInfo
    {
        enum class Source { manual, name, folder, sound, none };
        juce::File file;
        int lane = -1;                 // the drum, when sure
        Source source = Source::none;
        int guess = -1;                // what it sounds like (unnamed samples, never used on its own)
    };
    /** True when a name says it is not a single drum hit (guitar, vocal, synth, loop, fx...). */
    static bool isNotADrum (const juce::String& name);
    /** progress (optional) is told how far the scan is, 0..1 (from the thread that scans). */
    static std::vector<SampleInfo> scanSampleFolder (const juce::File& folder, int maxFiles = 4000,
                                                     std::function<void (float)> progress = {});
    /** The drum a name says, -1 if none. Whole words (kick, snare, clap...) beat abbreviations (bd,
        sd, ch...), later words beat earlier ones (pack prefixes come first), and abbreviations in
        `ignore` (the codes every file of a pack starts with) don't count. */
    static int laneFromWords (const juce::String& text, const juce::StringArray& ignore = {});
    static int laneFromSound (const juce::File&);          // -1: can't tell
    /** Remembers the drum you set for a sample by hand (the browser uses it from then on). */
    static void tagSample (const juce::File&, int lane);
    juce::File sampleFolder;                               // the last folder browsed (saved with the project)
    juce::StringArray sampleFolders;                       // every folder in the sample browser (saved with the project)

    /** Plays a sample file once, as it is (the sample browser). */
    void previewSampleFile (const juce::File&);

    // Audio export of the guide sound (the built-in kit or your samples).
    enum class AudioExport { clip = 0, song, stems };
    /** Writes WAV file(s) into `target` (a file, or a folder for the stems); returns the files written. */
    juce::Array<juce::File> writeAudio (AudioExport, int bars, const juce::File& target) const;
    juce::String audioFileName (AudioExport, int bars) const;

    // LINK: genre and swing shared with Fase Groove Piano (and other Fase Groove instances).
    bool isLinked() const { return linkOn.load(); }
    void setLinked (bool on);
    static juce::File linkFile();
    /** Writes the genre and swing for the others, or takes theirs (message thread, a few times a second). */
    void updateLink();

    /** Velocity shapes for one drum (what makes hats groove): the levels of its hits are set by hand. */
    static juce::StringArray velocityShapeNames();
    void applyVelocityShape (int lane, int shape);
    /** The velocity dice: a random shape with a little life on every hit. */
    void randomizeVelocities (int lane);

    bool isLaneMuted (int lane) const { return raw (ParamIDs::mute (lane)) > 0.5f; }
    void setLaneMuted (int lane, bool muted) { setParameterFromUI (ParamIDs::mute (lane), muted ? 1.0f : 0.0f); }
    int laneNote (int lane) const { return juce::roundToInt (raw (ParamIDs::note (lane))); }
    /** Every drum's note at once, from a ready-made map (fgd::noteMapName). */
    void applyNoteMap (int map);
    /** A sample of the project that wasn't found when it loaded (kept, so saving doesn't lose it). */
    juce::String missingSample (int lane) const;

    // Your own sounds: a sample (WAV, AIFF, FLAC, MP3...) in place of a drum's built-in sound.
    static bool isAudioFile (const juce::File&);
    bool loadSample (int lane, const juce::File&);
    void clearSample (int lane);
    juce::String samplePath (int lane) const;   // empty: the built-in sound

    /** Picks a groove; a preset brings its genre's swing (and makes its genre the dice's genre). */
    void selectGroove (int pattern);

    /** NEW BEAT: a brand new random beat in the chosen genre (with the genre's swing). */
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
    enum class ClipPart { all = 0, tracks, group1, group2, group3, group4 };   // tracks: one MIDI file, one track per drum
    /** The instrument groups of the export (as Fase Groove Piano's piano / bass / strings). */
    static constexpr int numPartGroups = 4;
    static const char* partGroupName (int group) noexcept;   // "KICK", "SNARE" ...
    static bool laneInPart (ClipPart, int lane) noexcept;
    static juce::String sectionClipName (int partIndex);
    /** Pass as `bars` to export the whole song: intro, main, break, build-up, drop, outro. */
    static constexpr int songArrangement = -1;

    /** Renders the groove as a standard MIDI file. bars == 0 means "one full loop". */
    juce::MidiFile createMidiClip (int bars, ClipPart part) const;
    /** Writes a .mid whole or not at all: a temporary file first, the one already there replaced
        only once the new one is complete (a full disk or no permission keeps the old clip). */
    static bool writeMidiFileSafely (const juce::MidiFile&, const juce::File& target);
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
    float editorScale = 0.0f;   // 0: as big as suits the screen (the first time)

    juce::AudioProcessorValueTreeState apvts;

private:
    // Every parameter's value, looked up once (in the constructor): reading them on the audio
    // thread then never allocates a String nor searches the parameter tree.
    FaseGroove::RawParameters rawParams;
    enum LaneParam { laneNoteParam = 0, laneLevelParam, laneMuteParam, lanePushParam, laneSwingParam, laneVolumeParam, numLaneParams };
    std::array<std::array<std::atomic<float>*, fgd::numLanes>, numLaneParams> laneParams {};
    std::array<std::atomic<float>*, fgd::numSegments> segmentParams {};
    void cacheParameters();
    float rawLane (LaneParam kind, int lane) const noexcept;
public:

    //==============================================================================
    /** Plays one hit of a drum now (the pads), through the guide kit and the MIDI out. */
    void audition (int lane, int velocity = 110)   { auditionRequest = juce::jlimit (0, fgd::numLanes - 1, lane) * 128 + juce::jlimit (1, 127, velocity); }

    std::atomic<bool> previewOn { false };   // play the groove without the host transport
    std::atomic<bool> previewFromStart { true }, previewRewind { false };
    // (With LINK on, play / pause / stop / back to the start are shared with the linked plug-ins.)
    void previewPlay();
    void previewPause();
    void previewStop();
    void previewToStart();
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
    void resetStepExtras (int lane, int index) { userChance[(size_t) lane][(size_t) index] = 0; userRoll[(size_t) lane][(size_t) index] = 0; }
    /** A 2-bar groove is stored twice so 4-bar mode plays it again. Keep that copy in step
        with an edit, unless the second half was already changed on its own. */
    void mirrorUserStepIfCopy (int lane, int index, char oldSymbol, int oldVelocity, int oldChance, int oldRoll);
    struct LoopAnalysis;
    std::unique_ptr<LoopAnalysis> analyseLoop (const juce::File&) const;
    void renderGuideAudio (const std::vector<fgd::NoteEvent>&, double beats, juce::AudioBuffer<float>&, double sampleRate, int onlyLane) const;
    static std::uint32_t freshSeed();

    // The user pattern, readable from the audio thread without locks.
    std::array<std::array<std::atomic<char>, fgd::userPatternLength>, fgd::numLanes> userGrid;
    std::array<std::array<std::atomic<int>, fgd::userPatternLength>, fgd::numLanes> userVelocity;   // 0 = automatic
    std::array<std::array<std::atomic<int>, fgd::userPatternLength>, fgd::numLanes> userChance, userRoll;   // 0 = always / one hit
    std::array<std::array<std::atomic<char>, 16>, fgd::numLanes> customFill;
    std::atomic<bool> templateActive { false };
    std::array<std::atomic<float>, 16> templateTiming, templateLevel;
    juce::String templateName;
    void setTemplate (const juce::String& name, const std::array<float, 16>& timing, const std::array<float, 16>& level);

    std::atomic<bool> linkOn { false };
    // What the linked plug-ins share: genre, swing, tempo, DAW sync and the play button's clock
    // (anchor: the system time, in high-resolution ms, at which that clock was at beat 0).
    struct LinkState
    {
        int genre = -2;
        float swing = -1.0f, bpm = -1.0f;
        bool sync = false, playing = false, paused = false;
        double anchor = -1.0;
        bool sameAs (const LinkState& o) const noexcept
        {
            return genre == o.genre && std::abs (swing - o.swing) < 0.001f && std::abs (bpm - o.bpm) < 0.01f && sync == o.sync
                && playing == o.playing && paused == o.paused && std::abs (anchor - o.anchor) < 1.0;
        }
    };
    LinkState linkShared;                      // what the link file says (or what we wrote)
    juce::String linkSeenText;
    FaseGroove::PolledFile linkPolled;   // message thread (LinkFile.h)
    std::atomic<double> linkAnchor { -1.0 };   // the shared clock while playing, -1 = none
    LinkState localLinkState() const;
    int currentGenre() const;
    juce::TimedCallback linkTimer { [this] { updateLink(); } };
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
    // The groove worked out ahead on the message thread (the audio thread only reads it), and the
    // old ones kept until the audio thread has let go of them.
    std::shared_ptr<const fgd::GrooveEngine::Prepared> preparedPattern;
    FaseGroove::RetireBin<fgd::GrooveEngine::Prepared> retiredPatterns;   // RetireBin.h
    void preparePattern();
    juce::TimedCallback patternTimer { [this] { preparePattern(); } };
    std::array<juce::String, fgd::numLanes> missingSamples;   // message thread
    double lastLinkAnchor = -1.0, lastLinkBpm = 0.0;           // audio thread: the linked clock as last played
    std::vector<fgd::NoteEvent> events;
    juce::MidiBuffer outMidi;
    std::array<int, 128> noteChannels {};   // channel used by each generated note-on
    std::atomic<int> auditionRequest { -1 };   // lane * 128 + velocity

    GuideDrums drums;
    DrumSamples samples;   // read and swapped with std::atomic_load / std::atomic_store
    std::shared_ptr<const DrumSample> previewSample;   // the file being auditioned (atomic_load / atomic_store)

    FaseGroove::RetireBin<DrumSample> sampleBin;   // samples replaced while the audio thread may still play them
    void swapSample (std::shared_ptr<const DrumSample>& slot, std::shared_ptr<const DrumSample> next)
    {
        sampleBin.retire (std::atomic_exchange (&slot, std::move (next)));
    }
    std::atomic<double> previewPosition { -1.0 };      // in the sample's own samples, -1 = silent
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
