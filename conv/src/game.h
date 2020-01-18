#ifndef GAME_H
#define GAME_H

#include <stdint.h>

#include "appvar.h"

typedef struct game {
    char name[24];
    char author[24];
    uint16_t year;
    uint8_t mapping[16];
    uint8_t color_fg;
    uint8_t color_bg;
    uint16_t speed;
    size_t size;
    char* contents;
} game_t;

void game_init(game_t* game);

void game_destroy(game_t* game);

void game_name(game_t* game, char* name);

void game_author(game_t* game, char* author);

void game_year(game_t* game, uint16_t year);

int game_mapkey(game_t* game, uint8_t ch_key, char* ti_key);

void game_color_fg(game_t* game, uint8_t color);

void game_color_bg(game_t* game, uint8_t color);

void game_speed(game_t* game, uint16_t speed);

int game_contents(game_t* game, size_t size, char* contents);

size_t game_size(game_t* game);

int game_write(game_t* game, appvar_t* var);

#endif