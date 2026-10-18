# Changelog

*🇮🇹 [Versione italiana](CHANGELOG.it.md)*

All notable changes to **Fase Groove Drums**.

## [0.2.0] - 2026-10-18

The interface of Fase Groove Piano, for the drums.

- **Dice everywhere:** TOTAL RANDOM in the header, NEW BEAT, a die on **every drum** (a new line for that
  drum only, in the chosen genre), a die and a lock on each part A-D.
- **Parts A-D:** the four half bars of the groove in any order (e.g. A A A B), each with its die and lock.
- **VELOCITY view:** a piano roll for velocities, one drum at a time (chosen from a menu): its hits on
  top, a bar per hit to drag; shapes (offbeat push, on the beat, shuffle 16ths, crescendo, humanize,
  flat), a velocity die, back to automatic. Double-click a drum's name to open it.
- **Your own samples:** load a WAV / AIFF / FLAC / MP3 for any drum (right-click its name, or drop the
  file on its row); saved with the project.
- **Song window (SONG):** bars of each section, its own variation, its own fill; drag the whole song,
  one section or every section as its own clip; save the song or one .mid per section.
- **Transport** ⏮ ▶/❚❚ ■, **undo / redo** (Ctrl+Z / Ctrl+Shift+Z), **full screen**.
- **PRESETS** (save / load everything) and **MY GROOVES** (save / load the grid, with its velocities).
- **Import the drums of a MIDI file** (MY GROOVES, or drop the .mid on the window): two bars on the
  grid, with their velocities.
- ◀ ▶ to go through the grooves; a mark on each cell with a velocity set by hand.

## [0.1.0] - 2026-10-18

The first version: the drum companion of Fase Groove Piano.

- 53 drum grooves in the 13 genres of Fase Groove Piano (Piano House, Soulful, Deep, Gospel,
  Jackin', Jackin' Piano, Garage & UK, Afro & Latin, Funky & Disco, '90s Piano House,
  Neo Soul & Broken, Chill & Pads, Dance Stabs).
- NEW BEAT: random beats by genre, with Density.
- STYLE: one-click setups by genre; RESET.
- Editable grid: 13 drums × 32 steps with accents, ghost notes and flams; mute, MIDI note and
  level per drum.
- Swing, humanize and NEW FEEL.
- Fills (7 styles) every 1, 2, 4 or 8 bars, a crash after fills, variations every 4 or 8 bars.
- Song sections (Intro, Main, Break, Build-up, Drop, Outro) and whole-song export with markers.
- Four synthesized guide kits (House 909, Deep 808, Disco Live, Lo-Fi Soul) with tone, reverb
  and volume.
- MIDI clips by drag and drop or `.mid` file (one track or one track per drum), live MIDI out on
  channel 10 (or any channel), DAW sync or manual tempo, preview without the DAW.
