/* player.h - Game loading/management */

#ifndef PLAYER_H
#define PLAYER_H

#include "game.h"

extern uint16_t player_ipf;     /* Player speed */
extern uint8_t  player_state;   /* Wait/run state */

/* Initialize player variables */
void player_reset();

/* Run a single frame */
uint8_t player_frame();

/* Load game *gm* into the player, core, and video subsystems. */
uint8_t player_load(struct game* gm);

#endif