/**
 * numerical_test.c:
 *
 * Test that all values work as expected.
 */
#include "test.h"
#include <stdint.h>
#include <ventilator/numerical.h>
#include <stdio.h>

extern uint16_t L_DIGIT_TO_SEGMENT[DIGIT_COUNT];
extern uint16_t R_DIGIT_TO_SEGMENT[DIGIT_COUNT];

int test_numerical_set_two_digit() {
    TEST_START("numeral to 2-digit display");
    int i = 0;
    TwoDigit testable;
    for (i = 0; i <= 99; i++) {
        numerical_set_two_digit(&testable, i);
        uint16_t expected = L_DIGIT_TO_SEGMENT[i/10] | R_DIGIT_TO_SEGMENT[i % 10];
        TEST_ASSERT(testable == expected, "Unexpected 2-digit result");
    }
    numerical_set_two_digit(&testable, 0xFFFFFFFF);
    TEST_ASSERT(testable == 0, "Blank flag failed to set blank")
    return 0;
}

int test_numerical_set_three_digit() {
    TEST_START("numeral to 3-digit display");
    int i = 0;
    ThreeDigit testable;
    for (i = 0; i <= 999; i++) {
        numerical_set_three_digit(&testable, i);
        uint32_t full_value = (((uint32_t)testable.high) << 16) | ((uint32_t)testable.low);
        // TODO - fix
        uint32_t expected = ((uint32_t)L_DIGIT_TO_SEGMENT[i/100] << 16) | ((uint32_t)R_DIGIT_TO_SEGMENT[(i/10) % 10] << 8) | ((uint32_t) R_DIGIT_TO_SEGMENT[i % 10]);
        //printf("%d: %lx vs %lx\n", i, full_value, expected);
        TEST_ASSERT(full_value == expected, "Unexpected 3-digit result");
    }
    numerical_set_three_digit(&testable, 0xFFFFFFFF);
    TEST_ASSERT(testable.high == 0 && testable.low == 0, "Blank flag failed to set blank")
    return 0;
}


int main(int argc, char** argv) {
    TEST(test_numerical_set_two_digit);
//    TEST(test_numerical_set_three_digit);
}
