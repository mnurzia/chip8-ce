#include "tests.h"

#include <stdint.h>

#include <debug.h>

#include "core.h"
#include "dbg.h"
#include "video.h"
#include "player.h"

#ifdef TESTS

uint8_t test_contents[128];
uint8_t test_ptr = 0;

void tests_reset() {
    int i;
    for (i = 0; i < (sizeof(test_contents)/sizeof(uint8_t)); i++) {
        test_contents[i] = 0x0;
    }
    test_ptr = 0;
}

static void test(char* ins, char* spec, int (*fn)()) {
    int res;
    tests_reset();
    res = fn();
    if (res == 1) {
        dbg_sprintf(dbgout, "[PASS] %6s -- %s\n", ins, spec);
    } else {
        dbg_sprintf(dbgout, "[FAIL] %6s -- %s\n", ins, spec);
    }
}

#define PROBE(a) dbg_sprintf(dbgout, "[PRBE] %s\n", (a))
#define RUNFOR(a) player_ipf = (a)
#define INS(a) test_contents[test_ptr++] = ((a) >> 8); test_contents[test_ptr++] = ((a) & 0xFF)
#define DAT(a) test_contents[test_ptr++] = ((a) >> 8); test_contents[test_ptr++] = ((a) & 0xFF)
#define EXIT() INS(0x00FD)
#define RUN() player_run(&(test_contents[0]), test_ptr, 1)
#define RUN_EX() player_run(&(test_contents[0]), test_ptr, 0)
#define RESET() core_reset(); video_reset()
#define MEM(a, b) if (core_memory[a] != b) { return 0; }
#define PC(a) if (core_reg_pc != a) { return 0; }
#define FLAGS(a) if (core_flags != a) { return 0; }
#define SP(a) if (core_reg_sp != a) { return 0; }
#define STACK(a, b) if (core_stack[a] != b) { return 0; }
#define REG(a, b) if (core_reg_v[a] != b) { return 0; }
#define IR(a) if (core_reg_i != a) { return 0; }
#define DT(a) if (core_timer_delay != a) { return 0; }
#define ST(a) if (core_timer_sound != a) { return 0; }

static int test_cls() {
    int i;
    
    INS(0x00E0); 
    EXIT();
    
    RUNFOR(2);
    RUN();
    
    PC(0x204);
    
    for (i=0; i<2048; i++) {
        if (video_memory[i] != 0) {
            return 0;
        }
    }
    
    return 1;
}

static int test_ret_default() {
    INS(0x00EE);
    EXIT();
    
    RUNFOR(1);
    RESET();
    core_reg_sp = 2;
    core_stack[0] = 0x0CD0;
    RUN_EX();
    
    PC(0xCD0);
    SP(0x00);
    
    return 1;
}

static int test_ret_underflow() {
    INS(0x00EE);
    EXIT();
    
    RUNFOR(2);
    RUN();
    
    PC(0x200);
    FLAGS(CORE_FLAG_STACK_UNDERFLOW);
    SP(0x00);
    
    return 1;
}

static int test_ret_edge() {
    INS(0x00EE); 
    EXIT();
    
    RUNFOR(1);
    RESET();
    core_reg_sp = 2;
    core_stack[0] = 0x0FFF;
    RUN_EX();
    
    PC(0xFFF);
    SP(0x00);
    
    return 1;
}

static int test_jp() {
    INS(0x1CD0);
    EXIT();
    
    RUNFOR(1);
    RUN();
    
    PC(0xCD0);
    
    return 1;
}

static int test_call_default() {
    INS(0x2CD0);
    EXIT();
    
    RUNFOR(1);
    RUN();
    
    PC(0xCD0);
    SP(0x02);
    STACK(0, 0x202);
    
    return 1;
}

static int test_call_overflow() {
    INS(0x2CD0);
    EXIT();
    
    RUNFOR(1);
    RESET();
    core_reg_sp = 0x40;
    RUN_EX();
    
    PC(0x200);
    FLAGS(CORE_FLAG_STACK_OVERFLOW);
    
    return 1;
}

