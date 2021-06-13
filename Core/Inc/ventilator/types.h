/*
 * types.h:
 *
 * Contains common type definitions for the system. These types are used across the system to represent data
 * types used in more than one module.
 *
 *  Created on: Apr 13, 2020
 *      Author: tcanham
 */
#include <stdint.h>
#include <stdbool.h>
#ifndef INC_VENTILATOR_TYPES_H_
#define INC_VENTILATOR_TYPES_H_

// Allows use of "STATIC" to keep variables static while removing this for unit-tests
#ifndef STATIC
    #define STATIC static
#endif
// Detect array length at compile time
#define ARRAY_LEN(x) (sizeof(x)/sizeof((x)[0]))

/**
 * Status of the alarm.  Three stages:
 *
 * 1. OFF: no alarm
 * 2. SET: alarm, but may be auto-cleared
 * 3. LATCHED: alarm is latched, must be cleared manually
 */
typedef enum {
    ALARM_OFF,
    ALARM_SET,
    ALARM_LATCH,
    ALARM_BLINK_ON,
    ALARM_BLINK_OFF
} AlarmStatus;

/**
 * Alarm:
 *
 * Alarm type containing the status, configuration settings, and active timing count of the alarm.
 */
typedef struct {
    AlarmStatus status; // Alarm is off, set, or latched
    uint32_t count; // Count used to time transitions between alarms
    uint32_t latch_time; // Latch time in counts
    uint32_t trip_time; // Trip time (set time) in counts
} Alarm;

/**
 * AlarmLedSet:
 *
 * Represents the uint16_t of ordered alarm LEDs for writing out to the shift register interface. This structure should be cleared before use, and then
 * each member can be assigned as needed. This type can be used directly as no special arrangement of bytes is needed.
 */
typedef struct {
    //5 most significant bits ignored
    Alarm disconnect;
    Alarm tidal_vol;
    Alarm peak_press;
    Alarm resp_rate;
    Alarm peep;
    Alarm fio2;
    Alarm machine_fault;
    Alarm low_power;
    Alarm power_off;
} Alarms;

/**
 * DisplayMode:
 *
 * The mode for the display. This indicates if we are actively editing a value/setpoint, or if we are not editing a value.  When
 * editing values, the display should blink the numerical display at a set rate so that the user is aware that the field is being
 * edited. When not editing, the display should be constant and not blinking.
 */
typedef enum {
    DISPLAY_VALUE,          // Currently displaying values only
    DISPLAY_SETPOINT,       // Currently displaying setpoint only
    DISPLAY_EDIT_ALARM,     // Editing an alarm shows editval returns to "DISPLAY_VALUE" when done editing
    DISPLAY_EDIT_SETPOINT,  // Editing a setpoint shows editval returns to "DISPLAY_SETPOINT" when done editing
    DISPLAY_EDIT_WITH_VALUE // Editing a with value shows val returns to "DISPLAY_VALUE" when done
} DisplayMode;

typedef struct {
    int32_t val;      // Value (active reading) from controller
    int32_t setpoint; // Setpoint (setting from operator) saved into the system
    int32_t editval;  // Current setting under edit
    int32_t step;     // Step size for display adjustment
    int32_t upper;    // Upper limit to the editpoint
    int32_t lower;    // Lower limit to the editpoint
    int32_t thresh_upper; // Threshold upper side for alarming
    int32_t thresh_lower; // Threshold lower side for alarming (should be negative)
    DisplayMode mode;
} NumericalValue;
/**
 * Set of all tracked values in the system. Combines set values from operator, and values read from controller.
 * This is the central GLOBAL state of the system.  Includes alarms for these values
 */
typedef struct {
    NumericalValue pressure_mean;
    NumericalValue pressure_min;
    NumericalValue pressure_plat;
    NumericalValue pressure;
    NumericalValue minute_volume;
    NumericalValue resp_rate;
    NumericalValue ins_time;
    NumericalValue peak_pressure;
    NumericalValue peak_pressure_average;
    NumericalValue backup_rate;
    NumericalValue tidal_volume;
    NumericalValue tidal_volume_last;
    NumericalValue PEEP;
    NumericalValue peep_pressure_average;
    NumericalValue FIO2;
    Alarms alarms;
} NumericalValues;
/**
 * Global power state of the system.
 */
typedef enum {
    POWER_OFF_STATE, // OFF (standby)
    POWERING_STATE,  // Powering on (displaying alive-hours)
    POWER_ON_STATE,  // ON and ventilating
} PowerState;

typedef enum {
    FAIL_SAFE_CLOCK_UNINITIALIZED = 0,
    FAIL_SAFE_CLOCK_RUNNING = 1,
    FAIL_SAFE_CLOCK_FAILED = 2
} FailSafeClockState;

/**
 * Statistics to communicate as telemetry.  **UNUSED** at this time.
 */
typedef struct {
    uint32_t maxCycle; //!< max cycle time
    uint32_t controlSpiErrors; //!< SPI CRC errors
    uint32_t switchI2CErrors; //!< switch I2C IOExpander errors
} FswStats;
/**
 * Statistics to communicate as telemetry.  **UNUSED** at this time.
 */
typedef struct {
    FswStats fswStats;
} UartDebug;


#endif /* INC_VENTILATOR_TYPES_H_ */
