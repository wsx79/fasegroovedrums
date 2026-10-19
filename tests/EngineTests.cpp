// Fase Groove Drums - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

// Engine tests (no JUCE): grooves, sections, random beats, fills and the note stream.

#include "GrooveEngine.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <map>
#include <set>
#include <string>

using namespace fgd;

static int failures = 0;

#define CHECK(cond) \
    do { if (! (cond)) { std::printf ("FAILED %s:%d: %s\n", __FILE__, __LINE__, #cond); ++failures; } } while (false)

namespace
{
    int countHits (const std::string& line)
    {
        return (int) std::count_if (line.begin(), line.end(), isHit);
    }

    /** Every note-on is closed by a note-off, in time order, and no note is started twice. */
    bool wellFormed (const std::vector<NoteEvent>& events)
    {
        std::map<int, bool> on;
        double last = -1.0e9;
        for (const auto& e : events)
        {
            if (e.ppq < last - 1.0e-9)
                return false;
            last = e.ppq;
            if (e.isNoteOn())
            {
                if (on[e.note])
                    return false;
                on[e.note] = true;
            }
            else
                on[e.note] = false;
        }
        return std::none_of (on.begin(), on.end(), [] (const auto& p) { return p.second; });
    }

    int countOns (const std::vector<NoteEvent>& events, int note = -1)
    {
        return (int) std::count_if (events.begin(), events.end(),
                                    [note] (const NoteEvent& e) { return e.isNoteOn() && (note < 0 || e.note == note); });
    }

    DrumSettings straight()
    {
        DrumSettings s;
        s.swing = 0.0f;
        s.humanize = 0.0f;
        return s;
    }
}

static void testAccentGroove()
{
    // The hats sing on the offbeat and are softer on the 16ths; 0 = as written.
    auto s = straight();
    s.pattern = userPatternIndex();
    for (int i = 0; i < userPatternLength; ++i)
        s.user[(size_t) Lane::closedHat][(size_t) i] = 'x';
    std::map<int, int> byStep;
    for (const auto& e : GrooveEngine::renderClip (s, 4.0))
        if (e.isNoteOn())
            byStep[(int) std::lround (e.ppq / stepLength) % 4] = e.velocity;
    CHECK (byStep[2] > byStep[0] && byStep[0] > byStep[3] && byStep[3] > byStep[1]);

    s.accentGroove = 0.0f;
    std::set<int> flat;
    for (const auto& e : GrooveEngine::renderClip (s, 4.0))
        if (e.isNoteOn())
            flat.insert (e.velocity);
    CHECK (flat.size() == 1);
}

static void testPresets()
{
    const auto& presets = drumPatterns();
    CHECK (presets.size() == 70);   // as the README and the guide say: change them together

    std::set<std::string> names;
    std::set<std::string> categories;
    for (const auto& p : presets)
    {
        CHECK (p.length() == 16 || p.length() == 32);
        for (const auto& line : p.lanes)
        {
            CHECK ((int) line.size() == p.length());
            CHECK (std::all_of (line.begin(), line.end(), [] (char c) { return c == '.' || isHit (c); }));
        }
        CHECK (categoryIndex (p.category) >= 0);
        CHECK (names.insert (p.name).second);   // no two grooves with the same name
        categories.insert (p.category);
    }

    // Every genre of Fase Groove Piano has drums.
    CHECK (grooveCategories().size() == 13);
    CHECK (categories.size() == grooveCategories().size());
    CHECK (userPatternIndex() == randomPatternIndex() + 1);
}

static void testLanes()
{
    for (int l = 0; l < numLanes; ++l)
    {
        CHECK (laneForGeneralMidiNote (defaultNote (l)) == l);
        CHECK (std::string (generalMidiDrumName (defaultNote (l))).size() > 0);
    }
    CHECK (laneForGeneralMidiNote (35) == (int) Lane::kick);
    CHECK (laneForGeneralMidiNote (100) == -1);
}

static void testRenderBasics()
{
    auto s = straight();
    s.pattern = 0;   // Piano House Classic: four on the floor

    const auto events = GrooveEngine::renderClip (s, 16.0);   // 4 bars
    CHECK (wellFormed (events));
    CHECK (countOns (events, 36) == 16);   // a kick on every beat

    // The kick lands exactly on the beat with no swing and no humanize.
    for (const auto& e : events)
        if (e.isNoteOn() && e.note == 36)
            CHECK (std::abs (e.ppq - std::round (e.ppq)) < 1.0e-9);

    // Rendering in small blocks gives the same notes as one big block.
    GrooveEngine engine;
    std::vector<NoteEvent> blocks;
    for (double t = 0.0; t < 16.0; t += 0.0937)
        engine.render (t, std::min (16.0, t + 0.0937), s, blocks);
    engine.allNotesOff (16.0, blocks);
    CHECK (wellFormed (blocks));
    CHECK (countOns (blocks) == countOns (events));
}

static void testSwingAndHumanize()
{
    auto s = straight();
    s.pattern = 0;
    s.accentGroove = 0.0f;
    s.swing = 1.0f;
    const auto events = GrooveEngine::renderClip (s, 4.0);

    // Odd 16ths are pushed a third of a 16th later; the beats stay put.
    bool sawOdd = false;
    for (const auto& e : events)
        if (e.isNoteOn())
        {
            const double steps = e.ppq / stepLength;
            const double frac = steps - std::floor (steps + 1.0e-9);
            CHECK (frac < 1.0e-6 || std::abs (frac - 1.0 / 3.0) < 1.0e-6);
            sawOdd = sawOdd || std::abs (frac - 1.0 / 3.0) < 1.0e-6;
        }
    CHECK (sawOdd);

    // Same settings, same feel; NEW FEEL changes it.
    auto h = straight();
    h.humanize = 1.0f;
    const auto a = GrooveEngine::renderClip (h, 8.0);
    const auto b = GrooveEngine::renderClip (h, 8.0);
    CHECK (a.size() == b.size());
    bool same = true;
    for (size_t i = 0; i < a.size() && i < b.size(); ++i)
        same = same && a[i].ppq == b[i].ppq && a[i].velocity == b[i].velocity;
    CHECK (same);

    h.humanizeSeed = 12345;
    const auto c = GrooveEngine::renderClip (h, 8.0);
    bool differs = false;
    for (size_t i = 0; i < a.size() && i < c.size(); ++i)
        differs = differs || a[i].ppq != c[i].ppq || a[i].velocity != c[i].velocity;
    CHECK (differs);
}

static void testMuteMappingAndLevels()
{
    auto s = straight();
    s.pattern = 0;
    s.muted[(size_t) Lane::kick] = true;
    CHECK (countOns (GrooveEngine::renderClip (s, 8.0), 36) == 0);

    s.muted[(size_t) Lane::kick] = false;
    s.notes[(size_t) Lane::kick] = 24;   // remapped
    const auto events = GrooveEngine::renderClip (s, 8.0);
    CHECK (countOns (events, 36) == 0);
    CHECK (countOns (events, 24) == 8);

    s.levels[(size_t) Lane::kick] = 0.5f;
    for (const auto& e : GrooveEngine::renderClip (s, 4.0))
        if (e.isNoteOn() && e.note == 24)
            CHECK (e.velocity <= 64);
}

static void testSections()
{
    const auto& base = drumPatterns()[0];

    const auto intro = applySection (base, Section::intro);
    CHECK (countHits (intro[Lane::kick]) > 0);
    CHECK (countHits (intro[Lane::clap]) == 0);
    CHECK (intro.length() == base.length());

    const auto br = applySection (base, Section::breakdown);
    CHECK (countHits (br[Lane::kick]) == 0);
    CHECK (countHits (br[Lane::clap]) == 0);

    const auto build = applySection (base, Section::buildup);
    CHECK (countHits (build[Lane::snare]) > countHits (base[Lane::snare]));

    // The drop: a crash on its first bar, then every 8 bars.
    auto s = straight();
    s.pattern = 2;   // Piano Pumper (no crash written)
    s.section = Section::drop;
    const auto drop = GrooveEngine::renderClip (s, 64.0);
    CHECK (countOns (drop, 49) == 2);
}

static void testRandomBeats()
{
    for (int genre = 0; genre < (int) grooveCategories().size(); ++genre)
        for (std::uint32_t seed = 1; seed < 40; ++seed)
        {
            const auto p = makeRandomPattern (seed, (float) (seed % 5) / 4.0f, genre);
            CHECK (p.length() == randomBeatLength);
            CHECK (countHits (p[Lane::kick]) > 0);
            CHECK (countHits (p[Lane::snare]) + countHits (p[Lane::clap]) + countHits (p[Lane::rim]) > 0);
            CHECK (p.category == grooveCategories()[(size_t) genre]);
        }

    // Same seed = same beat; another seed = another beat (most of the time).
    const auto a = makeRandomPattern (7, 0.5f, 0), b = makeRandomPattern (7, 0.5f, 0);
    CHECK (a.lanes == b.lanes);
    int different = 0;
    for (std::uint32_t seed = 1; seed < 20; ++seed)
        if (makeRandomPattern (seed, 0.5f, 1).lanes != makeRandomPattern (seed + 100, 0.5f, 1).lanes)
            ++different;
    CHECK (different > 10);

    // More density, more notes (on average).
    int sparse = 0, busy = 0;
    for (std::uint32_t seed = 1; seed < 50; ++seed)
    {
        for (const auto& l : makeRandomPattern (seed, 0.0f, 1).lanes) sparse += countHits (l);
        for (const auto& l : makeRandomPattern (seed, 1.0f, 1).lanes) busy += countHits (l);
    }
    CHECK (busy > sparse);

    // The random beat plays through the engine.
    auto s = straight();
    s.pattern = randomPatternIndex();
    s.randomGenre = 6;
    CHECK (wellFormed (GrooveEngine::renderClip (s, 16.0)));
}

static void testFillsAndVariations()
{
    auto s = straight();
    s.pattern = 2;   // Piano Pumper: no snare, no toms
    s.fillEveryBars = 4;

    for (int style = 0; style < numFillStyles; ++style)
    {
        s.fillStyle = (FillStyle) style;
        const auto events = GrooveEngine::renderClip (s, 32.0);
        CHECK (wellFormed (events));
        // A crash on the bar after each fill: bar 5, and bar 1 (the loop comes round from a fill).
        CHECK (countOns (events, 49) == 2);
    }

    s.fillStyle = FillStyle::snareRoll;
    const auto roll = GrooveEngine::renderClip (s, 16.0);
    CHECK (countOns (roll, 38) == 6);   // 8ths then 16ths over the last two beats of bar 4

    s.fillStyle = FillStyle::tomRun;
    std::set<int> toms;
    for (const auto& e : GrooveEngine::renderClip (s, 16.0))
        if (e.isNoteOn() && e.lane == (int) Lane::tom)
            toms.insert (e.note);
    CHECK (toms.size() == 4);   // four different toms

    s.crashAfterFill = false;
    CHECK (countOns (GrooveEngine::renderClip (s, 32.0), 49) == 0);

    // Fills only in the main part and the drop.
    s.crashAfterFill = true;
    s.section = Section::intro;
    CHECK (countOns (GrooveEngine::renderClip (s, 32.0), 49) == 0);

    // Variation: the turnaround bar differs from the groove.
    const auto& base = drumPatterns()[0];
    int changed = 0;
    for (std::uint32_t seed = 1; seed < 30; ++seed)
        if (makeVariation (base, seed).lanes != base.lanes)
            ++changed;
    CHECK (changed == 29);

    auto v = straight();
    v.variationEvery = 4;
    CHECK (GrooveEngine::loopLengthInSteps (v) == 64);
    CHECK (wellFormed (GrooveEngine::renderClip (v, 32.0)));
}

static void testUserPatternAndFlams()
{
    auto s = straight();
    s.pattern = userPatternIndex();
    for (auto& lane : s.user)
        lane.fill ('.');
    s.user[(size_t) Lane::snare][4] = 'f';
    s.user[(size_t) Lane::kick][0] = 'X';

    const auto events = GrooveEngine::renderClip (s, 8.0);
    CHECK (wellFormed (events));
    CHECK (countOns (events, 36) == 1);
    CHECK (countOns (events, 38) == 2);   // the flam: grace note + hit

    double grace = -1.0, hit = -1.0;
    int graceVel = 0, hitVel = 0;
    for (const auto& e : events)
        if (e.isNoteOn() && e.note == 38)
        {
            if (grace < 0.0) { grace = e.ppq; graceVel = e.velocity; }
            else             { hit = e.ppq; hitVel = e.velocity; }
        }
    CHECK (grace < hit && hit - grace < 0.05);
    CHECK (graceVel < hitVel);
}

static void testArrangement()
{
    auto s = straight();
    s.pattern = 0;
    std::vector<GrooveEngine::Marker> markers;
    const auto events = GrooveEngine::renderArrangement (s, defaultArrangement(), &markers);
    CHECK (wellFormed (events));
    CHECK (markers.size() == defaultArrangement().size());
    CHECK (markers.front().name == "Intro");
    CHECK (std::abs (markers[1].ppq - 32.0) < 1.0e-9);   // after the 8-bar intro

    // No kick in the break (bars 24..31 of the song).
    for (const auto& e : events)
        if (e.isNoteOn() && e.note == 36)
            CHECK (e.ppq < 24 * 4.0 || e.ppq >= 32 * 4.0);

    // The drop starts with a crash.
    const double dropStart = (8 + 16 + 8 + 4) * 4.0;
    CHECK (std::any_of (events.begin(), events.end(),
                        [dropStart] (const NoteEvent& e) { return e.isNoteOn() && e.note == 49 && std::abs (e.ppq - dropStart) < 1.0e-6; }));
    CHECK (events.back().ppq <= 60 * 4.0 + 1.0e-9);
}

static void testPartsSectionsAndLanes()
{
    // A-D order: A A B C repeats the first half bar.
    auto base = drumPatterns()[0];
    normalisePattern (base, randomBeatLength);
    const auto ordered = applySegmentOrder (base, { 0, 0, 1, 2 });
    for (int l = 0; l < numLanes; ++l)
        CHECK (ordered.lanes[(size_t) l].substr (8, 8) == base.lanes[(size_t) l].substr (0, 8));

    // A section's own version keeps the kick and is repeatable.
    const auto a = makeSectionVariation (base, 5), b = makeSectionVariation (base, 5);
    CHECK (a.lanes == b.lanes);
    CHECK (a.lanes != base.lanes);
    CHECK (countHits (a[Lane::kick]) >= countHits (base[Lane::kick]));

    // A single drum: never empty, the right length, repeatable.
    for (int genre = 0; genre < (int) grooveCategories().size(); ++genre)
        for (int l = 0; l < numLanes; ++l)
        {
            const auto line = makeRandomLane ((Lane) l, 3, 0.5f, genre);
            CHECK ((int) line.size() == randomBeatLength);
            CHECK (countHits (line) > 0);
            CHECK (line == makeRandomLane ((Lane) l, 3, 0.5f, genre));
        }

    // Velocity set by hand on a step of the user pattern.
    auto s = straight();
    s.pattern = userPatternIndex();
    s.user[(size_t) Lane::kick][0] = 'x';
    s.userVelocity[(size_t) Lane::kick][0] = 50;
    for (const auto& e : GrooveEngine::renderClip (s, 4.0))
        if (e.isNoteOn())
            CHECK (e.velocity == 50);

    // ...and it stays on the variation bar and under a fill (the kick keeps the floor).
    s.variationEvery = 4;
    s.fillEveryBars = 4;
    for (int i = 0; i < userPatternLength; i += 4)
    {
        s.user[(size_t) Lane::kick][(size_t) i] = 'x';
        s.userVelocity[(size_t) Lane::kick][(size_t) i] = 50;
    }
    for (const auto& e : GrooveEngine::renderClip (s, 16.0))
        if (e.isNoteOn() && e.lane == (int) Lane::kick)
            if (std::abs (e.ppq - std::round (e.ppq)) < 1.0e-6)   // the hand-set kicks (a variation may add a pick-up)
                CHECK (e.velocity == 50);

    // A section's own fill: only on its last bar, or none.
    std::vector<ArrangementPart> parts { { Section::main, 4, 0, (int) FillStyle::tomRun } };
    auto f = straight();
    f.pattern = 2;
    f.fillEveryBars = 1;
    int toms = 0;
    for (const auto& e : GrooveEngine::renderArrangement (f, parts))
        if (e.isNoteOn() && e.lane == (int) Lane::tom)
        {
            ++toms;
            CHECK (e.ppq >= 12.0);
        }
    CHECK (toms == 4);
    parts[0].endFill = -2;
    for (const auto& e : GrooveEngine::renderArrangement (f, parts))
        CHECK (! (e.isNoteOn() && (e.lane == (int) Lane::crash || e.lane == (int) Lane::snare)));
}

static void testKickAlwaysOnTheGrid()
{
    // Whatever the swing, humanize, feel, groove or random beat: every kick exactly on a 16th.
    for (int pattern = 0; pattern <= randomPatternIndex(); ++pattern)
    {
        DrumSettings s;
        s.pattern = pattern;
        s.swing = 1.0f;
        s.humanize = 1.0f;
        s.humanizeSeed = 777;
        s.fillEveryBars = 2;
        s.variationEvery = 4;
        s.randomGenre = pattern % 13;
        for (const auto& e : GrooveEngine::renderClip (s, 32.0))
            if (e.isNoteOn() && e.lane == (int) Lane::kick)
            {
                const double steps = e.ppq / stepLength;
                CHECK (std::abs (steps - std::round (steps)) < 1.0e-9);
            }
    }

    // A flam on the kick has no grace note.
    auto s = straight();
    s.pattern = userPatternIndex();
    s.user[(size_t) Lane::kick][3] = 'f';
    CHECK (countOns (GrooveEngine::renderClip (s, 8.0), 36) == 1);
}

static void testFourBarGrooves()
{
    // Parts of one bar: every groove plays four bars, the parts A-D are bars 1-4.
    auto s = straight();
    s.grooveBars = 4;
    CHECK (GrooveEngine::patternLength (s) == 64);
    CHECK (GrooveEngine::basePattern (s).length() == 64);
    s.segmentOrder = { 0, 0, 0, 1 };
    const auto p = GrooveEngine::resolvePattern (s);
    for (int l = 0; l < numLanes; ++l)
        CHECK (p.lanes[(size_t) l].substr (32, 16) == p.lanes[(size_t) l].substr (0, 16));

    // The random beat: bars 1-2 again, then a turnaround.
    s.pattern = randomPatternIndex();
    s.segmentOrder = { 0, 1, 2, 3 };
    const auto r = GrooveEngine::basePattern (s);
    CHECK (r.length() == 64);
    CHECK (r[Lane::kick].substr (32, 16) == r[Lane::kick].substr (0, 16));

    // The user pattern uses all four bars.
    s.pattern = userPatternIndex();
    s.user[(size_t) Lane::clap][50] = 'X';
    const auto events = GrooveEngine::renderClip (s, 16.0);
    CHECK (countOns (events, 39) == 1);
    CHECK (GrooveEngine::loopLengthInSteps (s) == 64);
}

static void testJackinGroove()
{
    const int jackin = categoryIndex ("Jackin'");
    CHECK (jackin >= 0);
    CHECK (genreSwing (jackin) >= 0.7f && genreSwing (jackin) <= 1.0f);

    // Random jackin beats: four on the floor, the clap on 2 and 4, a hat on every offbeat.
    for (std::uint32_t seed = 1; seed < 60; ++seed)
    {
        const auto p = makeRandomPattern (seed, (float) (seed % 5) / 4.0f, jackin);
        for (int i = 0; i < 32; ++i)
        {
            if (i < 29)
                CHECK (isHit (p[Lane::kick][(size_t) i]) == (i % 4 == 0));
            if (i % 4 == 2 && i != 14 && i != 30)   // (an open hat accent can take the last offbeat)
                CHECK (isHit (p[Lane::closedHat][(size_t) i]) || isHit (p[Lane::openHat][(size_t) i]));
        }
        CHECK (isHit (p[Lane::clap][4]) && isHit (p[Lane::clap][12]));
    }

    // The jackin grooves keep the clap on 2 and 4.
    for (const auto& g : drumPatterns())
        if (g.category == "Jackin'")
        {
            CHECK (isHit (g[Lane::clap][4]) && isHit (g[Lane::clap][12]));
        }

    // Most of them have the 909 snare under the clap, and the random ones often do.
    int withSnare = 0, jackinGrooves = 0, randomWithSnare = 0;
    for (const auto& g : drumPatterns())
        if (g.category == "Jackin'")
        {
            ++jackinGrooves;
            withSnare += std::any_of (g[Lane::snare].begin(), g[Lane::snare].end(), isHit) ? 1 : 0;
        }
    CHECK (withSnare * 2 > jackinGrooves);
    for (std::uint32_t seed = 1; seed < 60; ++seed)
    {
        const auto p = makeRandomPattern (seed, 0.6f, jackin);
        randomWithSnare += std::any_of (p[Lane::snare].begin(), p[Lane::snare].end(), isHit) ? 1 : 0;
    }
    CHECK (randomWithSnare > 25);

    // The clap lands a hair before the snare under it; the kick stays on the grid.
    int index = -1;
    for (size_t i = 0; i < drumPatterns().size(); ++i)
        if (drumPatterns()[i].name == "Jack Hybrid")
            index = (int) i;
    CHECK (index >= 0);
    auto s = straight();
    s.pattern = index;
    double clapAt = -1.0, snareAt = -1.0;
    for (const auto& e : GrooveEngine::renderClip (s, 4.0))
        if (e.isNoteOn() && e.ppq > 0.9 && e.ppq < 1.1)
        {
            if (e.lane == (int) Lane::clap) clapAt = e.ppq;
            if (e.lane == (int) Lane::snare) snareAt = e.ppq;
            if (e.lane == (int) Lane::kick) CHECK (std::abs (e.ppq - 1.0) < 1.0e-9);
        }
    CHECK (clapAt > 0.0 && snareAt > 0.0 && clapAt < snareAt);
}

static void testGenresHaveDistinctGrooves()
{
    // Within every genre the grooves are different beats, not copies with one hit changed.
    auto at = [] (const DrumPattern& g, int lane, int i)
    {
        const auto& line = g.lanes[(size_t) lane];
        return line.empty() ? '.' : line[(size_t) i % line.size()];
    };
    for (const auto& category : grooveCategories())
    {
        std::vector<const DrumPattern*> grooves;
        for (const auto& g : drumPatterns())
            if (g.category == category)
                grooves.push_back (&g);
        CHECK (grooves.size() >= 4);
        for (size_t a = 0; a < grooves.size(); ++a)
            for (size_t b = a + 1; b < grooves.size(); ++b)
            {
                int different = 0;
                for (int l = 0; l < numLanes; ++l)
                    for (int i = 0; i < 32; ++i)
                        different += isHit (at (*grooves[a], l, i)) != isHit (at (*grooves[b], l, i)) ? 1 : 0;
                if (different < 10)
                    std::printf ("  too alike in %s: %s / %s (%d)\n", category, grooves[a]->name.c_str(), grooves[b]->name.c_str(), different);
                CHECK (different >= 10);
            }
    }
}

static void testJackinCharacter()
{
    std::vector<const DrumPattern*> jack;
    for (const auto& g : drumPatterns())
        if (g.category == "Jackin'")
            jack.push_back (&g);
    CHECK (jack.size() >= 10);

    auto at = [] (const DrumPattern& g, int lane, int i)   // the groove's step, its line repeated to 32
    {
        const auto& line = g.lanes[(size_t) lane];
        return line.empty() ? '.' : line[(size_t) i % line.size()];
    };

    // No two jackin grooves are (almost) the same beat.
    for (size_t a = 0; a < jack.size(); ++a)
        for (size_t b = a + 1; b < jack.size(); ++b)
        {
            int different = 0;
            for (int l = 0; l < numLanes; ++l)
                for (int i = 0; i < 32; ++i)
                    different += isHit (at (*jack[a], l, i)) != isHit (at (*jack[b], l, i)) ? 1 : 0;
            CHECK (different >= 6);
        }

    // Their hats aren't all the same offbeat line, ghosts and accents are part of them, and
    // some kicks have a ghost or a pick-up (always on the grid).
    std::set<std::string> hatLines;
    int withGhosts = 0, kickVariations = 0;
    for (const auto* g : jack)
    {
        std::string hats;
        bool ghost = false;
        for (int i = 0; i < 32; ++i)
        {
            hats += at (*g, (int) Lane::closedHat, i);
            for (int l = 0; l < numLanes; ++l)
                ghost = ghost || at (*g, l, i) == 'g';
        }
        hatLines.insert (hats);
        withGhosts += ghost ? 1 : 0;
        bool variation = false;
        for (int i = 0; i < 32; ++i)
            variation = variation || (isHit (at (*g, (int) Lane::kick, i)) && i % 4 != 0);
        kickVariations += variation ? 1 : 0;
    }
    CHECK ((int) hatLines.size() * 3 >= (int) jack.size() * 2);
    CHECK (withGhosts * 3 >= (int) jack.size() * 2);
    CHECK (kickVariations >= 4 && kickVariations * 2 < (int) jack.size());   // most stay straight and strong

    // The same seed, the same random beat.
    const int jackin = categoryIndex ("Jackin'");
    for (std::uint32_t seed = 1; seed < 20; ++seed)
        CHECK (makeRandomPattern (seed, 0.5f, jackin).lanes == makeRandomPattern (seed, 0.5f, jackin).lanes);

    // Played with swing, humanize and fills: the MIDI is in order and every note ends.
    for (size_t i = 0; i < drumPatterns().size(); ++i)
        if (drumPatterns()[i].category == "Jackin'")
        {
            DrumSettings s;
            s.pattern = (int) i;
            s.swing = genreSwing (jackin);
            s.humanize = 0.3f;
            s.fillEveryBars = 4;
            const auto events = GrooveEngine::renderClip (s, 64.0);
            std::array<int, 128> open {};
            double last = 0.0;
            for (const auto& e : events)
            {
                CHECK (e.ppq >= last - 1.0e-9);
                last = e.ppq;
                open[(size_t) e.note] += e.isNoteOn() ? 1 : -1;
                CHECK (open[(size_t) e.note] >= 0 && open[(size_t) e.note] <= 1);
                if (e.lane == (int) Lane::kick && e.isNoteOn())
                    CHECK (std::abs (e.ppq * 4.0 - std::round (e.ppq * 4.0)) < 1.0e-9);   // the kick on the grid
            }
            for (int n : open)
                CHECK (n == 0);
        }
}

static void testHatsOnTheSameStep()
{
    // Closed and open hat on the same step: only the open hat plays (it would be choked).
    auto s = straight();
    s.pattern = userPatternIndex();
    for (int i = 0; i < userPatternLength; ++i)
    {
        s.user[(size_t) Lane::closedHat][(size_t) i] = 'x';
        s.user[(size_t) Lane::openHat][(size_t) i] = i % 4 == 2 ? 'x' : '.';
    }
    int open = 0, closedWithOpen = 0;
    for (const auto& e : GrooveEngine::renderClip (s, 4.0))
        if (e.isNoteOn())
        {
            const int step = (int) std::lround (e.ppq * 4.0) % 4;
            open += e.lane == (int) Lane::openHat ? 1 : 0;
            closedWithOpen += e.lane == (int) Lane::closedHat && step == 2 ? 1 : 0;
        }
    CHECK (open == 4);   // one bar
    CHECK (closedWithOpen == 0);
}

static void testDrumFeel()
{
    auto onsets = [] (const std::vector<NoteEvent>& events, int note)
    {
        std::vector<double> t;
        for (const auto& e : events)
            if (e.isNoteOn() && e.note == note)
                t.push_back (e.ppq);
        return t;
    };

    // A clap pushed 20 ms ahead at 120 BPM lands 0.04 beats early; the kick never moves.
    auto s = straight();
    s.pattern = userPatternIndex();
    s.user[(size_t) Lane::kick][0] = s.user[(size_t) Lane::kick][4] = 'X';
    s.user[(size_t) Lane::clap][4] = 'X';
    s.user[(size_t) Lane::closedHat][1] = 'x';
    s.tempo = 120.0;
    s.laneShift[(size_t) Lane::clap] = -20.0f;
    s.laneShift[(size_t) Lane::kick] = 30.0f;
    auto e = GrooveEngine::renderClip (s, 4.0);
    CHECK (std::abs (onsets (e, 39)[0] - (1.0 - 0.04)) < 1.0e-9);
    CHECK (std::abs (onsets (e, 36)[1] - 1.0) < 1.0e-9);

    // The heaviest swing: a dotted 16th (half a step late). A drum can take none of it.
    s.swing = 1.5f;
    e = GrooveEngine::renderClip (s, 4.0);
    CHECK (std::abs (onsets (e, 42)[0] - 1.5 * stepLength) < 1.0e-9);
    s.laneSwing[(size_t) Lane::closedHat] = 0.0f;
    e = GrooveEngine::renderClip (s, 4.0);
    CHECK (std::abs (onsets (e, 42)[0] - stepLength) < 1.0e-9);

    // A groove template: its timing and levels instead of the swing.
    s.laneSwing[(size_t) Lane::closedHat] = 1.0f;
    s.grooveTemplate.active = true;
    s.grooveTemplate.timing[1] = 0.25f;
    s.grooveTemplate.level[1] = 0.5f;
    e = GrooveEngine::renderClip (s, 4.0);
    CHECK (std::abs (onsets (e, 42)[0] - 1.25 * stepLength) < 1.0e-9);
    CHECK (std::abs (onsets (e, 36)[0]) < 1.0e-9);
    s.grooveTemplate.active = false;
    s.accentGroove = 0.0f;   // compare with the hat as written
    const int loud = [&] { for (const auto& n : GrooveEngine::renderClip (s, 4.0)) if (n.isNoteOn() && n.note == 42) return n.velocity; return 0; }();
    s.grooveTemplate.active = true;
    const int soft = [&] { for (const auto& n : GrooveEngine::renderClip (s, 4.0)) if (n.isNoteOn() && n.note == 42) return n.velocity; return 0; }();
    CHECK (soft < loud);
    s.grooveTemplate.active = false;

    // A roll of three on a hat; never on the kick.
    s.userRoll[(size_t) Lane::closedHat][1] = 3;
    s.userRoll[(size_t) Lane::kick][0] = 4;
    e = GrooveEngine::renderClip (s, 4.0);
    CHECK (countOns (e, 42) == 3);
    CHECK (countOns (e, 36) == 2);
    CHECK (wellFormed (e));

    // A step with a 50% chance plays about half the time, the same way every time.
    auto c = straight();
    c.pattern = userPatternIndex();
    for (int i = 0; i < 32; ++i)
    {
        c.user[(size_t) Lane::closedHat][(size_t) i] = 'x';
        c.userChance[(size_t) Lane::closedHat][(size_t) i] = 50;
    }
    const int played = countOns (GrooveEngine::renderClip (c, 64.0), 42);
    CHECK (played > 80 && played < 176);
    CHECK (played == countOns (GrooveEngine::renderClip (c, 64.0), 42));
}

static void testNewFills()
{
    // Every fill style plays something, and the kick stays on the grid.
    for (int style = 0; style < numFillStyles; ++style)
    {
        auto s = straight();
        s.pattern = 0;
        s.fillEveryBars = 1;
        s.fillStyle = (FillStyle) style;
        s.customFill[(size_t) Lane::tom][12] = 'X';
        s.customFill[(size_t) Lane::tom][14] = 'x';
        const auto e = GrooveEngine::renderClip (s, 4.0);
        CHECK (wellFormed (e));
        for (const auto& n : e)
            if (n.isNoteOn() && n.lane == (int) Lane::kick)
                CHECK (std::abs (n.ppq / stepLength - std::round (n.ppq / stepLength)) < 1.0e-9);
    }

    // The custom fill: the toms drawn, the drums it doesn't use keep the groove.
    auto s = straight();
    s.pattern = 0;
    s.fillEveryBars = 1;
    s.fillStyle = FillStyle::custom;
    s.customFill[(size_t) Lane::tom][12] = 'X';
    s.customFill[(size_t) Lane::tom][14] = 'x';
    auto e = GrooveEngine::renderClip (s, 4.0);
    CHECK (countOns (e, 45) == 2);
    CHECK (countOns (e, 36) == 4);

    // Kick build: 16th kicks on the last beat.
    s.fillStyle = FillStyle::kickBuild;
    e = GrooveEngine::renderClip (s, 4.0);
    CHECK (countOns (e, 36) == 7);
}

int main()
{
    testDrumFeel();
    testNewFills();
    testJackinGroove();
    testFourBarGrooves();
    testKickAlwaysOnTheGrid();
    testPartsSectionsAndLanes();
    testPresets();
    testAccentGroove();
    testHatsOnTheSameStep();
    testJackinCharacter();
    testGenresHaveDistinctGrooves();
    testLanes();
    testRenderBasics();
    testSwingAndHumanize();
    testMuteMappingAndLevels();
    testSections();
    testRandomBeats();
    testFillsAndVariations();
    testUserPatternAndFlams();
    testArrangement();

    if (failures == 0)
        std::printf ("All engine tests passed.\n");
    return failures == 0 ? 0 : 1;
}
