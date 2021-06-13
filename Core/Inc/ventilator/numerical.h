/*
 * numerical.h:
 *
 * Contains the code necessary for displaying numerical displays. These displays come in two forms, a two digit display, and a three digit display. These
 * displays currently display integral values between [0, 99] for the two digit displays and [0, 999] for the three digit displays.
 */
#ifndef INC_VENTILATOR_NUMERICAL_H_
#define INC_VENTILATOR_NUMERICAL_H_
#include <stdint.h>
#define DIGIT_COUNT 10

#define BLANK_CONSTANT 0xFFFFFFFF
/**
 * TwoDigit:
 *
 * A two digit display can be entirely backed by a single U16 type. The MSB represents the 8 segments for the high order digit, and the LSB of the
 * represents the segments of the low order display. Outputs to this display are expected in the form [0, 99]. Anything outside of that value will raise
 * an assertion.
 */
typedef uint16_t TwoDigit;

/**
 * ThreeDigit:
 *
 * Three digit displays are conceptually two TwoDigit displays where the higher-order two digit display is disconnected. Thus it can be treated as two
 * displays with the addition constraint that its N numerical input is limited to the range [0, 999] and mapped out as [0, 9] to one display, and [0, 99]
 * as the other.
 */
typedef struct {
    TwoDigit high;
    TwoDigit low;
} ThreeDigit;

/**
 * numerical_set_two_digit:
 *
 * Assigns a numerical value to the two digit display. The value is limited to the range [0, 99] and will produce the corresponding 8-segment output into
 * the supplied two digit display.
 *
 * TwoDigit* digit: (output) two digit display to modify by assigning it to the given value.
 * uint8_t value: (input) value to assign to the display. Must be in the range [0, 99]
 */
void numerical_set_two_digit(TwoDigit* digit, int32_t value);

/**
 * numerical_set_two_digit:
 *
 * Assigns a numerical value to the three digit display. The value is limited to the range [0, 999] and will produce the corresponding 8-segment output
 * into the supplied three digit display.
 *
 * TwoDigit* digit: (output) two digit display to modify by assigning it to the given value.
 * uint16_t value: (input) value to assign to the display. Must be in the range [0, 999]
 */
void numerical_set_three_digit(ThreeDigit* digit, int32_t value);

/**
 * numerical_digit_to_segment_helper:
 *
 * Helper function that converts a given digit into the 8-segment display output.
 *
 * uint8_t digit: (input) digit to convert. Must be in range [0, 9]
 * return: uint8_t representation of 8 segments
 */
uint8_t numerical_digit_to_segment_helper(uint32_t digit);

#endif /* INC_VENTILATOR_NUMERICAL_H_ */
