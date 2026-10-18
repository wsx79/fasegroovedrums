// Fase Groove Drums - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "GrooveEngine.h"

#include <algorithm>
#include <cmath>
#include <numeric>

namespace fgd
{

namespace
{
    long long floorDiv (long long a, long long b) noexcept
    {
        long long q = a / b;
        if ((a % b != 0) && ((a < 0) != (b < 0))) --q;
        return q;
    }

    int mod (long long a, long long m) noexcept { return (int) (((a % m) + m) % m); }

    /** Deterministic 0..1 noise, so re-rendering a step always gives the same feel. */
    double noise (long long step, int salt) noexcept
    {
        auto x = (std::uint64_t) (step * 0x9E3779B97F4A7C15LL) ^ ((std::uint64_t) salt * 0xBF58476D1CE4E5B9ULL);
        x ^= x >> 31; x *= 0x94D049BB133111EBULL; x ^= x >> 29;
        return (double) (x >> 11) * (1.0 / 9007199254740992.0);
    }

    double symbolLevel (char symbol) noexcept
    {
        switch (symbol)
        {
            case 'X': return 1.0;
            case 'f': return 0.9;
            case 'g': return 0.42;
            case 'x':
            default:  return 0.8;
        }
    }

    /** How much a lane drifts with Humanize: the kick stays tight, hands drift more. */
    double laneLooseness (int lane) noexcept
    {
        switch ((Lane) lane)
        {
            case Lane::kick:       return 0.0;   // never moves (see timeFor)
            case Lane::snare:
            case Lane::clap:       return 0.6;
            case Lane::shaker:
            case Lane::tambourine:
            case Lane::conga:      return 1.2;
            default:               return 0.9;
        }
    }

    double noteLength (int lane) noexcept
    {
        switch ((Lane) lane)
        {
            case Lane::openHat: return 1.8 * stepLength;
            case Lane::ride:
            case Lane::crash:   return 4.0 * stepLength;
            default:            return 0.9 * stepLength;
        }
    }

    int lcm (int a, int b) noexcept { return a / std::gcd (a, b) * b; }

    /** The level a hat (or shaker, tambourine, ride) has at each 16th of the beat, as house
        drummers and drum machines play it: the offbeat 8th sings, everything else is less than
        half as loud, the 16th just after the beat the softest. Percussion has a gentler curve. */
    double accentProfile (int lane, int stepInBeat) noexcept
    {
        static constexpr double hats[] { 0.45, 0.28, 1.0, 0.4 };
        static constexpr double percussion[] { 0.82, 0.55, 1.0, 0.8 };
        return (lane == (int) Lane::conga || lane == (int) Lane::cowbell ? percussion : hats)[stepInBeat & 3];
    }

    /** Whether a drum keeps playing the groove under a fill. */
    bool fillKeeps (FillStyle style, int lane, const std::array<bool, numLanes>& customLanes) noexcept
    {
        const auto l = (Lane) lane;
        const bool hats = l == Lane::closedHat || l == Lane::openHat;
        const bool backbeat = l == Lane::snare || l == Lane::clap;
        switch (style)
        {
            case FillStyle::dropOut:     return false;
            case FillStyle::hatLift:     return ! hats && ! backbeat;
            case FillStyle::kickBuild:   return l != Lane::kick;
            case FillStyle::rimShuffle:  return l == Lane::kick || hats || l == Lane::shaker || l == Lane::ride;
            case FillStyle::openHatRun:  return ! hats;
            case FillStyle::clapStutter: return ! backbeat;
            case FillStyle::percBreak:   return l == Lane::closedHat || l == Lane::shaker || l == Lane::tambourine;
            case FillStyle::custom:      return ! customLanes[(size_t) lane];
            default:                     return l == Lane::kick;   // the kick keeps the floor
        }
    }

