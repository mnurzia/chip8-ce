/* input.h - CHIP-8 input handling, logic, and remapping */

#ifndef INPUT_H
#define INPUT_H

extern uint8_t input_mapping[16];   /* Current key mapping */
extern uint8_t input_keys[16];      /* Current key state */
extern uint8_t input_exit;          /* Should the emulator exit? */

/* Initialize input_mapping, input_keys, and set input_exit to 0. */
void input_reset();

/* If *write*, set core_reg_v[core_reg_key] to the current pressed key. */
uint8_t input_scan(uint8_t write);

/* Wait for a key to be pressed. */
uint8_t input_wait();

#endif