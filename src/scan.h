/* scan.h - Game scanning */

#ifndef SCAN_H
#define SCAN_H

#include "game.h"

extern struct game *scan_games;     /* Game list (malloc'd) */
extern uint16_t scan_games_count;   /* Number of games in the game list */

/* Reset the game list */
void scan_reset();

/* Try to scan a single appvar for games. */
int scan_step();

#endif