/* game.h - Game binary structure */

#ifndef GAME_H
#define GAME_H

#include <stdint.h>

/* Game info container */
struct game {
    char var[8];            /* TI variable name */
    uint16_t var_off;       /* Offset in variable */
    char name[24];          /* Game name */
    char auth[24];          /* Game author */
    uint16_t year;          /* Game year */
    uint8_t mapping[16];    /* Key mapping */
    uint8_t fg;             /* Foreground color */
    uint8_t bg;             /* Background color */
    uint16_t speed;         /* Instructions per frame */
    uint16_t size;          /* Size of game */
}

/* Initialize all variables within game *gm* */
void game_reset(struct game* gm);

#endif