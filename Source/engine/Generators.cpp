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
    // A beat is built by roles, as a producer would: the kick, the backbeat, ONE main hat line,
    // maybe a second, softer hat line, ONE shaker or tambourine, at most ONE percussion line and
    // a few ghost notes; every genre has its own allowed choices. Bar 2 repeats bar 1, with at
    // most one small answer at its end.
    Random r (seed);
    density = std::clamp (density, 0.0f, 1.0f);
    genre = std::clamp (genre, 0, (int) grooveCategories().size() - 1);

    DrumPattern p;
    p.name = "Random beat";
    p.category = grooveCategories()[(size_t) genre];
    for (auto& line : p.lanes)
        line.assign ((size_t) userPatternLength, '.');

    auto setBar = [&p] (Lane lane, const char* cell)   // one bar, written into both bars
    {
        const auto c = compact (cell);
        for (int i = 0; i < 16 && i < (int) c.size(); ++i)
            if (c[(size_t) i] != '.')
                p[lane][(size_t) i] = p[lane][(size_t) (i + 16)] = c[(size_t) i];
    };
    auto pickCell = [&r] (std::initializer_list<const char*> cells) { return *(cells.begin() + r.nextInt ((int) cells.size())); };

    // ---- Kick: on the grid, as the genre wants it ----
    Kick kick = Kick::fourFloor;
    if (genre == garage)        kick = r.chance (0.6f) ? Kick::twoStep : Kick::fourFloor;
    else if (genre == neoSoul)  kick = r.chance (0.7f) ? Kick::broken : Kick::halfTime;
    else if (genre == chill)    kick = r.chance (0.6f) ? Kick::fourFloor : Kick::halfTime;

    switch (kick)
    {
        case Kick::fourFloor: setBar (Lane::kick, genre == deep || genre == chill ? "x... x... x... x..." : "X... X... X... X..."); break;
        case Kick::twoStep:   setBar (Lane::kick, pickCell ({ "X... .... ..x. ....", "X... .... ..X. ...x", "X... ...x ..x. ...." })); break;
        case Kick::broken:    setBar (Lane::kick, pickCell ({ "X... ..x. .x.. ....", "X... .... ..x. .x..", "X..x .... ..x. ...." })); break;
        case Kick::halfTime:  setBar (Lane::kick, pickCell ({ "X... .... ..x. ....", "X... .... .... ...." })); break;
    }

    // ---- Backbeat: on 2 and 4 (on 3 in half time) ----
    const char* backbeat = kick == Kick::halfTime ? ".... .... X... ...." : ".... X... .... X...";
    const char* softBackbeat = kick == Kick::halfTime ? ".... .... x... ...." : ".... x... .... x...";
    switch (genre)
    {
        case garage: case funky: case neoSoul:
            setBar (Lane::snare, backbeat); break;
        case chill:
            setBar (r.chance (0.5f) ? Lane::rim : Lane::snare, softBackbeat); break;
        case deep:
            setBar (r.chance (0.4f) ? Lane::rim : Lane::clap, softBackbeat); break;
        case gospel: case nineties: case dance:
            setBar (Lane::clap, backbeat);
            if (r.chance (genre == dance ? 0.7f : 0.5f)) setBar (Lane::snare, softBackbeat);
            break;
        default:
            setBar (Lane::clap, backbeat); break;
    }

    // ---- Main hat line (the accent groove gives it its life) ----
    enum Hat { openOff, closedOff, closed8, closed16, closedShuffle };
    Hat hat = openOff;
    switch (genre)
    {
        case pianoHouse: case nineties: case dance: case gospel: case jackinPiano:
                        hat = r.chance (0.75f) ? openOff : closedOff; break;
        case soulful:   hat = r.chance (0.6f) ? openOff : closed8; break;
        case deep:      hat = r.chance (0.6f) ? closedOff : openOff; break;
        case jackin:    hat = r.chance (0.6f) ? closed16 : openOff; break;
        case garage:    hat = r.chance (0.6f) ? closedShuffle : closed8; break;
        case afro:      hat = r.chance (0.5f) ? closedOff : openOff; break;
        case funky:     hat = r.chance (0.5f) ? openOff : closed8; break;
        case neoSoul:   hat = closed8; break;
        case chill:     hat = r.chance (0.5f) ? closed8 : closedOff; break;
        default: break;
    }
    const bool softHats = genre == deep || genre == chill;
    switch (hat)
    {
        case openOff:       setBar (Lane::openHat, softHats ? "..x. ..x. ..x. ..x." : "..X. ..X. ..X. ..X."); break;
        case closedOff:     setBar (Lane::closedHat, "..x. ..x. ..x. ..x."); break;
        case closed8:       setBar (Lane::closedHat, "x.x. x.x. x.x. x.x."); break;
        case closed16:      setBar (Lane::closedHat, "xxxx xxxx xxxx xxxx"); break;
        case closedShuffle: setBar (Lane::closedHat, "x.xx x.xx x.xx x.xx"); break;
    }
    // A second, softer hat line under an open offbeat hat: the 16ths around it.
    if (hat == openOff && density > 0.5f && r.chance (0.6f))
        setBar (Lane::closedHat, pickCell ({ "x... x... x... x...", "xg.g xg.g xg.g xg.g", ".g.g .g.g .g.g .g.g" }));
    // ...or an open hat lifting the end of each bar over closed hats.
    else if (hat != openOff && hat != closed16 && density > 0.4f && r.chance (0.5f))
        p[Lane::openHat][14] = p[Lane::openHat][30] = 'x', p[Lane::closedHat][14] = p[Lane::closedHat][30] = '.';

    // ---- One shaker or tambourine ----
    const bool shakerGenre = genre == soulful || genre == pianoHouse || genre == afro || genre == chill || genre == deep || genre == jackinPiano;
    const bool tambGenre = genre == gospel || genre == funky || genre == nineties;
    if (shakerGenre && density > 0.3f && r.chance (0.5f + density * 0.4f))
        setBar (Lane::shaker, density > 0.5f ? "xxxx xxxx xxxx xxxx" : "x.x. x.x. x.x. x.x.");
    else if (tambGenre && density > 0.3f && r.chance (0.6f))
        setBar (Lane::tambourine, pickCell ({ ".... x... .... x...", "x.x. x.x. x.x. x.x.", "..x. ..x. ..x. ..x." }));

    // ---- At most one percussion line ----
    if (genre == afro)
        setBar (Lane::rim, pickCell ({ "x..x ..x. ..x. x...", "x..x ..x. x..x ..x." }));   // the clave is the afro groove
    const float percChance = genre == afro ? density * 0.8f : density > 0.55f ? 0.6f : 0.0f;
    if (r.chance (percChance))
        switch (genre)
        {
            case soulful: case afro: case chill: setBar (Lane::conga, pickCell ({ "..x. .x.. x.x. ..x.", ".... ..x. .... x.x.", "..x. .... ..x. .x.." })); break;
            case jackin: case jackinPiano:       setBar (Lane::cowbell, pickCell ({ "..x. .x.. ..x. ....", "x..x ..x. ..x. x..." })); break;
            case deep: case garage:              setBar (Lane::rim, pickCell ({ "...x ..x. ...x ....", "..x. ...x .x.. ..x." })); break;
            case funky:                          setBar (Lane::cowbell, ".... .... ..x. ...."); break;
            case gospel:                         setBar (Lane::tambourine, "..x. ..x. ..x. ..x."); break;
            default: break;
        }

    // ---- Ghost notes (only where the genre has them), one per bar ----
    if ((genre == gospel || genre == funky || genre == neoSoul) && density > 0.45f)
    {
        const int at = r.pick ({ 7, 10, 15 });
        p[Lane::snare][(size_t) at] = p[Lane::snare][(size_t) (at + 16)] = 'g';
    }

    // ---- Bar 2 answers at its very end ----
    if (density > 0.4f && r.chance (0.5f))
    {
        if (hat != openOff)                               p[Lane::openHat][31] = 'x';
        else if (p[Lane::conga][31] == '.' && genre == soulful) p[Lane::conga][31] = 'x';
        else if (genre == funky || genre == gospel)       p[Lane::snare][31] = 'g';
    }

    normalisePattern (p, userPatternLength);
    return p;
}

