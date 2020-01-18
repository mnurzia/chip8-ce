#include <stdint.h>
#include <stdlib.h>

#include <graphx.h>

#include "colors.h"
#include "core.h"
#include "video.h"

uint8_t     video_sprite_x;
uint8_t     video_sprite_y;
uint8_t     video_sprite_h;
uint8_t     video_sprite_buffer[32];
uint8_t     *video_blit_dest;
uint8_t     video_memory[2048];
uint8_t     video_mode;

void video_reset() {
    int i = 0;
    video_sprite_x = 0;
    video_sprite_y = 0;
    video_sprite_h = 0;
    for (; i < 32; i++) {
        video_sprite_buffer[i] = 0;
    }
    video_blit_dest = NULL;
    for (i = 0; i < 2048; i++) {
        video_memory[i] = color_player_low;
    }
    video_mode = 4;
}

void video_refresh() {
    if (video_mode == 0) {
        video_blit_dest = (*gfx_vbuffer)+(104*320)+128;
    } else if (video_mode == 1) {
        video_blit_dest = (*gfx_vbuffer)+(88*320)+96;
    } else if (video_mode == 2) {
        video_blit_dest = (*gfx_vbuffer)+(72*320)+64;
    } else if (video_mode == 3) {
        video_blit_dest = (*gfx_vbuffer)+(56*320)+32;
    } else if (video_mode == 4) {
        video_blit_dest = (*gfx_vbuffer)+(40*320);
    } else if (video_mode == 5) {
        video_blit_dest = (*gfx_vbuffer)+(26*320)+212;
    }
}

void video_draw() {
    if (video_mode == 0) {
        video_blit_1x();
    } else if (video_mode == 1) {
        video_blit_2x();
    } else if (video_mode == 2) {
        video_blit_3x();
    } else if (video_mode == 3) {
        video_blit_4x();
    } else if (video_mode == 4) {
        video_blit_5x();
    } else if (video_mode == 5) {
        video_blit_1_5x();
    }
}