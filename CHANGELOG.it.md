# Novità

*🇬🇧 [English version](CHANGELOG.md)*

Tutte le modifiche importanti di **Fase Groove Drums**.

## [0.20.1] - 2026-10-24

- Editor più leggero: icone e logo in cache, ridisegni, colori e tooltip solo quando cambia qualcosa, preset indicizzati per genere. I campioni sostituiti vengono liberati tramite un unico contenitore. Il suono non cambia.

## [0.20.0] - 2026-10-23

- NUOVO BEAT più vario in Dance, Neo Soul, Deep e Jackin' (ora suona anche la variante jackin' con hat chiuso); ogni genere mantiene il suo ritmo.
- Corretto: con Humanize il giro jackin' rimetteva clap e rullante in intro, break, build-up e outro; le variazioni di sezione potevano togliere l'hat in levare; i parametri si leggono senza allocare nel thread audio.

## [0.19.0] - 2026-10-22

- Beat casuali di piano house, soulful, deep e funky: quattro sul pavimento, clap su 2 e 4, open hat in levare. Nel deep il rim sta insieme al clap.
- Beat casuali jackin': open hat in levare, charleston in sedicesimi con swing al 62%, percussioni sincopate.
- I beat garage / two-step saltano la cassa più spesso. Lo shuffle resta.
- Venti seed per genere: il ritmo cambia, non solo la velocity.

## [0.18.0] - 2026-10-22

- Variazioni e fill Mix seguono il genere: ghost garage verso l'uno, conga afro, rullante in ritardo nel neo soul, un solo hat lift (o nessuno) nel chill. L'house tiene le mosse di prima.
- Beat casuali: Garage, Neo Soul e Chill hanno i propri pesi. Il chill non riceve più lo shaker fitto in sedicesimi.
- Build-up e drop conservano cassa e hi-hat di two-step, broken beat e chill.
- L'accent groove segue il genere: house sul levare, jackin' e garage sul sedicesimo in swing, afro più piatto, neo soul sul battere con hi-hat più morbidi, chill più gentile. Una griglia scritta a mano resta sulla curva house.
- I pattern dei preset non cambiano.

## [0.17.0] - 2026-10-22

- Jackin' Piano non aggiunge più il turnaround di rullante 909 a fine frase. Quel turnaround resta solo su Jackin'.
- Le variazioni seguono il seed del beat anche in playback, così il MIDI dal vivo coincide con la clip esportata.
- NEW BEAT con parti bloccate scrive un solo groove intorno a quelle parti.
- Una modifica su una griglia di 2 bar resta quando il groove passa a quattro bar, salvo che quel bar successivo sia già stato cambiato da solo.
- La corsia velocity mostra l'accent groove sui colpi automatici: charleston in levare più forti, ghost più morbidi.

## [0.16.0] - 2026-10-20

- Tredici preset aggiuntivi `Arranger:`: palette, articolazioni e bilanciamenti coordinati con Fase Groove Arranger 0.4.7, incluso il nuovo Jackin nella direzione funky/disco.
- Indici e valori dei preset precedenti conservati; nessuna riscrittura automatica dei progetti salvati e nessuna modifica al tema o alla disposizione dei controlli.

## [0.15.0] - 2026-10-19

