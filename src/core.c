#include <stdint.h>

#include "core.h"

uint8_t     core_memory[4096];
uint8_t     core_reg_v[16];
uint16_t    core_reg_i;
uint16_t    core_reg_pc;
uint8_t     core_timer_delay;
uint8_t     core_timer_sound;
uint16_t    core_stack[32];
uint8_t     core_reg_sp;
uint8_t     core_flags;
uint16_t    core_lfsr;
uint8_t     core_reg_key;
uint8_t     core_font[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,
    0x20, 0x60, 0x20, 0x20, 0x70,
    0xF0, 0x10, 0xF0, 0x80, 0xF0,
    0xF0, 0x10, 0xF0, 0x10, 0xF0,
    0x90, 0x90, 0xF0, 0x10, 0x10,
    0xF0, 0x80, 0xF0, 0x10, 0xF0,
    0xF0, 0x80, 0xF0, 0x90, 0xF0,
    0xF0, 0x10, 0x20, 0x40, 0x40,
    0xF0, 0x90, 0xF0, 0x90, 0xF0,
    0xF0, 0x90, 0xF0, 0x10, 0xF0,
    0xF0, 0x90, 0xF0, 0x90, 0x90,
    0xE0, 0x90, 0xE0, 0x90, 0xE0,
    0xF0, 0x80, 0x80, 0x80, 0xF0,
    0xE0, 0x90, 0x90, 0x90, 0xE0,
    0xF0, 0x80, 0xF0, 0x80, 0xF0,
    0xF0, 0x80, 0xF0, 0x80, 0x80
};

void core_reset() {
    unsigned int i;
    for (i = 0; i < 4096; i++) {
        core_memory[i] = 0;
    }
    for (i = 0; i < 80; i++) {
        core_memory[i] = core_font[i];
    }
    for (i = 0; i < 16; i++) {
        core_reg_v[i] = 0;
    }
    core_reg_i = 0;
    core_reg_pc = 0;
    core_timer_delay = 0;
    core_timer_sound = 0;
    for (i = 0; i < 16; i++) {
        core_stack[i] = 0;
    }
    core_reg_sp = 0;
    core_flags = 0;
    core_lfsr = 0xB416;
    core_reg_key = 0;
}

void core_put16(uint16_t addr, uint16_t d) {
    core_memory[addr+1] = d & 0xFF;
    core_memory[addr] = d >> 8;
}

void core_put8(uint16_t addr, uint8_t d) {
    core_memory[addr] = d;
}