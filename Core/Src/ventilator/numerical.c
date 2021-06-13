/*
 * numerical.c:
 *
 * Implementation for the numerical display outputs.
 */
#include <ventilator/types.h>
#include <ventilator/numerical.h>
#include <swassert.h>

// Connections - Common anode

// Left digit - Right Digit
//    2              5
// 0     3        4     6
//    1              9
// 15    13       11    8
//    14             10
//
// Dec        12          7

STATIC uint16_t L_DIGIT_TO_SEGMENT[DIGIT_COUNT] = {
        (1 << 0) | (1 << 2) | (1 << 3) | (1 << 13) | (1 << 14) | (1 << 15),               // 0
        (1 << 3) | (1 << 13),                                                             // 1
        (1 << 2) | (1 << 3) | (1 << 1) | (1 << 15) | (1 << 14) ,                          // 2
        (1 << 2) | (1 << 3) | (1 << 1) | (1 << 13) | (1 << 14) ,                          // 3
        (1 << 0) | (1 << 1) | (1 << 3) | (1 << 13) ,                                      // 4
        (1 << 2) | (1 << 0) | (1 << 1) | (1 << 13) | (1 << 14) ,                          // 5
        (1 << 2) | (1 << 0) | (1 << 1) | (1 << 13) | (1 << 14) | (1 << 15) ,              // 6
        (1 << 2) | (1 << 3) | (1 << 13),                                                  // 7
        (1 << 0) | (1 << 2) | (1 << 3) | (1 << 1)  | (1 << 13) | (1 << 14) | (1 << 15),    // 8
        (1 << 0) | (1 << 2) | (1 << 3) | (1 << 1)  | (1 << 13) | (1 << 14)                 // 9
};


STATIC uint16_t R_DIGIT_TO_SEGMENT[DIGIT_COUNT] = {
        (1 << 4) | (1 << 5) | (1 << 6) | (1 << 8) | (1 << 10) | (1 << 11),                // 0
        (1 << 6) | (1 << 8),                                                              // 1
        (1 << 5) | (1 << 6) | (1 << 9) | (1 << 11) | (1 << 10) ,                          // 2
        (1 << 5) | (1 << 6) | (1 << 9) | (1 << 8) | (1 << 10) ,                           // 3
        (1 << 4) | (1 << 9) | (1 << 6) | (1 << 8) ,                                       // 4
        (1 << 5) | (1 << 4) | (1 << 9) | (1 << 8) | (1 << 10) ,                           // 5
        (1 << 5) | (1 << 4) | (1 << 9) | (1 << 8) | (1 << 10) | (1 << 11) ,               // 6
        (1 << 5) | (1 << 6) | (1 << 8),                                                   // 7
        (1 << 4) | (1 << 5) | (1 << 6) | (1 << 9) | (1 << 8) | (1 << 10) | (1 << 11),     // 8
        (1 << 4) | (1 << 5) | (1 << 6) | (1 << 9) | (1 << 8) | (1 << 10)                  // 9
};




uint16_t l_numerical_digit_to_segment_helper(uint32_t digit) {
    SW_ASSERT(digit < DIGIT_COUNT);
    return L_DIGIT_TO_SEGMENT[digit];
}

uint16_t r_numerical_digit_to_segment_helper(uint32_t digit) {
    SW_ASSERT(digit < DIGIT_COUNT);
    return R_DIGIT_TO_SEGMENT[digit];
}

void numerical_set_two_digit(TwoDigit* two_digit, int32_t value) {
    if ((value > 99) && (value != BLANK_CONSTANT)) {
        value = 99;
    } else if ((value < 0) && (value != BLANK_CONSTANT)) {
        value = 0;
    }
    SW_ASSERT(two_digit != 0);
    // Assign each of the parts, 0s if blank otherwise each digit
    if (value == BLANK_CONSTANT) {
        *two_digit = 0x0000;
    } else {
        *two_digit = l_numerical_digit_to_segment_helper(value/10) | r_numerical_digit_to_segment_helper(value % 10);
    }
}

// U25 - MSW - A0-A15
// U26 - LSW - B0-B15

// Left Digit    Middle Digit    Right Digit
//     A14           B10             B14
// A13    A15    B9       B11    B13     B15
//     A12           B8              B12
// A7     A5     A3       A1     B7      B5
//     A6            A2              B6
//
// Dec    A4              A0             B4

enum {
    A = 0,
    B = 16
};

STATIC uint32_t L_3DIGIT_TO_SEGMENT[DIGIT_COUNT] = {
    (1 << (13+A))  | (1 << (14+A))  | (1 << (15+A))  | (1 << (5+A))  | (1 << (6+A))  | (1 << (7+A)),                   // 0
    (1 << (15+A))  | (1 << (5+A)),                                                                                     // 1
    (1 << (14+A))  | (1 << (15+A))  | (1 << (12+A))  | (1 << (7+A))  | (1 << (6+A)),                                   // 2
    (1 << (14+A))  | (1 << (15+A))  | (1 << (12+A))  | (1 << (5+A))  | (1 << (6+A)),                                   // 3
    (1 << (13+A))  | (1 << (12+A))  | (1 << (15+A))  | (1 << (5+A)),                                                   // 4
    (1 << (14+A))  | (1 << (13+A))  | (1 << (12+A))  | (1 << (5+A))  | (1 << (6+A)),                                   // 5
    (1 << (14+A))  | (1 << (13+A))  | (1 << (12+A))  | (1 << (5+A))  | (1 << (6+A))  | (1 << (7+A)),                   // 6
    (1 << (14+A))  | (1 << (15+A))  | (1 << (5+A)),                                                                    // 7
    (1 << (13+A))  | (1 << (14+A))  | (1 << (15+A))  | (1 << (12+A)) | (1 << (7+A))  | (1 << (6+A)) | (1 << (5+A)),    // 8
    (1 << (13+A))  | (1 << (14+A))  | (1 << (15+A))  | (1 << (12+A)) | (1 << (6+A))  | (1 << (5+A)),                   // 9
};

