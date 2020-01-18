/* video.h - CHIP-8 video interface */

#ifndef VIDEO_H
#define VIDEO_H

#include <stdint.h>

extern uint8_t      video_sprite_x;                 /* Sprite x position */
extern uint8_t      video_sprite_y;                 /* Sprite y position */
extern uint8_t      video_sprite_h;                 /* Sprite height */
extern uint8_t      video_sprite_buffer[32];        /* Sprite data */    
extern uint8_t      *video_blit_dest;               /* Screen draw address */
extern uint8_t      video_memory[2048];             /* Screen contents */
extern uint8_t      video_mode;                     /* Screen size (on LCD) */
extern uint16_t     *video_palette_marker_base;     /* SMC byte base */
extern uint16_t     video_palette_markers_low[];    /* Low color SMC bytes */
extern uint16_t     video_palette_markers_high[];   /* High color SMC bytes */

/* Initialize video state */
void video_reset();

/* Clear the CHIP-8 screen */
void video_clear();

/* Draw a sprite (args are passed in global variables XD) */
void video_sprite();

/* Draw the screen @ 1x */
void video_blit_1x();

/* Draw the screen @ 2x */
void video_blit_2x();

/* Draw the screen @ 3x */
void video_blit_3x();

/* Draw the screen @ 4x */
void video_blit_4x();

/* Draw the screen @ 5x */
void video_blit_5x();

/* Draw the screen @ 1.5x */
void video_blit_1_5x();

/* Set video_blit_dest to the draw location (buffer or main) */
void video_refresh();

/* Call one of the blit functions based off of video_mode */
void video_draw();

#endif