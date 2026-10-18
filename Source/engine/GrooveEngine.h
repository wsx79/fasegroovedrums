// Fase Groove Drums - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

// Drum sequencer. Turns a musical-time window (in quarter notes, "ppq") into
// note on/off events. Pure C++ (no JUCE).

#include "Drums.h"
#include "Generators.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace fgd
{

struct DrumSettings
{
    DrumSettings() noexcept;

    int pattern = 0;
    float swing = 0.15f;                   // 0 = straight, 1 = full triplet shuffle
    int velocity = 100;
    float humanize = 0.2f;
    std::uint32_t humanizeSeed = 0;        // NEW FEEL: another, repeatable humanization

    int fillEveryBars = 0;                 // 0 = no fills
    FillStyle fillStyle = FillStyle::snareRoll;
    bool crashAfterFill = true;            // a crash on the bar after a fill (and every 8 bars of the drop)
    int variationEvery = 0;                // 0 = off, else every 4 or 8 bars the last bar varies

    Section section = Section::main;
    long long sectionStartBar = 0;         // where the section began (the drop's crash counts from there)

    std::uint32_t riffSeed = 1;            // used when pattern == randomPatternIndex()
    float density = 0.5f;
    int randomGenre = 0;                   // the genre NEW BEAT writes in (grooveCategories() index)

    std::array<int, numLanes> notes {};    // the MIDI note of each lane (General MIDI by default)
    std::array<float, numLanes> levels {}; // velocity scale of each lane (1 = as written)
    std::array<bool, numLanes> muted {};

    // Used when pattern == userPatternIndex(): the hand-edited grid.
    std::array<std::array<char, userPatternLength>, numLanes> user {};

    long long forcedFillBar = -1;          // a fill played on this bar only (live keys)...
    int forcedFillStyle = -1;              // ...in this style
};

struct NoteEvent
{
    double ppq;
    int note;
    int velocity;   // 0 for note-off
    int lane = 0;
    bool isNoteOn() const noexcept { return velocity > 0; }
};

//==============================================================================
class GrooveEngine
{
public:
    GrooveEngine();

    /** Appends (time-ordered) events falling in [startPpq, endPpq). */
    void render (double startPpq, double endPpq, const DrumSettings& settings, std::vector<NoteEvent>& out);

    /** Releases everything currently sounding at the given time. */
    void allNotesOff (double ppq, std::vector<NoteEvent>& out);

    bool anyNoteSounding() const noexcept;

    /** The groove before the section is applied: a preset, the random beat or the user pattern. */
    static DrumPattern basePattern (const DrumSettings& s);

    /** The groove actually played: the base pattern rewritten for the song section. */
    static DrumPattern resolvePattern (const DrumSettings& s);
    static int patternLength (const DrumSettings& s) noexcept;

    /** Length (in 16th steps) after which groove, variations and fills all repeat. */
    static int loopLengthInSteps (const DrumSettings& s) noexcept;

    /** Offline render of a whole clip starting at bar 1, with every note closed. */
    static std::vector<NoteEvent> renderClip (const DrumSettings& s, double lengthInBeats);

    /** Renders a whole song (intro, main, break...) with section markers. */
    struct Marker { double ppq; std::string name; };
    static std::vector<NoteEvent> renderArrangement (const DrumSettings& s, const std::vector<ArrangementPart>& parts,
                                                     std::vector<Marker>* markers = nullptr);

    /** The fill style a bar plays ("Mix" picks one per bar). */
    static FillStyle effectiveFillStyle (FillStyle style, long long bar) noexcept;

private:
    struct PendingOn { double ppq; int note; int velocity; double duration; int lane; };

    const DrumPattern& patternFor (const DrumSettings& s);
    void collectStep (long long step, const DrumSettings& s, const DrumPattern& pattern, const DrumPattern& variation);
    void flushOffsUpTo (double ppq, bool inclusive, std::vector<NoteEvent>& out);

    std::array<double, 128> offTime {};   // < 0 when the note is not sounding
    std::array<int, 128> laneOf {};
    std::vector<PendingOn> pending;
    std::vector<int> scratch;

    // Cache of the resolved pattern, rebuilt only when its inputs change.
    DrumPattern cachedPattern, cachedVariation;
    int cachedIndex = -1;
    std::uint32_t cachedSeed = 0;
    float cachedDensity = -1.0f;
    int cachedGenre = -1;
    Section cachedSection = Section::main;
    std::array<std::array<char, userPatternLength>, numLanes> cachedUser {};
};

} // namespace fgd
