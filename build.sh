#!/bin/sh
# Build Lemmings 2: The Tribes from your own copy of the game:
#     ./build.sh "Lemmings 2 - The Tribes (1994) (Krisalis).zip"
#             (TOSEC's four .adf, zipped or not, or the JASPP .jfd set F1022800)
# then: cd recomp && ./lemmings2      (the introduction: ./l2intro)
# The steps are recipe.json's "build", run by SuperSpider's tools/weave.py.
set -e
HERE="$(cd "$(dirname "$0")" && pwd)"
SUPERSPIDER="${SUPERSPIDER:-$HOME/stuff/superspider}"
[ $# -gt 0 ] || { echo "usage: $0 GAME.zip | DISC.adf... | DISC.jfd..." >&2; exit 2; }
(cd "$SUPERSPIDER" && dune build)
WEAVE_RECIPES="$HERE" exec python3 -I "$SUPERSPIDER/tools/weave.py" build "$@"
