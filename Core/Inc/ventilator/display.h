/*
 * display.h:
 *
 * Defines all of the available types for operating the LED, 8 segment, and bar graph displays available on the panel controller. Each display will have
 * its own setter functions, but this file rolls up all of those individual definitions into a single file where the display structure can be defined.
 *
 * @author mstarch
 */

#ifndef INC_VENTILATOR_DISPLAY_H_
#define INC_VENTILATOR_DISPLAY_H_
#include <ventilator/bargraph.h>
#include <ventilator/numerical.h>
#include <ventilator/mcp23017.h>
#include <ventilator/button.h>
#include <stm32f0xx_hal.h>
#include <ventilator/types.h>
#include <ventilator/constants.h>


// Number of total 16bit words that compose the SPI transaction to write to the display. Each word is backed by a specific shift register that drives the
// LED outputs
#define DISPLAY_U16_COUNT 19

/**
 * Display:
 *
 * The display for the panel is a series of single unit displays that represent the output of a number of items. The entire display is driven by shifting
 * out in-order 19x16 bits that will each drive specific LEDs on the part displays.  This is all done by writing these words to an SPI bus. The last item
 * on the bus is the most significant bits of the display output.
 *
 * The display is defined as a union of the composition of each individual display, and the 19 raw 16-bit words that need to be shifted onto the wire.
 * These words should be shifted in-order onto the wire.  Each individual sub-display is internally responsible for the order of its individual bits and
 * the API functions to set them.
 */
typedef struct {
    //uint16_t x 19 sent
    GreenBarGraph red_green_green; // 3 uint16_t
    GreenBarGraph pressure;        // 3 uint16_t
    GreenBarGraph tidal;           // 3 uint16_t
    uint16_t alarm;                // 1 uint16_t
    TwoDigit minute_volume;        // 1 uint16_t
    TwoDigit resp_rate;            // 1 uint16_t
    TwoDigit ins_time;             // 1 uint16_t
    TwoDigit peak_pressure;        // 1 uint16_t
    TwoDigit backup_rate;          // 1 uint16_t
    ThreeDigit tidal_volume;       // 2 uint16_t
    TwoDigit PEEP;                 // 1 uint16_t
    TwoDigit FIO2;                 // 1 uint16_t
    // Nothing below here is sent
    GreenBarGraph red_green_red; // Sent via I2C
    SPI_HandleTypeDef* spi;
    McpHandle mcp_upper;
    McpHandle mcp_middle;
    McpHandle mcp_lower;
    GPIO_TypeDef* gpio_port;
    uint16_t latch;
    uint16_t blank;
} Display;

/**
 * Shift data for alarms output.
 */
typedef enum {
    DISPLAY_ALARM_DISCONNECT_SHIFT = 15,
    DISPLAY_ALARM_TIDAL_VOL_SHIFT  = 14,
    DISPLAY_ALARM_PEAK_PRES_SHIFT  = 13,
    DISPLAY_ALARM_RESP_RATE_SHIFT  = 12,
    DISPLAY_ALARM_PEEP_PRES_SHIFT  = 11,
    DISPLAY_ALARM_FIO2_PERC_SHIFT  = 6,
    DISPLAY_ALARM_MACH_FALT_SHIFT  = 5,
    DISPLAY_ALARM_LOW_POWER_SHIFT  = 4,
    DISPLAY_ALARM_POWER_OFF_SHIFT  = 3,
} DisplayAlarmShifts;

/**
 * run_display:
 *
 * Use the current switch states and controller transaction to update the displays.
 * bool force_blank: force the display to blank. Done in standby.
 * uint32_t alive_hours: alive hours to display when transitioning.
 */

void run_display(bool force_blank, uint32_t alive_hours);

/**
 * display_init:
 *
 * Initialize the display and the variables used by it.
 */
void display_init(void);

/**
 * display_machine_fault:
 *
 * Re-initialize the display and display the machine fault as a last attempt to notify there is an issue.
 */
void display_machine_fault(void);
/**
 * display_blank:
 *
 * Blank out the display, entirely. This is done only on startup to clear all the random data in the
 * buffers. Then display_standby is used to display the power only led on standby.
 */
void display_blank(void);

/**
 * display_standby:
 *
 * Blank out the display, with the exception of the power LED. This is done in power-off state to represent
 * the device is on, but hasn't started.
 * uint32_t upper: upper digit to display (for hours display). Use BLANK_CONSTANT for true blank.
 * uint32_t lower: lower digit to display (for hours display). Use BLANK_CONSTANT for true blank.
 */
void display_standby(uint32_t upper, uint32_t lower);

/**
 * display_spi_send:
 *
 * Send an updated set of values out to the display. This will convert the display into a series of bytes that gets written into the SPI device whose handle is associated with
 * this particular display.
 * NumericalValues* values: state to send out to the display.
 */
void display_send_update(NumericalValues *values);

/**
 * display_raw_send:
 *
 * Send the raw SPI and I2C packets to the display.
 */
HAL_StatusTypeDef display_raw_send(void);


#endif /* INC_VENTILATOR_DISPLAY_H_ */
