#include "ui.h"

#include <string.h>

#include <debug.h>
#include <graphx.h>
#include <keypadc.h>

#include "colors.h"
#include "player.h"
#include "scan.h"
#include "video.h"

#define KEY_PRESSED(a) (ui_inputs & (1 << (a)))
#define KEY_HELD(a) (ui_inputs_held & (1 << (a)))
#define KEY_PRESSED_ARROW(a) (ui_inputs & 0xF)

/* How the UI works: */
/* The UI is basically a huge state machine of all the different places that
the user can be in during the program's runtime. I can get away with this
because there are very few dialogs, and everything is (for the most part) very
specialized. Each dialog has its own state and objects (rectangles, text, etc.)
have their positions hardcoded for the most part. The upside is that we also
have very fine-grained control over what we draw and when we draw it. Since the
eZ80 is nowhere near fast enough to speedily deal with a 320*240 screen, I
decided to focus on minimizing overdraw in order to get the responsiveness of
the UI to be within reason. There are typically 1-2 frames of lag when the UI
is drawing to both buffers but it is usually not a problem. I'd like to think
that it's at least a bit faster than the regular UI of the calculator itself.
However, I had to compromise code structure for minimizing overdraw. Although
globals aren't a huge problem on this architecture, I think I went a little
overboard here. Additionally there are quite a few states possible, most of
which are probably redundant. There are a bunch of weird functions scattered
everywhere which serve to keep code size relatively optimal, since they share
common code that is only ever called from about 2-4 different places. Still,
the UI is responsive and reasonably fast, as well as easy to use. */

uint8_t ui_state = 0;               /* Current state */
uint8_t ui_loaded_game = 0;         /* If a game is currently previewing */
uint8_t ui_runind_state = 0;        /* Run indicator animation index */
int ui_selected_game = 0;           /* Selected game index in scan_games */
uint8_t ui_inputs_held = 0;         /* Held keys bitfield */
uint8_t ui_inputs = 0;              /* Down keys bitfield */
uint8_t ui_draw_ctr = 0;            /* 2-frame countdown timer, minimizes overdraw (see ui_first()) */
uint8_t ui_selected_color = 0;      /* Color picker selected color */
uint8_t ui_selected_control = 0;    /* General purpose selected widget index */    
uint8_t ui_previous_state = 0;      /* For color/key picker, which state to return to (should be a stack but isn't) */
uint8_t ui_color_return = 0;        /* Color picker chosen color */
uint8_t ui_key_return = 0;          /* Key picker chosen key */
uint8_t ui_selected_key = 0;        /* Key picker - which key to choose */
uint8_t ui_palette_sprite[38];      /* Color picker - palette color data holder */
uint8_t ui_enter_pressed = 0;       /* Key picker - is enter pressed? */

enum {
    UI_STATE_BEGIN,                 /* Initial state (draw basic background) */
    UI_STATE_SCAN,                  /* Scanning games */
    UI_STATE_FOUND,                 /* Found games, able to select them */
    UI_STATE_PLAYING,               /* Playing a game */
    UI_STATE_SETTINGS_0,            /* Drawing settings dialog */
    UI_STATE_SETTINGS_1,            /* Drawing settings controls */    
    UI_STATE_GAME_SETTINGS_0,       /* Drawing game settings dialog */    
    UI_STATE_GAME_SETTINGS_1,       /* Drawing game settings controls */
    UI_STATE_COLOR_SEL_0,           /* Drawing color selector dialog */
    UI_STATE_COLOR_SEL_1,           /* Drawing color selector controls */
    UI_STATE_KEY_SEL_0,             /* Drawing key selector dialog */
    UI_STATE_KEY_SEL_1              /* Drawing key selector controls */
};

