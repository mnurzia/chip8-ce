/* core.h - Emulator internals, linked with core.asm */

#ifndef CORE_H
#define CORE_H

#include <stdint.h>

/* Flag set (could have been an enum) */
#define CORE_FLAG_INVALID           0x01
#define CORE_FLAG_DISPLAY           0x02
#define CORE_FLAG_STACK_OVERFLOW    0x04
#define CORE_FLAG_STACK_UNDERFLOW   0x08
#define CORE_FLAG_WAIT              0x10
#define CORE_FLAG_EXIT              0x20
#define CORE_EXIT_FLAGS (   CORE_FLAG_INVALID | \
                            CORE_FLAG_STACK_OVERFLOW | \
                            CORE_FLAG_STACK_UNDERFLOW | \
                            CORE_FLAG_EXIT)

extern uint8_t      core_memory[4096];  /* RAM */   
extern uint8_t      core_reg_v[16];     /* Registers */
extern uint16_t     core_reg_i;         /* I register */
extern uint16_t     core_reg_pc;        /* Program counter */
extern uint8_t      core_timer_delay;   /* Delay timer */
extern uint8_t      core_timer_sound;   /* Sound timer */
extern uint16_t     core_stack[32];     /* Call stack */
extern uint8_t      core_reg_sp;        /* Stack pointer */
extern uint8_t      core_flags;         /* Error / stop flags */
extern uint16_t     core_lfsr;          /* RNG shift register */
extern uint8_t      core_reg_key;       /* Key target register */
extern uint8_t      core_font[80];      /* Default font */    

/* Initialize core variables */
void core_reset();

/* Run *count* instructions */
void core_instruction(uint16_t count);

/* Set a 16-bit value at addr to val */
void core_put16(uint16_t addr, uint16_t val);

/* Set an 8-bit value to addr to val */
void core_put8(uint16_t addr, uint8_t val);

#endif