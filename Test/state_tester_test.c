/**
 * test_state_tester:
 *
 * Runs a test of the test state machine. This gives confidence on it before we test on the hardware itself.
 */
#include <ventilator/display.h>
#include <ventilator/mcp23017.h>
#include <ventilator/sound.h>
#include <ventilator/test_cycle.h>
#include <test.h>

#define EXTERN
#include <ventilator/panel_public.h>

#define CYCLE_DISPLAY_COUNT 19
#define TOTAL_WORD_COUNT 19

int timer = 1;

//
int words_count[] = {1,   1,  2,  1,  1,  1,  1,  1, 0, 0, 0, 0, 0, 0, 0, 0, 1,  3,  3,  3,  0,  0,  0, 0, 0, 0};
int cycle_count[] = {19, 19, 28, 19, 19, 19, 19, 19, 1, 1, 1, 1, 1, 1, 1, 1, 1, 41, 41, 41, 41, 41, 41, SOUND_BEEP_DURATION_CYCLES + 1, SOUND_BEEP_DURATION_CYCLES*3 + 1, SOUND_BEEP_DURATION_CYCLES*5 + 1};
uint16_t value[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint16_t value_last[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xed7d, 0xed7d, 0xfd7d, 0xed7d, 0xed7d, 0xeee0, 0xe0ee, 0xed7d, 0xed7d};

int test_test_smoke_test() {
    TEST_START("assert the unit test is correct");
    int i = 0;
    int total = 0;
    for (i = 0; i < ARRAY_LEN(words_count); i++) {
        total += words_count[i];
    }
    TEST_ASSERT(total == TOTAL_WORD_COUNT, "Word counts specified poorly"); // LEDs tested separately
    TEST_ASSERT(ARRAY_LEN(words_count) == ARRAY_LEN(cycle_count), "Parallel arrays not parallel");
    TEST_ASSERT(ARRAY_LEN(value) == ARRAY_LEN(value_last) && ARRAY_LEN(value_last) == TOTAL_WORD_COUNT, "Parallel arrays not parallel");
    return 0;
}


int test_test_state() {
    int i = 0, j = 0, cyc_it = 0;
    int total_words = 0;
    char* error = 0;
    uint64_t contin = 0;
    // Initialize resources
    display_init();
    sound_init(&timer);

    TEST_START("function test state machine test");
    for (i = 0; i < ARRAY_LEN(cycle_count); i++) {
        for (cyc_it = 0; cyc_it < cycle_count[i]; cyc_it++) {
            //fprintf(stderr, "Cycle count: %d\n", cyc_it);
            doTestCycle(); // Cycle it
            int ll = 0;
            error = 0;
            uint32_t word_touched = (cyc_it / CYCLE_DISPLAY_COUNT);
            uint32_t word_index = (TOTAL_WORD_COUNT - 1 - (total_words + word_touched));
            //printf(stderr, "Checking word: %lu    --- total %lu  --- touched: %lu\n", word_index, total_words, word_touched);
            //fprintf(stderr, "Raw Hex: ");
            for (ll = 0; ll < TOTAL_WORD_COUNT; ll++) {
                //fprintf(stderr, "%hx ", ((uint16_t*)&m_display)[ll]);
                value[ll] = ((uint16_t*)&m_display)[ll];
                // Unchanging values is the first 0 of the displays cycle, and anything not being edited
                if (word_index != ll || (cyc_it == 0 && i < 8)) {
                    error = (value[ll] == value_last[ll]) ? 0 : "Unexpected word changed";
                } else if (i < 8) {
                    error = (value[ll] != value_last[ll]) ? 0 : "Expected digit did not change";
                }
                // Alarm values (predicted) .... this order might not apply on big endian
                else if (i == 8)  {
                    error = (value[ll] == 0x8000) ? 0 : "Failed special alarm test -- 0";
                } else if (i == 9)  {
                    error = (value[ll] == 0xC000) ? 0 : "Failed special alarm test -- 1";
                } else if (i == 10)  {
                    error = (value[ll] == 0xD000) ? 0 : "Failed special alarm test -- 2";
                } else if (i == 11)  {
                    error = (value[ll] == 0xF000) ? 0 : "Failed special alarm test -- 3";
                } else if (i == 12)  {
                    error = (value[ll] == 0xF800) ? 0 : "Failed special alarm test -- 4";
                } else if (i == 13)  {
                    error = (value[ll] == 0xFC00) ? 0 : "Failed special alarm test -- 5";
                } else if (i == 14)  {
                    error = (value[ll] == 0xFC80) ? 0 : "Failed special alarm test -- 6";
                } else if (i == 15)  {
                    error = (value[ll] == 0xFCC0) ? 0 : "Failed special alarm test -- 7";
                } else if (i == 16)  {
                    error = (value[ll] == 0xFCD0) ? 0 : "Failed special alarm test -- 8";
                }
                value_last[ll] = value[ll];
            }
            TEST_ASSERT(error == 0, error);
            //fprintf(stderr, "\n");

            // Bargraph setups
            uint64_t final = 0;
            uint64_t bit = 1;
            if (i == 17) {
                final = (((uint64_t)little_to_big16(reverse_bit_order(value[6]))) << 32) |
                        (((uint64_t)little_to_big16(reverse_bit_order(value[7]))) << 16) |
                        ((uint64_t)little_to_big16(reverse_bit_order(value[8])));
                contin = contin | (bit << cyc_it) >> 1;
                TEST_ASSERT(final == contin, "Tidal bargraph failure");
            } else if (i == 18) {
                final = (((uint64_t)little_to_big16(reverse_bit_order(value[3]))) << 32) |
                        (((uint64_t)little_to_big16(reverse_bit_order(value[4]))) << 16) |
                        ((uint64_t)little_to_big16(reverse_bit_order(value[5])));
                contin = contin | (bit << cyc_it) >> 1;
                TEST_ASSERT(final == contin, "Pressure bargraph failure");
            } else if (i == 19) {
                final = (((uint64_t)value[0]) << 32) | (((uint64_t)value[1]) << 16) | ((uint64_t)value[2]);
                bit = (bit << cyc_it) >> 1;
                TEST_ASSERT(final == bit, "Failed to set quad point low as expected");
            } else if (i == 20) {
                bit = ((bit << cyc_it) >> 1) | 0x0000008000000000ll;
                final = (((uint64_t)value[0]) << 32) | (((uint64_t)value[1]) << 16) | ((uint64_t)value[2]);
                TEST_ASSERT(final == bit, "Failed to set quad point medium as expected");
            } else if (i == 21) {
                bit = ((bit << cyc_it) >> 1) | 0x0000008000000000ll;
                final = (((uint64_t)value[0]) << 32) | (((uint64_t)value[1]) << 16) | ((uint64_t)value[2]);
                TEST_ASSERT(final == bit, "Failed to set quad point high as expected");
            } else if (i == 22) {
                bit = ((bit << cyc_it) >> 1) | 0x0000008000000000ll;
                final = (((uint64_t)value[0]) << 32) | (((uint64_t)value[1]) << 16) | ((uint64_t)value[2]);
                TEST_ASSERT(final == bit, "Failed to set quad point yellow as expected");
            }
            // BEEP
            else if (i == 23 || i == 25) {
                if (cyc_it == cycle_count[i] - 1) {
                    TEST_ASSERT(!sound_running, "Beep/constant not off as expected");
                } else {
                    TEST_ASSERT(sound_running, "Beep/constant not on as expected");
                }
            }
            // BEEP BEEP
            else if (i == 24) {
                if (cyc_it == cycle_count[i] - 1 || (cyc_it >= SOUND_BEEP_DURATION_CYCLES && cyc_it < (SOUND_BEEP_DURATION_CYCLES*2))) {
                    TEST_ASSERT(!sound_running, "Beep beep not off as expected");
                } else {
                    TEST_ASSERT(sound_running, "Beep beep not on as expected");
                }
            }
        }
        contin = 0;
        // Update word counts,
        total_words += words_count[i];
        for (j = 0; j < ARRAY_LEN(value); j++) {
            value[j] = 0;
        }

    }
    return 0;
}

int main(int argc, char** argv) {
    TEST(test_test_smoke_test);
    TEST(test_test_state);
    return 0;
}
