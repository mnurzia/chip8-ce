/*
 *--------------------------------------
 * Program Name: CHIP8
 * Author: Max Nurzia
 * License: MIT
 * Description: Chip-8 Emulator
 *--------------------------------------
*/

/* Keep these headers */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>
#include <debug.h>
#include <graphx.h>
#include <intce.h>
#include <keypadc.h>

/* Standard headers (recommended) */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "colors.h"
#include "core.h"
#include "input.h"
#include "player.h"
#include "scan.h"
#include "video.h"
#include "tests.h"
#include "ui.h"

void main(void) {
/*     colors_reset();
    gfx_Begin();
    gfx_SetDrawBuffer();
    kb_SetMode(MODE_1_INDISCRIMINATE);
#ifdef TESTS
    tests_runall();
#else
    color_player_low = 0xC0;
    color_player_high = 0xD0;
    colors_player_set();
    player_run(game, sizeof(game), 1);
#endif
    kb_Reset();
    gfx_End(); */
/*     int retval = 0;
    int i = 0;
    ti_CloseAll();
    scan_reset();
    while (1) {
        retval = scan_step();
        if (retval) {
            break;
        } else {
            dbg_sprintf(dbgout, "found a game pack\n", retval);
        }
    }
    for (i = 0; i < scan_games_count; i++) {
        dbg_sprintf(dbgout, "Game: %.24s\n", scan_games[i].name);
        dbg_sprintf(dbgout, "\tAuthor: %.24s\n", scan_games[i].auth);
        dbg_sprintf(dbgout, "\tYear: %u\n", scan_games[i].year);
        dbg_sprintf(dbgout, "\tFG: %02X\n", scan_games[i].fg);
        dbg_sprintf(dbgout, "\tBG: %02X\n", scan_games[i].bg);
        dbg_sprintf(dbgout, "\tSize: %04X\n", scan_games[i].size);
    }
    return; */
    /* core_reset();
    video_reset();
    gfx_Begin();
    gfx_SetDrawBuffer();
    video_blit_dest = *gfx_vbuffer;
    core_memory[0] = 0xD5;
    core_memory[1] = 0x66;
    core_memory[16] = 0xBA;
    core_memory[17] = 0x7C;
    core_memory[18] = 0xD6;
    core_memory[19] = 0xFE;
    core_memory[20] = 0x54;
    core_memory[21] = 0xAA;
    core_reg_i = 0x10;
    core_reg_v[5] = 0;
    core_reg_v[6] = 0;
    dbg_Debugger();
    core_instruction();
    dbg_sprintf(dbgout, "------------------------------\n");
    dbg_sprintf(dbgout, "PC: %03X\n", core_reg_pc);
    dbg_sprintf(dbgout, "I:  %03X\n", core_reg_i);
    for (i = 0; i < 16; i++) {
        dbg_sprintf(dbgout, "R%X: %02X | ", i, core_reg_v[i]);
        if (i == 3 || i == 7 || i == 11 || i == 15) {
            dbg_sprintf(dbgout, "\n");
        }
    }
    dbg_Debugger();
    video_blit_1x();
    dbg_Debugger();
    gfx_SwapDraw();
    while (!os_GetCSC()) {;}
    gfx_End(); */
    ti_CloseAll();
    gfx_Begin();
    gfx_SetDrawBuffer();
    kb_SetMode(MODE_1_INDISCRIMINATE);
    scan_reset();
    while (!ui_frame());
    kb_Reset();
    gfx_End();
    ti_CloseAll();
}


/* Put other functions here */
