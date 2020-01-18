#include <debug.h>

#include "core.h"

#include "dbg.h"

void dbg_out() {
    int i;
    dbg_sprintf(dbgout, "------------------------------\n");
    dbg_sprintf(dbgout, "PC: %03X DT: %02X\n", core_reg_pc, core_timer_delay);
    dbg_sprintf(dbgout, "I:  %03X ST: %02X\n", core_reg_i, core_timer_sound);
    dbg_sprintf(dbgout, "F:  %02X  K:  %01X\n", core_flags, core_reg_key);
    dbg_sprintf(dbgout, "SP: %02X  SR: %02X\n", core_reg_sp, core_lfsr);
    for (i = 0; i < 16; i++) {
        dbg_sprintf(dbgout, "R%X: %02X | ", i, core_reg_v[i]);
        if (i == 3 || i == 7 || i == 11 || i == 15) {
            dbg_sprintf(dbgout, "\n");
        }
    }
    dbg_sprintf(dbgout, "------------------------------\n");
    dbg_Debugger();
}