/* ui.c - User interface control */

#ifndef UI_H
#define UI_H

#include <stdint.h>

extern uint8_t ui_sprites[];    /* 1bpp ui sprites */

/* Run one frame of the UI */
uint8_t ui_frame();

/* Blit a 1bpp sprite *data* at (*x*, *y*) */
void ui_sprite(int x, uint8_t y, uint8_t* data);

#endif