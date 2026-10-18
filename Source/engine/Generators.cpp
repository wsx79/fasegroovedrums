// Fase Groove Drums - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "Generators.h"

#include <algorithm>
#include <cmath>

namespace fgd
{

namespace
{
    /** Small, fast and repeatable (same seed = same beat on every computer). */
    struct Random
    {
        explicit Random (std::uint32_t seed) noexcept : state (seed * 0x9E3779B97F4A7C15ULL + 0x632BE59BD9B4E019ULL) {}

        std::uint32_t next() noexcept
        {
            state += 0x9E3779B97F4A7C15ULL;
            auto z = state;
            z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
            z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
            return (std::uint32_t) ((z ^ (z >> 31)) >> 32);
        }

        int nextInt (int n) noexcept           { return n > 0 ? (int) (next() % (std::uint32_t) n) : 0; }
        float nextFloat() noexcept             { return (float) (next() >> 8) * (1.0f / 16777216.0f); }
        bool chance (float probability) noexcept { return nextFloat() < probability; }

        template <typename T, size_t N>
        T pick (const T (&items)[N]) noexcept { return items[nextInt ((int) N)]; }

        std::uint64_t state;
    };

    std::string compact (const char* s)
    {
        std::string out;
        for (; *s != 0; ++s)
            if (*s != ' ')
                out += *s;
        return out;
    }

    /** A one-bar cell written twice (two bars). */
    std::string twoBars (const char* cell)
    {
        const auto c = compact (cell);
        return c + c;
    }

    void soften (std::string& line)
    {
        for (auto& c : line)
            if (c == 'X')
                c = 'x';
    }

    enum class Kick { fourFloor, twoStep, broken, halfTime };

