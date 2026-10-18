// Fase Groove Drums - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "Drums.h"

#include <algorithm>

namespace fgd
{

const char* laneName (int lane) noexcept
{
    static const char* names[] { "Kick", "Snare", "Clap", "Rim", "Closed hat", "Open hat", "Ride",
                                 "Shaker", "Tambourine", "Conga", "Cowbell", "Tom", "Crash" };
    return lane >= 0 && lane < numLanes ? names[lane] : "";
}

const char* laneShortName (int lane) noexcept
{
    static const char* names[] { "KICK", "SNARE", "CLAP", "RIM", "C.HAT", "O.HAT", "RIDE",
                                 "SHAKER", "TAMB", "CONGA", "BELL", "TOM", "CRASH" };
    return lane >= 0 && lane < numLanes ? names[lane] : "";
}

int defaultNote (int lane) noexcept
{
    static const int notes[] { 36, 38, 39, 37, 42, 46, 51, 82, 54, 63, 56, 45, 49 };
    return lane >= 0 && lane < numLanes ? notes[lane] : 36;
}

int laneForGeneralMidiNote (int note) noexcept
{
    switch (note)
    {
        case 35: case 36:                     return (int) Lane::kick;
        case 38: case 40:                     return (int) Lane::snare;
        case 39:                              return (int) Lane::clap;
        case 37:                              return (int) Lane::rim;
        case 42: case 44:                     return (int) Lane::closedHat;
        case 46:                              return (int) Lane::openHat;
        case 51: case 53: case 59:            return (int) Lane::ride;
        case 69: case 70: case 82:            return (int) Lane::shaker;
        case 54:                              return (int) Lane::tambourine;
        case 60: case 61: case 62: case 63: case 64:
                                              return (int) Lane::conga;
        case 56:                              return (int) Lane::cowbell;
        case 41: case 43: case 45: case 47: case 48: case 50:
                                              return (int) Lane::tom;
        case 49: case 52: case 55: case 57:   return (int) Lane::crash;
        default:                              return -1;
    }
}

const char* generalMidiDrumName (int note) noexcept
{
    static const char* names[] {
        "Acoustic Bass Drum", "Bass Drum 1", "Side Stick", "Acoustic Snare", "Hand Clap", "Electric Snare",
        "Low Floor Tom", "Closed Hi-Hat", "High Floor Tom", "Pedal Hi-Hat", "Low Tom", "Open Hi-Hat",
        "Low-Mid Tom", "Hi-Mid Tom", "Crash Cymbal 1", "High Tom", "Ride Cymbal 1", "Chinese Cymbal",
        "Ride Bell", "Tambourine", "Splash Cymbal", "Cowbell", "Crash Cymbal 2", "Vibraslap",
        "Ride Cymbal 2", "Hi Bongo", "Low Bongo", "Mute Hi Conga", "Open Hi Conga", "Low Conga",
        "High Timbale", "Low Timbale", "High Agogo", "Low Agogo", "Cabasa", "Maracas",
        "Short Whistle", "Long Whistle", "Short Guiro", "Long Guiro", "Claves", "Hi Wood Block",
        "Low Wood Block", "Mute Cuica", "Open Cuica", "Mute Triangle", "Open Triangle", "Shaker" };
    return note >= 35 && note <= 82 ? names[note - 35] : "";
}

//==============================================================================
void normalisePattern (DrumPattern& p, int minimumLength)
{
    size_t length = (size_t) std::max (1, minimumLength);
    for (const auto& line : p.lanes)
        length = std::max (length, line.size());

    for (auto& line : p.lanes)
    {
        for (auto& c : line)
            if (! isHit (c))
                c = '.';

        if (line.empty())
            line.assign (length, '.');

        // A one-bar line in a two-bar groove plays twice.
        const std::string base = line;
        while (line.size() < length)
            line += base.substr (0, std::min (base.size(), length - line.size()));
    }
}

void fitPattern (DrumPattern& p, int length)
{
    length = std::max (1, length);
    for (auto& line : p.lanes)
    {
        for (auto& c : line)
            if (! isHit (c))
                c = '.';
        if (line.empty())
            line.assign ((size_t) length, '.');
        const std::string base = line;
        while ((int) line.size() < length)
            line += base.substr (0, std::min (base.size(), (size_t) length - line.size()));
        line.resize ((size_t) length);
    }
}

namespace
{
    std::string compact (const char* s)
    {
        std::string out;
        for (; *s != 0; ++s)
            if (*s != ' ')
                out += *s;
        return out;
    }