STATIC uint32_t C_3DIGIT_TO_SEGMENT[DIGIT_COUNT] = {
    (1 << (9+B))   | (1 << (10+B))  | (1 << (11+B))  | (1 << (1+A))  | (1 << (2+A))  | (1 << (3+A)),                   // 0
    (1 << (11+B))  | (1 << (1+A)),                                                                                     // 1
    (1 << (10+B))  | (1 << (11+B))  | (1 << (8+B))   | (1 << (3+A))  | (1 << (2+A)),                                   // 2
    (1 << (10+B))  | (1 << (11+B))  | (1 << (8+B))   | (1 << (1+A))  | (1 << (2+A)),                                   // 3
    (1 << (9+B))   | (1 << (8+B))   | (1 << (11+B))  | (1 << (1+A)),                                                   // 4
    (1 << (10+B))  | (1 << (9+B))   | (1 << (8+B))   | (1 << (1+A))  | (1 << (2+A)),                                   // 5
    (1 << (10+B))  | (1 << (9+B))   | (1 << (8+B))   | (1 << (1+A))  | (1 << (2+A))  | (1 << (3+A)),                   // 6
    (1 << (10+B))  | (1 << (11+B))  | (1 << (1+A)),                                                                    // 7
    (1 << (10+B))  | (1 << (9+B))   | (1 << (8+B))   | (1 << (11+B)) | (1 << (3+A))  | (1 << (2+A))  | (1 << (1+A)),   // 8
    (1 << (10+B))  | (1 << (9+B))   | (1 << (8+B))   | (1 << (11+B)) | (1 << (2+A))  | (1 << (1+A)),                   // 9
};

STATIC uint32_t R_3DIGIT_TO_SEGMENT[DIGIT_COUNT] = {
    (1 << (13+B))   | (1 << (14+B))  | (1 << (15+B)) | (1 << (5+B))  | (1 << (6+B))  | (1 << (7+B)),                   // 0
    (1 << (15+B))   | (1 << (5+B)),                                                                                    // 1
    (1 << (14+B))   | (1 << (15+B))  | (1 << (12+B)) | (1 << (7+B))  | (1 << (6+B)),                                   // 2
    (1 << (14+B))   | (1 << (15+B))  | (1 << (12+B)) | (1 << (5+B))  | (1 << (6+B)),                                   // 3
    (1 << (13+B))   | (1 << (12+B))  | (1 << (15+B)) | (1 << (5+B)),                                                   // 4
    (1 << (14+B))   | (1 << (13+B))  | (1 << (12+B)) | (1 << (5+B))  | (1 << (6+B)),                                   // 5
    (1 << (14+B))   | (1 << (13+B))  | (1 << (12+B)) | (1 << (7+B))  | (1 << (6+B))  | (1 << (5+B)),                   // 6
    (1 << (14+B))   | (1 << (15+B))  | (1 << (5+B)),                                                                   // 7
    (1 << (13+B))   | (1 << (14+B))  | (1 << (15+B)) | (1 << (12+B)) | (1 << (5+B))  | (1 << (6+B))  | (1 << (7+B)),   // 8
    (1 << (13+B))   | (1 << (14+B))  | (1 << (15+B)) | (1 << (12+B)) | (1 << (5+B))  | (1 << (6+B)),                   // 9
};



void numerical_set_three_digit(ThreeDigit* three_digit, int32_t value) {
    if ((value > 999) && (value != BLANK_CONSTANT)) {
        value = 999;
    } else if ((value < 0) && (value != BLANK_CONSTANT)) {
        value = 0;
    }
    SW_ASSERT(three_digit != 0);
    // Assign each of the parts, 0s if blank otherwise each digit
    if (value == BLANK_CONSTANT) {
        three_digit->high = 0x0000;
        three_digit->low = 0x0000;
    } else {
        // Assert that all indexing values are in-bound before indexing into array
        SW_ASSERT((value / 100) < DIGIT_COUNT);
        SW_ASSERT(((value % 100)/10) < DIGIT_COUNT);
        SW_ASSERT((value % 10) < DIGIT_COUNT);
        uint32_t digits = L_3DIGIT_TO_SEGMENT[value/100] | C_3DIGIT_TO_SEGMENT[(value%100)/10] | R_3DIGIT_TO_SEGMENT[value%10];
        three_digit->high = (uint16_t)(digits >> 16);
        three_digit->low = (uint16_t)digits;
    }
}