std::string makeRandomLane (Lane lane, std::uint32_t seed, float density, int genre)
{
    // The genre's own idea of this drum first, from a few beats...
    for (std::uint32_t attempt = 0; attempt < 12; ++attempt)
    {
        const auto beat = makeRandomPattern (seed * 31u + attempt * 7919u + (std::uint32_t) lane, density, genre);
        const auto& line = beat[lane];
        if (std::any_of (line.begin(), line.end(), isHit))
            return line;
    }

    // ...otherwise a light part that fits any house groove.
    Random r (seed + 99u);
    static const char* cells[numLanes][3] {
        { "X... X... X... X...", "X... ..x. X... ....", "X... X... X... X..x" },        // kick
        { ".... X... .... X...", ".... X..g .... X...", "...g X... ..g. X..." },        // snare
        { ".... X... .... X...", ".... X... .... X..x", ".... f... .... X..." },        // clap
        { "...x ..x. ...x ....", "..x. ...x .x.. ..x.", "x..x ..x. ..x. x..." },        // rim
        { "xg.g xg.g xg.g xg.g", "x.x. x.x. x.x. x.x.", "xx.x xx.x xx.x xx.x" },        // closed hat
        { "..x. ..x. ..x. ..x.", ".... ..x. .... ..x.", "..X. ..X. ..X. ..X." },        // open hat
        { "x.x. x.x. x.x. x.x.", "x.X. x.X. x.X. x.X.", "x... x... x... x..." },        // ride
        { "xgxg xgxg xgxg xgxg", "gxXx gxXx gxXx gxXx", "g.x. g.x. g.x. g.x." },        // shaker
        { ".... x... .... x...", "..x. X.x. ..x. X.x.", "g.X. g.X. g.X. g.X." },        // tambourine
        { "..x. .x.. x.x. ..x.", ".... ..x. .... x.x.", "x..x ..x. x..x ..x." },        // conga
        { "..x. ..x. ..x. ..x.", "x..x ..x. ..x. x...", ".... .... ..x. ...." },        // cowbell
        { ".... .... .... ..x.", "..x. .x.. ..x. x.x.", ".... .... .... XxXx" },        // tom
        { "X... .... .... ....", "X... .... .... ....", "X... .... .... ...." },        // crash
    };
    const int choice = std::clamp ((int) (density * 2.99f) + (r.chance (0.3f) ? (r.chance (0.5f) ? -1 : 1) : 0), 0, 2);
    auto line = compact (cells[(int) lane][choice]);
    line += lane == Lane::crash ? std::string (16, '.') : line;
    if (density > 0.5f && lane != Lane::kick && lane != Lane::crash)
        line[(size_t) (16 + r.pick ({ 11, 13, 14, 15 }))] = 'g';
    return line;
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

DrumPattern makeSectionVariation (const DrumPattern& source, std::uint32_t seed)
{
    Random r (seed * 747796405u + 2891336453u);
    DrumPattern p = source;
    const int len = p.length();
    auto hits = [] (const std::string& line) { return std::any_of (line.begin(), line.end(), isHit); };

    // The closed hats: a few 16th ghosts added or taken away.
    auto& hats = p[Lane::closedHat];
    if (hits (hats))
        for (int n = 2 + r.nextInt (3); n > 0; --n)
        {
            const int i = r.nextInt (len);
            if (p[Lane::openHat][(size_t) i] == '.' && i % 4 != 0)
                hats[(size_t) i] = hats[(size_t) i] == '.' ? 'g' : '.';
        }

    // A new answer on a percussion line that plays (or a light one where none does).
    static constexpr Lane percussion[] { Lane::rim, Lane::conga, Lane::tambourine, Lane::cowbell, Lane::shaker };
    bool moved = false;
    for (auto lane : percussion)
        if (hits (p[lane]) && ! moved && r.chance (0.7f))
        {
            auto& line = p[lane];
            for (int bar = 0; bar < len / 16; ++bar)
            {
                const int from = bar * 16 + r.nextInt (16), to = bar * 16 + r.nextInt (16);
                if (to % 4 != 0)
                    std::swap (line[(size_t) from], line[(size_t) to]);
            }
            moved = true;
        }
    if (! moved)
    {
        const auto lane = r.chance (0.5f) ? Lane::rim : Lane::shaker;
        for (int bar = 0; bar < len / 16; ++bar)
            p[lane][(size_t) (bar * 16 + r.pick ({ 3, 7, 11, 14 }))] = 'g';
    }

    // Ghost notes on the snare now and then.
    if (r.chance (0.5f))
        p[Lane::snare][(size_t) (r.nextInt (len / 16) * 16 + r.pick ({ 7, 10, 15 }))] = 'g';

    // And the turnaround of the last bar.
    return makeVariation (p, seed + 7);
}

DrumPattern applySegmentOrder (const DrumPattern& source, const std::array<int, numSegments>& order)
{
    const int len = source.length();
    if (len % numSegments != 0 || order == std::array<int, numSegments> { 0, 1, 2, 3 })
        return source;

    DrumPattern p = source;
    const int segment = len / numSegments;
    for (int k = 0; k < numSegments; ++k)
    {
        const int from = std::clamp (order[(size_t) k], 0, numSegments - 1);
        for (int l = 0; l < numLanes; ++l)
            for (int i = 0; i < segment; ++i)
                p.lanes[(size_t) l][(size_t) (k * segment + i)] = source.lanes[(size_t) l][(size_t) (from * segment + i)];
    }
    return p;
}

void copySegment (DrumPattern& into, const DrumPattern& from, int segment)
{
    const int len = into.length();
    if (from.length() != len || len % numSegments != 0)
        return;
    const int size = len / numSegments;
    segment = std::clamp (segment, 0, numSegments - 1);
    for (int l = 0; l < numLanes; ++l)
        for (int i = segment * size; i < (segment + 1) * size; ++i)
            into.lanes[(size_t) l][(size_t) i] = from.lanes[(size_t) l][(size_t) i];
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