    struct Line { Lane lane; const char* steps; };

    DrumPattern groove (const char* name, const char* category, std::initializer_list<Line> lines)
    {
        DrumPattern p;
        p.name = name;
        p.category = category;
        for (const auto& l : lines)
            p[l.lane] = compact (l.steps);
        normalisePattern (p);
        return p;
    }

    constexpr auto K = Lane::kick, S = Lane::snare, C = Lane::clap, R = Lane::rim, CH = Lane::closedHat,
                   OH = Lane::openHat, RD = Lane::ride, SH = Lane::shaker, T = Lane::tambourine,
                   CG = Lane::conga, CB = Lane::cowbell, TM = Lane::tom, CR = Lane::crash;

    constexpr const char* fourFloor = "X... X... X... X...";
    constexpr const char* backbeat  = ".... X... .... X...";
    constexpr const char* offbeats  = "..X. ..X. ..X. ..X.";
    constexpr const char* crashBar1 = "X... .... .... .... .... .... .... ....";
}

const std::vector<DrumPattern>& drumPatterns()
{
    static const std::vector<DrumPattern> patterns {
        // ---- Piano House: four on the floor, claps on 2 & 4, open hats on the offbeats ----
        groove ("Piano House Classic", "Piano House",
                { { K, fourFloor }, { C, backbeat }, { OH, offbeats }, { CH, "xg.g xg.g xg.g xg.g" },
                  { SH, "gxXx gxXx gxXx gxXx" } }),
        groove ("Piano Anthem", "Piano House",
                { { K, fourFloor }, { C, backbeat }, { S, ".... x... .... x..." }, { RD, "..x. ..x. ..x. ..x." },
                  { T, ".... X... .... X..." }, { SH, "xgxg xgxg xgxg xgxg" }, { CR, crashBar1 } }),
        groove ("Piano Pumper", "Piano House",
                { { K, fourFloor }, { C, backbeat }, { OH, offbeats }, { CH, "x... x... x... x..." } }),
        groove ("Piano Bounce", "Piano House",
                { { K, "X... X... X... X... X... X... X... X..x" }, { C, ".... X..g .... X..." },
                  { OH, offbeats }, { SH, "xgxg xgxg xgxg xgxg" }, { R, "...x ..x. ...x ...." } }),

        // ---- Soulful: shakers, tambourines and congas around a warm four on the floor ----
        groove ("Soulful Sunday", "Soulful",
                { { K, fourFloor }, { C, backbeat }, { S, ".... x..g .... x.g." }, { OH, offbeats },
                  { SH, "gxXx gxXx gxXx gxXx" }, { T, ".... x... .... x..." } }),
        groove ("Soulful Congas", "Soulful",
                { { K, fourFloor }, { C, backbeat }, { OH, offbeats }, { CH, "xg.g xg.g xg.g xg.g" },
                  { CG, "..x. .x.. x.x. ..x. ..x. .x.. x.x. x.xg" } }),
        groove ("Shaker Groove", "Soulful",
                { { K, fourFloor }, { C, backbeat }, { SH, "Xgxg xgxg Xgxg xgxg" }, { R, "...x ..x. ...x .x.." },
                  { OH, offbeats } }),
        groove ("Soulful Tambourine", "Soulful",
                { { K, fourFloor }, { C, backbeat }, { S, ".... x... .... x..." }, { T, "..x. X.x. ..x. X.x." },
                  { RD, "x.g. x.g. x.g. x.g." } }),
        groove ("Vocal House Beat", "Soulful",
                { { K, fourFloor }, { C, ".... f... .... f..." }, { CH, "xg.g xg.g xg.g xg.g" }, { OH, offbeats },
                  { SH, "gxgx gxgx gxgx gxgx" } }),

        // ---- Deep: softer kick, rim shots and closed hats, lots of space ----
        groove ("Deep Rim Shot", "Deep",
                { { K, "x... x... x... x..." }, { C, ".... x... .... x..." }, { R, "..x. ...x .x.. ..x. ..x. ...x .x.. x..." },
                  { CH, "..x. ..x. ..x. ..x." }, { SH, ".gxg .gxg .gxg .gxg" } }),
        groove ("Deep Late Night", "Deep",
                { { K, "x... x... x... x..." }, { S, "...g .... ..g. ...." }, { C, ".... x... .... x..." },
                  { OH, "..x. ..x. ..x. ..x." }, { CG, ".... ..x. .... x.x." } }),
        groove ("Deep Dub Hats", "Deep",
                { { K, "x... x... x... x..." }, { R, ".... x... .... x..." }, { CH, "gg.g ggxg gg.g ggxg" },
                  { OH, "..x. .... ..x. ...." } }),
        groove ("Deep Minimal", "Deep",
                { { K, "x... x... x... x..." }, { R, "...x .... ..x. .... ...x ..x. .... .x.." },
                  { SH, "g.x. g.x. g.x. g.x." }, { C, ".... .... .... x..." } }),

        // ---- Gospel: tambourines, church claps and busy snare ghost notes ----
        groove ("Gospel Tambourine", "Gospel",
                { { K, fourFloor }, { C, backbeat }, { S, ".... X..g ..g. X.g." }, { T, "g.X. g.X. g.X. g.X." },
                  { CH, ".g.g .g.g .g.g .g.g" } }),
        groove ("Church Claps", "Gospel",
                { { K, fourFloor }, { C, "..x. X.x. ..x. X.x." }, { S, ".... x... .... x..." },
                  { T, ".... x... .... x..." }, { SH, "xgxg xgxg xgxg xgxg" } }),
        groove ("Gospel Shout", "Gospel",
                { { K, "X... X... X... X... X... X... X... X..x" }, { S, "..g. X.gg ..g. X.g." },
                  { T, "xgXg xgXg xgXg xgXg" }, { C, backbeat }, { CR, crashBar1 } }),
        groove ("Praise Break", "Gospel",
                { { K, "X..x X... X..x X..." }, { S, ".... X..g .g.. X.gg" }, { C, backbeat }, { OH, offbeats },
                  { T, "x.x. x.x. x.x. x.x." }, { TM, ".... .... .... .... .... .... .... XxXx" } }),

        // ---- Jackin': the Chicago bounce. A straight kick, claps on 2 and 4, open hats on the
        //      offbeats and SHUFFLED closed hats around them (the swing does the jacking) ----
        groove ("Jack Your Body", "Jackin'",
                { { K, fourFloor }, { C, backbeat }, { OH, offbeats }, { CH, "x..x x..x x..x x..x" } }),
        groove ("Chicago 909", "Jackin'",
                { { K, fourFloor }, { C, backbeat }, { S, ".... x... .... x..." }, { OH, offbeats },
                  { CH, "xg.x xg.x xg.x xg.x" }, { R, "...x .... .x.. ...x" } }),
        groove ("Jackin' Shuffle", "Jackin'",
                { { K, fourFloor }, { C, "...g X... ...g X..." }, { CH, "xg.x xg.x xg.x xg.x" }, { OH, offbeats },
                  { CB, "..x. .x.. ..x. ...." } }),
        groove ("Ghetto Jack", "Jackin'",
                { { K, "X... X... X... X..x X... X... X... X.xx" }, { C, backbeat }, { OH, offbeats },
                  { CH, "x..x x..x x..x x..x" }, { R, "...x ..x. .x.. ...." } }),
        groove ("Jack Cowbell", "Jackin'",
                { { K, fourFloor }, { C, backbeat }, { OH, offbeats }, { CH, "xg.x xg.x xg.x xg.x" },
                  { CB, "x..x ..x. ..x. x... x..x ..x. x.x. ..x." } }),

        // ---- Jackin' Piano: the jack bounce under piano stabs ----
        groove ("Jackin' Piano Bounce", "Jackin' Piano",
                { { K, fourFloor }, { C, ".... X... .... X..g" }, { OH, offbeats }, { SH, "xgxg xgxg xgxg xgxg" },
                  { T, ".... x... .... x..." } }),
        groove ("Swing Jack Piano", "Jackin' Piano",
                { { K, fourFloor }, { S, ".... X..g ..g. X..." }, { C, backbeat }, { CH, "xg.g xg.g xg.g xg.g" },
                  { OH, offbeats }, { R, "...x .... .x.. ...." } }),
        groove ("Jack Stab Beat", "Jackin' Piano",
                { { K, "X... X... X... X..x" }, { C, ".... X... .... X.x." }, { OH, offbeats },
                  { CH, ".g.g .g.g .g.g .g.g" } }),

        // ---- Garage & UK: two-step kicks, shuffled hats, rim shots ----
        groove ("Two-Step", "Garage & UK",
                { { K, "X... .... ..x. .... X... .... ..x. .x.." }, { S, backbeat }, { CH, "..x. .x.x ..x. .x.x" },
                  { R, "...x .... .x.. ...x" } }),
        groove ("Speed Garage", "Garage & UK",
                { { K, fourFloor }, { OH, offbeats }, { S, ".... x... .... x..." }, { CH, "xg.g xg.g xg.g xg.g" },
                  { R, ".x.. ..x. .x.. ...x" } }),
        groove ("UKG Shuffle", "Garage & UK",
                { { K, "X... .... ..x. .... X... ..x. ..x. ...." }, { S, ".... X... .... X..g" },
                  { CH, "xgxx xgxg xgxx xgxg" }, { C, ".... x... .... x..." } }),
        groove ("Bassline House", "Garage & UK",
                { { K, fourFloor }, { C, backbeat }, { OH, offbeats }, { R, "...x ..x. ...x .x.." },
                  { SH, "xgxg xgxg xgxg xgxg" } }),

        // ---- Afro & Latin: clave, tresillo, congas, bells and toms ----
        groove ("Afro House", "Afro & Latin",
                { { K, fourFloor }, { R, "x..x ..x. ..x. x..." }, { SH, "xgxg xgxg xgxg xgxg" },
                  { CG, ".... x.x. .x.. x.x. .... x.x. .x.. xxx." }, { C, ".... x... .... x..." },
                  { TM, ".... .... .... ..x." } }),
        groove ("Tresillo Congas", "Afro & Latin",
                { { K, fourFloor }, { CG, "x..x ..x. x..x ..x." }, { CB, "..x. ..x. ..x. ..x." },
                  { SH, "gxgx gxgx gxgx gxgx" }, { C, backbeat } }),
        groove ("Clave 3-2", "Afro & Latin",
                { { K, fourFloor }, { R, "x..x ..x. ..x. x..." }, { OH, offbeats }, { CG, "..x. .x.x ..x. .xx." },
                  { C, backbeat } }),
        groove ("Tribal Toms", "Afro & Latin",
                { { K, fourFloor }, { TM, "..x. .x.. ..x. x.x. ..x. .x.. x.x. xxXx" }, { SH, "xgxg xgxg xgxg xgxg" },
                  { C, ".... .... .... x..." }, { R, "...x ...x ...x ...x" } }),
        groove ("Latin House Cowbell", "Afro & Latin",
                { { K, fourFloor }, { CB, "x.x. x.xx .x.x x.x." }, { CG, "..x. .x.. ..xx .x.." }, { C, backbeat },
                  { SH, "xgxg xgxg xgxg xgxg" } }),

        // ---- Funky & Disco: snare backbeat, "ts" open hats, funky kicks ----
        groove ("Disco Classic", "Funky & Disco",
                { { K, fourFloor }, { S, backbeat }, { OH, offbeats }, { CH, "xx.x xx.x xx.x xx.x" } }),
        groove ("Disco Open Hats", "Funky & Disco",
                { { K, fourFloor }, { S, backbeat }, { C, ".... x... .... x..." }, { OH, offbeats },
                  { CH, "x... x... x... x..." }, { T, "x.x. x.x. x.x. x.x." }, { CR, crashBar1 } }),
        groove ("Funky Breaks", "Funky & Disco",
                { { K, "X.x. ..x. ..X. .x.. X.x. ..x. ..x. .x.x" }, { S, ".... X..g .g.. X..g" },
                  { CH, "x.x. x.x. x.x. x..." }, { OH, ".... .... .... ..x." } }),
        groove ("Nu Disco", "Funky & Disco",
                { { K, fourFloor }, { C, backbeat }, { OH, offbeats }, { SH, "xgxg xgxg xgxg xgxg" },
                  { CB, ".... .... ..x. ...." } }),
        groove ("Philly Groove", "Funky & Disco",
                { { K, fourFloor }, { S, backbeat }, { T, "..x. x.x. ..x. x.x." }, { RD, "x.x. x.x. x.x. x.x." },
                  { CG, ".... ..x. .... x.x." } }),

        // ---- '90s Piano House: 909 kick and claps, offbeat hats, tambourine ----
        groove ("90s Italo", "'90s Piano House",
                { { K, fourFloor }, { C, backbeat }, { OH, offbeats }, { CH, "xg.g xg.g xg.g xg.g" },
                  { T, ".... x... .... x..." } }),
        groove ("Ride The Piano", "'90s Piano House",
                { { K, fourFloor }, { C, backbeat }, { RD, "x.X. x.X. x.X. x.X." }, { SH, "xgxg xgxg xgxg xgxg" } }),
        groove ("Club 909", "'90s Piano House",
                { { K, fourFloor }, { C, backbeat }, { S, ".... x... .... x..." }, { OH, offbeats },
                  { CH, "xx.x xx.x xx.x xx.x" }, { CR, crashBar1 } }),
        groove ("Rave Piano", "'90s Piano House",
                { { K, fourFloor }, { S, ".... X... .... X... .... X... .... XxXX" }, { C, backbeat }, { OH, offbeats } }),

        // ---- Neo Soul & Broken: no four on the floor, lazy pockets, broken kicks ----
        groove ("Neo Soul Pocket", "Neo Soul & Broken",
                { { K, "X... .... ..x. .... X... ...x ..x. ...." }, { S, ".... X..g ..g. X..." },
                  { CH, "x.xg x.xg x.xg x.xg" } }),
        groove ("Broken Beat", "Neo Soul & Broken",
                { { K, "X... ..x. .x.. .... X... ..x. ...x ..x." }, { S, ".... X... .... .X.. .... X... ..g. X..." },
                  { CH, "x.x. x.xx x.x. x.x." }, { R, ".... .... x... ...." } }),
        groove ("Dilla Swing", "Neo Soul & Broken",
                { { K, "X... ...x ..x. .... X... .... ..x. .x.." }, { S, ".... X... .... X... .... X... .... f..." },
                  { CH, "x.xg x.x. x.xg x.x." } }),
        groove ("West London Broken", "Neo Soul & Broken",
                { { K, "X... ..x. ..x. .... X..x ..x. .... ..x." }, { C, ".... x... .... X... .... x... ..x. ..X." },
                  { CH, "x.xx x.x. x.xx x.x." }, { CG, ".... .x.. .... ..x." } }),

        // ---- Chill & Pads: soft kicks, rims and shakers under long chords ----
        groove ("Chill House", "Chill & Pads",
                { { K, "x... x... x... x..." }, { R, ".... x... .... x..." }, { SH, "g.x. g.x. g.x. g.x." },
                  { OH, "..g. ..g. ..g. ..g." } }),
        groove ("Lounge Rim", "Chill & Pads",
                { { K, "x... ..x. x... ...." }, { R, ".... x..g .... x..." }, { RD, "x.g. x.g. x.g. x.g." } }),
        groove ("Half-Time Chill", "Chill & Pads",
                { { K, "X... .... ..x. ...." }, { S, ".... .... X... ...." }, { CH, "x.g. x.g. x.g. x.g." },
                  { SH, "..x. ..x. ..x. ..x." } }),
        groove ("Sunset Shaker", "Chill & Pads",
                { { K, "x... x... x... x..." }, { C, ".... x... .... x..." }, { SH, "xgxg xgxg xgxg xgxg" },
                  { CG, ".... ..x. .... .x.x" } }),

        // ---- Dance Stabs: big, straight club beats ----
        groove ("Dance Floor", "Dance Stabs",
                { { K, fourFloor }, { C, backbeat }, { S, ".... x... .... x..." }, { OH, offbeats },
                  { CH, "xx.x xx.x xx.x xx.x" } }),
        groove ("Euro Dance", "Dance Stabs",
                { { K, fourFloor }, { S, backbeat }, { C, backbeat }, { OH, offbeats }, { CR, crashBar1 } }),
        groove ("Stab Pump", "Dance Stabs",
                { { K, fourFloor }, { OH, offbeats }, { C, backbeat } }),
        groove ("Hands Up", "Dance Stabs",
                { { K, fourFloor }, { S, backbeat }, { C, backbeat }, { OH, offbeats }, { CH, "xg.g xg.g xg.g xg.g" },
                  { CR, crashBar1 } }),

        // ---- v0.7: more Jackin' (added at the end: saved projects keep their groove) ----
        groove ("Percolator Jack", "Jackin'",
                { { K, fourFloor }, { C, backbeat }, { OH, "..x. ..x. ..x. ..x." }, { CH, "x..x x..x x..x x..x" },
                  { TM, "..x. .x.. x..x .x.. ..x. .x.. x..x .xx." }, { CB, "x... ..x. ..x. x..." } }),
        groove ("Jackin' Claps", "Jackin'",
                { { K, fourFloor }, { C, "...g X... ...g X... ...g X... ...g X..g" }, { OH, offbeats },
                  { CH, "xg.x xg.x xg.x xg.x" } }),
        groove ("Warehouse Jack", "Jackin'",
                { { K, fourFloor }, { C, backbeat }, { RD, "..X. ..X. ..X. ..X." }, { CH, "xg.x xg.x xg.x xg.x" },
                  { R, "...x ..x. .... .x.." } }),
        groove ("Raw Jack Claps", "Jackin'",
                { { K, fourFloor }, { C, ".x.. X... X... X... ..x. X... X.x. ...." }, { S, "x... x... x... .... x... x... x... ...." },
                  { OH, offbeats }, { CH, "xg.x xg.x xg.x xg.X" } }),
        groove ("Chicago Layers", "Jackin'",
                { { K, "X... X... X... X... X... X... X... X..g" }, { C, ".... X... .... X... .... X... .... X.x." },
                  { S, ".... x... .... x..g .... x... .... x.gg" }, { OH, offbeats }, { CH, "xg.g xg.g xg.g xg.X" } }),
        groove ("Rim Jack", "Jackin'",
                { { K, fourFloor }, { C, backbeat }, { OH, offbeats }, { CH, "x..x x..x x..x x..X" },
                  { R, ".x.. ...x .x.. .x.. .x.. ...x ...x .x.." } }),
    };
    return patterns;
}

const std::vector<const char*>& grooveCategories()
{
    static const std::vector<const char*> categories { "Piano House", "Soulful", "Deep", "Gospel", "Jackin'", "Jackin' Piano", "Garage & UK",
                                                       "Afro & Latin", "Funky & Disco", "'90s Piano House", "Neo Soul & Broken",
                                                       "Chill & Pads", "Dance Stabs" };
    return categories;
}

int categoryIndex (const std::string& category) noexcept
{
    const auto& c = grooveCategories();
    for (size_t i = 0; i < c.size(); ++i)
        if (category == c[i])
            return (int) i;
    return -1;
}

float genreSwing (int category) noexcept
{
    static const float swing[] { 0.15f, 0.25f, 0.3f, 0.35f, 0.9f, 0.75f, 0.6f, 0.2f, 0.3f, 0.1f, 0.55f, 0.25f, 0.0f };
    return category >= 0 && category < (int) std::size (swing) ? swing[category] : 0.2f;
}

int randomPatternIndex() noexcept
{
    return (int) drumPatterns().size();
}

int userPatternIndex() noexcept
{
    return randomPatternIndex() + 1;
}

} // namespace fgd