static int test_sei_skip() {
    INS(0x3000);
    INS(0x00EE);
    EXIT();
    
    RUNFOR(2);
    RUN();
    
    PC(0x206);
    
    return 1;
}

static int test_sei_continue() {
    INS(0x3000);
    EXIT();
    INS(0x00EE);
    
    RUNFOR(2);
    RUN();
    
    PC(0x204);
    
    return 1;
}

static int test_snei_skip() {
    INS(0x40FF);
    INS(0x00EE);
    EXIT();
    
    RUNFOR(2);
    RUN();
    
    PC(0x206);
    
    return 1;
}

static int test_snei_continue() {
    INS(0x40FF);
    EXIT();
    INS(0x00EE);
    
    RUNFOR(2);
    RUN();
    
    PC(0x204);
    
    return 1;
}

static int test_ser_skip() {
    INS(0x5010);
    INS(0x00EE);
    EXIT();
    
    RUNFOR(2);
    RUN();
    
    PC(0x206);
    
    return 1;
}

static int test_ser_continue() {
    INS(0x61CC);
    INS(0x5010);
    EXIT();
    INS(0x00EE);
    
    RUNFOR(3);
    RUN();
    
    PC(0x206);
    
    return 1;
}

static int test_ldi() {
    INS(0x6CEF);
    EXIT();
    
    RUNFOR(2);
    RUN();
    
    PC(0x204);
    REG(0xC, 0xEF);
    
    return 1;
}

static int test_addi() {
    INS(0x64CC);
    INS(0x741F);
    EXIT();
    
    RUNFOR(3);
    RUN();
    
    PC(0x206);
    REG(0x4, 0xEB);
    
    return 1;
}    

static int test_ldr() {
    INS(0x63CC);
    INS(0x641F);
    INS(0x8340);
    EXIT();
    
    RUNFOR(4);
    RUN();
    
    PC(0x208);
    REG(0x3, 0x1F);
    
    return 1;
}

static int test_or() {
    INS(0x63CC);
    INS(0x641F);
    INS(0x8341);
    EXIT();
    
    RUNFOR(4);
    RUN();
    
    PC(0x208);
    REG(0x3, 0xDF);
    
    return 1;
}

static int test_and() {
    INS(0x63CC);
    INS(0x641F);
    INS(0x8342);
    EXIT();
    
    RUNFOR(4);
    RUN();
    
    PC(0x208);
    REG(0x3, 0x0C);
    
    return 1;
}

static int test_xor() {
    INS(0x63CC);
    INS(0x641F);
    INS(0x8343);
    EXIT();
    
    RUNFOR(4);
    RUN();
    
    PC(0x208);
    REG(0x3, 0xD3);
    
    return 1;
}

static int test_add_default() {
    INS(0x63CC);
    INS(0x641F);
    INS(0x8344);
    EXIT();
    
    RUNFOR(4);
    RUN();
    
    PC(0x208);
    REG(0x3, 0xEB);
    REG(0xF, 0x0);
    
    return 1;
}

static int test_add_carry() {
    INS(0x63FF);
    INS(0x6401);
    INS(0x8344);
    EXIT();
    
    RUNFOR(4);
    RUN();
    
    PC(0x208);
    REG(0x3, 0x00);
    REG(0xF, 0x1);
    
    return 1;
}

static int test_sub_default() {
    INS(0x63CC);
    INS(0x641F);
    INS(0x8345);
    EXIT();
    
    RUNFOR(4);
    RUN();
    
    PC(0x208);
    REG(0x3, 0xAD);
    REG(0xF, 0x1);
    
    return 1;
}

static int test_sub_borrow() {
    INS(0x6300);
    INS(0x64FF);
    INS(0x8345);
    EXIT();
    
    RUNFOR(4);
    RUN();
    
    PC(0x208);
    REG(0x3, 0x1);
    REG(0xF, 0x0);
    
    return 1;
}