/* Key name strings (4 bytes per key -- generated in scripts/keys/generate_keys.py) */
static char ui_key_names[256] = {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x47,0x52,0x50,0x48,0x54,0x52,0x43,0x45,0x5A,0x4F,0x4F,0x4D,0x57,0x49,0x4E,0x00,
    0x59,0x45,0x51,0x55,0x32,0x4E,0x44,0x00,0x4D,0x4F,0x44,0x45,0x44,0x45,0x4C,0x00,
    0x00,0x00,0x00,0x00,0x53,0x54,0x4F,0x00,0x4C,0x4E,0x00,0x00,0x4C,0x4F,0x47,0x00,
    0x58,0x5E,0x32,0x00,0x31,0x2F,0x58,0x00,0x4D,0x41,0x54,0x48,0x41,0x4C,0x50,0x48,
    0x30,0x00,0x00,0x00,0x31,0x00,0x00,0x00,0x34,0x00,0x00,0x00,0x37,0x00,0x00,0x00,
    0x2C,0x00,0x00,0x00,0x53,0x49,0x4E,0x00,0x41,0x50,0x50,0x53,0x58,0x54,0x30,0x4E,
    0x2E,0x00,0x00,0x00,0x32,0x00,0x00,0x00,0x35,0x00,0x00,0x00,0x38,0x00,0x00,0x00,
    0x28,0x00,0x00,0x00,0x43,0x4F,0x53,0x00,0x50,0x52,0x47,0x4D,0x53,0x54,0x41,0x54,
    0x28,0x2D,0x29,0x00,0x33,0x00,0x00,0x00,0x36,0x00,0x00,0x00,0x39,0x00,0x00,0x00,
    0x29,0x00,0x00,0x00,0x54,0x41,0x4E,0x00,0x56,0x41,0x52,0x53,0x00,0x00,0x00,0x00,
    0x45,0x4E,0x54,0x52,0x2B,0x00,0x00,0x00,0x2D,0x00,0x00,0x00,0x58,0x00,0x00,0x00,
    0x2F,0x00,0x00,0x00,0x5E,0x00,0x00,0x00,0x43,0x4C,0x52,0x00,0x00,0x00,0x00,0x00,
    0x1F,0x00,0x00,0x00,0x3C,0x00,0x00,0x00,0x3E,0x00,0x00,0x00,0x1E,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

static uint8_t input_logic(uint8_t idx, uint8_t pressed) {
    uint8_t mask = 1 << idx;
    if (pressed) {
        if (ui_inputs_held & mask) {
            return 0;
        } else {
            ui_inputs_held |= mask;
            return mask;
        }
    } else {
        if (ui_inputs_held & mask) {
            ui_inputs_held &= ~mask;
            return 0;
        } else {
            return 0;
        }
    }
}

void ui_input() {
    ui_inputs = 0;
    kb_Scan();
    ui_inputs |= input_logic(0, kb_Data[7] & kb_Up);
    ui_inputs |= input_logic(1, kb_Data[7] & kb_Down);
    ui_inputs |= input_logic(2, kb_Data[7] & kb_Left);
    ui_inputs |= input_logic(3, kb_Data[7] & kb_Right);
    ui_inputs |= input_logic(4, kb_Data[6] & kb_Enter);
    ui_inputs |= input_logic(5, kb_Data[1] & kb_2nd);
    ui_inputs |= input_logic(6, kb_Data[1] & kb_Mode);
    ui_inputs |= input_logic(7, kb_Data[1] & kb_Del);
}

void ui_indicator() {
    uint8_t subfr = ui_runind_state >> 2;
    if (subfr != 0) {
        gfx_FillRectangle_NoClip(304, 3, 3, 3);
    }
    if (subfr != 1) {
        gfx_FillRectangle_NoClip(309, 3, 3, 3);
    }
    if (subfr != 2) {
        gfx_FillRectangle_NoClip(314, 3, 3, 3);
    }
    if (subfr != 3) {
        gfx_FillRectangle_NoClip(314, 8, 3, 3);
    }
    if (subfr != 4) {
        gfx_FillRectangle_NoClip(314, 13, 3, 3);
    }
    if (subfr != 5) {
        gfx_FillRectangle_NoClip(309, 13, 3, 3);
    }
    if (subfr != 6) {
        gfx_FillRectangle_NoClip(304, 13, 3, 3);
    }
    if (subfr != 7) {
        gfx_FillRectangle_NoClip(304, 8, 3, 3);
    }
}

void ui_indicator_wait(uint8_t a) {
    gfx_SetColor(color_player_low);
    gfx_FillRectangle_NoClip(304, 3, 13, 13);
    gfx_SetColor(color_player_high);
    if (!a) {
        return;
    }
    ui_indicator();
}

void ui_indicator_run(uint8_t a) {
    gfx_SetColor(color_ui_bg_1);
    gfx_FillRectangle_NoClip(304, 3, 13, 13);
    gfx_SetColor(color_ui_fg_1);
    if (!a) {
        return;
    }
    ui_indicator();
}

void ui_mask_color() {
    gfx_SetColor(color_ui_bg_1);
    gfx_SetTextBGColor(color_ui_transparent_1);
    gfx_SetTextFGColor(color_ui_fg_1);
    gfx_SetTextTransparentColor(color_ui_transparent_1);
}

void ui_player_color() {
    gfx_SetColor(color_player_high);
    gfx_SetTextBGColor(color_ui_transparent_1);
    gfx_SetTextFGColor(color_player_high);
    gfx_SetTextBGColor(color_ui_transparent_1);
}

void ui_header(char* text) {
    gfx_FillRectangle_NoClip(4, 6, 296, 8);
    gfx_PrintStringXY(text, 4, 6);
}

void ui_footer(char* text) {
    gfx_FillRectangle_NoClip(4, 227, 296, 8);
    gfx_PrintStringXY(text, 4, 227);
}

void ui_masktext(char* text, int x, uint8_t y, uint8_t maxwid) {
    gfx_FillRectangle_NoClip(x, y, maxwid, 8);
    gfx_PrintStringXY(text, x, y);
}

void ui_gameinfo() {
    struct game* gm = &scan_games[ui_selected_game];
    char buf[25];
    unsigned int wid = 0;
    gfx_SetTextConfig(gfx_text_clip);
    if (gm->auth[0]) {
        strncpy(buf, gm->auth, 25);
    } else {
        strcpy(buf, "Unknown");
    }
    gfx_SetClipRegion(0, 0, 316, 240);
    ui_masktext(buf, 215, 83, 101);
    if (gm->year) {
        sprintf(buf, "%04u", gm->year);
    } else {
        strcpy(buf, "Unknown");
    }
    ui_masktext(buf, 215, 95, 80);
    sprintf(buf, "%u bytes", gm->size);
    ui_masktext(buf, 215, 107, 100);
    sprintf(buf, "%u i/f", gm->speed);
    ui_masktext(buf, 215, 119, 100);
    strncpy(buf, gm->var, 8);
    buf[8] = '\0';
    strcat(buf, ".8xv");
    ui_masktext(buf, 215, 131, 100);
    gfx_SetClipRegion(0, 0, 320, 240);
    gfx_SetTextConfig(gfx_text_noclip);
}

void ui_controls() {
    gfx_PrintStringXY("[\x1e / \x1f]", 214, 162);
    gfx_PrintStringXY("[ENTER]", 204, 174);
    gfx_PrintStringXY("[2ND]", 220, 186);
    gfx_PrintStringXY("[MODE]", 212, 198);
    gfx_PrintStringXY("[DEL]", 220, 210);
    gfx_PrintStringXY("Sel ROM", 257, 162);
    gfx_PrintStringXY("Load ROM", 257, 174);
    gfx_PrintStringXY("Quit", 257, 186);
    gfx_PrintStringXY("Settings", 257, 198);
    gfx_PrintStringXY("ROM Set.", 257, 210);
}

uint8_t ui_first() {
    if (ui_draw_ctr > 0) {
        return ui_draw_ctr--;
    } else {
        return 0;
    }
}

void ui_state_set(uint8_t state) {
    ui_state = state;
    ui_draw_ctr = 2;
}

void ui_state_set_imm(uint8_t state) {
    ui_state = state;
    ui_draw_ctr = 0;
}

void ui_palette() {
    uint8_t i, j;
    uint8_t acc = 0;
    uint8_t pal_temp = ui_selected_color;
    uint16_t pal_color = gfx_palette[ui_selected_color];
    char buf[8];
    sprintf(buf, "R: %02u", (pal_color >> 10) & 0x1F);
    ui_masktext(buf, 106, 139, 30);
    buf[0] = 'G';
    sprintf(&buf[3], "%02u", ((pal_color >> 5) & 0x1F) | ((pal_color >> 15) << 5));
    ui_masktext(buf, 106, 151, 30);
    buf[0] = 'B';
    sprintf(&buf[3], "%02u", pal_color & 0x1F);
    ui_masktext(buf, 106, 163, 30);
    gfx_SetColor(color_ui_fg_1);
    gfx_Rectangle_NoClip(70, 139, 32, 31);
    gfx_SetColor(ui_selected_color);
    gfx_FillRectangle_NoClip(71, 140, 30, 29);
    gfx_SetColor(color_ui_bg_1);
    gfx_Rectangle_NoClip(95, 81, 130, 10);
    gfx_Rectangle_NoClip(69, 93, 182, 32);
    gfx_Rectangle_NoClip(75, 127, 170, 9);
    ui_palette_sprite[0] = 16;
    ui_palette_sprite[1] = 1;
    for (i = 0; i < 16; i++) {
        ui_palette_sprite[2+i] = i;
    }
    gfx_ScaledSprite_NoClip((gfx_sprite_t*)(&ui_palette_sprite), 96, 82, 8, 8);
    ui_palette_sprite[0] = 6;
    ui_palette_sprite[1] = 6;
    for (i = 0; i < 6; i++) {
        for (j = 0; j < 36; j++) {
            ui_palette_sprite[2+j] = 16+acc+j;
        }
        gfx_ScaledSprite_NoClip((gfx_sprite_t*)(&ui_palette_sprite), 70 + 30 * i, 94, 5, 5);
        acc += 36;
    }
    ui_palette_sprite[0] = 24;
    ui_palette_sprite[1] = 1;
    for (i = 0; i < 24; i++) {
        ui_palette_sprite[2+i] = 232+i;
    }
    gfx_ScaledSprite_NoClip((gfx_sprite_t*)(&ui_palette_sprite), 76, 128, 7, 7);
    gfx_SetColor(color_ui_select);
    if (ui_selected_color < 16) {
        gfx_Rectangle_NoClip(95 + (ui_selected_color << 3), 81, 10, 10);
    } else if (ui_selected_color < 232) {
        uint8_t cube_z = 0;
        uint8_t cube_y = 0;
        pal_temp -= 16;
        while (pal_temp >= 36) {
            pal_temp -= 36;
            cube_z++;
        }
        while (pal_temp >= 6) {
            pal_temp -= 6;
            cube_y++;
        }
        gfx_Rectangle_NoClip(69 + (cube_z * 30) + (pal_temp * 5), 93 + (5 * cube_y), 7, 7);
    } else {
        gfx_Rectangle_NoClip(75 + ((ui_selected_color - 232) * 7), 127, 9, 9);
    }
}

void ui_dashboard() {
    if (player_state == 1) {
        
    }
}

uint8_t ui_frame() {
    ui_input();
    if (ui_state == UI_STATE_BEGIN) {
        uint8_t i;
        colors_reset();
        gfx_SetTextConfig(gfx_text_noclip);
        for (i = 0; i < 2; i++) {
            gfx_FillScreen(color_ui_bg_0);
            ui_mask_color();
            gfx_FillRectangle_NoClip(0, 0, 320, 19);
            gfx_FillRectangle_NoClip(0, 221, 320, 19);
            gfx_FillRectangle_NoClip(200, 19, 120, 202);
            ui_header("CHIP-8 Emulator");
            ui_sprite(204, 82, &ui_sprites[0]);
            ui_sprite(204, 94, &ui_sprites[8]);
            ui_sprite(204, 106, &ui_sprites[16]);
            ui_sprite(204, 118, &ui_sprites[24]);
            ui_sprite(204, 130, &ui_sprites[32]);
            gfx_SwapDraw();
        }
        ui_state_set(UI_STATE_SCAN);
    } else if (ui_state == UI_STATE_SCAN) {
        int retval = scan_step();
        ui_mask_color();
        if (ui_first()) {
            ui_footer("Scanning...");
        }
        ui_indicator_run(1);
        if (retval == 1) {
            ui_loaded_game = 0;
            ui_state_set(UI_STATE_FOUND);
        }
        if (KEY_PRESSED(5)) {
            return 1;
        }
    } else if (ui_state == UI_STATE_FOUND) {
        char buf[30];
        int gcount = scan_games_count;
        int i = 0;
        if (ui_first()) {
            sprintf(buf, "%u games found", gcount);
            ui_mask_color();
            ui_footer(buf);
            ui_gameinfo();
            ui_controls();
            gfx_SetTextTransparentColor(color_ui_transparent_0);
            gfx_SetTextBGColor(color_ui_bg_0);
            gfx_SetTextFGColor(color_ui_fg_0);
            for (i = 0; i < scan_games_count; i++) {
                strncpy(buf, scan_games[i].name, 24);
                buf[24] = '\0';
                gfx_SetTextTransparentColor(color_ui_transparent_0);
                if (i == ui_selected_game) {
                    gfx_SetTextBGColor(color_ui_select);
                } else if (i == ui_selected_game + 1 || i == ui_selected_game - 1) {
                    gfx_SetTextBGColor(color_ui_bg_0);
                }
                gfx_PrintStringXY(buf, 4, 26 + 10 * i);
            }
            ui_indicator_run(1);
        } else {
            ui_indicator_run(0);
        }
        if (!ui_loaded_game) {
            int rv = 0;
            struct game* out = &scan_games[ui_selected_game];
            player_reset();
            rv = player_load(out);
            video_mode = 5;
            video_refresh();
            rv = player_frame();
            ui_loaded_game = 1;
        } else {
            int rv = 0;
            rv = player_frame();
        }
        if (KEY_PRESSED(0)) {
            if (ui_selected_game > 0) {
                ui_selected_game--;
                ui_loaded_game = 0;
                ui_draw_ctr = 2;
            }
        }
        if (KEY_PRESSED(1)) {
            if (ui_selected_game < (scan_games_count - 1)) {
                ui_selected_game++;
                ui_loaded_game = 0;
                ui_draw_ctr = 2;
            }
        }
        if (KEY_PRESSED(4)) {
            ui_state_set(UI_STATE_PLAYING);
        }
        if (KEY_PRESSED(5)) {
            return 1;
        }
        if (KEY_PRESSED(6)) {
            ui_state_set(UI_STATE_SETTINGS_0);
        }
        if (KEY_PRESSED(7)) {
            ui_state_set(UI_STATE_GAME_SETTINGS_0);
        }
    } else if (ui_state == UI_STATE_PLAYING) {
        switch (ui_first()) {
            case 2: {
                int rv = 0;
                struct game* out = &scan_games[ui_selected_game];
                ui_player_color();
                gfx_FillScreen(out->bg);
                player_reset();
                rv = player_load(out);
                if (player_frame()) {
                    ui_state_set(UI_STATE_BEGIN);
                }
            }
            case 1: {
                gfx_FillScreen(color_player_low);
                if (player_frame()) {
                    ui_state_set(UI_STATE_BEGIN);
                }
            }
            case 0: {
                gfx_FillScreen(color_player_low);
                if (player_frame()) {
                    ui_state_set(UI_STATE_BEGIN);
                }
            }
            if (player_state == 1) {
                ui_indicator_wait(1);
            } else {
                ui_indicator_wait(0);
            }
        }
    } else if (ui_state == UI_STATE_SETTINGS_0) {
        if (ui_first()) {
            gfx_SetColor(color_ui_fg_1);
            gfx_Rectangle_NoClip(32, 32, 256, 176);
            ui_mask_color();
            gfx_FillRectangle_NoClip(33, 33, 254, 174);
            gfx_PrintStringXY("Settings", 130, 37);
        }
        if (KEY_PRESSED(5)) {
            ui_state_set(UI_STATE_BEGIN);
        }
    } else if (ui_state == UI_STATE_GAME_SETTINGS_0) {
        if (ui_first()) {
            int i;
            int x = 124;
            int y = 69;
            char buf[40];
            struct game* gm = &scan_games[ui_selected_game];
            if (ui_color_return == 1) {
                gm->fg = ui_selected_color;
            } else if (ui_color_return == 2) {
                gm->bg = ui_selected_color;
            }
            ui_color_return = 0;
            if (ui_key_return) {
                gm->mapping[ui_key_return - 1] = ui_selected_key;
            }
            ui_key_return = 0;
            ui_mask_color();
            gfx_FillRectangle_NoClip(33, 33, 254, 174);
            gfx_PrintStringXY("Game Settings", 113, 37);
            strncpy(buf, gm->name, 24);
            buf[24] = '\0';
            gfx_PrintStringXY(buf, 37, 49);
            sprintf(buf, "FG: %02X", gm->fg);
            gfx_PrintStringXY(buf, 37, 81);
            buf[0] = 'B';
            sprintf(&buf[4], "%02X", gm->bg);
            gfx_PrintStringXY(buf, 37, 117);
            strcpy(buf, "I/F:");
            gfx_PrintStringXY(buf, 37, 153);
            sprintf(buf, "%d", gm->speed);
            gfx_PrintStringXY(buf, 88, 148);
            if (gm->speed < 5) {
                gfx_PrintStringXY("[VSLOW]", 70, 158);
            } else if (gm->speed < 10) {
                gfx_PrintStringXY("[SLOW]", 74, 158);
            } else if (gm->speed < 15) {
                gfx_PrintStringXY("[MED]", 78, 158);
            } else if (gm->speed < 20) {
                gfx_PrintStringXY("[FAST]", 74, 158);
            } else if (gm->speed < 25) {
                gfx_PrintStringXY("[VFAST]", 70, 158);
            }
            gfx_SetColor(gm->fg);
            gfx_FillRectangle_NoClip(80, 70, 30, 30);
            gfx_SetColor(gm->bg);
            gfx_FillRectangle_NoClip(80, 106, 30, 30);
            gfx_SetColor(color_ui_fg_1);
            gfx_Rectangle_NoClip(32, 32, 256, 176);
            gfx_Rectangle_NoClip(79, 69, 32, 32);
            gfx_Rectangle_NoClip(79, 105, 32, 32);
            buf[1] = '\0';
            for (i = 0; i < 16; i++) {
                uint8_t idx = 0;
                buf[0] = "123C456D789EA0BF"[i];
                buf[1] = '\0';
                idx = (buf[0] < 58) ? (buf[0] - '0') : (buf[0] - 'A' + 10);
                gfx_PrintStringXY(buf, x+16, y+3);
                strncpy(buf, &ui_key_names[(gm->mapping[idx] & 0x3F) << 2], 4);
                buf[4] = '\0';
                gfx_PrintStringXY(buf, (x+19) - (gfx_GetStringWidth(buf) >> 1), y+12);
                gfx_Rectangle_NoClip(x+1, y+1, 36, 20);
                x += 40;
                if ((i & 3) == 3) {
                    y += 24;
                    x = 124;
                }
            }
            gfx_PrintStringXY("[\x1e / \x1f / \x1d]", 174, 172);
            gfx_PrintStringXY("[2ND]", 202, 184);
            gfx_PrintStringXY("[ENTER]", 186, 196);
            gfx_PrintStringXY("Select", 239, 172);
            gfx_PrintStringXY("Exit", 239, 184);
            gfx_PrintStringXY("Edit", 239, 196);
        } else {
            ui_state_set(UI_STATE_GAME_SETTINGS_1);
        }
    } else if (ui_state == UI_STATE_GAME_SETTINGS_1) {
        if (ui_first()) {
            int i;
            int x = 123;
            int y = 68;
            gfx_SetColor(color_ui_bg_1);
            gfx_Rectangle_NoClip(35, 67, 78, 36);
            gfx_Rectangle_NoClip(35, 103, 78, 36);
            gfx_Rectangle_NoClip(35, 139, 82, 36);
            for (i = 0; i < 16; i++) {
                gfx_Rectangle_NoClip(x, y, 40, 24);
                x += 40;
                if ((i & 3) == 3) {
                    y += 24;
                    x = 123;
                }
            }
            gfx_SetColor(color_ui_select);
            if (ui_selected_control == 0) {
                gfx_Rectangle_NoClip(35, 67, 78, 36);
            } else if (ui_selected_control == 1) {
                gfx_Rectangle_NoClip(35, 103, 78, 36);
            } else if (ui_selected_control == 2) {
                gfx_Rectangle_NoClip(35, 139, 82, 36);
            } else {
                int x = (ui_selected_control - 3) & 0x3;
                int y = (ui_selected_control - 3) >> 2;
                gfx_Rectangle_NoClip(123 + x*40, 68 + y*24, 40, 24);
            }
        } else {
            if (KEY_PRESSED(5)) {
                ui_state_set(UI_STATE_BEGIN);
            } else if (KEY_PRESSED(4)) {
                if (ui_selected_control == 0) {
                    struct game* gm = &scan_games[ui_selected_game];
                    ui_selected_color = gm->fg;
                    ui_color_return = 1;
                    ui_previous_state = UI_STATE_GAME_SETTINGS_0;
                    ui_state_set(UI_STATE_COLOR_SEL_0);
                } else if (ui_selected_control == 1) {
                    struct game* gm = &scan_games[ui_selected_game];
                    ui_selected_color = gm->bg;
                    ui_color_return = 2;
                    ui_previous_state = UI_STATE_GAME_SETTINGS_0;
                    ui_state_set(UI_STATE_COLOR_SEL_0);
                } else if (ui_selected_control == 2) {
                    
                } else {
                    uint8_t idx = "123C456D789EA0BF"[(ui_selected_control - 3)];
                    struct game* gm = &scan_games[ui_selected_game];
                    idx = (idx < 58) ? (idx - '0') : (idx - 'A' + 10);
                    ui_selected_key = gm->mapping[idx];
                    ui_key_return = idx+1;
                    ui_previous_state = UI_STATE_GAME_SETTINGS_0;
                    ui_state_set(UI_STATE_KEY_SEL_0);
                }
            } else if (KEY_PRESSED_ARROW()) {
                if (ui_selected_control == 0) {
                    if (KEY_PRESSED(0)) {
                        ui_selected_control = 2;
                    } else if (KEY_PRESSED(1)) {
                        ui_selected_control = 1;
                    } else if (KEY_PRESSED(2)) {
                        ui_selected_control = 6;
                    } else if (KEY_PRESSED(3)) {
                        ui_selected_control = 3;
                    }
                } else if (ui_selected_control == 1) {
                    if (KEY_PRESSED(0)) {
                        ui_selected_control = 0;
                    } else if (KEY_PRESSED(1)) {
                        ui_selected_control = 2;
                    } else if (KEY_PRESSED(2)) {
                        ui_selected_control = 14;
                    } else if (KEY_PRESSED(3)) {
                        ui_selected_control = 11;
                    }
                } else if (ui_selected_control == 2) {
                    if (KEY_PRESSED(0)) {
                        ui_selected_control = 1;
                    } else if (KEY_PRESSED(1)) {
                        ui_selected_control = 0;
                    } else if (KEY_PRESSED(2)) {
                        ui_selected_control = 18;
                    } else if (KEY_PRESSED(3)) {
                        ui_selected_control = 15;
                    }
                } else {
                    int x = (ui_selected_control - 3) & 0x3;
                    int y = (ui_selected_control - 3) >> 2;
                    if (KEY_PRESSED(0)) {
                        if (y == 0) {
                            ui_selected_control += 12;
                        } else {
                            ui_selected_control -= 4;
                        }
                    } else if (KEY_PRESSED(1)) {
                        if (y == 3) {
                            ui_selected_control -= 12;
                        } else {
                            ui_selected_control += 4;
                        }
                    } else if (KEY_PRESSED(2)) {
                        if (x == 0) {
                            if (y == 0) {
                                ui_selected_control = 0;
                            } else if (y == 1) {
                                ui_selected_control = 1;
                            } else if (y == 2) {
                                ui_selected_control = 1;
                            } else {
                                ui_selected_control = 2;
                            }
                        } else {
                            ui_selected_control--;
                        }
                    } else if (KEY_PRESSED(3)) {
                        if (x == 3) {
                            if (y == 0) {
                                ui_selected_control = 0;
                            } else if (y == 1) {
                                ui_selected_control = 1;
                            } else if (y == 2) {
                                ui_selected_control = 1;
                            } else {
                                ui_selected_control = 2;
                            }
                        } else {
                            ui_selected_control++;
                        }
                    }
                }
                ui_state_set(UI_STATE_GAME_SETTINGS_1);
            }
        }
    } else if (ui_state == UI_STATE_COLOR_SEL_0) {
        if (ui_first()) {
            gfx_SetColor(color_ui_fg_1);
            gfx_Rectangle_NoClip(65, 65, 190, 110);
            ui_mask_color();
            gfx_FillRectangle_NoClip(66, 66, 188, 108);
            gfx_PrintStringXY("Color Select", 119, 70);
            gfx_PrintStringXY("[\x1e / \x1f / \x1d]", 141, 139);
            gfx_PrintStringXY("[2ND]", 169, 151);
            gfx_PrintStringXY("[ENTER]", 153, 163);
            gfx_PrintStringXY("Select", 206, 139);
            gfx_PrintStringXY("Cancel", 206, 151);
            gfx_PrintStringXY("OK", 206, 163);
            ui_palette();
        } else {
            ui_state_set_imm(UI_STATE_COLOR_SEL_1);
        }
    } else if (ui_state == UI_STATE_COLOR_SEL_1) {
        if (ui_first()) {
            ui_mask_color();
            ui_palette();
        }
        if (KEY_PRESSED(5)) {
            ui_state_set(ui_previous_state);
            ui_color_return = 0;
        } else if (KEY_PRESSED(4)) {
            ui_state_set(ui_previous_state);
        }
        if (KEY_PRESSED_ARROW()) {
            if (ui_selected_color < 16) {
                uint8_t shift_idx = 0;
                if (KEY_PRESSED(0)) {
                    shift_idx = ui_selected_color;
                    shift_idx += (ui_selected_color >> 1);
                    ui_selected_color = shift_idx + 232;
                } else if (KEY_PRESSED(1)) {
                    shift_idx = (ui_selected_color / 3) * 36;
                    ui_selected_color = shift_idx + 16;
                } else if (KEY_PRESSED(2)) {
                    if (ui_selected_color == 0) {
                        ui_selected_color = 15;
                    } else {
                        ui_selected_color--;
                    }
                } else if (KEY_PRESSED(3)) {
                    if (ui_selected_color == 15) {
                        ui_selected_color = 0;
                    } else {
                        ui_selected_color++;
                    }
                }
            } else if (ui_selected_color < 232) {
                uint8_t cube_z = 0;
                uint8_t cube_y = 0;
                uint8_t cube_x = ui_selected_color - 16;
                while (cube_x >= 36) {
                    cube_x -= 36;
                    cube_z++;
                }
                while (cube_x >= 6) {
                    cube_x -= 6;
                    cube_y++;
                }
                if (KEY_PRESSED(0)) {
                    if (cube_y == 0) {
                        ui_selected_color = cube_z * 3;
                    } else {
                        ui_selected_color -= 6;
                    }
                } else if (KEY_PRESSED(1)) {
                    if (cube_y == 5) {
                        ui_selected_color = 232 + cube_z * 4;
                    } else {
                        ui_selected_color += 6;
                    }
                } else if (KEY_PRESSED(2)) {
                    if (cube_x == 0) {
                        if (cube_z == 0) {
                            ui_selected_color += 185;
                        } else {
                            ui_selected_color -= 36;
                        }
                    } else {
                        ui_selected_color--;
                    }
                } else if (KEY_PRESSED(3)) {
                    if (cube_x == 5) {
                        if (cube_z == 5) {
                            ui_selected_color -= 185;
                        } else {
                            ui_selected_color += 36;
                        }
                    } else {
                        ui_selected_color++;
                    }
                }
            } else {
                uint8_t shift_idx = 0;
                if (KEY_PRESSED(0)) {
                    shift_idx = ui_selected_color - 232;
                    shift_idx += ((ui_selected_color - 232) >> 1);
                    ui_selected_color = ((shift_idx / 6) * 36) + 49;
                } else if (KEY_PRESSED(1)) {
                    shift_idx = ui_selected_color - 232;
                    if (shift_idx < 4) {
                        shift_idx = 0;
                    } else if (shift_idx > 19) {
                        shift_idx = 15;
                    } else {
                        shift_idx -= 4;
                    }
                    ui_selected_color = shift_idx;
                } else if (KEY_PRESSED(2)) {
                    if (ui_selected_color == 232) {
                        ui_selected_color = 255;
                    } else {
                        ui_selected_color--;
                    }
                } else if (KEY_PRESSED(3)) {
                    if (ui_selected_color == 255) {
                        ui_selected_color = 232;
                    } else {
                        ui_selected_color++;
                    }
                }
            }
            ui_state_set(UI_STATE_COLOR_SEL_1);
        }        
    } else if (ui_state == UI_STATE_KEY_SEL_0) {
        if (ui_first()) {
            char buf[6];
            struct game* gm = &scan_games[ui_selected_game];
            ui_enter_pressed = 1;
            gfx_SetColor(color_ui_fg_1);
            gfx_Rectangle_NoClip(82, 82, 156, 76);
            ui_mask_color();
            gfx_FillRectangle_NoClip(83, 83, 154, 74);
            gfx_PrintStringXY("Key Select", 125, 86);
            gfx_PrintStringXY("[2ND]", 152, 146);
            gfx_PrintStringXY("Cancel", 189, 146);
            buf[0] = "0123456789ABCDEF"[ui_key_return - 1];
            buf[1] = '\0';
            gfx_PrintStringXY(buf, 157, 99);
            strncpy(buf, &ui_key_names[(ui_selected_key & 0x3F) << 2], 4);
            buf[4] = '\0';
            gfx_PrintStringXY(buf, 160 - (gfx_GetStringWidth(buf) >> 1), 108);
            gfx_SetColor(color_ui_fg_1);
            gfx_Rectangle_NoClip(142, 97, 36, 20);
            //gfx_PrintStringXY("[\x1e / \x1f / \x1d]", 141, 139);
            //gfx_PrintStringXY("[2ND]", 169, 151);
            //gfx_PrintStringXY("[ENTER]", 153, 163);
            //gfx_PrintStringXY("Select", 206, 139);
            //gfx_PrintStringXY("Cancel", 206, 151);
            //gfx_PrintStringXY("OK", 206, 163);
        } else {
            ui_state_set(UI_STATE_KEY_SEL_1);
        }
    } else if (ui_state == UI_STATE_KEY_SEL_1) {
        if (ui_first()) {
            char buf[6];
            struct game* gm = &scan_games[ui_selected_game];
            ui_mask_color();
            gfx_FillRectangle_NoClip(143, 107, 34, 9);
            buf[0] = "0123456789ABCDEF"[ui_key_return - 1];
            buf[1] = '\0';
            gfx_PrintStringXY(buf, 157, 99);
            strncpy(buf, &ui_key_names[(ui_selected_key & 0x3F) << 2], 4);
            buf[4] = '\0';
            gfx_PrintStringXY(buf, 160 - (gfx_GetStringWidth(buf) >> 1), 108);
            gfx_PrintStringXY("Press a key", 121, 121);
            gfx_SetColor(color_ui_select);
            gfx_Rectangle_NoClip(140, 95, 40, 24);
        } else {
            uint8_t i, j;
            if (KEY_PRESSED(5)) {
                ui_key_return = 0;
                ui_state_set(ui_previous_state);
            } else if (ui_enter_pressed) {
                if (!KEY_HELD(4)) {
                    ui_enter_pressed = 0;
                }
            } else {
                uint8_t bflag = 0;
                for (i = 1; i < 8; i++) {
                    for (j = 0; j < 8; j++) {
                        if (kb_Data[i] & (1 << j)) {
                            ui_selected_key = ((i << 3) | j);
                            ui_state_set(ui_previous_state);
                            bflag = 1;
                            break;
                        }
                    }
                    if (bflag) {
                        break;
                    }
                }
            }
        }
    }
    gfx_SwapDraw();
    ui_runind_state++;
    ui_runind_state &= 0x1F;
    return 0;
}

#undef KEY_PRESSED
#undef KEY_HELD
#undef KEY_PRESSED_ARROW