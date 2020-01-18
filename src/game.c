#include "game.h"

#include <string.h>

void game_reset(struct game* gm) {
    memset(gm->var, 0, 8);
    gm->var_off = 0;
    memset(gm->name, 0, 24);
    memset(gm->auth, 0, 24);
    gm->year = 0;
    memset(gm->mapping, 0, 16);
    gm->fg = 0;
    gm->bg = 255;
    gm->speed = 15;
    gm->size = 0;
}