static int test_shr_default() {
    INS(0x63CC);
    INS(0x8306);
    EXIT();
    
    RUNFOR(3);
    RUN();
    
    PC(0x206);
    REG(0x3, 0x66);
    REG(0xF, 0x0);
    
    return 1;
}

static int test_shr_lsb() {
    INS(0x6301);
    INS(0x8306);
    EXIT();
    
    RUNFOR(3);
    RUN();
    
    PC(0x206);
    REG(0x3, 0x00);
    REG(0xF, 0x1);
    
    return 1;
}

static int test_subn_default() {
    INS(0x63CC);
    INS(0x641F);
    INS(0x8347);
    EXIT();
    
    RUNFOR(4);
    RUN();
    
    PC(0x208);
    REG(0x3, 0xAD);
    REG(0xF, 0x1);
    
    return 1;
}

static int test_subn_borrow() {
    INS(0x6300);
    INS(0x64FF);
    INS(0x8347);
    EXIT();
    
    RUNFOR(4);
    RUN();
    
    PC(0x208);
    REG(0x3, 0x1);
    REG(0xF, 0x0);
    
    return 1;
}

static int test_shl_default() {
    INS(0x6301);
    INS(0x830E);
    EXIT();
    
    RUNFOR(3);
    RUN();
    
    PC(0x206);
    REG(0x3, 0x02);
    REG(0xF, 0x0);
    
    return 1;
}

static int test_shl_msb() {
    INS(0x63CC);
    INS(0x830E);
    EXIT();
    
    RUNFOR(3);
    RUN();
    
    PC(0x206);
    REG(0x3, 0x98);
    REG(0xF, 0x1);
    
    return 1;
}

static int test_sner_skip() {
    INS(0x61CC);
    INS(0x9120);
    INS(0x00EE);
    EXIT();
    
    RUNFOR(3);
    RUN();
    
    PC(0x208);
    
    return 1;
}

static int test_sner_continue() {
    INS(0x9120);
    EXIT();
    INS(0x00EE);
    
    RUNFOR(3);
    RUN();
    
    PC(0x204);
    
    return 1;
}

static int test_ldii() {
    INS(0xACDC);
    EXIT();
    
    RUNFOR(2);
    RUN();
    
    PC(0x204);
    IR(0xCDC);
    
    return 1;
}

static int test_jpo() {
    INS(0x6033);
    INS(0xB6EF);
    
    RUNFOR(2);
    RUN();
    
    PC(0x722);
    
    return 1;
}

static int test_jpo_edge() {
    INS(0x60EE);
    INS(0xBFFC);
    
    RUNFOR(2);
    RUN();
    
    PC(0xEA);
    
    return 1;
}

static int test_rnd() {
    INS(0xC70A);
    INS(0xC8BB);
    INS(0xC980);
    EXIT();
    
    RUNFOR(4);
    RUN();
    
    PC(0x208);
    REG(0x7, 0x0A);
    REG(0x8, 0x01);
    REG(0x9, 0x80);
    
    return 1;
}

// these were checked visually...
static int test_drw() {
    INS(0x120A);
    DAT(0xAAAA);
    DAT(0x5555);
    DAT(0xAAAA);
    DAT(0x5555);
    INS(0xA202);
    INS(0xD008);
    EXIT();
    
    RUNFOR(4);
    RUN();
 
    PC(0x210);
    
    return 1;
}

static int test_drw_clip_x() {
    INS(0x120A);
    DAT(0xAAAA);
    DAT(0x5555);
    DAT(0xAAAA);
    DAT(0x5555);
    INS(0x603B);
    INS(0xA202);
    INS(0xD018);
    EXIT();
    
    RUNFOR(5);
    RUN();
 
    PC(0x212);
    
    return 1;
}

static int test_drw_clip_y() {
    INS(0x120A);
    DAT(0xAAAA);
    DAT(0x5555);
    DAT(0xAAAA);
    DAT(0x5555);
    INS(0x611B);
    INS(0xA202);
    INS(0xD018);
    EXIT();
    
    RUNFOR(5);
    RUN();
    
    PC(0x212);
    
    return 1;
}

