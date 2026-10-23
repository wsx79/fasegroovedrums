# Fase Groove Drums: audit (v0.19.0 + fixes)

Scope: engine (`Source/engine`), processor (`PluginProcessor`), guide kit, tests. Built and tested
with CMake/Ninja Release, `ctest` green (engine + plugin).

## Confirmed problems, fixed

| # | Area | Problem | Fix |
|---|------|---------|-----|
| 1 | Realtime | `processBlock()` → `currentSettings()` read about 100 parameters per block through `raw (const juce::String&)`. Each read built a `juce::String` (a heap allocation, plus `"note" + String (l)` concatenations) and searched the APVTS tree, all on the audio thread. | The parameter pointers are looked up once (`cacheParameters()` in the constructor). `raw (const char*)` and `rawLane()` never allocate, and `segmentAt()` uses cached pointers too. New plugin test: `testCachedParameterReads`. |
| 2 | Musical / sections | Jackin': the end-of-phrase turnaround (snare or clap 16ths, with Humanize > 0) also played in **Intro, Break, Build-up and Outro**, which are meant to have no backbeat. It happened in all 39 cases (13 presets × 3 sections). | It now plays only where fills are allowed (Main, Drop). Regression test: `testJackinTurnaroundRespectsSections`. |
| 3 | Musical / section variation | `makeSectionVariation` could toggle any non-downbeat hat step, which deleted the offbeat 8th that carries the house groove. | Only the 16ths in between (odd steps) get toggled. The RNG stream is unchanged. |
| 4 | Random / dead code | In `makeRandomPattern` the closed-hat jack branch (`jackHatCells`) was unreachable, so every Jackin' NEW BEAT had the same open-hat skeleton and only 3 closed-hat lines. The presets mostly use closed hats on the offbeat. | About 70% of beats use the offbeat open hat (now with 5 closed lines under it). The other 30% use the closed-hat jack cells with ghosts plus an open-hat accent. |

Hardening (potential problems, not seen to fail):
- `pending` is now reserved to 2048 (it was 512). A large offline block at a fast tempo, on a user grid full of rolls and flams, could have grown it on the audio thread.
- `outMidi.ensureSize()` is now called in `prepareToPlay`.

## Genres analysed (13, all house-family; there is **no Jazz genre** in this engine)

Piano House, Soulful, Deep, Gospel, Jackin', Jackin' Piano, Garage & UK, Afro & Latin, Funky & Disco, '90s Piano House, Neo Soul & Broken, Chill & Pads, Dance Stabs.
For each genre I checked the presets, NEW BEAT, the turnarounds (`makeVariation`), the fills ("Mix" per genre), the sections, the pocket (`genrePocket`), the accent curve and the swing:
- **Kept as valid:**
  - four-on-the-floor everywhere except Garage, Neo Soul and Chill
  - claps on 2 and 4
  - the open/closed hat choke
  - garage skip kicks with hats swinging harder (×1.15)
  - afro clave and 3-against-4, with straighter hands
  - neo soul snare 10 ms late
  - chill half-time
  - jack swing at about 62%
  - fills per genre (no tom or kick builds in garage or chill)
