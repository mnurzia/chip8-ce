#include <keypadc.h>

#include "core.h"
#include "player.h"

#include "input.h"

#define SET_MAP(ch, off, key) input_mapping[ch] = ((off) << 3) | (highest_bit((key)))

uint8_t input_mapping[16];
uint8_t input_keys[16];
uint8_t input_exit;

static uint8_t highest_bit(uint8_t a) {
    uint8_t out = 255;
    while (a) {
        a >>= 1;
        out++;
    }
    return out;
}

void input_reset() {
    uint8_t i = 0;
    for (;i<16;i++) {
        input_keys[i] = 0;
    }
    input_exit = 0;
    SET_MAP(0x0, 4, kb_DecPnt);
    SET_MAP(0x1, 3, kb_7);
    SET_MAP(0x2, 4, kb_8);
    SET_MAP(0x3, 5, kb_9);
    SET_MAP(0x4, 3, kb_4);
    SET_MAP(0x5, 4, kb_5);
    SET_MAP(0x6, 5, kb_6);
    SET_MAP(0x7, 3, kb_1);
    SET_MAP(0x8, 4, kb_2);
    SET_MAP(0x9, 5, kb_3);
    SET_MAP(0xA, 3, kb_0);
    SET_MAP(0xB, 5, kb_Chs);
    SET_MAP(0xC, 6, kb_Mul);
    SET_MAP(0xD, 6, kb_Sub);
    SET_MAP(0xE, 6, kb_Add);
    SET_MAP(0xF, 6, kb_Enter);
}

uint8_t input_scan(uint8_t write) {
    uint8_t i = 0;
    uint8_t bitidx;
    uint8_t offset;
    uint8_t ispressed = 0;
    kb_key_t curkey;
    kb_Scan();
    curkey = kb_Data[1];
    if (curkey & kb_2nd) {
        input_exit = 1;
        return 1;
    }
    for (;i<16;i++) {
        input_keys[i] = 0;
        offset = (input_mapping[i] & 0x38) >> 3;
        bitidx = (input_mapping[i] & 0x07);
        curkey = kb_Data[offset];
        if (curkey & (1 << bitidx)) {
            if (write) {
                core_reg_v[core_reg_key] = i;
            }
            input_keys[i] = 1;
            ispressed = 1;
        }
    }
    return ispressed;
}

uint8_t input_wait() {
    if (kb_AnyKey()) {
        return input_scan(1);
    }
    return 0;
}