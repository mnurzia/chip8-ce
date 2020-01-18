#include "gamepack.h"

#include <string.h>

#define VERSION_BYTE 0x01

void gamepack_init(gamepack_t* gamepack) {
    gamepack->n_games = 0;
    gamepack->games = NULL;
}

void gamepack_destroy(gamepack_t* gamepack) {
    size_t i;
    for (i = 0; i < gamepack->n_games; i++) {
        game_destroy(&(gamepack->games[i]));
    }
    if (i) {
        free(gamepack->games);
    }
}

game_t* gamepack_new_game(gamepack_t* gamepack) {
    if (!gamepack->n_games) {
        gamepack->games = (game_t*)malloc(sizeof(game_t));
    } else {
        gamepack->games = (game_t*)realloc(gamepack->games, sizeof(game_t)*(gamepack->n_games + 1));
    }
    if (!gamepack->games) {
        perror("Unable to allocate gamepack games buffer");
        exit(1);
    }
    game_init(&(gamepack->games[gamepack->n_games]));
    gamepack->n_games++;
    return &(gamepack->games[gamepack->n_games - 1]);
}

int gamepack_write(gamepack_t* gamepack, appvar_t* var) {
    size_t i;
    appvar_append(var, "CH8\0", 4);
    appvar_append_byte(var, VERSION_BYTE);
    appvar_append_byte(var, (gamepack->n_games >> 0) & 0xFF);
    for (i = 0; i < gamepack->n_games; i++) {
        if (game_write(&(gamepack->games[i]), var)) {
            return (int)(i + 1);
        } 
    }
    return 0;
}