    bool followsAccentGroove (int lane) noexcept
    {
        switch ((Lane) lane)
        {
            case Lane::closedHat: case Lane::openHat: case Lane::ride: case Lane::shaker: case Lane::tambourine:
            case Lane::conga: case Lane::cowbell: return true;
            default: return false;
        }
    }
}

DrumSettings::DrumSettings() noexcept
{
    for (int l = 0; l < numLanes; ++l)
    {
        notes[(size_t) l] = defaultNote (l);
        levels[(size_t) l] = 1.0f;
        laneSwing[(size_t) l] = 1.0f;
        user[(size_t) l].fill ('.');
        userVelocity[(size_t) l].fill (0);
        userChance[(size_t) l].fill (0);
        userRoll[(size_t) l].fill (0);
        customFill[(size_t) l].fill ('.');
    }
    grooveTemplate.level.fill (1.0f);
}

//==============================================================================
GrooveEngine::GrooveEngine()
{
    offTime.fill (-1.0);
    laneOf.fill (0);
    pending.reserve (512);
    scratch.reserve (128);
}

DrumPattern GrooveEngine::basePattern (const DrumSettings& s)
{
    const auto& presets = drumPatterns();

    const int length = patternLength (s);

    if (s.pattern == randomPatternIndex())
    {
        auto p = makeRandomPattern (s.riffSeed, s.density, s.randomGenre);
        fitPattern (p, length);
        if (length > randomBeatLength)   // four bars: the last one turns around
            p = makeVariation (p, s.riffSeed + 31u);
        return p;
    }

    if (s.pattern == userPatternIndex())
    {
        DrumPattern p;
        p.name = "User pattern";
        for (int l = 0; l < numLanes; ++l)
            p.lanes[(size_t) l].assign (s.user[(size_t) l].begin(), s.user[(size_t) l].end());
        fitPattern (p, length);
        return p;
    }

    // Every groove plays as two (or four) bars, a one-bar groove repeated: A-D are always a
    // quarter of it.
    auto p = presets[(size_t) std::clamp (s.pattern, 0, (int) presets.size() - 1)];
    fitPattern (p, length);
    return p;
}

DrumPattern GrooveEngine::resolvePattern (const DrumSettings& s)
{
    auto p = applySegmentOrder (basePattern (s), s.segmentOrder);
    if (s.sectionVariation != 0)
        p = makeSectionVariation (p, s.sectionVariation);
    return applySection (p, s.section);
}

int GrooveEngine::patternLength (const DrumSettings& s) noexcept
{
    return s.grooveBars >= 4 ? 64 : 32;
}

int GrooveEngine::loopLengthInSteps (const DrumSettings& s) noexcept
{
    int steps = patternLength (s);
    if (s.variationEvery > 0)
        steps = lcm (steps, s.variationEvery * 16);
    if (s.fillEveryBars > 0)
        steps = lcm (steps, s.fillEveryBars * 16);
    return steps;
}

FillStyle GrooveEngine::effectiveFillStyle (FillStyle style, long long bar) noexcept
{
    if (style != FillStyle::mix)
        return style;

    // Mix: a different fill now and then, always the same one on the same bar.
    static constexpr FillStyle choices[] { FillStyle::snareRoll, FillStyle::tomRun, FillStyle::clapBuild, FillStyle::hatLift,
                                           FillStyle::snareRoll, FillStyle::congaRun, FillStyle::kickBuild, FillStyle::rimShuffle };
    return choices[(size_t) (noise (bar, 4242) * (double) std::size (choices)) % std::size (choices)];
}

const DrumPattern& GrooveEngine::patternFor (const DrumSettings& s)
{
    const bool usesSeed = s.pattern == randomPatternIndex();
    const bool usesUser = s.pattern == userPatternIndex();

    if (s.pattern != cachedIndex || s.section != cachedSection || s.segmentOrder != cachedOrder
        || s.sectionVariation != cachedSectionVariation || s.grooveBars != cachedBars
        || (usesSeed && (s.riffSeed != cachedSeed || s.density != cachedDensity || s.randomGenre != cachedGenre))
        || (usesUser && s.user != cachedUser))
    {
        cachedPattern = resolvePattern (s);
        cachedVariation = makeVariation (cachedPattern, (std::uint32_t) std::max (0, s.pattern) * 7919u + s.riffSeed);
        cachedIndex = s.pattern;
        cachedSection = s.section;
        cachedSeed = s.riffSeed;
        cachedDensity = s.density;
        cachedGenre = s.randomGenre;
        cachedUser = s.user;
        cachedOrder = s.segmentOrder;
        cachedSectionVariation = s.sectionVariation;
        cachedBars = s.grooveBars;
    }
    return cachedPattern;
}

//==============================================================================
void GrooveEngine::collectStep (long long step, const DrumSettings& s, const DrumPattern& normal, const DrumPattern& variation)
{
    const long long bar = floorDiv (step, 16);
    const int stepInBar = mod (step, 16);

    // Every N bars the last bar plays a variation of the groove (the turnaround).
    const bool varied = s.variationEvery > 0 && mod (bar, s.variationEvery) == s.variationEvery - 1
                     && variation.length() == normal.length();
    const DrumPattern& pattern = varied ? variation : normal;
    const int len = std::max (1, pattern.length());
    const int pos = mod (step, len);

    const auto& groove = s.grooveTemplate;
    const double swingDelay = groove.active ? (double) groove.timing[(size_t) stepInBar] * stepLength
                            : (step % 2 != 0) ? std::clamp (s.swing, 0.0f, 1.5f) * stepLength / 3.0 : 0.0;
    const double human = std::clamp (s.humanize, 0.0f, 1.0f);
    const int seedSalt = (int) (s.humanizeSeed % 100003u) * 7919;

    // Fills: the end of every N bars (or a fill asked for on one bar).
    auto isFillBar = [&s] (long long b)
    {
        if (s.forcedFillBar >= 0)
            return b == s.forcedFillBar && s.forcedFillStyle >= 0;
        return s.fillEveryBars > 0 && sectionAllowsFills (s.section) && mod (b, s.fillEveryBars) == s.fillEveryBars - 1;
    };
    const bool fillBar = isFillBar (bar);
    const FillStyle style = (s.forcedFillBar >= 0 && bar == s.forcedFillBar && s.forcedFillStyle >= 0)
                                ? effectiveFillStyle ((FillStyle) std::clamp (s.forcedFillStyle, 0, numFillStyles - 1), bar)
                                : effectiveFillStyle (s.fillStyle, bar);
    // The custom fill takes over from its first written step.
    int fillStart = fillStartStep (style);
    std::array<bool, numLanes> customLanes {};
    if (style == FillStyle::custom)
    {
        fillStart = 16;
        for (int l = 0; l < numLanes; ++l)
            for (int i = 0; i < 16; ++i)
                if (isHit (s.customFill[(size_t) l][(size_t) i]))
                {
                    customLanes[(size_t) l] = true;
                    fillStart = std::min (fillStart, i);
                }
    }
    const bool inFill = fillBar && stepInBar >= fillStart;

    // A velocity set by hand on this step of the user pattern (stored per step as written,
    // before the A-D order). It stays on the variation bar and under a fill, except on the hits
    // they change.
    // The same goes for the chance and the roll set on a step.
    auto userIndex = [&] (int lane) -> int
    {
        if (s.pattern != userPatternIndex() || len != patternLength (s))
            return -1;
        if (varied && variation.lanes[(size_t) lane][(size_t) pos] != normal.lanes[(size_t) lane][(size_t) pos])
            return -1;
        const int segment = len / numSegments;
        return std::clamp (s.segmentOrder[(size_t) (pos / segment)], 0, numSegments - 1) * segment + pos % segment;
    };
    auto handVelocity = [&] (int lane) -> int
    {
        const int index = userIndex (lane);
        return index < 0 ? 0 : s.userVelocity[(size_t) lane][(size_t) index];
    };

    auto velocityFor = [&] (int lane, double level, int salt)
    {
        double v = s.velocity * level * (double) s.levels[(size_t) lane] * sectionVelocityScale (s.section);
        if (s.section == Section::buildup)   // crescendo through the build-up
            v *= 0.65 + 0.45 * (double) pos / (double) std::max (1, len - 1);
        v += human * 12.0 * (noise (step, seedSalt + salt + 7) - 0.5) * 2.0;
        return std::clamp ((int) std::lround (v), 1, 127);
    };

    auto timeFor = [&] (int lane, int salt)
    {
        // The kick is always exactly on the grid: no swing, no push or pull, no humanize.
        if (lane == (int) Lane::kick)
            return (double) step * stepLength;
        // Each drum takes its share of the swing and sits ahead of or behind the beat; humanize
        // only ever delays a hit a little (a drummer sits on or behind the grid).
        const double shift = std::clamp ((double) s.laneShift[(size_t) lane], -40.0, 40.0) / 1000.0 * std::clamp (s.tempo, 20.0, 400.0) / 60.0;
        const double t = (double) step * stepLength + swingDelay * std::clamp ((double) s.laneSwing[(size_t) lane], 0.0, 2.0) + shift
                       + human * laneLooseness (lane) * 0.025 * noise (step, seedSalt + salt);
        return step >= 0 ? std::max (t, 0.0) : t;
    };

    auto add = [&] (int lane, char symbol, int note, double levelScale = 1.0, bool fromFill = false, int roll = 1)
    {
        if (s.muted[(size_t) lane] || ! isHit (symbol))
            return;
        const int salt = lane * 13 + 1;
        if (! fromFill)
            if (const int index = userIndex (lane); index >= 0)
            {
                // A step that only plays now and then (the same way every time it comes round).
                const int chance = s.userChance[(size_t) lane][(size_t) index];
                if (chance > 0 && chance < 100 && noise (step, seedSalt + salt + 31) * 100.0 >= chance)
                    return;
                roll = std::max (roll, (int) s.userRoll[(size_t) lane][(size_t) index]);
            }
        const double t = timeFor (lane, salt);
        double level = symbolLevel (symbol) * levelScale;
        if (groove.active && lane != (int) Lane::kick)
            level *= std::clamp ((double) groove.level[(size_t) stepInBar], 0.2, 1.6);
        if (const int set = fromFill ? 0 : handVelocity (lane); set > 0)
            level = set / 100.0;   // 100 = as the Velocity control
        else if (followsAccentGroove (lane) && symbol != 'g' && ! groove.active)
        {
            // The accent groove: written accents stay on top, the rest follows the beat.
            const double a = std::clamp ((double) s.accentGroove, 0.0, 1.0);
            const double shape = 1.0 - a * (1.0 - accentProfile (lane, stepInBar % 4));
            level *= symbol == 'X' ? std::max (shape, 0.9) : shape * (1.0 + 0.25 * a);
        }
        roll = lane == (int) Lane::kick ? 1 : std::clamp (roll, 1, 4);   // a roll would take the kick off the grid
        if (roll > 1)
        {
            // A roll: that many quick hits in the 16th, the first one the loudest.
            const double gap = stepLength / roll;
            for (int k = 0; k < roll; ++k)
                pending.push_back ({ t + gap * k, note, velocityFor (lane, level * (k == 0 ? 1.0 : 0.82), salt + k), gap * 0.9, lane });
            return;
        }
        pending.push_back ({ t, note, velocityFor (lane, level, salt), noteLength (lane), lane });
        if (symbol == 'f' && lane != (int) Lane::kick)   // the flam's grace note, just before (never on the kick)
            pending.push_back ({ t - 0.035, note, velocityFor (lane, 0.4 * levelScale, salt + 3), 0.03, lane });
    };

    // ---- The groove as written (what a fill leaves of it) ----
    for (int lane = 0; lane < numLanes; ++lane)
    {
        const char symbol = pattern.lanes[(size_t) lane][(size_t) pos];
        if (inFill && ! fillKeeps (style, lane, customLanes))
            continue;
        add (lane, symbol, s.notes[(size_t) lane]);
    }

    // ---- The fill ----
    if (inFill)
    {
        const int start = fillStart;
        const double ramp = 0.62 + 0.45 * (double) (stepInBar - start) / (double) std::max (1, 15 - start);
        const int snare = (int) Lane::snare, clap = (int) Lane::clap, tom = (int) Lane::tom, conga = (int) Lane::conga;

        switch (style)
        {
            case FillStyle::snareRoll:
            case FillStyle::clapBuild:
            {
                const int lane = style == FillStyle::snareRoll ? snare : clap;
                if (stepInBar >= 12 || stepInBar % 2 == 0)
                    add (lane, stepInBar >= 14 ? 'X' : 'x', s.notes[(size_t) lane], ramp, true);
                break;
            }
            case FillStyle::tomRun:
            {
                static constexpr int down[] { 5, 3, 0, -2 };   // high tom to floor tom
                add (tom, stepInBar == 12 ? 'X' : 'x', s.notes[(size_t) tom] + down[std::clamp (stepInBar - 12, 0, 3)], ramp, true);
                break;
            }
            case FillStyle::hatLift:
                if (stepInBar == 12 || stepInBar == 14)
                    add ((int) Lane::openHat, 'X', s.notes[(size_t) Lane::openHat], 1.0, true);
                if (stepInBar == 15)
                    add (snare, 'g', s.notes[(size_t) snare], 1.0, true);
                break;
            case FillStyle::dropOut:
                if (stepInBar == 15)
                    add (snare, 'X', s.notes[(size_t) snare], 1.0, true);
                break;
            case FillStyle::congaRun:
                add (conga, stepInBar == 12 || stepInBar == 15 ? 'X' : 'x', s.notes[(size_t) conga] + (stepInBar >= 14 ? 1 : 0), ramp, true);
                break;
            case FillStyle::kickBuild:   // 16th kicks into the next bar (on the grid)
                if (stepInBar >= 12)
                    add ((int) Lane::kick, stepInBar == 15 ? 'X' : 'x', s.notes[(size_t) Lane::kick], ramp, true);
                break;
            case FillStyle::rimShuffle:
            {
                static constexpr char line[] = "x.xxx.xX";
                add ((int) Lane::rim, line[stepInBar - 8], s.notes[(size_t) Lane::rim], ramp, true);
                break;
            }
            case FillStyle::openHatRun:
                if (stepInBar >= 12 || stepInBar % 2 == 0)
                    add ((int) Lane::openHat, stepInBar >= 14 ? 'X' : 'x', s.notes[(size_t) Lane::openHat], ramp, true);
                break;
            case FillStyle::clapStutter:
                if (stepInBar != 13)
                    add (clap, stepInBar == 15 ? 'X' : 'x', s.notes[(size_t) clap], ramp, true, stepInBar >= 14 ? 2 : 1);
                break;
            case FillStyle::percBreak:
            {
                static constexpr char congas[] = "x.x.xx.xx.xxxxxX";
                add (conga, congas[stepInBar], s.notes[(size_t) conga] + (stepInBar % 3 == 1 ? 1 : 0), ramp, true);
                if (stepInBar == 8 || stepInBar == 11 || stepInBar == 14)
                    add ((int) Lane::cowbell, 'x', s.notes[(size_t) Lane::cowbell], 1.0, true);
                break;
            }
            case FillStyle::custom:
                for (int l = 0; l < numLanes; ++l)
                    add (l, s.customFill[(size_t) l][(size_t) stepInBar], s.notes[(size_t) l], 1.0, true);
                break;
            case FillStyle::mix:
            default:
                break;
        }
    }

    // ---- Crash: after a fill, and every 8 bars of the drop ----
    if (stepInBar == 0 && s.crashAfterFill && pattern.lanes[(size_t) Lane::crash][(size_t) pos] == '.')
    {
        const bool afterFill = isFillBar (bar - 1);
        const bool dropPhrase = s.section == Section::drop && mod (bar - s.sectionStartBar, 8) == 0;
        if (afterFill || dropPhrase)
            add ((int) Lane::crash, 'X', s.notes[(size_t) Lane::crash], 1.0, true);
    }
}

//==============================================================================
void GrooveEngine::flushOffsUpTo (double ppq, bool inclusive, std::vector<NoteEvent>& out)
{
    scratch.clear();
    for (int n = 0; n < 128; ++n)
    {
        const double off = offTime[(size_t) n];
        if (off >= 0.0 && (off < ppq || (inclusive && off <= ppq)))
            scratch.push_back (n);
    }

    std::sort (scratch.begin(), scratch.end(),
               [this] (int a, int b) { return offTime[(size_t) a] < offTime[(size_t) b]; });

    for (int n : scratch)
    {
        out.push_back ({ offTime[(size_t) n], n, 0, laneOf[(size_t) n] });
        offTime[(size_t) n] = -1.0;
    }
}

void GrooveEngine::render (double startPpq, double endPpq, const DrumSettings& s, std::vector<NoteEvent>& out)
{
    if (endPpq <= startPpq)
        return;

    const auto& pattern = patternFor (s);
    pending.clear();

    // Swing, a template, a drum pulled behind and humanize delay a hit (by < 3 steps), so look back
    // a little; a drum pushed ahead and a flam's grace note come before their step, so look ahead too.
    const auto firstStep = (long long) std::floor (startPpq / stepLength) - 3;
    const auto lastStep  = (long long) std::floor (endPpq / stepLength) + 2;
    for (auto step = firstStep; step <= lastStep; ++step)
        collectStep (step, s, pattern, cachedVariation);

    pending.erase (std::remove_if (pending.begin(), pending.end(),
                                   [&] (const PendingOn& p) { return p.ppq < startPpq || p.ppq >= endPpq || p.note < 0 || p.note > 127; }),
                   pending.end());
    std::stable_sort (pending.begin(), pending.end(), [] (const PendingOn& a, const PendingOn& b) { return a.ppq < b.ppq; });

    for (const auto& p : pending)
    {
        flushOffsUpTo (p.ppq, true, out);

        auto& off = offTime[(size_t) p.note];
        if (off >= 0.0)
            out.push_back ({ p.ppq, p.note, 0, laneOf[(size_t) p.note] });   // retrigger

        out.push_back ({ p.ppq, p.note, p.velocity, p.lane });
        off = p.ppq + p.duration;
        laneOf[(size_t) p.note] = p.lane;
    }

    flushOffsUpTo (endPpq, false, out);
}

void GrooveEngine::allNotesOff (double ppq, std::vector<NoteEvent>& out)
{
    for (int n = 0; n < 128; ++n)
        if (offTime[(size_t) n] >= 0.0)
        {
            out.push_back ({ ppq, n, 0, laneOf[(size_t) n] });
            offTime[(size_t) n] = -1.0;
        }
}

bool GrooveEngine::anyNoteSounding() const noexcept
{
    return std::any_of (offTime.begin(), offTime.end(), [] (double t) { return t >= 0.0; });
}

//==============================================================================
std::vector<NoteEvent> GrooveEngine::renderClip (const DrumSettings& s, double lengthInBeats)
{
    GrooveEngine engine;
    std::vector<NoteEvent> events;
    engine.render (0.0, lengthInBeats, s, events);
    engine.allNotesOff (lengthInBeats, events);
    return events;
}

std::vector<NoteEvent> GrooveEngine::renderArrangement (const DrumSettings& s, const std::vector<ArrangementPart>& parts,
                                                        std::vector<Marker>* markers)
{
    GrooveEngine engine;
    std::vector<NoteEvent> events;
    long long bar = 0;

    for (const auto& part : parts)
    {
        if (part.bars <= 0)
            continue;

        auto settings = s;
        settings.section = part.section;
        settings.sectionStartBar = bar;
        settings.sectionVariation = part.variation;
        settings.forcedFillBar = -1;
        if (part.endFill != -1)
        {
            // Its own fill on its last bar (or none at all).
            settings.forcedFillBar = bar + part.bars - 1;
            settings.forcedFillStyle = part.endFill;
        }

        const double start = (double) bar * 4.0, end = (double) (bar + part.bars) * 4.0;
        if (markers != nullptr)
            markers->push_back ({ start, sectionName ((int) part.section) });

        engine.render (start, end, settings, events);
        engine.allNotesOff (end, events);
        bar += part.bars;
    }
    return events;
}

} // namespace fgd
