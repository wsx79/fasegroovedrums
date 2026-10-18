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
            case Lane::kick:       return 0.25;
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
}

DrumSettings::DrumSettings() noexcept
{
    for (int l = 0; l < numLanes; ++l)
    {
        notes[(size_t) l] = defaultNote (l);
        levels[(size_t) l] = 1.0f;
        user[(size_t) l].fill ('.');
        userVelocity[(size_t) l].fill (0);
    }
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

    if (s.pattern == randomPatternIndex())
        return makeRandomPattern (s.riffSeed, s.density, s.randomGenre);

    if (s.pattern == userPatternIndex())
    {
        DrumPattern p;
        p.name = "User pattern";
        for (int l = 0; l < numLanes; ++l)
            p.lanes[(size_t) l].assign (s.user[(size_t) l].begin(), s.user[(size_t) l].end());
        normalisePattern (p, userPatternLength);
        return p;
    }

    // Every groove plays as two bars (a one-bar groove twice), so A-D are always half bars.
    auto p = presets[(size_t) std::clamp (s.pattern, 0, (int) presets.size() - 1)];
    normalisePattern (p, userPatternLength);
    return p;
}

DrumPattern GrooveEngine::resolvePattern (const DrumSettings& s)
{
    auto p = applySegmentOrder (basePattern (s), s.segmentOrder);
    if (s.sectionVariation != 0)
        p = makeSectionVariation (p, s.sectionVariation);
    return applySection (p, s.section);
}

int GrooveEngine::patternLength (const DrumSettings&) noexcept
{
    return userPatternLength;
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
    static constexpr FillStyle choices[] { FillStyle::snareRoll, FillStyle::tomRun, FillStyle::clapBuild,
                                           FillStyle::hatLift, FillStyle::snareRoll, FillStyle::congaRun };
    return choices[(size_t) (noise (bar, 4242) * (double) std::size (choices)) % std::size (choices)];
}

const DrumPattern& GrooveEngine::patternFor (const DrumSettings& s)
{
    const bool usesSeed = s.pattern == randomPatternIndex();
    const bool usesUser = s.pattern == userPatternIndex();

    if (s.pattern != cachedIndex || s.section != cachedSection || s.segmentOrder != cachedOrder
        || s.sectionVariation != cachedSectionVariation
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

    const double swingDelay = (step % 2 != 0) ? std::clamp (s.swing, 0.0f, 1.0f) * stepLength / 3.0 : 0.0;
    const double baseTime = (double) step * stepLength + swingDelay;
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
    const bool inFill = fillBar && stepInBar >= fillStartStep (style);

    // A velocity set by hand on this step of the user pattern (stored per step as written,
    // before the A-D order).
    auto handVelocity = [&] (int lane) -> int
    {
        if (s.pattern != userPatternIndex() || len != userPatternLength || inFill || varied)
            return 0;
        const int segment = userPatternLength / numSegments;
        const int index = std::clamp (s.segmentOrder[(size_t) (pos / segment)], 0, numSegments - 1) * segment + pos % segment;
        return s.userVelocity[(size_t) lane][(size_t) index];
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
        // Humanize only ever delays a hit a little (a drummer sits on or behind the grid).
        return baseTime + human * laneLooseness (lane) * 0.025 * noise (step, seedSalt + salt);
    };

    auto add = [&] (int lane, char symbol, int note, double levelScale = 1.0)
    {
        if (s.muted[(size_t) lane] || ! isHit (symbol))
            return;
        const int salt = lane * 13 + 1;
        const double t = timeFor (lane, salt);
        double level = symbolLevel (symbol) * levelScale;
        if (const int set = handVelocity (lane); set > 0 && levelScale == 1.0)
            level = set / 100.0;   // 100 = as the Velocity control
        pending.push_back ({ t, note, velocityFor (lane, level, salt), noteLength (lane), lane });
        if (symbol == 'f')   // the flam's grace note, just before
            pending.push_back ({ t - 0.035, note, velocityFor (lane, 0.4 * levelScale, salt + 3), 0.03, lane });
    };

    // ---- The groove as written (what a fill leaves of it) ----
    for (int lane = 0; lane < numLanes; ++lane)
    {
        const char symbol = pattern.lanes[(size_t) lane][(size_t) pos];
        if (inFill)
        {
            // The kick keeps the floor (unless the fill drops out); the hats lift keeps the percussion.
            const bool keep = (lane == (int) Lane::kick && style != FillStyle::dropOut)
                           || (style == FillStyle::hatLift && lane != (int) Lane::closedHat && lane != (int) Lane::openHat
                               && lane != (int) Lane::snare && lane != (int) Lane::clap);
            if (! keep)
                continue;
        }
        add (lane, symbol, s.notes[(size_t) lane]);
    }

    // ---- The fill ----
    if (inFill)
    {
        const int start = fillStartStep (style);
        const double ramp = 0.62 + 0.45 * (double) (stepInBar - start) / (double) std::max (1, 15 - start);
        const int snare = (int) Lane::snare, clap = (int) Lane::clap, tom = (int) Lane::tom, conga = (int) Lane::conga;

        switch (style)
        {
            case FillStyle::snareRoll:
            case FillStyle::clapBuild:
            {
                const int lane = style == FillStyle::snareRoll ? snare : clap;
                if (stepInBar >= 12 || stepInBar % 2 == 0)
                    add (lane, stepInBar >= 14 ? 'X' : 'x', s.notes[(size_t) lane], ramp);
                break;
            }
            case FillStyle::tomRun:
            {
                static constexpr int down[] { 5, 3, 0, -2 };   // high tom to floor tom
                add (tom, stepInBar == 12 ? 'X' : 'x', s.notes[(size_t) tom] + down[std::clamp (stepInBar - 12, 0, 3)], ramp);
                break;
            }
            case FillStyle::hatLift:
                if (stepInBar == 12 || stepInBar == 14)
                    add ((int) Lane::openHat, 'X', s.notes[(size_t) Lane::openHat]);
                if (stepInBar == 15)
                    add (snare, 'g', s.notes[(size_t) snare]);
                break;
            case FillStyle::dropOut:
                if (stepInBar == 15)
                    add (snare, 'X', s.notes[(size_t) snare]);
                break;
            case FillStyle::congaRun:
                add (conga, stepInBar == 12 || stepInBar == 15 ? 'X' : 'x', s.notes[(size_t) conga] + (stepInBar >= 14 ? 1 : 0), ramp);
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
            add ((int) Lane::crash, 'X', s.notes[(size_t) Lane::crash]);
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

    // Swing and humanize only ever delay a hit (by < 2 steps), so look back a little;
    // a flam's grace note comes before its step, so look one step ahead too.
    const auto firstStep = (long long) std::floor (startPpq / stepLength) - 2;
    const auto lastStep  = (long long) std::floor (endPpq / stepLength) + 1;
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
