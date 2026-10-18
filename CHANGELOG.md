# Changelog

*🇮🇹 [Versione italiana](CHANGELOG.it.md)*

All notable changes to **Fase Groove Drums**.

## [0.7.1] - 2026-10-18

- **Heavier jack**: the Jackin' swing goes up to 90% (Jackin' Piano 75%), the classic heavy
  Chicago shuffle.
- The random Jackin' beat can now use the **raw jack clap** (syncopated claps around the beats,
  a different answer in bar 2, with a soft layer on the beats), an extra clap closing the loop,
  short snare ghosts at the end of each bar, a soft kick turning the loop round (still on the
  grid), an accented closed hat lifting the end of each bar and rims on the in-between 16ths.
- Three new Jackin' grooves: Raw Jack Claps, Chicago Layers, Rim Jack.

## [0.7.0] - 2026-10-18

- **Jackin' sounds like jackin'**: the Jackin' grooves are rewritten around the classic Chicago
  feel: four on the floor, claps on 2 and 4 with pickups, open hats on every offbeat, shuffled
  closed hats around them, percolating cowbell, rim and toms. Three new grooves: Percolator Jack,
  Jackin' Claps, Warehouse Jack. The random Jackin' beat follows the same rules.
- **The genre's swing comes with the groove**: picking a groove (browser or arrows) sets its genre
  and the genre's swing, and NEW BEAT starts from the genre's swing, so house grooves no longer
  play almost straight. Jackin' swings more (60%), Jackin' Piano 50%.

## [0.6.1] - 2026-10-18

- Technical terms stay in English in every language, as producers say them: the drums (kick,
  snare, clap, hi-hat, ride, shaker, conga, cowbell, tom, crash...), bar and beat, fill and the fill
  names, ghost note, flam, velocity, swing, sample, mute, mixer, the song sections (intro, main,
  break, build-up, drop, outro) and the velocity shapes.

## [0.6.0] - 2026-10-18

- **Part length** (GROOVE row): the parts A-D are half a bar each (a 2-bar groove, as before) or
  **one bar each (a 4-bar groove)**. The grid, the velocity view, the part cards, the dice, the
  imports and the clips all follow it; a 2-bar groove fills the four bars twice, so you can start
  from it and change bars 3 and 4.

## [0.5.0] - 2026-10-18

- **Interface in 6 languages**, as in Fase Groove Piano: English, Italiano, Español, Français,
  Deutsch, Português. Choose it in **OPTIONS → Language**: everything changes at once and the
  choice is kept for every project (the first time, the computer's language is used).
- Buttons, captions, menus, tooltips, the song window, the browser, the mixer, messages and the
  Standalone app's audio settings are all translated; groove and genre names stay as they are.

## [0.4.0] - 2026-10-18

- **Groove browser**: the scrolling menu is gone. Genres on the left, their grooves on the right,
  each with a picture of its rhythm (hats, backbeat, kick); a click loads it, arrows go through
  them, a die makes a new beat in the genre.
- **Audio loop → grid** (IMPORT... → audio, or drop a loop on the window): kick, clap, closed and
  open hats, with their velocities and the loop's swing.
- **The kick is always on the grid**: no swing, no humanize, no flam on the kick, ever.
- **ACCENTS**: hats, shaker, ride and tambourine sing on the offbeat and are softer on the 16ths,
  like a real hat player (0% = flat).
- **VOLUME** per drum (kit panel) and a **MIXER** (a fader and a mute for every drum of the guide
  sound and your samples). LEVEL still changes the MIDI velocities.
- Velocities set by hand now stay on the variation bar and under fills (they used to change at
  the end of the bar).
- **Random beats rebuilt** by roles, as a producer works: kick, backbeat, one main hat line, at most
  one second hat, one shaker or tambourine, at most one percussion; bar 2 repeats bar 1.
- Genre swing and the Offbeat push velocity shape follow real house loops more closely.

## [0.3.0] - 2026-10-18

The same interface as Fase Groove Piano, adapted to the drums.

- **Header** as in the piano: PRESETS, TOTAL (random), undo / redo, full screen; STYLE, RESET,
  OPTIONS; a TRANSPORT & TEMPO box (⏮ ▶ ■, Sequencer switch, SYNC DAW, BPM) and a big display
  with the groove, its genre, the section and where the playhead is.
- **GROOVE row** (the piano's progression): groove presets, RANDOM, genre, density, and the four
  **part cards A-D** (in place of the chord slots), each with a picture of its hits, a die and a lock.
- **KIT & SOUNDS**: the drum picked, its MIDI note, level, mute, its die and your own sample.
- **DRUM PADS** (in place of the circle of fifths): 13 pads that light up on the hits; click one to
  hear it and pick it, right-click to mute it.
- **GROOVE FEEL & SONG** and **GUIDE SOUND & MIDI OUT** panels.
- **STEP GRID / VELOCITY** at the bottom, and **EXPORT MIDI** with the big orange DRAG MIDI TO DAW
  tile, SAVE .MID... and FULL SONG...
- On the grid, a click on a drum's name now picks it (mute: the kit panel, the pads or right-click).

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
