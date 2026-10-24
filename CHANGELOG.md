# Changelog

*🇮🇹 [Versione italiana](CHANGELOG.it.md)*

All notable changes to **Fase Groove Drums**.

## [0.21.3] - 2026-10-24

- The overview shows the bars as they play, fills and variations included.
- The step lights and the playhead follow the sound without lag: the position is carried forward between audio blocks and drawn at 60 Hz, redrawing only what moves.

## [0.21.2] - 2026-10-24

- Editor widgets and the update overlay now come from the shared FaseGroove module. Look and sound unchanged.

## [0.21.1] - 2026-10-24

- Lighter internals: parameters read through one shared table, timers on JUCE's TimedCallback (shared FaseGroove module). Sound unchanged.

## [0.21.0] - 2026-10-24

- Afro House grooves always have a snare part.
- In Italian the SNARE button is no longer translated.
- The grid lights follow the sound again (they lagged since 0.20.1).
- Shared FaseGroove code now comes from the common module; no change to saved projects.

## [0.20.1] - 2026-10-24

- Lighter editor: icons and logo cached, repaints, colours and tooltips only when something changes, presets indexed by genre. Replaced samples are freed through one shared bin. No change to the sound.

## [0.20.0] - 2026-10-23

- NEW BEAT more varied in Dance, Neo Soul, Deep and Jackin' (the jackin' closed-hat variant now plays); every genre keeps its rhythm.
- Fixed: with Humanize the jackin' turnaround put claps and snares back into intro, break, build-up and outro; section variations could remove the offbeat hat; parameters are read without allocating on the audio thread.

## [0.19.0] - 2026-10-22

- Random beats for piano house, soulful, deep and funky keep four on the floor, the clap on 2 and 4, and an offbeat open hat. Deep plays the rim with the clap.
- Jackin' random beats: offbeat open hat, swung 16th hats at 62%, syncopated percussion.
- Garage / two-step random beats skip the kick more often. The shuffle is unchanged.
- Twenty seeds of every genre are checked for a different rhythm, not only a different velocity.

## [0.18.0] - 2026-10-22

- Turnarounds and Mix fills follow the genre: garage ghosts into the one, afro congas, a late snare in neo soul, and a single hat lift (or none) in chill. House keeps its previous moves.
- Random beats: Garage, Neo Soul and Chill have their own layer weights. Chill no longer gains the busy 16th shaker filler.
- Build-up and drop keep the kick and hats of two-step, broken beat and chill.
- The accent groove follows the genre: house on the offbeat, jackin' and garage on the swung 16th, afro flatter, neo soul on the downbeat with softer hats, chill gentler. A hand-written grid stays on the house curve.
- Preset patterns are unchanged.

## [0.17.0] - 2026-10-22

- Jackin' Piano no longer grows a 909 snare turnaround at the end of a phrase. That turnaround stays on Jackin' only.
- Turnarounds follow the beat seed while playing, so live MIDI matches an exported clip.
- NEW BEAT with locked parts writes one groove around those parts.
- An edit on a 2-bar grid is still there when the groove is shown as four bars, unless that later bar was already changed on its own.
- The velocity lane shows the accent groove for automatic hits: offbeat hats louder, ghosts softer.

## [0.16.0] - 2026-10-20

- Thirteen additional `Arranger:` presets coordinate palettes, articulation and balance with Fase Groove Arranger 0.4.7, including the updated Jackin in the funky/disco direction.
- Previous preset indexes and values remain available; saved projects are not rewritten and the theme and control layout stay unchanged.

## [0.15.0] - 2026-10-19

