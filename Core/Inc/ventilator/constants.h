/*
 * constants.h:
 *
 * Constants file for constants used in this code. Used to globally adjust timings and various other
 * settings in the system without needing to edit in other modules.
 *
 *  Created on: Apr 13, 2020
 *      Author: mstarch
 */

#ifndef INC_VENTILATOR_CONSTANTS_H_
#define INC_VENTILATOR_CONSTANTS_H_

// Convenient macros
#define WATCHDOG_GPIO   GPIO_PIN_5

// Constant definitions:
// Placed in a enum for extra enum safety checks.
typedef enum {
    DO_PLATEAU_SENDS  =     3,  // Send plateau commands for this many cycles
    CYCLES_PER_SECOND =     50, // Cycles per second (50 at 50Hz)
    ALARM_RESTART_DELAY =   CYCLES_PER_SECOND*60, // Time alarm is silenced when alarm_silence called (cycles)
    ALARM_CLEAR_DELAY =     CYCLES_PER_SECOND*3,  // Time to hold down alarm clear button to clear instead of silence (cycles)
    BUTTON_DELAY =          CYCLES_PER_SECOND*3,  // Normal button hold delay (cycles)
    POWER_OFF_DELAY =       CYCLES_PER_SECOND*5,  // Time to hold power-off to transition to stand-by (off) state (cycles)
    POWERING_ON_TIME =      CYCLES_PER_SECOND*10, // Time to display alive-time when powering on (cycles)
    EDIT_TIMEOUT =          CYCLES_PER_SECOND*60, // One-minute in-action timeout on modify (cycles)
    DISPLAY_BLINK_CYCLES =  CYCLES_PER_SECOND/2,      // Total cycles for the blink period. ~2Hz
    DISPLAY_BLINK_OFF_CYCLES = CYCLES_PER_SECOND/4,   // Number of cycles the display is off. ~250ms
    SOUND_BEEP_DURATION_CYCLES = CYCLES_PER_SECOND/10, // Duration a beep is played for. ~100ms
    SETPOINT_MODIFY_TIMEOUT = CYCLES_PER_SECOND*10, // Ten second timeout while in modify state. Resets each time up/down button is pushed
    FIO2_MODIFY_TIMEOUT = CYCLES_PER_SECOND*60, // Ten second timeout while in modify state. Resets each time up/down button is pushed
    EEPROM_WRITE_TIMEOUT_CYCLES = 10, // Total number of cycles before an EEPROM write times out
    DISCONNECT_CMH20_CAP = 1, // 1cmH20
    BREATH_PERIOD_ADJUSTMENT =  10, // Breath period can be 0-20ms over-measured.  Subtract 10ms to get an average.
    // Thresholding initial constants
    FIO2_THRESHOLD_OFFSET = 3, // +/- 3% offset for FIO2
    PEEP_THRESHOLD_OFFSET = 50, // +/- cmH20
    TIDAL_THRESHOLD_OFFSET = 50, // +/- 3ML
    PEAK_THRESHOLD_OFFSET = 50, // +/- cmH20
    BUTTON_STUCK_CYCLES = CYCLES_PER_SECOND * 60 // 60 seconds of held down buttons trips a machine fault

} PanelConstants;

#endif /* INC_VENTILATOR_CONSTANTS_H_ */