    // The genres, in grooveCategories() order.
    enum Genre { pianoHouse = 0, soulful, deep, gospel, jackin, jackinPiano, garage, afro, funky, nineties, neoSoul, chill, dance };
}

//==============================================================================
const char* sectionName (int section) noexcept
{
    static const char* names[] { "Main", "Intro", "Break", "Build-up", "Drop", "Outro" };
    return section >= 0 && section < numSections ? names[section] : "";
}

double sectionVelocityScale (Section section) noexcept
{
    switch (section)
    {
        case Section::intro:     return 0.85;
        case Section::breakdown: return 0.75;
        case Section::buildup:   return 0.95;
        case Section::drop:      return 1.08;
        case Section::outro:     return 0.85;
        case Section::main:
        default:                 return 1.0;
    }
}

bool sectionAllowsFills (Section section) noexcept
{
    return section == Section::main || section == Section::drop;
}

DrumPattern applySection (const DrumPattern& source, Section section)
{
    DrumPattern p = source;
    const int len = p.length();
    auto any = [&p] (Lane l) { return std::any_of (p[l].begin(), p[l].end(), isHit); };

    switch (section)
    {
        case Section::intro:
        case Section::outro:
        {
            // Only the kick and the hats (and the shaker), a little softer.
            for (int l = 0; l < numLanes; ++l)
            {
                const auto lane = (Lane) l;
                if (lane == Lane::kick || lane == Lane::closedHat || lane == Lane::openHat || lane == Lane::shaker)
                    soften (p.lanes[(size_t) l]);
                else
                    p.lanes[(size_t) l].assign ((size_t) len, '.');
            }

            if (! any (Lane::kick))
                for (int i = 0; i < len; i += 4)
                    p[Lane::kick][(size_t) i] = 'x';
            break;
        }

        case Section::breakdown:
        {
            // No kick, no backbeat: the hats and the percussion carry it, softly.
            for (auto lane : { Lane::kick, Lane::snare, Lane::clap, Lane::tom, Lane::crash })
                p[lane].assign ((size_t) len, '.');
            for (auto& line : p.lanes)
                soften (line);

            if (std::none_of (p.lanes.begin(), p.lanes.end(), [] (const std::string& l) { return std::any_of (l.begin(), l.end(), isHit); }))
                for (int i = 0; i < len; i += 2)
                    p[Lane::shaker][(size_t) i] = 'x';
            break;
        }

        case Section::buildup:
        {
            // Kick on every beat, a snare roll that doubles up (quarters, 8ths, 16ths), 8th hats.
            for (auto& line : p.lanes)
                line.assign ((size_t) len, '.');

            for (int i = 0; i < len; ++i)
            {
                const double at = (double) i / (double) len;
                const int every = at < 0.5 && len > 16 ? 4 : at < 0.75 ? 2 : 1;
                if (i % every == 0)
                    p[Lane::snare][(size_t) i] = i % 4 == 0 ? 'X' : 'x';
                if (i % 4 == 0)
                    p[Lane::kick][(size_t) i] = 'X';
                if (i % 2 == 0)
                    p[Lane::closedHat][(size_t) i] = 'x';
            }
            break;
        }

        case Section::drop:   // the full groove (the engine adds a crash every 8 bars)
        case Section::main:
        default:
            break;
    }

    if (section != Section::main)
        p.name = source.name + " / " + sectionName ((int) section);

    return p;
}

//==============================================================================
DrumPattern makeRandomPattern (std::uint32_t seed, float density, int genre)
{
    Random r (seed);
    density = std::clamp (density, 0.0f, 1.0f);
    genre = std::clamp (genre, 0, (int) grooveCategories().size() - 1);

    DrumPattern p;
    p.name = "Random beat";
    p.category = grooveCategories()[(size_t) genre];
    for (auto& line : p.lanes)
        line.assign ((size_t) userPatternLength, '.');

    auto set = [&p] (Lane lane, int step, char symbol)
    {
        auto& c = p[lane][(size_t) (step % userPatternLength)];
        if (c == '.' || symbol == 'X')
            c = symbol;
    };

    // ---- Kick ----
    Kick kick = Kick::fourFloor;
    if (genre == garage)        kick = r.chance (0.6f) ? Kick::twoStep : Kick::fourFloor;
    else if (genre == neoSoul)  kick = r.chance (0.75f) ? Kick::broken : Kick::halfTime;
    else if (genre == chill)    kick = r.chance (0.5f) ? Kick::fourFloor : Kick::halfTime;
    else if (genre == funky)    kick = r.chance (0.3f) ? Kick::broken : Kick::fourFloor;

    const char kickHit = genre == deep || genre == chill ? 'x' : 'X';
    for (int bar = 0; bar < 2; ++bar)
    {
        const int o = bar * 16;
        switch (kick)
        {
            case Kick::fourFloor:
                for (int b = 0; b < 4; ++b) set (Lane::kick, o + b * 4, kickHit);
                break;
            case Kick::twoStep:
                set (Lane::kick, o, 'X');
                set (Lane::kick, o + r.pick ({ 10, 10, 11, 7 }), 'x');
                if (bar == 1 && r.chance (0.5f)) set (Lane::kick, o + 13, 'x');
                break;
            case Kick::broken:
                set (Lane::kick, o, 'X');
                for (int n = 1 + r.nextInt (2); n > 0; --n)
                    set (Lane::kick, o + r.pick ({ 6, 7, 10, 11, 14 }), 'x');
                break;
            case Kick::halfTime:
                set (Lane::kick, o, 'X');
                if (r.chance (0.6f)) set (Lane::kick, o + 10, 'x');
                break;
        }
    }
    if (kick == Kick::fourFloor && r.chance (density * 0.5f))
        set (Lane::kick, r.pick ({ 31, 31, 30, 27 }), r.chance (0.5f) ? 'x' : 'g');

    // ---- Backbeat: clap or snare (or both) on 2 and 4, on 3 in half time ----
    const bool snareGenre = genre == funky || genre == neoSoul || genre == garage || genre == chill;
    const Lane backbeat = snareGenre ? Lane::snare : (genre == deep && r.chance (0.4f)) ? Lane::rim : Lane::clap;
    for (int bar = 0; bar < 2; ++bar)
    {
        if (kick == Kick::halfTime)
            set (backbeat, bar * 16 + 8, 'X');
        else
        {
            set (backbeat, bar * 16 + 4, genre == chill || genre == deep ? 'x' : 'X');
            set (backbeat, bar * 16 + 12, genre == chill || genre == deep ? 'x' : 'X');
        }
    }
    if (backbeat == Lane::clap && (genre == dance || genre == nineties || genre == gospel || r.chance (0.3f)))
        for (int i = 0; i < userPatternLength; ++i)
            if (p[Lane::clap][(size_t) i] != '.' && genre != chill)
                set (Lane::snare, i, 'x');

    // Ghost notes on the snare: more with more density (gospel, funk and neo soul love them).
    const bool ghostGenre = genre == gospel || genre == funky || genre == neoSoul || genre == jackinPiano || genre == soulful;
    if (ghostGenre && density > 0.3f)
        for (int n = (int) std::lround (density * 4.0f); n > 0; --n)
            set (Lane::snare, r.nextInt (2) * 16 + r.pick ({ 3, 7, 10, 11, 14, 15 }), 'g');

    // ---- Hats ----
    const bool openOffbeats = genre != neoSoul && genre != chill && kick == Kick::fourFloor && r.chance (genre == deep ? 0.4f : 0.8f);
    if (openOffbeats)
        for (int i = 2; i < userPatternLength; i += 4)
            set (Lane::openHat, i, genre == deep ? 'x' : 'X');

    static const char* closedCells[] { "xg.g xg.g xg.g xg.g", "xx.x xx.x xx.x xx.x", "x... x... x... x...",
                                       "x.x. x.x. x.x. x.x.", "xxXx xxXx xxXx xxXx", "x.xg x.xg x.xg x.xg" };
    const bool closed = ! openOffbeats || r.chance (0.35f + density * 0.5f);
    if (closed)
    {
        int cell = openOffbeats ? r.pick ({ 0, 1, 2 }) : r.pick ({ 3, 4, 5 });
        if (density < 0.3f && ! openOffbeats) cell = 3;
        const auto line = twoBars (closedCells[cell]);
        for (int i = 0; i < userPatternLength; ++i)
            if (line[(size_t) i] != '.' && p[Lane::openHat][(size_t) i] == '.')
                set (Lane::closedHat, i, line[(size_t) i]);
    }

    // ---- Shaker ----
    if (r.chance (0.2f + density * 0.6f) && genre != jackin)
    {
        static const char* cells[] { "xgxg xgxg xgxg xgxg", "gxXx gxXx gxXx gxXx", "g.x. g.x. g.x. g.x.", "Xgxg xgxg Xgxg xgxg" };
        p[Lane::shaker] = twoBars (r.pick (cells));
    }

    // ---- Percussion by genre ----
    auto cellInto = [&] (Lane lane, std::initializer_list<const char*> cells)
    {
        const auto line = twoBars (*(cells.begin() + r.nextInt ((int) cells.size())));
        for (int i = 0; i < userPatternLength; ++i)
            if (line[(size_t) i] != '.')
                set (lane, i, line[(size_t) i]);
        // The second bar answers a little differently.
        if (density > 0.4f)
            set (lane, 16 + r.pick ({ 13, 14, 15, 11 }), 'x');
    };

    switch (genre)
    {
        case afro:
            cellInto (Lane::rim, { "x..x ..x. ..x. x...", "x..x ..x. x..x ..x." });
            if (r.chance (0.5f + density * 0.4f)) cellInto (Lane::conga, { ".... x.x. .x.. x.x.", "..x. .x.. x.x. ..x.", "x..x ..x. x..x ..x." });
            if (r.chance (density)) cellInto (Lane::tom, { "..x. .x.. ..x. x.x.", ".... .... .... ..x." });
            break;
        case soulful:
        case gospel:
            if (r.chance (0.6f + density * 0.3f)) cellInto (Lane::tambourine, { ".... x... .... x...", "g.X. g.X. g.X. g.X.", "..x. X.x. ..x. X.x." });
            if (r.chance (density * 0.7f)) cellInto (Lane::conga, { "..x. .x.. x.x. ..x.", ".... ..x. .... x.x." });
            break;
        case jackin:
        case jackinPiano:
            if (r.chance (0.5f)) cellInto (Lane::cowbell, { "x..x ..x. ..x. x...", "..x. .x.. ..x. ...." });
            if (r.chance (density)) cellInto (Lane::rim, { "...x ..x. .... ..x.", "...x .... .x.. ...." });
            break;
        case garage:
        case deep:
            cellInto (Lane::rim, { "...x .... .x.. ...x", "..x. ...x .x.. ..x.", ".x.. ..x. .x.. ...x" });
            break;
        case funky:
            if (r.chance (0.6f)) cellInto (Lane::tambourine, { "x.x. x.x. x.x. x.x.", "..x. x.x. ..x. x.x." });
            if (r.chance (density * 0.6f)) cellInto (Lane::cowbell, { ".... .... ..x. ....", "x.x. x.xx .x.x x.x." });
            break;
        case nineties:
            if (r.chance (0.5f)) cellInto (Lane::tambourine, { ".... x... .... x..." });
            else                 cellInto (Lane::ride, { "x.X. x.X. x.X. x.X." });
            break;
        case chill:
            cellInto (Lane::rim, { ".... x... .... x...", ".... x..g .... x..." });
            if (r.chance (density)) cellInto (Lane::conga, { ".... ..x. .... .x.x" });
            break;
        case neoSoul:
            if (r.chance (density * 0.6f)) cellInto (Lane::rim, { ".... .... x... ...." });
            break;
        case pianoHouse:
        case dance:
        default:
            if (r.chance (density * 0.6f)) cellInto (Lane::tambourine, { ".... x... .... x..." });
            if (r.chance (0.6f)) set (Lane::crash, 0, 'X');
            break;
    }

    normalisePattern (p, userPatternLength);
    return p;
}

DrumPattern makeVariation (const DrumPattern& source, std::uint32_t seed)
{
    Random r (seed * 2654435761u + 17u);
    DrumPattern p = source;
    const int len = p.length();
    if (len < 16)
        return p;

    const int bar = len - 16;   // the last bar plays the turnaround
    auto at = [&p, bar] (Lane lane, int step) -> char& { return p[lane][(size_t) (bar + step)]; };
    const bool fourFloor = at (Lane::kick, 0) != '.' && at (Lane::kick, 4) != '.' && at (Lane::kick, 8) != '.' && at (Lane::kick, 12) != '.';

    const int moves = 1 + r.nextInt (2);
    for (int m = 0; m < moves; ++m)
    {
        switch (r.nextInt (5))
        {
            case 0:   // open hat lift into the next bar
                at (Lane::openHat, 14) = 'X';
                at (Lane::closedHat, 14) = '.';
                at (Lane::closedHat, 15) = '.';
                break;
            case 1:   // kick pick-up
                if (fourFloor) at (Lane::kick, 15) = 'x';
                else           at (Lane::kick, 14) = 'x';
                break;
            case 2:   // ghost / flam on the backbeat
                if (std::any_of (p[Lane::clap].begin(), p[Lane::clap].end(), isHit))
                    at (Lane::clap, 12) = 'f';
                at (Lane::snare, 15) = 'g';
                break;
            case 3:   // snare drag into the bar
                at (Lane::snare, 14) = 'g';
                at (Lane::snare, 15) = 'x';
                break;
            default:  // a tom at the very end
                at (Lane::tom, 15) = 'x';
                break;
        }
    }
    return p;
}

//==============================================================================
const char* fillStyleName (int style) noexcept
{
    static const char* names[] { "Snare roll", "Tom run", "Clap build", "Hat lift", "Drop out", "Conga run", "Mix" };
    return style >= 0 && style < numFillStyles ? names[style] : "";
}

int fillStartStep (FillStyle style) noexcept
{
    switch (style)
    {
        case FillStyle::snareRoll:
        case FillStyle::clapBuild: return 8;
        case FillStyle::tomRun:
        case FillStyle::hatLift:
        case FillStyle::dropOut:
        case FillStyle::congaRun:
        case FillStyle::mix:
        default:                   return 12;
    }
}

const std::vector<ArrangementPart>& defaultArrangement()
{
    static const std::vector<ArrangementPart> parts {
        { Section::intro, 8 }, { Section::main, 16 }, { Section::breakdown, 8 },
        { Section::buildup, 4 }, { Section::drop, 16 }, { Section::outro, 8 },
    };
    return parts;
}

} // namespace fgd
