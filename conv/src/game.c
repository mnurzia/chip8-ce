#include "game.h"

#include <inttypes.h>
#include <string.h>

#define GAME_HEADER_SIZE 0x36
#define GAME_CONTENTS_MAX_SIZE 0xDFF

struct game_key_name {
    uint8_t offset;
    uint8_t bit;
    char* name;
};

struct game_key_name game_ti_names[] = {
    { 1, 0, "Graph" },      { 1, 1, "Trace" },      { 1, 2, "Zoom" },       { 1, 3, "Window" },
    { 1, 4, "Yequ" },       { 1, 5, "2nd" },        { 1, 6, "Mode" },       { 1, 7, "Del" },
    { 2, 1, "Sto" },        { 2, 2, "Ln" },         { 2, 3, "Log" },        { 2, 4, "Square" },
    { 2, 5, "Recip" },      { 2, 6, "Math" },       { 2, 7, "Alpha" },      { 3, 0, "0" },
    { 3, 1, "1" },          { 3, 2, "4" },          { 3, 3, "7" },          { 3, 4, "Comma" },
    { 3, 5, "Sin" },        { 3, 6, "Apps" },       { 3, 7, "GraphVar" },   { 4, 0, "DecPnt" },
    { 4, 1, "2" },          { 4, 2, "5" },          { 4, 3, "8" },          { 4, 4, "LParen" },
    { 4, 5, "Cos" },        { 4, 6, "Prgm" },       { 4, 7, "Stat" },       { 5, 0, "Chs" },
    { 5, 1, "3" },          { 5, 2, "6" },          { 5, 3, "9" },          { 5, 4, "RParen" },
    { 5, 5, "Tan" },        { 5, 6, "Vars" },       { 6, 0, "Enter" },      { 6, 1, "Add" },
    { 6, 2, "Sub" },        { 6, 3, "Mul" },        { 6, 4, "Div" },        { 6, 5, "Power" },
    { 6, 6, "Clear" },      { 7, 0, "Down" },       { 7, 1, "Left" },       { 7, 2, "Right" },
    { 7, 3, "Up" }
};

void game_init(game_t* game) {
    memset(&(game->name), 0, 24);
    memset(&(game->author), 0, 24);
    game->year = 0;
    game_mapkey(game,  0, "DecPnt");
    game_mapkey(game,  1, "7");
    game_mapkey(game,  2, "8");
    game_mapkey(game,  3, "9");
    game_mapkey(game,  4, "4");
    game_mapkey(game,  5, "5");
    game_mapkey(game,  6, "6");
    game_mapkey(game,  7, "1");
    game_mapkey(game,  8, "2");
    game_mapkey(game,  9, "3");
    game_mapkey(game, 10, "0");
    game_mapkey(game, 11, "Chs");
    game_mapkey(game, 12, "Mul");
    game_mapkey(game, 13, "Sub");
    game_mapkey(game, 14, "Add");
    game_mapkey(game, 15, "Enter");
    game->color_fg = 255;
    game->color_bg = 0;
    game->speed = 15;
    game->size = 0;
    game->contents = NULL;
}

void game_destroy(game_t* game) {
    if (game->contents) {
        free(game->contents);
    }
}

void game_name(game_t* game, char* name) {
    strncpy((char*)&(game->name), (char*)&(name[0]), 24); 
}

void game_author(game_t* game, char* author) {
    strncpy((char*)&(game->author), (char*)&(author[0]), 24); 
}

void game_year(game_t* game, uint16_t year) {
    game->year = year;
}

int game_mapkey(game_t* game, uint8_t ch_key, char* ti_key) {
    size_t i = 0;
    for (i = 0; i < (sizeof(game_ti_names)/sizeof(struct game_key_name)); i++) {
        if (strcmp(game_ti_names[i].name, ti_key) == 0) {
            game->mapping[ch_key & 0xF] = (game_ti_names[i].offset << 3) | (game_ti_names[i].bit);
            return 0;
        }
    }
    return 1;
}

void game_color_fg(game_t* game, uint8_t color) {
    game->color_fg = color;
}

void game_color_bg(game_t* game, uint8_t color) {
    game->color_bg = color;
}

void game_speed(game_t* game, uint16_t speed) {
    game->speed = speed;
}

int game_contents(game_t* game, size_t size, char* contents) {
    if (size > GAME_CONTENTS_MAX_SIZE) {
        return 1;
    }
    game->contents = (char*)malloc(sizeof(char) * size);
    if (!game->contents) {
        perror("Unable to allocate game contents array");
        exit(1);
    }
    memcpy(game->contents, contents, size);
    game->size = size;
    return 0;
}

size_t game_size(game_t* game) {
    return GAME_HEADER_SIZE + game->size;
}

int game_write(game_t* game, appvar_t* var) {
    size_t i;
    if (!appvar_can_append(var, game_size(game))) {
        return 1;
    }
    appvar_append(var, game->name, 24);
    appvar_append(var, game->author, 24);
    appvar_append_byte(var, (game->year >> 0) & 0xFF);
    appvar_append_byte(var, (game->year >> 8) & 0xFF);
    for (i = 0; i < 16; i++) {
        appvar_append_byte(var, game->mapping[i]);
    }
    appvar_append_byte(var, game->color_fg);
    appvar_append_byte(var, game->color_bg);
    appvar_append_byte(var, (game->speed >> 0) & 0xFF);
    appvar_append_byte(var, (game->speed >> 8) & 0xFF);
    appvar_append_byte(var, (game->size >> 0) & 0xFF);
    appvar_append_byte(var, (game->size >> 8) & 0xFF);
    appvar_append(var, game->contents, game->size);
    return 0;
}