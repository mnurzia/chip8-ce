#include "colors.h"

#include <graphx.h>

#include "video.h"

uint8_t color_player_low;
uint8_t color_player_high;
uint8_t color_ui_bg_0;
uint8_t color_ui_bg_1;
uint8_t color_ui_fg_0;
uint8_t color_ui_fg_1;
uint8_t color_ui_select;
uint8_t color_ui_transparent_0;
uint8_t color_ui_transparent_1;

uint16_t colors_565[256] = {
    0x0000, 0x6400, 0x8320, 0xE720, 0x001D, 0x6419, 0x8339, 0xF39C, 0x3DEF, 0x7C00, 0x83E0, 0xFFE0, 0x2D7F, 0x7C1F, 0x83FF, 0xFFFF,
    0x0000, 0x000B, 0x0010, 0x0015, 0x001A, 0x001F, 0x0160, 0x016B, 0x0170, 0x0175, 0x017A, 0x017F, 0x8200, 0x820B, 0x8210, 0x8215,
    0x821A, 0x821F, 0x82A0, 0x82AB, 0x82B0, 0x82B5, 0x82BA, 0x82BF, 0x8340, 0x834B, 0x8350, 0x8355, 0x835A, 0x835F, 0x83E0, 0x83EB,
    0x83F0, 0x83F5, 0x83FA, 0x83FF, 0x2C00, 0x2C0B, 0x2C10, 0x2C15, 0x2C1A, 0x2C1F, 0x2D60, 0x2D6B, 0x2D70, 0x2D75, 0x2D7A, 0x2D7F,
    0xAE00, 0xAE0B, 0xAE10, 0xAE15, 0xAE1A, 0xAE1F, 0xAEA0, 0xAEAB, 0xAEB0, 0xAEB5, 0xAEBA, 0xAEBF, 0xAF40, 0xAF4B, 0xAF50, 0xAF55,
    0xAF5A, 0xAF5F, 0xAFE0, 0xAFEB, 0xAFF0, 0xAFF5, 0xAFFA, 0xAFFF, 0x4000, 0x400B, 0x4010, 0x4015, 0x401A, 0x401F, 0x4160, 0x416B,
    0x4170, 0x4175, 0x417A, 0x417F, 0xC200, 0xC20B, 0xC210, 0xC215, 0xC21A, 0xC21F, 0xC2A0, 0xC2AB, 0xC2B0, 0xC2B5, 0xC2BA, 0xC2BF,
    0xC340, 0xC34B, 0xC350, 0xC355, 0xC35A, 0xC35F, 0xC3E0, 0xC3EB, 0xC3F0, 0xC3F5, 0xC3FA, 0xC3FF, 0x5400, 0x540B, 0x5410, 0x5415,
    0x541A, 0x541F, 0x5560, 0x556B, 0x5570, 0x5575, 0x557A, 0x557F, 0xD600, 0xD60B, 0xD610, 0xD615, 0xD61A, 0xD61F, 0xD6A0, 0xD6AB,
    0xD6B0, 0xD6B5, 0xD6BA, 0xD6BF, 0xD740, 0xD74B, 0xD750, 0xD755, 0xD75A, 0xD75F, 0xD7E0, 0xD7EB, 0xD7F0, 0xD7F5, 0xD7FA, 0xD7FF,
    0x6800, 0x680B, 0x6810, 0x6815, 0x681A, 0x681F, 0x6960, 0x696B, 0x6970, 0x6975, 0x697A, 0x697F, 0xEA00, 0xEA0B, 0xEA10, 0xEA15,
    0xEA1A, 0xEA1F, 0xEAA0, 0xEAAB, 0xEAB0, 0xEAB5, 0xEABA, 0xEABF, 0xEB40, 0xEB4B, 0xEB50, 0xEB55, 0xEB5A, 0xEB5F, 0xEBE0, 0xEBEB,
    0xEBF0, 0xEBF5, 0xEBFA, 0xEBFF, 0x7C00, 0x7C0B, 0x7C10, 0x7C15, 0x7C1A, 0x7C1F, 0x7D60, 0x7D6B, 0x7D70, 0x7D75, 0x7D7A, 0x7D7F,
    0xFE00, 0xFE0B, 0xFE10, 0xFE15, 0xFE1A, 0xFE1F, 0xFEA0, 0xFEAB, 0xFEB0, 0xFEB5, 0xFEBA, 0xFEBF, 0xFF40, 0xFF4B, 0xFF50, 0xFF55,
    0xFF5A, 0xFF5F, 0xFFE0, 0xFFEB, 0xFFF0, 0xFFF5, 0xFFFA, 0xFFFF, 0x0421, 0x0842, 0x0C63, 0x1084, 0x18C6, 0x1CE7, 0x2108, 0x2529,
    0x2D6B, 0x318C, 0x35AD, 0x39CE, 0xC210, 0xC631, 0xCA52, 0xCE73, 0xD6B5, 0xDAD6, 0xDEF7, 0xE318, 0xEB5A, 0xEF7B, 0xF39C, 0xF7BD
};

void colors_reset() {
    int i;
    for (i = 0; i < 256; i++) {
        gfx_palette[i] = colors_565[i];
    }
    color_player_low = 0x00;
    color_player_high = 0x0F;
    color_ui_bg_0 = 0x0F;
    color_ui_bg_1 = 0x21;
    color_ui_fg_0 = 0x00;
    color_ui_fg_1 = 0x0F;
    color_ui_select = 0xE4;
    colors_transparent_get();
    colors_player_set();
}

void colors_player_set() {
    int i = 0;
    while (video_palette_markers_low[i]) {
        uint8_t* base = (uint8_t*)(&video_palette_marker_base);
        uint24_t offset = video_palette_markers_low[i];
        if (offset >> 15) {
            offset += 0xFF0000;
        }
        base += offset;
        *base = color_player_low;
        i++;
    }
    i = 0;
    while (video_palette_markers_high[i]) {
        uint8_t* base = (uint8_t*)(&video_palette_marker_base);
        uint24_t offset = video_palette_markers_high[i];
        if (offset >> 15) {
            offset += 0xFF0000;
        }
        base += offset;
        *base = color_player_high;
        i++;
    }
}

void colors_transparent_get() {
    uint16_t i = 0;
    for (i = 0; i <= 255; i++) {
        if (i == color_ui_bg_0) {
            continue;
        }
        if (i == color_ui_bg_1) {
            continue;
        }
        if (i == color_ui_fg_0) {
            continue;
        }
        if (i == color_ui_fg_1) {
            continue;
        }
        if (i == color_ui_select) {
            continue;
        }
        color_ui_transparent_0 = i;
        break;
    }
    for (i = 0; i <= 255; i++) {
        if (i == color_ui_bg_0) {
            continue;
        }
        if (i == color_ui_bg_1) {
            continue;
        }
        if (i == color_ui_fg_0) {
            continue;
        }
        if (i == color_ui_fg_1) {
            continue;
        }
        if (i == color_ui_select) {
            continue;
        }
        if (i == color_ui_transparent_0) {
            continue;
        }
        color_ui_transparent_1 = i;
        return;
    }
}