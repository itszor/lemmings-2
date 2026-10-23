Lemmings 2: The Tribes - static recompilation
=============================================

Lemmings 2: The Tribes (DMA Design for Psygnosis, 1993), in Krisalis'
Archimedes conversion (Shaun Hollingworth and Jason Butler, music by Matt
Furniss; version of 4th January 1994), recompiled from ARM2 code to C with
SuperSpider (~/stuff/superspider) and run on libroger (~/stuff/libroger),
which stands in for the Archimedes and RISC OS. Two programs, as the discs
have them:

    recomp/lemmings2     the game (!Lemmings2: discs 1-3)
    recomp/l2intro       the introduction (!L2Intro, on a disc of its own)

This repository is a recipe: it holds no game data, and builds the game
from your own copy (recipe.json recognises the discs by content):

    ./build.sh "Lemmings 2 - The Tribes (1994) (Krisalis).zip"
    cd recomp && ./lemmings2          (or ./l2intro)

Either known release will do: TOSEC's four .adf (zipped or not) or the
JASPP set F1022800 (four .jfd). build.sh runs recipe.json's steps with
SuperSpider's tools/weave.py: it extracts the discs (extracted/, game/),
installs them as the hard disc installation (tools/install.py), unpacks
!RunImage (tools/unpack.py), then make regen (the analyses and C) and make
in recomp/. Needs python3, OCaml/dune, gcc and SDL3.

Useful options (./lemmings2 --help for all): --crop (show only the rows the
picture uses, from lemmings2.ini / l2intro.ini), --fullscreen, --shader=NAME,
--wav=FILE, --limit=N (headless), --trace-gaps.

The discs
---------
Both releases hold the same files, byte for byte, with one exception: the
TOSEC disk 1 also carries the Extend virus (1989): a module ("Extend 1.56")
copied into !Lemmings2 under random names (CodeRM, MonitorRM, OSextend)
with a !Boot that runs it. tools/install.py removes it.

    Intro disc   !L2Intro: !Run, Intro (the introduction, an absolute at
                 &8000, 787K), GameModes, Filler (only fills the disc)
    Disk 1       !Lemmings2: !Run, RmMin, GameModes, DiskTest, Kris2
                 (KrisHardware), MUSMOD5 (MusicModule4), Lemtitle (the title
                 tune), DispScreen, TSLOT (BASIC), !RunImage; Disk1.l2DT
    Disk 2       disk2: L2EXTFX (more sound effects, for 4MB machines)
    Disk 3       disk3: the twelve tribes' styles (ASTYLE), L2EXTFX2,
                 the music (Lemmus, LemClassic, Lme), Lemendanip, RmMin

The hard disc installation (!Help) puts Disk1, disk2 and disk3 inside
!Lemmings2; that's game/!Lemmings2 here. Saved games go to recomp/savedata/
(libroger keeps the game's own files unchanged).

How it runs
-----------
!Run runs RmMin (frees RMA: libroger starts that way), RMLoads GameModes,
sets Tribe$Dir, runs DiskTest (Tribe$Floppies: FALSE for an installed
copy), RMLoads KrisHardware and MusicModule4, *MusSampleRate 3, runs
Lemtitle (a &CC5 music file: MusicModule4 sets Alias$@RunType_CC5 to
*MusLoad, then *MusOn), *MusSongPos 1, runs DispScreen (the Krisalis,
Psygnosis and DMA Design logos, while the title tune plays), TSLOT (a
1800K WimpSlot) and !RunImage. recomp/game.c does the same: the modules
and variables are its roger_config, DispScreen is the program libroger
starts, and its OS_Exit starts !RunImage.

!RunImage is crunched (a backwards bit stream from the Amiga original, as
in James Pond and Chuck Rock): its decruncher copies itself to &13D000 and
unpacks the program over &8000. tools/unpack.py runs it under SuperSpider's
tools/arm2.py and keeps what it leaves at &8000 (unpacked/runimage, 1.2M:
code and graphics) - that is what's translated (gen/).

Translated images (recomp/Makefile):

    gen/        !RunImage, unpacked, at &8000
    gendisp/    DispScreen at &8000
    genmodes/   GameModes at &1800000 (both programs)
    genkris/    KrisHardware at &1810000
    genmus/     MusicModule4 at &1820000
    genintro/   Intro at &8000

What SuperSpider needed (general mechanisms, in ~/stuff/superspider):

  - Krisalis' assembler for this game doesn't clear the should-be-zero Rd
    of test ops (cmp r0,#&100 is &E3502C01; cmp r0,lr &E150400E): an image
    in which most test ops have the same non-zero Rd takes any Rd as code
    (Image.test_rd_habit). Without it the analysis stopped at the first
    compare.
  - Code addresses an entry point hands back in a register (KrisHardware's
    SWI Kris_RoutinesBase returns its table of routines, a run of
    branches) are address-taken candidates, each branch of such a table
    one with strong evidence.
  - --with tailcall (opt-in, used for gen/): the game calls those routines
    with add pc,r6,r7,lsl #2 and lr its caller's - a tail call that
    returns.
The rest is in analysis/*.roots, from --trace-gaps runs (make roots): the
game's calls through its workspace (a circle routine's plot routine, a
table of tags and handlers at &13A1B4) and MusicModule4's effect dispatch.
Anything still missed runs interpreted (interpret_gaps).

What libroger needed:

  - KrisHardware puts its IRQ handler at &100 (vsync, and timer 1 for the
    panel's palette, LemTimer): roger_config.irq1v.
  - *Run of a typed file with an Alias$@RunType_TTT runs that alias with
    the parameters (%0-%9, %*N; |M separating commands): Lemtitle.
  - A RISC OS filename ends at a space, as FileSwitch reads it (*MusLoad
    passes its whole command tail to OS_File).

Playing
-------
The mouse selects skills and lemmings; F1-F8 skills, F9 music on/off,
F10/F11 volume (libroger's own keys - shaders, fullscreen, screenshot - are
Ctrl+F10-F12 here), Z/X move the skill selector, S scroll lock, P pause,
Space the fan, Esc restarts the level. Double-click EXIT on the title to
quit. The level select cheat: on the title click top left, top right,
bottom right, bottom left, then double-click them in the same order.

Tested headless and in real time: the logos and title, MAP, PLAY into a
level (Beach, Quad Quirks on the Quay), fast forward, the nuke and the
"extinct" screen, PRACTICE with every skill and the four practice tribes
(Space, Medieval, Egyptian, Polar), EXIT; the whole introduction and its
closing click; music and sound effects. A headless run (--limit) doesn't
wait in OS_Byte 19, so the game's double-click on EXIT can't be scripted
headless (it works in real time).

Not yet done: playing levels through to completion in every tribe, LOAD
and SAVE, the ending.