- Song: fills, crashes, turnarounds and phrases count from each section's start (a drop after a 4-bar build-up fills on its own 8th and 16th bars); a crash opens the main part, the break, the drop and the outro, and follows a section's own end fill.
- Build-up: the snare roll and the crescendo run over the whole section (quarters, 8ths, 16ths, 32nds in the last bar, from soft to loud), and the kick stops for the last two beats before the drop.
- The drop lifts the groove (offbeat open hat, 16th shaker when missing); the main part starts without hand percussion for 8 bars.
- Every phrase has its own turnaround, a bigger one every eight bars; the humanized "life" changes every eight bars.
- Clips: a drum pushed ahead no longer plays twice at the loop point; song exports keep notes across section boundaries at full length.
- MIDI: the closed hat chokes the open hat with a note-off; longer notes for drums and percussion (samplers in gate mode); fill toms and congas never land on another drum's note.
- Note maps for every drum at once: General MIDI, 16-pad drum rack, MPC pads, one note per drum from C1 (in the note menu).
- GENRE FEEL (song window, on by default): each genre's pocket per drum (laid-back deep and neo-soul backbeats, harder garage hat swing, straighter afro hands). BASS SPACE: kick pick-ups only at the end of four bars, never just before the one.
- A kick on an off 16th (two-step, broken beats) now swings with the hats; on the beat it never moves.
- Steadier velocities: the kick and the backbeat barely vary with Humanize; softer hat and snare ghosts; the open hat accents the "and". Humanize centres hats on the grid. Snare and clap fills ramp from soft and end in 32nds; a crash follows only fills that close four bars.
- Random beats: afro 3-against-4 figures (the clap often makes way for the clave), deep rim ghosts, garage snare ghosts before the one. Some jackin' shaker lines are more varied.
- LINK: a tempo change while playing keeps the beat at once (no jump back and forth).
- A missing sample (a drive not plugged in) is kept in the project instead of being lost on the next save.
- The groove is built on the message thread: no allocation on the audio thread while editing; a host position wobbling by less than a sample no longer doubles a hit.

## [0.14.3] - 2026-10-19

- Preserve early template/push hits at high tempos; regression checks compare different block sizes. README and privacy now describe the existing optional update check.
- Shared Piano/Drums/Guitar control theme: matching buttons, menus, embedded Inter fonts, focus and active states; existing layouts are preserved.
- Note-offs keep the channel used by their note-on, including a channel change while playing.
- Old presets restore defaults for missing parameters instead of inheriting the previous preset.
- MIDI Start and Song Position Pointer replace the old clock position in the current block.
- Equal-time event ordering no longer needs the temporary allocation used by stable_sort.
- Empty audio blocks are handled without invalid sample positions.

## [0.14.2] - 2026-10-19

- Safer exports: every .mid (and .wav) is written to a temporary file first; the one already there is replaced only once the new one is complete (a full disk or no permission keeps the old file).
- Room reserved for very large audio blocks (offline rendering): nothing grows on the audio thread.

## [0.14.1] - 2026-10-19

