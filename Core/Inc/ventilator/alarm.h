/*
 * alarm.h:
 *
 * Alarms set both sound, and LEDs to indicate some off-nominal event to the operator of the device. Alarms are configured per the user-manual
 * to have three states "off", indicating no alarm condition,  "set", indicating an alarm condition that may resolve within a set interval and
 * auto-clear, and "latch", indicating that an alarm condition must be manually cleared by the operator.
 *
 *  Created on: Apr 17, 2020
 *      Author: mstarch
 */

#ifndef INC_VENTILATOR_ALARM_H_
#define INC_VENTILATOR_ALARM_H_
#include <ventilator/types.h>
#include <ventilator/sound.h>

/**
 * alarm_clear:
 *
 * Clears the alarm status. This will call "alarm_silence" and then will clear any latched alarms. Sound will remain off per "alarm_silence"
 * but set and latch counts of the alarms will continue to count, and possibly re-latch.
 *
 * Alarms* alarms: alarms object to read and update.
 */
void alarm_clear(Alarms* alarms);
/**
 * alarm_silence:
 *
 * Silences any alarms being emitted by the device. This will keep all alarms (new and old) from triggering sound for a specified duration
 * of time. This does not clear the alarm LEDs, only the sound. Note: the actual timing is detected in "alarm_detect", which handles the
 * timing of alarms. Note: this *will not* silence alarms if none are active.
 *
 * Alarms* alarms: alarms object to read and update.
 */
void alarm_silence();
/**
 * alarm_detect:
 *
 * Detect any new alarms. An alarm condition that has been detected will increment the count of the alarm.  When the count goes above
 * the set count of the alarm, the alarm will be "set".  When the alarm goes above the "latch" count of the alarm, the alarm will "latch".
 * Latching takes precedence over setting. Calls "alarm_run" to determine counts, and timing of an alarm.
 *
 * NumericalValues* values: numerical values used to detect the alarms states for off-nominal conditions
 *
 * \return: 1 if an alarm should sound 0 otherwise
 */
uint8_t alarm_detect(NumericalValues* values);
/**
 * Run the alarm state machines with respect to the given power state. This will handle timing of the alarms states and walk through the
 * set and latched states.
 *
 * Alarm* alarm: alarm to walk through the the states
 * bool tripped: is the above alarm currently in violation
 */
uint32_t alarm_run_state(Alarm* alarm, bool tripped);
/**
 * Runs the alarm module by detecting alarms for new trip states.
 * NumericalValues* values: values to check and alarms object to update:
 * PowerState state: current powered state of the system
 */
void alarm_run(NumericalValues* values, PowerState state);

#endif /* INC_VENTILATOR_ALARM_H_ */
