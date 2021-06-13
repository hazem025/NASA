/**
 * test.h:
 *
 * A very basic set of test functions to help for testing the various functions.
 */
#include <stdio.h>
#include <swassert.h>
#include <ventilator/mcp23017.h>
#include <stm32f0xx_hal.h>
#include <ventilator/display.h>
#ifndef VENTILATOR_PANLE_TEST_H_
#define VENTILATOR_PANLE_TEST_H_

#define TEST_ASSERT(EXPECTED, MSG) if (!(EXPECTED)) {fprintf(stderr, "FAILED: at %s:%d with error: %s\n", __FILE__, __LINE__, MSG); return -1; } else if (SW_ASSERT_FLAG) {fprintf(stderr, "FAILED: previous assertion occurred\n");}

#define TEST(FUN) fprintf(stderr, #FUN " - "); if (FUN()) { fprintf(stderr, "FAILED\n"); return 1; } else if (SW_ASSERT_FLAG) {fprintf(stderr, "FAILED: assertion occurred\n");} else { fprintf(stderr, "SUCCESS\n");}

#define TEST_START(DESC) fprintf(stderr, "TEST: '%s' at '%s'\n", DESC, __PRETTY_FUNCTION__)

extern int sound_running;

extern Display m_display;

#endif
