/* colors.h - Color definition and global manipulation */

#ifndef COLORS_H
#define COLORS_H

#include <stdint.h>

extern uint8_t color_player_low;        /* CHIP-8 off color */
extern uint8_t color_player_high;       /* CHIP-8 on color */
extern uint8_t color_ui_bg_0;           /* UI background color 0 */
extern uint8_t color_ui_bg_1;           /* UI background color 1 */
extern uint8_t color_ui_fg_0;           /* UI foreground color 0 */
extern uint8_t color_ui_fg_1;           /* UI foreground color 1 */
extern uint8_t color_ui_select;         /* UI selected item color */
extern uint8_t color_ui_transparent_0;  /* Main transparent placeholder */
extern uint8_t color_ui_transparent_1;  /* Alt transparent placeholder */

/* Initialize all colors, calls colors_player_set() and colors_transparent_get() */
void colors_reset();

/* Set all SMC bytes in video.asm to the chip-8 colors */
void colors_player_set();

/* Set transparents to unused color numbers */
void colors_transparent_get();

#endif