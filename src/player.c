#include <stdint.h>
#include <string.h>

#include <debug.h>
#include <graphx.h>

#include "colors.h"
#include "core.h"
#include "input.h"
#include "player.h"
#include "video.h"

#include "dbg.h"

uint16_t player_ipf = 15;
uint8_t player_state = 0;

void player_reset() {
    player_state = 0;
}

uint8_t player_frame() {
    if (player_state == 0) {
        core_instruction(player_ipf);
        if (core_flags & CORE_EXIT_FLAGS) {
            return 1;
        } else if (core_flags & CORE_FLAG_WAIT) {
            player_state = 1;
        }
    } else if (player_state == 1) {
        if (video_mode != 5) {
            if (input_wait()) {
                core_flags &= ~(CORE_FLAG_WAIT);
                player_state = 0;
            }
        }
    }
    video_refresh();
    video_draw();
    if (video_mode != 5) {
        input_scan(0);
        if (input_exit) {
            return 1;
        }
    }
    return 0;
}

uint8_t player_load(struct game* gm) {
    uint8_t should_exit = 0;
    uint16_t frame_ins = 0;
    int i = 0;
    char var_n_buf[9] = { 0 };
    int seek_val = 0;
    size_t read_sz = 0;
    size_t read_out = 0;
    ti_var_t var = 0;
    for (i = 0; i < 8; i++) {
        var_n_buf[i] = gm->var[i];
    }
    var = ti_Open(var_n_buf, "r");
    if (!var) { 
        goto os_error; 
    }
    
    seek_val = ti_Seek(gm->var_off, SEEK_SET, var);
    if (seek_val == EOF) {
        goto early_eof;
    }
    
    input_reset();
    core_reset();
    video_reset();
    video_refresh();
    
    read_sz = gm->size;
    read_out = ti_Read(&(core_memory[0x200]), 1, read_sz, var);
    if (read_sz != read_out) {
        goto early_eof;
    }
    
    player_ipf = gm->speed;
    core_reg_pc = 0x200;
    color_player_low = gm->bg;
    color_player_high = gm->fg;
    colors_player_set();
    memcpy(input_mapping, gm->mapping, 16);
    
    if (!ti_Close(var)) {
        var = 0;
        goto os_error;
    }
    
    return 0;
os_error:
    if (var) {
        ti_Close(var);
    }
    return 2;
early_eof:
    if (var) {
        ti_Close(var);
    }
    return 4;
}
