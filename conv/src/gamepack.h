#ifndef GAMEPACK_H
#define GAMEPACK_H

#include "appvar.h"
#include "game.h"

typedef struct gamepack {
    size_t n_games;
    game_t *games;
} gamepack_t;

void gamepack_init(gamepack_t* gamepack);

void gamepack_destroy(gamepack_t* gamepack);

game_t* gamepack_new_game(gamepack_t* gamepack);

int gamepack_write(gamepack_t* gamepack, appvar_t* var);

#endif