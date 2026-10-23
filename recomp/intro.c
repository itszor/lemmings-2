/* Lemmings 2: The Tribes - the introduction (!L2Intro, on its own disc):
 * what libroger needs to know about it.
 *
 * !L2Intro's !Run sets a 1134K WimpSlot, RMLoads GameModes (modes 48 and
 * 49, for VGA monitors: genmodes/, shared with the game) and runs Intro, an
 * absolute at &8000 (genintro/): the Story-Keeper tells the tale of the
 * twelve tribes. At its end, when the Lemmings look towards you, a click
 * returns to the desktop. (Filler, beside it, only fills the disc.) */
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <roger.h>

extern const struct roger_image recompI_image, recompG_image;   /* gen{intro,modes}/entries.c */

#define APP "../game/!L2Intro"

static const char *const prefixes[] = {"<Obey$Dir>.", NULL};
static struct roger_module modules[] = {{"GameModes", &recompG_image, 0}, {NULL, NULL, 0}};

/* a file in !L2Intro, whatever ,load-exec suffix the extraction gave it */
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

static struct roger_config config = {
    .title = "Lemmings 2: The Tribes - Introduction (recompiled)",
    .image = &recompI_image,
    .modules = modules,
    .appdir = APP,
    .dir_prefixes = prefixes,
    .command_line = "<Obey$Dir>.Intro",
    .initial_mode = 12,                 /* the desktop's: a TV mode */
    .wimpslot = 1134 * 1024,            /* !Run's WimpSlot 1134k */
    /* code the analysis missed (none seen: the whole introduction, and
     * the click that ends it) runs interpreted */
    .interpret_gaps = 1,
};

int main(int argc, char **argv)
{
    config.image_path = in_app("Intro");
    modules[0].path = in_app(modules[0].path);
    roger_main(argc, argv, &config);
}
