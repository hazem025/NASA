/*
 * bargraph.h:
 *
 * Contains the code necessary to drive the 2 green and the red-green bargraph displays in the system. These bargraphs
 * illuminate LEDs.  The red-green bargraph must set both the red and the green  led to produce yellow, as they share
 * a lead drain-path and thus must be run together.
 *
 * @author mstarch
 */

#ifndef INC_VENTILATOR_BARGRAPH_H_
#define INC_VENTILATOR_BARGRAPH_H_
#include <stdint.h>
// Lowest value on tidal bargraph (stenciled onto panel)
#define BARGRAPH_TIDAL_SHIFT 100
// Total height of tidal bargraph
#define BARGRAPH_TIDAL_HEIGHT 800

// Lowest value on pressure bargraph (stenciled onto panel)
#define BARGRAPH_PRESSURE_SHIFT 0
// Total height of tidal bargraph
#define BARGRAPH_PRESSURE_HEIGHT 100
// Height in units of bargraph display
#define BARGRAPH_DISPLAY_HEIGHT 40

/**
 * GreenBarGraph:
 *
 * Displays a number 0-40 in bargraph of level form. For N in [0, 40] N leds will be illuminated from the bottom of the graph to the top of the graph
 * to represent that level. The output is represented by 3 16 bit integers, composing 8 unused and 40 used bits each turning on an led.
 *
 * e.g. to display 35 the following would be the order of bits in the word:
 *
 * 0x0007 FFFF FFFF
 */
typedef  struct {
    uint16_t upper;
    uint16_t middle;
    uint16_t lower;
} GreenBarGraph;

/**
 * little_to_big16:
 *
 * Swap the bytes of a 16bit integer to produce a endian-swapped value.
 */
uint16_t little_to_big16(uint16_t word);

/**
 * bar_assign_value:
 *
 * Assigns a value to the bar graph display. The value must be in the range [0, 40] and the resulting assignment will illuminate all LEDs with index less
 * than or equal to the given number. Any value larger than 40 will result in an assertion failure.
 *
 * GreenBar bargraph: (output) bargraph to assign value to
 * uint8_t value: value to assign to. *must* be in the range [0, 40]
 */
void bargraph_assign_value(GreenBarGraph* bargraph, uint32_t value);

/**
 * bargraph_scaled_value:
 *
 *
 * Scale a value for display on a bargraph. This will take the offset (shift) and height of the graph in real units. It will then scale it back down to the
 * 0-40 values expected by the bargraph itself.  If too large, 40 is returned, if less than shift 0 is returned. 0xFFFFFFFF is treated as a blank or zero.
 * uint32_t value: value to scale to bargraph level
 * uint32_t shift: minimum value of the bar chart
 * uint32_t height: height in pre-scaled units of the bar chart
 * return: value 0-40 in post-scaled units
 */
uint32_t bargraph_scaled_value(int32_t value, int32_t shift, int32_t height);

/**
 * bargraph_assign_red_green_value:
 *
 * Assign the red-green bargraph pairs with the upper (green), middle (red + green) and lower (green) values. All values should be limited to the range 0-40 or
 * an assert will be tripped. A middle "geen_mid" value is also displayed.
 * GreenBarGraph* green: green bargraph to receive all 3 points
 * GreenBarGraph* red: red bargraph to receive a single point
 * uint32_t green_upper: upper value to set in green only
 * uint32_t green_mid: middle geen value to display
 * uint32_t green_lower: value to set in green only
 * uint32_t yellow: middle value to set in both to provide "yellow"
 */
void bargraph_assign_red_green_value(GreenBarGraph* green, GreenBarGraph* red, uint32_t green_upper, uint32_t green_mid, uint32_t green_lower, uint32_t yellow);

// **** Internal Representation ****

/**
 * bargraph_get_u16_helper:
 *
 * A helper to get a word of set bits expanded for a given value.
 *
 * uint8_t value: value *must* be between [0, 16] otherwise an assertion is raised.
 * return: value number of sequential 1bits set starting with the LSB. e.g. 3 => 0x0007
 */
uint16_t bargraph_get_u16_helper(uint32_t value);

/**
 * bargraph_assign_single_point:
 *
 * Assigns a single point LED to a bargraph. ****WARNING**** this does not clear. Calling multiple times will set multiple points.  Always clear memory sometime before
 * calling or results are not guarenteed.
 * GreenBarGraph* bargraph: to assign point into
 * uint32_t value: value, must be between [0, 40]
 */
void bargraph_assign_single_point(GreenBarGraph* bargraph, uint32_t value);

/**
 * bargraph_assign_single_point:
 *
 * Assigns a three point LEDs to a bargraph. This clears and then sets all three points with "bargraph_assign_single_point". Used
 * to establish the green points in the red green set.
 * GreenBarGraph* bargraph: to assign points into
 * uint32_t value1: value1, must be between [0, 40]
 * uint32_t value2: value2, must be between [0, 40]
 * uint32_t value3: value3, must be between [0, 40]
 */
void bargraph_assign_triple_point(GreenBarGraph* bargraph, uint32_t value1, uint32_t value2, uint32_t value3, uint32_t value4);

/**
 * reverse_byte:
 *
 * Reverse the bits in a byte
 */
uint8_t reverse_byte(uint8_t byte);

/**
 * reverse_bit_order:
 *
 * Reverse the bits in each bytes of the 16 bit word
 */
uint16_t reverse_bit_order(uint16_t value);


#endif /* INC_VENTILATOR_BARGRAPH_H_ */
