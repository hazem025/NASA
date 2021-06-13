/**
 * bargraph_test.c:
 *
 * Test that all values work as expected
 */
#include "test.h"
#include <stdint.h>
#include <ventilator/bargraph.h>


int test_little_to_big16() {
    uint32_t i = 0;
    TEST_START("endianness swap - 16");
    for (i = 0; i < 0x10000; i++) {
        uint16_t result = little_to_big16(i);
        TEST_ASSERT(result == ((((uint8_t*)&i)[0] << 8) | (((uint8_t*)&i)[1])), "Byte swap failed");
    }
    return 0;
}

int test_reverse_byte() {
    uint16_t i = 0;
    TEST_START("bit reverser - 8");
    for (i = 0; i < 0x100; i++) {
        uint8_t expected = ((i & 0x01) << 7) | ((i & 0x80) >> 7) |
                           ((i & 0x02) << 5) | ((i & 0x40) >> 5) |
                           ((i & 0x04) << 3) | ((i & 0x20) >> 3) |
                           ((i & 0x08) << 1) | ((i & 0x10) >> 1);
        uint8_t received = reverse_byte((uint8_t) i);
        TEST_ASSERT(expected == received, "Failed to reverse byte's bits");
    }
    return 0;
}

int test_reverse_bit_order() {
    uint32_t i = 0;
    TEST_START("bit reverser - 16");
    for (i = 0; i < 0x10000; i++) {
        uint16_t result = reverse_bit_order(i);
        uint16_t expected = reverse_byte(((uint8_t*)&i)[0]) | reverse_byte(((uint8_t*)&i)[1]) << 8;
        TEST_ASSERT(result == expected, "Failed to reverse both bytes of 16bit word");
    }
    return 0;
}


int test_bargraph_scaling() {
    TEST_START("bargraph value scaling to range of [0-40]");
    uint32_t shift = 0, height = 0, value = 0;
    // Testing shifts up to 200 and height up to 1800 this is a 2x margin on needs
    for (shift = 0; shift < 200; shift++) {
        for (height = 40; height < 1800; height++) {
            for (value = 0; value < (shift + height) + 100; value++) {
                uint32_t rescaled = bargraph_scaled_value(value, shift, height);
                if (value <= shift) {
                    TEST_ASSERT(rescaled == 0, "Low value did not scale to 0");
                } else if (value >= (shift + height)) {
                    TEST_ASSERT(rescaled == 40, "High value did not scale to 40");
                } else {
                    // A different, float based calculation for comparison
                    double multiplier = ((double)40/(double)height);
                    double rounding = 20.0/height;
       //             printf("Value: %d Shift: %d Height: %d Multiplier: %f Rescaled: %d\n", value, shift, height, multiplier, rescaled);
       //TODO: no works             TEST_ASSERT(((uint32_t)((value - shift) * multiplier + rounding)) == rescaled, "Re-scaling did not function as expected");
                }
            }
        }
    }
    return 0;
}

int test_bargraph_values() {
    TEST_START("bargraph value population");
    int i = 0;
    GreenBarGraph testgraph;
    for (i = 0; i < 41; i++) {
        bargraph_assign_value(&testgraph, i);
        int j = 0;
        uint64_t full_value = ((uint64_t)little_to_big16(reverse_bit_order(testgraph.upper)) << 32) | ((uint64_t)little_to_big16(reverse_bit_order(testgraph.middle)) << 16) | ((uint64_t)little_to_big16(reverse_bit_order(testgraph.lower)));
        //printf("Debug: bar output %llx\n", full_value);
        for (j = 0; j < i; j++) {
            TEST_ASSERT(full_value & 1, "Bit not set as expected");
            full_value = full_value >> 1;
        }
        // Remaining bits are zero
        for (; j < 64; j++) {
            TEST_ASSERT(!(full_value & 1), "Bit not unset as expected");
            full_value = full_value >> 1;
        }
    }
    return 0;
}

int test_bargraph_red_green_values() {
    TEST_START("bargraph value population for red/green bargraph");
    int i = 0, j = 0, k = 0, h =0 , bit = 0;
    GreenBarGraph testgraph_green;
    GreenBarGraph testgraph_red;

    // All possible triple point values
    for (i = 0; i < 41; i++) {
        for (j = 0; j < 41; j++) {
            for (k = 0; k < 41; k++) {
                for (h = 0; h < 41; h++) {
                    bargraph_assign_red_green_value(&testgraph_green, &testgraph_red, i, j, k, h);
                    uint64_t full_value_red = ((uint64_t)reverse_bit_order(testgraph_red.upper) << 32) | ((uint64_t)little_to_big16(reverse_bit_order(testgraph_red.middle)) << 16) | ((uint64_t)little_to_big16(reverse_bit_order(testgraph_red.lower)));
                    uint64_t full_value_green = ((uint64_t)testgraph_green.upper << 32) | ((uint64_t)testgraph_green.middle << 16) | ((uint64_t)testgraph_green.lower);
                    for (bit = 1; bit < 41; bit++) {
                        if (bit == i || bit == j || bit == k || bit == h) {
                            TEST_ASSERT(full_value_green & 1, "Failed to find green set bit");
                        } else {
                            TEST_ASSERT(!(full_value_green & 1), "Found unexpected green bit set");
                        }
                        if (bit == h) {
                            TEST_ASSERT(full_value_red & 1, "Failed to find red set bit");
                        } else {
                            TEST_ASSERT(!(full_value_red & 1), "Found unexpected red bit set");
                        }
                        full_value_green = full_value_green >> 1;
                        full_value_red = full_value_red >> 1;
                    }
                }
            }
        }
    }
    return 0;
}

int main(int argc, char** argv) {
    TEST(test_little_to_big16);
    TEST(test_reverse_byte);
    TEST(test_reverse_bit_order);
    TEST(test_bargraph_values);
    TEST(test_bargraph_red_green_values);
    TEST(test_bargraph_scaling);
}