- Dragging a clip: the window shrinks at once to the small strip of Fase Groove Piano (it waited for the drop before, so it often didn't), and comes back by itself.

## [0.14.0] - 2026-10-19

- Export by instrument, as Fase Groove Piano's: ALL, KICK, SNARE (snare, clap, rim), CYMBALS (hats, ride, crash), PERCUSSION, TRACKS, in the song window and under EXPORT MIDI.
- A replaced sample is freed on the message thread, never on the audio thread.
- The README and the guide give the right number of grooves (70); a test keeps them in step.
- New tests on every preset groove: clean notes, one track and the tracks per drum alike, the same settings the same clip.

## [0.13.2] - 2026-10-19

- The interface always in the chosen language (some texts stayed in English when the plug-in opened).
- Song window identical to Fase Groove Piano's: orange drag tiles, total on two lines, fills as wide as the row, "Main groove" on the timeline, RESET beside the title.
- Bigger PUSH and SWING knobs in the mixer.

## [0.13.1] - 2026-10-19

- Much lighter interface: the undo history is saved only after an edit (no more snapshots of the whole state twice a second) and the panels refresh 10 times a second, the lights 30.
- While a clip is dragged the window shrinks to a small strip, as in Fase Groove Piano, so the DAW's tracks are in view.
- The song window laid out as Fase Groove Piano's: the three drag tiles, SAVE and CLOSE on one row.

## [0.13.0] - 2026-10-19

- **A smaller window and a big editor.** The main window shows the groove at a glance (click it to
  edit) and, in a second tab, the volume faders; the dice, MY GROOVES, IMPORT, TEMPLATE and CLEAR
  stay there. **EDITOR...** opens the editor over the whole window, as Fase Groove Piano's piano
  roll: transport, a brush (accent, hit, ghost, flam), step grid, velocity, mixer and fill. The
  right-click menu of a step can delete it or move it one 16th left or right.
- **LINK and PLAY: no more stutters.** Linked, the play button's clock read the system time block by
  block; that jitters, so notes played twice or not at all (a lost kick, the sound breaking up). The
  beat now goes on smoothly and follows the shared clock gently.
- **LINK** (was LINK PIANO) links with **Fase Groove Guitar** too.

## [0.12.3] - 2026-10-19

- **No more look-alike grooves**: every pair of grooves was compared hit by hit, genre by genre.
  The real doubles are now different beats: *Hands Up* (16th hats, a snare build into the next
  loop) was one hit away from *Dance Floor*; *Euro Dance* (snare pick-ups, tambourine on the
  offbeat) and *Stab Pump* (rim pushing on the 16th before each beat, no snare) were five hits
  apart; *Deep Three-Step Rim* now really plays the 3-3-2 rim, with a soft tom answer; *Jack
  Snare Turnaround* has its own tambourine and rim. A test keeps every genre's grooves apart.

## [0.12.2] - 2026-10-19

- **Jackin' grooves each with their own character**: hats with ghost 16ths and pick-ups instead
  of the same offbeat line, and in a few grooves a ghost kick or a kick pick-up into the next bar
  (always on the grid; most kicks stay straight and strong). The random Jackin' beat uses the same
  hat vocabulary and sometimes a ghost kick at the end of the loop.
- New tests on the music itself: no two Jackin' grooves are the same beat, ghosts and kick
  variations are there, and the MIDI played with swing, humanize and fills is in order, every
  note ends and the kick stays on the grid.

## [0.12.1] - 2026-10-18

- **No sound cuts another one short** in the guide sound: each drum keeps at most four voices of
  its own and fades its oldest one out, so a busy drum with long samples (shaker, open hat,
  crash) never takes the voice of another drum (the kick's tail, a clap...). The open / closed hat
  fix of 0.12.0 is the only choke, as on a real hi-hat.

## [0.12.0] - 2026-10-18

- **The same interface as Fase Groove Piano** in the controls the two share: the header buttons
  (PRESETS, TOTAL, undo / redo, full screen, STYLE, RESET, OPTIONS), the TRANSPORT & TEMPO box
  (same size and places; with SYNC DAW on it shows the DAW's tempo, as the piano), the display,
  and the same words in every language (TOTAL is TUTTO in Italian, as in the piano).
- **OPTIONS like the piano's**: a SETTINGS tab (language, interface size, tips on / off, automatic
  updates with CHECK NOW, the shortcuts) and an ABOUT tab with the same credits and links. Fully
  translated.
- **The standalone app's own Options menu** (audio / MIDI settings, save / load state) follows
  the language too.
- **The orange MIDI drag tile** is back, at the piano's size: DRAG MIDI TO DAW, with what it drags.
- **Jackin' with more jack**: a 909 snare under the clap in most Jackin' grooves and random beats
  (soft on the beats, loud swung pick-ups, a turnaround into the next loop); the clap lands a hair
  before the snare so the two flam; with Humanize the snare (or the clap) turns the four-bar phrase
  round with swung 16ths. Two new grooves: *Jack Hybrid* and *Jack Snare Turnaround*.
- **Sample browser: FOLDERS READ** shows the folders your samples are read from (with how many
  samples each); open one, read it again, or leave it out (the drums keep their samples).
- Fixed: a choice in a menu (genre, fills, section, kit...) sometimes didn't stick and had to be
  picked again.
- Fixed: hi-hats skipped now and then. A closed hat on the same step as an open hat (a few ms
  later with Humanize) choked it; now the open hat plays, in the guide sound and in your DAW.

## [0.11.0] - 2026-10-18

- **Automatic updates**, as in Fase Groove Piano: when it opens it checks the releases page; a new
  version is downloaded (the Windows setup, or the zip) and shown with what's new, and installed
  only when you say INSTALL NOW (LATER, SKIP THIS VERSION). OPTIONS: turn it off, or CHECK FOR
  UPDATES by hand. Nothing else is ever sent.
- **A bigger grid**: the view (step grid, velocity, mixer, fill) is about a third taller; the
  panels above are more compact, with smaller knobs. The window opens a little smaller too.
- **The MIDI drag tile** is small and quiet (no more big orange block).
- **Sample browser**: a progress bar shows how far the reading of a folder is; **ADD FOLDER...**
  adds folders instead of replacing them (CLEAR LIST empties the list; the drums keep their
  samples), and the folders come back with the project.

## [0.10.0] - 2026-10-18

- **Grooves with life.** Humanize (30% by default) now also lets the soft notes come and go over
  a four-bar phrase, as in a played or programmed loop: ghost notes, the hats' and shakers' 16ths
  between the offbeats, the clap's pick-ups; now and then a soft note before the next bar. The
  phrase repeats; NEW FEEL makes another. The kick and the accents never change.
- **A soft 16th layer** (shaker or tambourine) in the grooves that had almost none, and in most
  random beats: that's where the swing and the accents make the groove.
- An open hat is an accent wherever it is (no longer softened on the 16ths).
- **Your samples, recognised for sure**: only by the file's name or its folder's name, whole words
  first (kick, snare, clap... beat the abbreviations, and the codes a pack puts in front of every
  name, such as "AB_XY_", no longer count: "AB_XY_CLAP_001" is a clap). Samples whose name or
  folder says something else (guitar, bass, vocal, synth, chord, fx, loop...) are left out; the
  unnamed ones only get a "maybe" from their sound (in ALL FILES), never used by themselves.
- **YOUR SAMPLES...** (Kit & sounds and Guide sound) opens the sample browser: the three steps are
  written on top, a big button chooses the folder, BUILT-IN / ONE FILE... / ALL BUILT-IN are at
  hand. Wrongly recognised? Drag the samples (several with shift / ctrl) onto the right drum, or
  right-click: It is a... - remembered next time. Your files stay where they are.
- **Bigger interface**: the window opens as big as the screen allows, and the smaller buttons
  (view tabs, NEW FEEL, CRASH, LINK, the groove arrows) are bigger.
- **Windows installer** (FaseGrooveDrums-Windows-Setup.exe): VST3 + Standalone, VST3 only or
  Standalone only, as for Fase Groove Piano.

## [0.9.0] - 2026-10-18

- **Grooves that groove.** Hats, shakers, tambourines and rides now follow the accent curve of
  house drumming: a loud offbeat, everything else less than half as loud (the 16th after the beat
  the softest); congas and cowbells a gentler one. Accents are at 85% by default.
- **The swing of each genre**, from almost straight (Soulful 53%, Funky & Disco 55%) to Chicago
  (Jackin' 63%); Deep and Piano House about 58%.
- **The house grooves are rebuilt**, and the random beat with them, as house is programmed: the
  kick, the clap on 2 and 4 (now and then with its pick-up), ONE hat line (mostly on the offbeat),
  a single open hat accent, and only a few sparse percussion hits. No more crowded beats.
- **Sample browser** (Guide sound -> KIT FOLDER..., a drum's right-click menu, or drop a folder on
  the window): every sample of a folder is recognised as a drum by its name, its folder's name or
  its sound. Pick a drum, click a sample to hear it, USE it; AUTO KIT does the whole kit.
- **LINK** now shares the tempo, SYNC DAW and the play / pause / stop buttons too, on the same
  beat; the LINK PIANO button moved next to Sequencer (as in Fase Groove Piano 2.19).

## [0.8.1] - 2026-10-18

- **LINK PIANO** works with Fase Groove Piano 2.18 (its **LINK DRUMS** button): when the piano picks
  a groove of another genre, the drums take the first groove of that genre too (and the other way
  round), with the same swing.

## [0.8.0] - 2026-10-18

- **Swing up to the heaviest shuffle**, shown as on a drum machine (50% straight, 67% triplet,
  75%). New swing for every genre: Piano House 55%, Soulful, Deep and Disco about 58%, Garage about 63%,
  Jackin' 70%, Neo Soul 65%...
- **Feel of each drum** (MIXER): push / pull in milliseconds and its share of the swing, with
  ready-made **TIMING** feels. The kick always stays on the grid.
- **Groove templates** (TEMPLATE...): the timing and accents of every 16th from an audio loop or
  a MIDI file, on any groove.
- **Chance and rolls** on any step (right-click): hits that play now and then, quick 2-4 hit rolls.
- **FILL view**: draw your own fill (Fill style: Custom), or start from a style. Five new fill
  styles: Kick build, Rim shuffle, Open hat run, Clap stutter, Percussion break.
- **KIT FOLDER...**: a whole kit of samples at once, matched to the drums by their names (or drop
  the folder on the window).
- **SAVE .WAV...**: the guide sound as audio: a clip that loops seamlessly, the whole song, or stems.
- **LINK PIANO**: genre and swing shared with Fase Groove Piano.
- Genres: deep (syncopated rims, ghost kicks), soulful (tambourine, congas), disco (16th hats
  around the open hat), garage (rim shuffle), afro (son clave, tom loop), '90s (Jersey hats and
  kick) get their own signatures in the random beats, and six new grooves.
- The tooltips of knobs and menus follow the interface language too.

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