static int test_drw_clip_xy() {
    INS(0x120A);
    DAT(0xAAAA);
    DAT(0x5555);
    DAT(0xAAAA);
    DAT(0x5555);
    INS(0x603B);
    INS(0x611B);
    INS(0xA202);
    INS(0xD018);
    EXIT();
    
    RUNFOR(6);
    RUN();
 
    PC(0x214);
    
    return 1;
}

static int test_drw_flag() {
    INS(0x120A);
    DAT(0xAAAA);
    DAT(0x5555);
    DAT(0xAAAA);
    DAT(0x5555);
    INS(0x6001);
    INS(0xA202);
    INS(0xD008);
    INS(0xD118);
    EXIT();
    
    RUNFOR(7);
    RUN();
    
    PC(0x214);
    REG(0xF, 1);
    
    return 1;
}

static int test_skp_skip() {
    INS(0x6C04);
    INS(0xEC9E);
    INS(0xB202);
    EXIT();
    
    RUNFOR(3);
    RUN();
    
    PC(0x208);
    
    return 1;
}

static int test_skp_continue() {
    INS(0x6C04);
    INS(0xEC9E);
    EXIT();
    INS(0xB202);
    
    RUNFOR(3);
    RUN();
    
    PC(0x206);
    
    return 1;
}

static int test_sknp_skip() {
    INS(0x6C04);
    INS(0xECA1);
    INS(0xB202);
    EXIT();
    
    RUNFOR(3);
    RUN();
    
    PC(0x208);
    
    return 1;
}

static int test_sknp_continue() {
    INS(0x6C04);
    INS(0xECA1);
    EXIT();
    INS(0xB202);
    
    RUNFOR(3);
    RUN();
    
    PC(0x206);
    
    return 1;
}

static int test_lddt() {
    INS(0xF507);
    EXIT();
    
    RUNFOR(1);
    RESET();
    core_timer_delay = 0x05;
    RUN_EX();
    
    PC(0x204);
    REG(5, 0x05);
    DT(0x03);
    
    return 1;
}

// press 4
static int test_wkey() {
    INS(0xF50A);
    EXIT();
    
    RUNFOR(2);
    RUN();
    
    PC(0x204);
    REG(5, 0x04);
    
    return 1;
}

static int test_ldrdt() {
    INS(0x6533);
    INS(0xF515);
    EXIT();
    
    RUNFOR(1);
    RUN();
    
    PC(0x206);
    DT(0x31);
    
    return 1;
}

static int test_ldrst() {
    INS(0x6533);
    INS(0xF518);
    EXIT();
    
    RUNFOR(1);
    RUN();
    
    PC(0x206);
    ST(0x31);
    
    return 1;
}

static int test_addri() {
    INS(0xACCC);
    INS(0x631B);
    INS(0xF31E);
    EXIT();
    
    RUNFOR(4);
    RUN();
    
    PC(0x208);
    IR(0xCE7);
    
    return 1;
}

static int test_fnt() {
    INS(0x6305);
    INS(0xF329);
    EXIT();
    
    RUNFOR(3);
    RUN();
    
    PC(0x206);
    IR(0x019);
    
    return 1;
}

static int test_bcd() {
    INS(0x63DA);
    INS(0xA500);
    INS(0xF333);
    EXIT();
    
    RUNFOR(4);
    RUN();
    
    PC(0x208);
    MEM(0x500, 2);
    MEM(0x501, 1);
    MEM(0x502, 8);
    
    return 1;
}

static int test_sri() {
    INS(0x60CC);
    INS(0x6FFF);
    INS(0xA500);
    INS(0xFF55);
    EXIT();
    
    RUNFOR(4);
    RUN();
    
    PC(0x20A);
    MEM(0x500, 0xCC);
    MEM(0x50F, 0xFF);
    
    return 1;
}

