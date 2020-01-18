/* tests.h - CPU instruction tests (coarse) */

#ifndef TESTS_H
#define TESTS_H

#include <stdint.h>

#ifdef TESTS

extern uint8_t test_contents[128];  /* Test machine code */

/* Run all tests */
void tests_runall();

#endif

#endif