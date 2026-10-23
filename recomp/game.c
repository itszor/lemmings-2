/* Lemmings 2: The Tribes (DMA Design for Psygnosis, 1993; Archimedes
 * conversion by Krisalis - Shaun Hollingworth and Jason Butler, music by
 * Matt Furniss, 1994): what libroger needs to know about the game.
 *
 * !Lemmings2's !Run:
 *   - runs RmMin (a utility that frees what it can of the RMA: as libroger
 *     starts out, so it isn't run here);
 *   - RMLoads GameModes (modes 48 and 49, for VGA monitors: genmodes/);
 *   - sets Tribe$Dir and ATribe$Dir and runs DiskTest, which sets
 *     Tribe$Floppies to TRUE when Tribe$Dir is on the floppy (disc name
 *     Tribes1), FALSE otherwise. The game is installed on "hard disc" here
 *     (../tools/install.py), so it's FALSE, and DiskTest isn't run;
 *   - RMLoads Kris2 (KrisHardware 1.03: genkris/) and MUSMOD5
 *     (MusicModule4, the tracker player that also plays the sound effects:
 *     genmus/), *MusSampleRate 3;
 *   - runs LemTitle, a music file (&CC5: MusicModule4 makes its run type
 *     *MusLoad), *MusSongPos 1;
 *   - runs DispScreen, which unpacks and shows the title picture (gendisp/);
 *   - runs TSLOT, a BASIC program that raises the WimpSlot to 1800K if
 *     there's that much free (here done by hand, below);
 *   - runs !RunImage, the game: crunched, and unpacked by its own decruncher
 *     by ../tools/unpack.py; the image it leaves at &8000 is translated (gen/);
 *   - afterwards turns the music off, kills the modules and runs disk3's
 *     RmMin.
 * DispScreen is the program libroger starts; game.c runs !RunImage when it
 * exits, as !Run does. */
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <roger.h>

extern const struct roger_image recomp_image, recompD_image, recompG_image, recompK_image,
    recompM_image;                      /* gen{,disp,modes,kris,mus}/entries.c */

#define APP "../game/!Lemmings2"
#define GAME "../unpacked/runimage"
#define SLOT (1800 * 1024)              /* TSLOT's MaxMem */

static const char *const vars[] = {
    "Tribe$Dir", "!Lemmings2", "ATribe$Dir", "!Lemmings2", "Tribe$Floppies", "FALSE", NULL};
static const char *const prefixes[] = {"<Obey$Dir>.", NULL};
static struct roger_module modules[] = {
    {"GameModes", &recompG_image, 0},
    {"Kris2", &recompK_image, 0},
    {"MUSMOD5", &recompM_image, 0},
    {NULL, NULL, 0}};

/* a file in !Lemmings2, whatever ,load-exec suffix the extraction gave it */
static const char *in_app(const char *leaf)
{
    DIR *d = opendir(APP);
    struct dirent *e;
    char *path = NULL;
    while (d && !path && (e = readdir(d)))
        if (strcspn(e->d_name, ",") == strlen(leaf) && !strncasecmp(e->d_name, leaf, strlen(leaf))) {
            path = malloc(strlen(APP) + strlen(e->d_name) + 2);
            sprintf(path, "%s/%s", APP, e->d_name);
        }
    if (d) closedir(d);
    if (!path) { fprintf(stderr, "no %s/%s: build.sh extracts it\n", APP, leaf); exit(1); }
    return path;
}

/* !Run's commands between loading the modules and running DispScreen */
static void init(void)
{
    roger_cli("MusSampleRate 3");
    roger_cli("<ATribe$Dir>.LemTitle");
    roger_cli("MusSongPos 1");
}

/* DispScreen's OS_Exit: TSLOT, then !RunImage, as a fresh program. When it
 * exits, the run ends. */
static int game_swi(uint32_t n, int x)
{
    if ((n & ~0x20000u) == 0x11 && roger_current_image() == &recompD_image) {
        if (roger_load_file(GAME, ROGER_LOAD) <= 0) { perror(GAME); roger_finish("no !RunImage"); }
        roger_set_image(&recomp_image);
        R[0] = SLOT; R[1] = (uint32_t)-1;
        roger_swi(0x400EC);             /* Wimp_SlotSize */
        R[13] = ROGER_LOAD + SLOT;
        R[12] = 0;
        roger_call_program(ROGER_LOAD, ROGER_HANDLER_RET);
        roger_finish("returned to OS");
    }
    return 0;
}

static struct roger_config config = {
    .title = "Lemmings 2: The Tribes (recompiled)",
    .image = &recompD_image,
    .modules = modules,
    .appdir = "../game",
    .dir_prefixes = prefixes,
    .variables = vars,
    .command_line = "<ATribe$Dir>.DispScreen",
    .initial_mode = 12,                 /* the desktop's: a TV mode, so the game uses 9 and 13 */
    .ctrl_meta_keys = 1,                /* F10/F11 are the game's (volume): libroger's are Ctrl+F10-F12 */
    .wimpslot = 1280 * 1024,            /* !Run's WimpSlot -Min 1280K */
    /* KrisHardware's initialisation puts its IRQ handler at &100 (passing
     * interrupts on to the one it found there): it takes vsync and timer 1,
     * which it programs part way down the frame (LemTimer, &6780 by
     * default) to switch to the panel's palette */
    .irq1v = 1,
    /* code the analysis missed runs interpreted (see ../analysis/*.roots) */
    .interpret_gaps = 1,
    .init = init,
    .swi = game_swi,
};

int main(int argc, char **argv)
{
    config.image_path = in_app("DispScreen");
    for (struct roger_module *m = modules; m->path; m++) m->path = in_app(m->path);
    roger_main(argc, argv, &config);
}
