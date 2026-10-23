# Static recompilation of Lemmings 2: The Tribes - two programs, as the
# discs have them: lemmings2 (!Lemmings2: the game, gen/, unpacked by
# ../tools/unpack.py; DispScreen, the title picture, gendisp/) and l2intro
# (!L2Intro, the introduction: genintro/). Both use GameModes (genmodes/);
# the game also KrisHardware (genkris/) and MusicModule4 (genmus/). All from
# SuperSpider's analyse and gen, with game.c or intro.c, linked with
# libroger (the RISC OS runtime).
ROGER   ?= $(HOME)/stuff/libroger
CC      ?= gcc
CFLAGS  ?= -O2 -g -w -fno-strict-aliasing $(if $(TRACE),-DRECOMP_TRACE)
gensrc   = $(wildcard $(1)/code*.c) $(1)/entries.c
GAMEOBJ := $(patsubst %.c,%.o,$(foreach d,gen gendisp genmodes genkris genmus,$(call gensrc,$(d)))) game.o
INTROOBJ := $(patsubst %.c,%.o,$(foreach d,genintro genmodes,$(call gensrc,$(d)))) intro.o
LIBS    := $(shell pkg-config --libs sdl3) -ljpeg -lm -lpthread
INC     := -I$(ROGER)/include

all: lemmings2 l2intro

lemmings2: $(GAMEOBJ) $(ROGER)/libroger.a
	$(CC) $(CFLAGS) -o $@ $(GAMEOBJ) $(ROGER)/libroger.a $(LIBS)

l2intro: $(INTROOBJ) $(ROGER)/libroger.a
	$(CC) $(CFLAGS) -o $@ $(INTROOBJ) $(ROGER)/libroger.a $(LIBS)

$(ROGER)/libroger.a: FORCE
	$(MAKE) -C $(ROGER) libroger.a

%.o: %.c $(ROGER)/include/roger.h $(ROGER)/include/roger_cpu.h
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

# Code/data analysis: SuperSpider, into ../analysis/<name>.json, then
# superspider gen. Each image's ../analysis/<name>.roots adds entry points
# to the analysis: "make roots" merges the gap entries a --trace-gaps run
# logged (untranslated.log, or LOG=...) into them, so the next "make regen"
# translates them.
SS      ?= $(HOME)/stuff/superspider
SSBIN   := $(SS)/_build/default/bin/main.exe
AN      := ../analysis
APP     := ../game/!Lemmings2
INTRO   := ../game/!L2Intro
orig     = $(firstword $(wildcard $(1),*) $(1))
GAME    := ../unpacked/runimage
DISP    := $(call orig,$(APP)/DispScreen)
MODES   := $(call orig,$(APP)/GameModes)
KRIS    := $(call orig,$(APP)/Kris2)
MUSMOD  := $(call orig,$(APP)/MUSMOD5)
INTROIMG := $(call orig,$(INTRO)/Intro)
# where game.c and intro.c load the modules
MODES_AT := 1800000
KRIS_AT  := 1810000
MUSMOD_AT := 1820000
LOG     ?= untranslated.log

# The game calls KrisHardware's routines through the table Kris_RoutinesBase
# gives (&13B150: add pc,r6,r7,lsl #2 with lr its caller's): --with tailcall
# takes such a jump as a tail call that returns.

# --roots for ../analysis/<name>.roots, if there is one
roots = $(if $(wildcard $(AN)/$(1).roots),--roots $(AN)/$(1).roots)

# $(call image,name,file,base,gendir,prefix[,analyse options])
define image
	$(SSBIN) analyse "$(2)" $(3) $(6) --full $(call roots,$(1)) -o $(AN)/$(1).json
	rm -rf $(4)
	$(SSBIN) gen "$(2)" $(3) $(AN)/$(1).json --c $(4) --leaders=dispatch \
	    $(if $(5),--prefix=$(5) --tabprefix=recomp$(5) --covtag=$(5))
endef

regen:
	cd $(SS) && dune build
	mkdir -p $(AN)
	$(call image,game,$(GAME),8000,gen,,--with tailcall)
	$(call image,dispscreen,$(DISP),8000,gendisp,D)
	$(call image,modes,$(MODES),$(MODES_AT),genmodes,G,--module)
	$(call image,kris,$(KRIS),$(KRIS_AT),genkris,K,--module)
	$(call image,musmod,$(MUSMOD),$(MUSMOD_AT),genmus,M,--module)
	$(call image,intro,$(INTROIMG),8000,genintro,I)

roots:
	python3 -I $(SS)/tools/gap_roots.py "$(GAME)" 8000 $(AN)/game.roots $(LOG)
	python3 -I $(SS)/tools/gap_roots.py "$(DISP)" 8000 $(AN)/dispscreen.roots $(LOG)
	python3 -I $(SS)/tools/gap_roots.py "$(MODES)" $(MODES_AT) $(AN)/modes.roots $(LOG)
	python3 -I $(SS)/tools/gap_roots.py "$(KRIS)" $(KRIS_AT) $(AN)/kris.roots $(LOG)
	python3 -I $(SS)/tools/gap_roots.py "$(MUSMOD)" $(MUSMOD_AT) $(AN)/musmod.roots $(LOG)

intro-roots:
	python3 -I $(SS)/tools/gap_roots.py "$(INTROIMG)" 8000 $(AN)/intro.roots $(LOG)
	python3 -I $(SS)/tools/gap_roots.py "$(MODES)" $(MODES_AT) $(AN)/modes.roots $(LOG)

clean:
	rm -f gen*/*.o game.o intro.o lemmings2 l2intro

.PHONY: all FORCE regen roots intro-roots clean