- **Improved:**
  - **Jackin'**: hat skeleton (see #4). The kick pick-up stays at the end of the loop, so the jack stays steady.
  - **Deep**: the offbeat open hat dropped from 95% to 60%. The rest are closed-hat offbeat lines, as in the deep presets.
  - **Neo Soul & Broken**: 3 more broken kicks, 4 more snare lines (ghosts, a dragged or pushed backbeat, a flam), a ride option (20%), and 3 more hat lines.
  - **Dance Stabs**: a closed hat under the open hat even at low density, a tambourine that grows with density, an occasional clap pick-up, and a snare 16th pick-up into the loop.
  - **House family**: 4 more closed lines under the offbeat open hat, and kick pick-ups at step 30, step 31, or the end of each bar (not Jackin').

New choices are drawn from a **second RNG** (`extra`), so where the first die takes the same path a seed still makes the beat it made before. Beats unchanged out of 1000 seeds:

| Genre | Unchanged |
|---|---|
| Garage | 100% |
| Chill | 99% |
| Afro | 87% |
| Gospel and Jackin' Piano | 85% |
| Piano House, Soulful and Funky | 76% |
| Deep | 53% |
| Jackin' | 47% |
| Dance | 22% |
| Neo Soul | 16% |

## Dice and diversity tests (in `tests/EngineTests.cpp`)

- **`testRandomBeatDiversityInMidi` (new):** for every genre it renders 48 seeds (densities 0 to 1) through the engine and compares the **MIDI onsets** (lane and 16th, velocity ignored). Each seed is checked for:
  - the same seed giving byte-identical MIDI
  - well-formed note on/off
  - at least 70% distinct onset patterns
  - no beat on more than 10% of seeds
  - at most 1 identical pair in a row
  - at least 4 drums whose onsets change
  - genre frame kept: four-on-the-floor on every seed for the house genres; at most 25% four-on-the-floor in garage; none in neo soul; a mix in chill
  - the backbeat present (afro is allowed to drop the clap with the clave; neo soul may drag its second backbeat)
- **Results:**

  | Genre | Distinct beats (of 48) |
  |---|---|
  | Jackin', Jackin' Piano, Afro, Neo Soul | 48 |
  | Deep, Gospel, Chill | 46 |
  | Garage, Funky | 45 |
  | Dance | 44 |
  | Soulful | 42 |
  | Piano House, '90s | 40 |

  The same test on the v0.19.0 engine: Dance had 28 distinct beats, its commonest beat came back 12 times, only 3 drums moved and there were 4 repeats in a row, so it fails. Neo Soul had 44. The other genres were the same within ±1 and are now kept by the test.
- **`testGenreAudit` (updated):** Deep and Jackin' now require an offbeat hat (open **or** closed) on every seed instead of an open hat on 80% of seeds.
- **Limits:**
  - Diversity is measured on onsets, not on musical quality.
  - At low density the house genres stay deliberately close (kick, clap, offbeat hat). Most of their variety is in the hats and the percussion, not the kick.
  - The thresholds are calibrated on fixed seeds, so the test is deterministic but not a statistical proof.

## Residual issues / notes

- When the settings change, the audio thread still builds the pattern itself (it allocates once) until the 40 ms message-thread timer publishes the prepared one. I kept this because an offline bounce with pattern automation needs it.
- `loopLengthInSteps` ignores the rotation of the 4 turnarounds and the 32-bar "life" cycle of Humanize. An exported "loop" (4 bars with variation every 4) is one turnaround, not the full live rotation. The tests pin this on purpose.
- `std::atomic_load` on `shared_ptr` (pattern and samples) uses libstdc++'s lock pool. That is short and bounded, but not lock-free.
- There are pre-existing compiler warnings (`-Wfloat-equal`, shadowing in `PluginEditor.cpp`, deprecated `createWriterFor` in the tests). I left them alone.

## Integration with Fase Groove Arranger (`ThirdParty/fasegroovedrums`)

- To pick up fixes 2–4 and the new genre rules, re-vendor **`Source/engine/Generators.cpp`** and **`Source/engine/GrooveEngine.cpp`**. The headers are unchanged and the API is the same.
- Fix 1 is plugin-only (`PluginProcessor.*`). If the Arranger reads drum parameters on its audio thread through `String` IDs, apply the same cached-pointer pattern there.
- Songs saved in the Arranger with a NEW BEAT seed can play a different beat in the genres listed above. Garage and Chill are effectively unchanged. Section variations (`makeSectionVariation`) can differ slightly on hat 16ths.
- Copy the two new engine tests if the Arranger runs the engine suite.
