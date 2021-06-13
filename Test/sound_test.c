/**
 * sound_test.c:
 *
 * Test that all values work as expected.
 */
#include "test.h"
#include <string.h>
#include <stdint.h>
#include <ventilator/sound.h>

extern Sound SOUND;

int timer = 0;

int test_sound_beep() {
    TEST_START("sound single beep");
    int i = 0;
    sound_init(&timer);
    sound_running = 0; // Reset
    sound_start(SOUND_BEEP);
    for (; i < SOUND_BEEP_DURATION_CYCLES; i++) {
        TEST_ASSERT(sound_running, "Sound not running as expected");
        sound_cycle();
    }
    sound_cycle();
    TEST_ASSERT(!sound_running, "Sound didn't shut off as expected");
    return 0;
}


int test_sound_beep_beep() {
    TEST_START("sound double beep");
    int i = 0;
    sound_init(&timer);
    sound_running = 0; // Reset
    sound_start(SOUND_TWO_BEEP);
    for (i = 0; i < SOUND_BEEP_DURATION_CYCLES; i++) {
        TEST_ASSERT(sound_running, "Sound not running as expected");
        sound_cycle();
    }
    for (i = 0; i < SOUND_BEEP_DURATION_CYCLES; i++) {
        TEST_ASSERT(!sound_running, "Sound not paused as expected");
        sound_cycle();
    }
    for (i = 0; i < SOUND_BEEP_DURATION_CYCLES; i++) {
        TEST_ASSERT(sound_running, "Sound not back on as expected");
        sound_cycle();
    }
    TEST_ASSERT(!sound_running, "Sound didn't shut off as expected");
    return 0;
}

int test_sound_continuous() {
    TEST_START("sound continuous alarm");
    int i = 0;
    sound_init(&timer);
    sound_running = 0; // Reset
    sound_start(SOUND_CONSTANT);
    for (; i < SOUND_BEEP_DURATION_CYCLES * 10000; i++) {
        TEST_ASSERT(sound_running, "Sound not running as expected");
        sound_cycle();
    }
    sound_stop();
    sound_cycle();
    TEST_ASSERT(!sound_running, "Sound didn't shut off as expected");
    return 0;
}

int test_is_alarming() {
    TEST_START("sound alarming state");
    SoundState state = 0;
    for (state = SOUND_OFF ; state < MAX_SOUND_STATE; state++) {
        SOUND.state = state;
        if (state == SOUND_CONSTANT) {
            TEST_ASSERT(sound_is_alarming(), "Sound not alarming when it should");
        } else {
            TEST_ASSERT(!sound_is_alarming(), "Sound alarming when it should not");
        }
    }
    return 0;
}

int main(int argc, char** argv) {
    TEST(test_sound_beep);
    TEST(test_sound_beep_beep);
    TEST(test_sound_continuous);
    TEST(test_is_alarming);
}