static int test_lri() {
    INS(0x1206);
    DAT(0xFF55);
    DAT(0xCCAA);
    INS(0xA202);
    INS(0xF365);
    EXIT();
    
    RUNFOR(4);
    RUN();
    
    dbg_out();
    
    PC(0x20C);
    REG(0, 0xFF);
    REG(1, 0x55);
    REG(2, 0xCC);
    REG(3, 0xAA);
    
    return 1;
}

#undef PROBE
#undef RUNFOR
#undef INS
#undef DAT
#undef EXIT
#undef RUN
#undef RUN_EX
#undef RESET
#undef MEM
#undef PC
#undef FLAGS
#undef SP
#undef STACK
#undef REG
#undef IR
#undef DT
#undef ST

void tests_runall() {
/*     test("CLS", "video memory clear", test_cls);
    test("RET", "return from subroutine", test_ret_default);
    test("RET", "force stack underflow", test_ret_underflow);
    test("RET", "return to memory edge", test_ret_edge);
    test("JP", "jump to address", test_jp);
    test("CALL", "call subroutine", test_call_default);
    test("CALL", "force stack overflow", test_call_overflow);
    test("SEI", "skip next instruction", test_sei_skip);
    test("SEI", "continue next instruction", test_sei_continue);
    test("SNEI", "skip next instruction", test_snei_skip);
    test("SNEI", "continue next instruction", test_snei_continue);
    test("SER", "skip next instruction", test_ser_skip);
    test("SER", "continue next instruction", test_ser_continue);
    test("LDI", "load value into register", test_ldi);
    test("ADDI", "add value to register", test_addi);
    test("LDR", "load register to other register", test_ldr);
    test("OR", "or register to other register", test_or);
    test("AND", "and register to other register", test_and);
    test("XOR", "xor register to other register", test_xor);
    test("ADD", "add register to register", test_add_default);
    test("ADD", "add register to register, producing carry", test_add_carry);
    test("SUB", "subtract register from register", test_sub_default);
    test("SUB", "subtract register from register, producing borrow", test_sub_borrow);
    test("SHR", "shift right register", test_shr_default);
    test("SHR", "shift right register, producing carry", test_shr_lsb);
    test("SUBN", "subtract register from register, producing not borrow", test_subn_default);
    test("SUBN", "subtract register from register", test_subn_borrow);
    test("SHL", "shift left register", test_shl_default);
    test("SHL", "shift left register, producing carry", test_shl_msb);
    test("SNER", "skip next instruction", test_sner_skip);
    test("SNER", "continue next instruction", test_sner_continue);
    test("LDII", "load value into I", test_ldii);
    test("JPO", "jump to v0 + offset", test_jpo);
    test("JPO", "jump to v0 + offset, memory edge", test_jpo_edge);
    test("RND", "random number LFSR", test_rnd);
    test("DRW", "draw sprite 0, 0", test_drw);
    test("DRW", "draw sprite 59, 0 (clip)", test_drw_clip_x);
    test("DRW", "draw sprite 0, 27 (clip)", test_drw_clip_y);
    test("DRW", "draw sprite 59, 27 (clip)", test_drw_clip_xy);
    test("DRW", "draw sprite, set flag", test_drw_flag);
    test("SKP", "skip next instruction", test_skp_skip);
    test("SKP", "continue next instruction", test_skp_continue);
    test("SKNP", "skip next instruction", test_sknp_skip);
    test("SKNP", "continue next instruction", test_sknp_continue);
    test("LDDT", "load delay timer to register", test_lddt);
    test("WKEY", "wait for key", test_wkey);
    test("LDRDT", "load register to delay timer", test_ldrdt);
    test("LDRST", "load register to sound timer", test_ldrst);
    test("ADDRI", "add register to I", test_addri);
    test("FNT", "load font address to I", test_fnt);
    test("BCD", "load register contents as bcd to I -> I+2", test_bcd);
    test("SRI", "store registers to I -> I+0xF", test_sri);
    test("LRI", "load I -> I+0x3 to registers", test_lri); */
}

#endif