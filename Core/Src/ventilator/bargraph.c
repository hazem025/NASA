/*
 * bargraph.c:
 *
 * Implementation functions for the bargraph code.
 */
#include <swassert.h>
#include <ventilator/bargraph.h>
#include <ventilator/types.h>
#include <stdio.h>

uint16_t little_to_big16(uint16_t word) {
    return (word << 8) | (word >> 8); //Swap the byte order of a 16bit word
}

uint8_t reverse_byte(uint8_t byte) {
    uint32_t i = 0;
    uint8_t rshifter = 0x80;
    uint8_t lshifter = 0x01;
    uint8_t output = 0x00;
    // Loop through half the bits producing 8 total shifts
    for (i = 0; i < 4; i++) {
        uint8_t shift_distance = 7 - (i << 1); // Shift by 7 - 2 * i
        output |= ((byte & lshifter) << shift_distance) | ((byte & rshifter) >> shift_distance);
        lshifter = lshifter << 1;
        rshifter = rshifter >> 1;
    }
    return output;
}

uint16_t reverse_bit_order(uint16_t value)  {
    return ((uint16_t)reverse_byte((uint8_t) value)) | (((uint16_t)reverse_byte(value >> 8)) << 8);
}

uint32_t bargraph_scaled_value(int32_t value, int32_t shift, int32_t height) {
    if (value <= shift) {
        return 0;
    }
    // Shift value to bottom of chart
    value = value - shift;
    if (value >= height) {
        return BARGRAPH_DISPLAY_HEIGHT;
    }
    // Scale the value to the chart
    uint32_t result = ((value * BARGRAPH_DISPLAY_HEIGHT)+(BARGRAPH_DISPLAY_HEIGHT/2))/height;
    return result;
}

void bargraph_assign_single_point(GreenBarGraph* bargraph, uint32_t value) {
    SW_ASSERT(bargraph != 0);
    SW_ASSERT1(value <= 40, value);
    if (value == 0) {
        return;
    }
    // Assign a single point. **Assume the value already cleared**
    if (value > 32) {
        bargraph->upper |= 1 << (value - 33);
    } else if (value > 16) {
        bargraph->middle |= 1 << (value - 17);
    } else {
        bargraph->lower |= 1 << (value - 1);
    }
}

void bargraph_assign_triple_point(GreenBarGraph* bargraph, uint32_t value1, uint32_t value2, uint32_t value3, uint32_t value4) {
    // Contractual checks
    SW_ASSERT(bargraph != 0);
    SW_ASSERT1(value1 <= 40, value1);
    SW_ASSERT1(value2 <= 40, value2);
    SW_ASSERT1(value3 <= 40, value3);
    SW_ASSERT1(value3 <= 40, value4);
    // Clear once first
    bargraph->lower = 0;
    bargraph->middle = 0;
    bargraph->upper = 0;
    // Assign all three points without clearing
    bargraph_assign_single_point(bargraph, value1);
    bargraph_assign_single_point(bargraph, value2);
    bargraph_assign_single_point(bargraph, value3);
    bargraph_assign_single_point(bargraph, value4);
}

void bargraph_assign_red_green_value(GreenBarGraph* green, GreenBarGraph* red, uint32_t green_upper, uint32_t green_mid, uint32_t green_lower, uint32_t yellow) {
    SW_ASSERT(green != NULL);
    SW_ASSERT(red != NULL);
    SW_ASSERT1(green_upper <= 40, green_upper);
    SW_ASSERT1(green_mid <= 40, green_mid);
    SW_ASSERT1(green_lower <= 40, green_lower);
    SW_ASSERT1(yellow <= 40, yellow);
    //Green has all three points
    bargraph_assign_triple_point(green, green_upper, green_mid, green_lower, yellow);
    // Red assigns only a single point, that matches for a "yellow"ish, and a single red-only point
    red->lower = 0;
    red->middle = 0;
    red->upper = 0;
    bargraph_assign_single_point(red, yellow);  // Set the single point of 0-40 for the bargraph
    // (A LSB) represents the high order bits for the lower 2 words, but the lower bits for the upper word
    // (B MSB) represents the lower order bits for the lower 2 words, and is disconnected on the upper word
    // All individual bytes have reversed-ordered bits (LSB becomes MSB etc)
    // Since memory is little endian, and the registers are little endian then only the top byt need to be swapped
    red->lower = little_to_big16(reverse_bit_order(red->lower));
    red->middle = little_to_big16(reverse_bit_order(red->middle));
    red->upper = reverse_bit_order(red->upper);
}

void bargraph_assign_value(GreenBarGraph* bargraph, uint32_t value) {
    // Contractual checks
    SW_ASSERT(bargraph != 0);
    SW_ASSERT1(value <= 40, value);
    bargraph->upper = 0;
    bargraph->middle = 0;
    bargraph->lower = bargraph_get_u16_helper(value > 16 ? 16 : value);
    // Prevent overflow by only assigning if more than 16
    if (value > 16) {
        value -= 16;
        bargraph->middle = bargraph_get_u16_helper(value > 16 ? 16 : value);
        // Another protection against overflow
        if (value > 16) {
            value -= 16;
            bargraph->upper = bargraph_get_u16_helper(value > 16 ? 16 : value);
        }
    }
    bargraph->upper = little_to_big16(reverse_bit_order(bargraph->upper));
    bargraph->middle = little_to_big16(reverse_bit_order(bargraph->middle));
    bargraph->lower = little_to_big16(reverse_bit_order(bargraph->lower));

}

uint16_t bargraph_get_u16_helper(uint32_t value) {
    // The shift by value code seen here is dependent on value being less than or equal to 16
    SW_ASSERT1(value <= 16, value);
    // Using 32-bit integer to do the math without overflow of the 16 bit types
    // Shifting one will result in a power of two. Subtracting 1 will result in that number of 1 bits.
    uint32_t shifted = (1 << ((uint32_t)value)) - 1;
    return (uint16_t) shifted;
}