- Brano: fill, crash, variazioni e frasi contano dall'inizio di ogni sezione (un drop dopo un build-up di 4 bar fa il fill sui suoi bar 8 e 16); un crash apre la parte principale, il break, il drop e l'outro, e segue il fill finale di una sezione.
- Build-up: il rullo di rullante e il crescendo coprono tutta la sezione (quarti, ottavi, sedicesimi, trentaduesimi nell'ultimo bar, da piano a forte) e la cassa si ferma negli ultimi due battiti prima del drop.
- Il drop alza il groove (open hat in levare, shaker in sedicesimi se mancano); la parte principale inizia senza percussioni a mano per 8 bar.
- Ogni frase ha la sua variazione, una più ampia ogni otto bar; la "vita" dell'umanizzazione cambia ogni otto bar.
- Clip: uno strumento in anticipo non suona più due volte al punto di loop; l'export del brano tiene le note a cavallo delle sezioni alla loro durata piena.
- MIDI: il charleston chiuso strozza quello aperto con un note-off; note più lunghe per batteria e percussioni (sampler in modalità gate); tom e conga dei fill non finiscono mai sulla nota di un altro strumento.
- Mappe note per tutti gli strumenti insieme: General MIDI, drum rack a 16 pad, pad MPC, una nota per strumento da C1 (nel menu della nota).
- FEEL DEL GENERE (finestra del brano, attivo di default): il pocket di ogni genere per strumento (backbeat rilassato in deep e neo soul, swing più marcato sugli hi-hat garage, mani più dritte nell'afro). SPAZIO AL BASSO: riprese della cassa solo alla fine di quattro bar, mai subito prima dell'uno.
- Una cassa su un sedicesimo in levare (two-step, broken beat) ora swinga con gli hi-hat; sul battito non si sposta mai.
- Velocity più stabili: cassa e backbeat variano appena con Umanizza; ghost di hi-hat e rullante più morbidi; l'open hat accenta il levare. Umanizza centra gli hi-hat sulla griglia. I fill di rullante e clap partono piano e finiscono in trentaduesimi; il crash segue solo i fill che chiudono quattro bar.
- Beat casuali: figure afro 3 contro 4 (il clap spesso lascia spazio alla clave), ghost di rim nel deep, ghost di rullante garage prima dell'uno. Alcune linee di shaker jackin' sono più varie.
- LINK: un cambio di tempo durante la riproduzione mantiene subito il beat (nessun salto avanti e indietro).
- Un sample mancante (un disco non collegato) resta nel progetto invece di perdersi al salvataggio successivo.
- Il groove viene preparato sul thread dei messaggi: nessuna allocazione sul thread audio durante le modifiche; una posizione dell'host che oscilla di meno di un campione non raddoppia più un colpo.

## [0.14.3] - 2026-10-19

- Corretti i colpi anticipati da template e push ai tempi elevati; test di indipendenza dalla dimensione dei blocchi. README e privacy descrivono il controllo aggiornamenti facoltativo già presente.
- Controlli comuni a Piano, Drums e Guitar: stessi pulsanti, menu, font Inter, focus, stati attivi e colori dei comandi, mantenendo la disposizione attuale.
- I rilasci MIDI mantengono il canale della nota iniziale, anche cambiando canale durante la riproduzione.
- I preset vecchi ripristinano i valori predefiniti dei parametri mancanti, senza ereditare il preset precedente.
- MIDI Start e Song Position Pointer aggiornano la posizione del clock nel blocco corrente.
- L’ordinamento degli eventi simultanei evita il buffer temporaneo di stable_sort.
- Gestione dei blocchi audio vuoti senza posizioni di campione non valide.

## [0.14.2] - 2026-10-19

- Export più sicuri: ogni .mid (e .wav) si scrive prima in un file temporaneo; quello esistente viene sostituito solo quando il nuovo è completo (disco pieno o permessi mancanti lasciano il file vecchio).
- Spazio riservato per blocchi audio molto grandi (render offline): niente cresce nel thread audio.

## [0.14.1] - 2026-10-19

- Trascinando una clip la finestra si riduce subito alla piccola striscia di Fase Groove Piano (prima aspettava il rilascio, quindi spesso non succedeva) e torna grande da sola.

## [0.14.0] - 2026-10-19

- Export per strumento, come in Fase Groove Piano: TUTTO, KICK, RULLANTE (rullante, clap, rim), PIATTI (hi-hat, ride, crash), PERCUSSIONI, TRACCE, nella finestra del brano e in EXPORT MIDI.
- Un campione sostituito viene liberato nel thread dei messaggi, mai nel thread audio.
- README e guida riportano il numero giusto di groove (70); un test li tiene allineati.
- Nuovi test su ogni groove: note pulite, traccia unica e tracce per strumento coerenti, stesse impostazioni stessa clip.

## [0.13.2] - 2026-10-19

- Interfaccia sempre nella lingua scelta (all'apertura alcuni testi restavano in inglese).
- Finestra del brano identica a quella di Fase Groove Piano: tessere arancioni, totale su due righe, fill larghi quanto la riga, "Groove principale" sulla linea del tempo, RESET accanto al titolo; battute in italiano.
- Manopole PUSH e SWING del mixer più grandi.

## [0.13.1] - 2026-10-19

- Interfaccia molto più leggera: la cronologia annulla si salva solo dopo una modifica (non più copie dell'intero stato due volte al secondo) e i pannelli si aggiornano 10 volte al secondo, le luci 30.
- Mentre trascini una clip la finestra si riduce a una piccola striscia, come in Fase Groove Piano, così vedi le tracce della DAW.
- La finestra del brano intero disposta come quella di Fase Groove Piano: le tre tessere da trascinare, SALVA e CHIUDI su una riga.

## [0.13.0] - 2026-10-19

- **Finestra più piccola ed editor grande.** La finestra principale mostra il groove a colpo d'occhio
  (clic per modificarlo) e, in una seconda scheda, i fader dei volumi; restano lì i dadi, I MIEI
  GROOVE, IMPORTA, TEMPLATE e SVUOTA. **EDITOR...** apre l'editor sopra tutta la finestra, come il
  piano roll di Fase Groove Piano: trasporto, pennello (accento, colpo, ghost, flam), griglia,
  velocity, mixer e fill. Il menu col tasto destro di uno step lo cancella o lo sposta di un sedicesimo.
- **LINK e PLAY: niente più scatti.** Con LINK attivo l'orologio del tasto play leggeva l'ora di
  sistema a ogni blocco audio; quell'ora oscilla, e le note suonavano due volte o per niente (un kick
  perso, il suono che si interrompe). Ora il tempo scorre continuo e segue con dolcezza l'orologio condiviso.
- **LINK** (prima LINK PIANO) collega anche **Fase Groove Guitar**.

## [0.12.3] - 2026-10-19

- **Niente più groove quasi uguali**: ho confrontato colpo per colpo ogni coppia di groove, genere
  per genere. I doppioni veri ora sono beat diversi: *Hands Up* (hi-hat a 16esimi, un crescendo di
  snare verso il loop dopo) differiva di un solo colpo da *Dance Floor*; *Euro Dance* (pick-up di
  snare, tambourine in controtempo) e *Stab Pump* (rim che spinge sul 16esimo prima di ogni beat,
  senza snare) differivano di cinque; *Deep Three-Step Rim* ora suona davvero il rim 3-3-2, con una
  risposta leggera di tom; *Jack Snare Turnaround* ha tambourine e rim suoi. Un test tiene distinti
  i groove di ogni genere.

## [0.12.2] - 2026-10-19

- **Ogni groove Jackin' con il suo carattere**: hi-hat con 16esimi ghost e pick-up invece della
  stessa linea in controtempo, e in alcuni groove un ghost kick o un pick-up di kick verso la bar
  dopo (sempre sulla griglia; la maggior parte dei kick resta dritta e potente). Il beat casuale
  Jackin' usa le stesse linee di hi-hat e a volte un ghost kick alla fine del loop.
- Nuovi test sulla musica stessa: nessun groove Jackin' è lo stesso beat di un altro, ghost e
  variazioni del kick ci sono, e il MIDI suonato con swing, umanizza e fill è in ordine, ogni nota
  si chiude e il kick resta sulla griglia.

## [0.12.1] - 2026-10-18

- **Nessun suono ne taglia un altro** nel suono guida: ogni strumento usa al massimo quattro voci
  sue e sfuma la più vecchia, così uno strumento fitto con sample lunghi (shaker, open hat, crash)
  non prende mai la voce di un altro strumento (la coda del kick, un clap...). La correzione open /
  closed hat della 0.12.0 è l'unico choke, come su un hi-hat vero.

## [0.12.0] - 2026-10-18

- **La stessa interfaccia di Fase Groove Piano** nei comandi che hanno in comune: i pulsanti in
  alto (PRESET, TUTTO, annulla / ripeti, schermo intero, STILE, RESET, OPZIONI), il riquadro
  TRASPORTO E TEMPO (stessa misura e stesse posizioni; con SYNC DAW acceso mostra il tempo della
  DAW, come il piano), il display, e le stesse parole in ogni lingua (TOTAL ora è TUTTO, come nel piano).
- **OPZIONI come quelle del piano**: una linguetta IMPOSTAZIONI (lingua, dimensione
  dell'interfaccia, suggerimenti sì / no, aggiornamenti automatici con CONTROLLA ORA, le
  scorciatoie) e una linguetta INFO con gli stessi crediti e link. Tutto tradotto.
- **Il menu Options dell'app standalone** (impostazioni audio / MIDI, salva / carica lo stato)
  segue anch'esso la lingua.
- **Torna il riquadro arancione per trascinare il MIDI**, grande come nel piano: TRASCINA MIDI
  NELLA DAW, con cosa trascina.
- **Jackin' con più jack**: uno snare 909 sotto il clap nella maggior parte dei groove Jackin' e
  dei beat casuali (piano sui beat, pick-up forti e swingati, un giro che porta al loop dopo); il
  clap arriva un soffio prima dello snare, così i due fanno flam; con Umanizza lo snare (o il clap)
  chiude la frase di quattro bar con 16esimi swingati. Due groove nuovi: *Jack Hybrid* e *Jack
  Snare Turnaround*.
- **Sample browser: CARTELLE LETTE** mostra le cartelle da cui vengono letti i tuoi sample (con
  quanti sample ciascuna); aprine una, rileggila o toglila dalla lettura (gli strumenti tengono i
  loro sample).
- Corretto: una scelta in un menu a tendina (genere, fill, sezione, kit...) a volte non restava e
  bisognava sceglierla di nuovo.
- Corretto: gli hi-hat ogni tanto saltavano. Un closed hat sullo stesso passo di un open hat (pochi
  ms dopo con Umanizza) lo zittiva; ora suona l'open hat, nel suono guida e nella tua DAW.

## [0.11.0] - 2026-10-18

- **Aggiornamenti automatici**, come in Fase Groove Piano: all'apertura controlla la pagina delle
  release; una nuova versione viene scaricata (il setup per Windows o lo zip) e mostrata con le
  novità, e installata solo quando premi INSTALLA ORA (PIÙ TARDI, SALTA QUESTA VERSIONE). OPZIONI:
  disattivali, o CERCA AGGIORNAMENTI a mano. Nient'altro viene mai inviato.
- **Griglia più grande**: la vista (griglia, velocity, mixer, fill) è alta circa un terzo in più;
  i pannelli sopra sono più compatti, con manopole più piccole. Anche la finestra si apre un po'
  più piccola.
- **Il riquadro per trascinare il MIDI** è piccolo e sobrio (niente più blocco arancione).
- **Sample browser**: una barra mostra a che punto è la lettura di una cartella; **AGGIUNGI
  CARTELLA...** aggiunge cartelle invece di sostituirle (SVUOTA LISTA svuota l'elenco; gli
  strumenti tengono i loro sample), e le cartelle tornano con il progetto.

## [0.10.0] - 2026-10-18

- **Groove con più vita.** Humanize (30% di default) ora fa anche andare e venire le note morbide
  in una frase di quattro bar, come in un loop suonato o programmato: ghost note, i 16esimi di
  hi-hat e shaker tra i controtempi, le anticipazioni del clap; ogni tanto una nota morbida prima
  della bar dopo. La frase si ripete; NEW FEEL ne crea un'altra. Il kick e gli accenti non
  cambiano mai.
- **Uno strato morbido di 16esimi** (shaker o tambourine) nei groove che non ne avevano quasi, e
  nella maggior parte dei beat casuali: è lì che swing e accenti fanno il groove.
- L'open hat è un accento ovunque sia (non viene più ammorbidito sui 16esimi).
- **I tuoi sample, riconosciuti con certezza**: solo dal nome del file o della cartella, prima le
  parole intere (kick, snare, clap... vincono sulle sigle, e i codici che un pack mette davanti a
  ogni nome, come "AB_XY_", non contano più: "AB_XY_CLAP_001" è un clap). I sample il cui nome o
  cartella dice altro (guitar, bass, vocal, synth, chord, fx, loop...) sono esclusi; quelli senza
  nome hanno solo un "forse" dal suono (in TUTTI I FILE), mai usato da solo.
- **I TUOI SAMPLE...** (Kit e suoni e Suono guida) apre il sample browser: i tre passi sono scritti
  in alto, un pulsante grande sceglie la cartella, INTERNO / UN FILE... / TUTTI INTERNI sono a
  portata di mano. Riconosciuto male? Trascina i sample (più di uno con shift / ctrl) sullo
  strumento giusto, o tasto destro: È un... - e se lo ricorda. I tuoi file restano dove sono.
- **Interfaccia più grande**: la finestra si apre grande quanto lo schermo permette, e i pulsanti
  più piccoli (schede della vista, NEW FEEL, CRASH, LINK, frecce dei groove) sono più grandi.
- **Installer per Windows** (FaseGrooveDrums-Windows-Setup.exe): VST3 + Standalone, solo VST3 o
  solo Standalone, come per Fase Groove Piano.

## [0.9.0] - 2026-10-18

- **Groove che hanno groove.** Hi-hat, shaker, tambourine e ride seguono la curva degli accenti
  della batteria house: il controtempo forte, tutto il resto a meno della metà (il 16esimo dopo il
  beat il più piano); conga e cowbell una curva più dolce. Gli accenti sono all'85% di default.
- **Lo swing di ogni genere**, da quasi dritto (Soulful 53%, Funky & Disco 55%) a Chicago
  (Jackin' 63%); Deep e Piano House circa 58%.
- **I groove house sono ricostruiti**, e con loro il beat casuale, come si programma la house: il
  kick, il clap su 2 e 4 (ogni tanto con la sua anticipazione), UNA sola linea di hi-hat (quasi
  sempre in controtempo), un solo accento di open hat e poche percussioni rade. Basta beat affollati.
- **Sample browser** (Suono guida -> CARTELLA KIT..., il menu col tasto destro di uno strumento, o
  trascina una cartella sulla finestra): ogni sample della cartella viene riconosciuto come
  strumento dal nome, dal nome della cartella o dal suono. Scegli lo strumento, clicca un sample
  per ascoltarlo, USA; KIT AUTO fa tutto il kit.
- **LINK** ora condivide anche tempo, SYNC DAW e i pulsanti play / pausa / stop, sullo stesso
  beat; il pulsante LINK PIANO è accanto a Sequencer (come in Fase Groove Piano 2.19).

## [0.8.1] - 2026-10-18

- **LINK PIANO** funziona con Fase Groove Piano 2.18 (il suo pulsante **LINK DRUMS**): quando il
  piano sceglie un groove di un altro genere, anche la drums prende il primo groove di quel genere
  (e viceversa), con lo stesso swing.

## [0.8.0] - 2026-10-18

- **Swing fino allo shuffle più pesante**, mostrato come su una drum machine (50% dritto, 67%
  terzina, 75%). Nuovo swing per ogni genere: Piano House 55%, Soulful, Deep e Disco circa 58%,
  Garage circa 63%, Jackin' 70%, Neo Soul 65%...
- **Feel di ogni strumento** (MIXER): push / pull in millisecondi e la sua parte di swing, con
  feel di **TIMING** pronti. Il kick resta sempre sulla griglia.
- **Groove template** (TEMPLATE...): timing e accenti di ogni 16esimo da un loop audio o da un
  file MIDI, su qualsiasi groove.
- **Probabilità e roll** su ogni step (tasto destro): colpi che suonano ogni tanto, roll veloci da 2-4 colpi.
- **Vista FILL**: disegna il tuo fill (stile di fill: Custom), o parti da uno stile. Cinque nuovi
  stili di fill: Kick build, Rim shuffle, Open hat run, Clap stutter, Percussion break.
- **CARTELLA KIT...**: un kit intero di sample in un colpo, assegnati agli strumenti dal nome (o
  trascina la cartella sulla finestra).
- **SALVA .WAV...**: il suono guida in audio: una clip in loop senza stacchi, il brano intero o gli stem.
- **LINK PIANO**: genere e swing condivisi con Fase Groove Piano.
- Generi: deep (rim sincopati, ghost kick), soulful (tambourine, conga), disco (hi-hat in 16esimi
  attorno all'open hat), garage (rim shuffle), afro (son clave, giro di tom), '90s (hi-hat e kick
  Jersey) hanno la loro firma nei beat casuali, e sei groove nuovi.
- Anche i suggerimenti di manopole e menu seguono la lingua dell'interfaccia.

## [0.7.1] - 2026-10-18

- **Jack piu pesante**: lo swing Jackin' sale al 90% (Jackin' Piano 75%), il classico shuffle
  pesante di Chicago.
- Il beat casuale Jackin' ora puo usare il **raw jack clap** (clap sincopati attorno ai beat,
  una risposta diversa nella bar 2, con un layer morbido sui beat), un clap in piu che chiude il
  loop, ghost note di snare corte a fine bar, un kick morbido che gira il loop (sempre sulla
  griglia), un closed hat accentato che alza la fine di ogni bar e rim sui 16esimi in mezzo.
- Tre groove Jackin' nuovi: Raw Jack Claps, Chicago Layers, Rim Jack.

## [0.7.0] - 2026-10-18

- **Il Jackin' suona jackin'**: i groove Jackin' sono riscritti attorno al classico feel Chicago:
  cassa in quattro, clap su 2 e 4 con le anticipazioni, open hat su ogni controtempo, closed hat
  shufflati attorno, cowbell, rim e tom che "percolano". Tre groove nuovi: Percolator Jack,
  Jackin' Claps, Warehouse Jack. Il beat casuale Jackin' segue le stesse regole.
- **Lo swing del genere arriva col groove**: scegliendo un groove (browser o frecce) si impostano
  il suo genere e lo swing del genere, e NEW BEAT parte dallo swing del genere, cosi i groove
  house non suonano piu quasi dritti. Jackin' ha piu swing (60%), Jackin' Piano 50%.

## [0.6.1] - 2026-10-18

- I termini tecnici restano in inglese in tutte le lingue, come li dicono i produttori: gli
  strumenti (kick, snare, clap, hi-hat, ride, shaker, conga, cowbell, tom, crash...), bar e beat,
  fill e i nomi dei fill, ghost note, flam, velocity, swing, sample, mute, mixer, le sezioni del
  brano (intro, main, break, build-up, drop, outro) e le forme di velocity.

## [0.6.0] - 2026-10-18

- **Durata delle parti** (riga GROOVE): le parti A-D durano mezza battuta ciascuna (groove di 2
  battute, come prima) oppure **una battuta ciascuna (groove di 4 battute)**. Griglia, vista
  velocity, schede delle parti, dadi, import e clip seguono la scelta; un groove di 2 battute riempie
  le quattro battute due volte, così parti da lì e cambi le battute 3 e 4.

## [0.5.0] - 2026-10-18

- **Interfaccia in 6 lingue**, come in Fase Groove Piano: English, Italiano, Español, Français,
  Deutsch, Português. Si sceglie in **OPZIONI → Lingua**: cambia tutto subito e la scelta vale per
  ogni progetto (la prima volta si usa la lingua del computer).
- Tradotti pulsanti, didascalie, menu, suggerimenti, finestra del brano, browser, mixer, messaggi e
  le impostazioni audio dell'app Standalone; i nomi dei groove e dei generi restano com'erano.

## [0.4.0] - 2026-10-18

- **Browser dei groove**: niente più menu a scorrimento. Generi a sinistra, i loro groove a destra,
  ognuno con il disegno del ritmo (hat, rullante/clap, cassa); un clic lo carica, le frecce li
  scorrono, un dado crea un nuovo beat nel genere.
- **Loop audio → griglia** (IMPORT... → audio, o trascina un loop sulla finestra): cassa, clap, hat
  chiusi e aperti, con le velocity e lo swing del loop.
- **La cassa è sempre sulla griglia**: niente swing, humanize o flam sulla cassa, mai.
- **ACCENTS**: hat, shaker, ride e tamburello più forti in levare e più leggeri sui 16esimi, come
  un vero batterista (0% = piatto).
- **VOLUME** per strumento (pannello kit) e un **MIXER** (un fader e un muto per ogni strumento del
  suono guida e dei tuoi campioni). LEVEL cambia ancora le velocity MIDI.
- Le velocity impostate a mano ora restano anche nella battuta di variazione e sotto i fill
  (prima cambiavano a fine battuta).
- **Beat casuali rifatti** per ruoli, come lavora un produttore: cassa, backbeat, una linea di hat
  principale, al massimo un secondo hat, uno shaker o un tamburello, al massimo una percussione;
  la battuta 2 ripete la 1.
- Swing per genere e forma "Offbeat push" più vicini ai veri loop house.

## [0.3.0] - 2026-10-18

La stessa interfaccia di Fase Groove Piano, adattata alla batteria.

- **Intestazione** come nel piano: PRESETS, TOTAL (casuale), annulla / ripeti, schermo intero;
  STYLE, RESET, OPTIONS; un riquadro TRANSPORT & TEMPO (⏮ ▶ ■, interruttore Sequencer, SYNC DAW,
  BPM) e un grande display con il groove, il genere, la sezione e la posizione.
- **Riga GROOVE** (la progressione del piano): preset dei groove, RANDOM, genere, densità e le quattro
  **schede delle parti A-D** (al posto degli slot degli accordi), con il disegno dei colpi, dado e lucchetto.
- **KIT & SOUNDS**: lo strumento scelto, la sua nota MIDI, livello, muto, il suo dado e il tuo campione.
- **DRUM PADS** (al posto del circolo delle quinte): 13 pad che si accendono sui colpi; clic per
  sentirlo e sceglierlo, tasto destro per il muto.
- Pannelli **GROOVE FEEL & SONG** e **GUIDE SOUND & MIDI OUT**.
- **STEP GRID / VELOCITY** in basso ed **EXPORT MIDI** con il grande riquadro arancione DRAG MIDI
  TO DAW, SAVE .MID... e FULL SONG...
- Nella griglia, il clic sul nome di uno strumento ora lo sceglie (muto: pannello kit, pad o tasto destro).

## [0.2.0] - 2026-10-18

L'interfaccia di Fase Groove Piano, per la batteria.

- **Dadi ovunque:** TOTAL RANDOM in alto, NEW BEAT, un dado su **ogni strumento** (una nuova linea solo
  per quello strumento, nel genere scelto), un dado e un lucchetto su ogni parte A-D.
- **Parti A-D:** le quattro mezze battute del groove in qualsiasi ordine (es. A A A B), ognuna con dado e lucchetto.
- **Vista VELOCITY:** un piano roll per le velocity, uno strumento alla volta (scelto da un menu): i
  colpi sopra, una barra per colpo da trascinare; forme (offbeat push, on the beat, shuffle 16ths,
  crescendo, humanize, flat), un dado per le velocity, ritorno all'automatico. Doppio clic sul nome di
  uno strumento per aprirla.
- **I tuoi campioni:** carica un WAV / AIFF / FLAC / MP3 per qualsiasi strumento (tasto destro sul nome,
  o trascina il file sulla sua riga); salvato con il progetto.
- **Finestra del brano (SONG):** battute di ogni sezione, sua variazione, suo fill; trascina il brano
  intero, una sezione o tutte le sezioni come clip separate; salva il brano o un .mid per sezione.
- **Trasporto** ⏮ ▶/❚❚ ■, **annulla / ripeti** (Ctrl+Z / Ctrl+Shift+Z), **schermo intero**.
- **PRESETS** (salva / carica tutto) e **MY GROOVES** (salva / carica la griglia, con le velocity).
- **Importa la batteria di un file MIDI** (MY GROOVES, o trascina il .mid sulla finestra): due battute
  sulla griglia, con le loro velocity.
- ◀ ▶ per scorrere i groove; un segno su ogni cella con velocity impostata a mano.

## [0.1.0] - 2026-10-18

La prima versione: il compagno di batteria di Fase Groove Piano.

- 53 groove di batteria nei 13 generi di Fase Groove Piano (Piano House, Soulful, Deep, Gospel,
  Jackin', Jackin' Piano, Garage & UK, Afro & Latin, Funky & Disco, '90s Piano House,
  Neo Soul & Broken, Chill & Pads, Dance Stabs).
- NEW BEAT: beat casuali per genere, con Density.
- STYLE: impostazioni per genere in un clic; RESET.
- Griglia modificabile: 13 strumenti × 32 step con accenti, ghost note e flam; muto, nota MIDI e
  livello per strumento.
- Swing, humanize e NEW FEEL.
- Fill (7 stili) ogni 1, 2, 4 o 8 battute, crash dopo i fill, variazioni ogni 4 o 8 battute.
- Sezioni del brano (Intro, Main, Break, Build-up, Drop, Outro) ed export del brano intero con i marker.
- Quattro kit guida sintetizzati (House 909, Deep 808, Disco Live, Lo-Fi Soul) con tono,
  riverbero e volume.
- Clip MIDI con drag and drop o file `.mid` (una traccia o una traccia per strumento), uscita MIDI
  dal vivo sul canale 10 (o qualsiasi canale), sync con la DAW o tempo manuale, anteprima senza DAW.
