/*
 * alarm.c:
 *
 * Handles the functionality associated with the alarms. This includes the following items:
 *
 * 1. Alarms trip when in violation of set alarm condition
 * 2. Alarm clearing through button pushes
 * 3. Alarm silence time after clearing
 * 4. Alarm timing for alarms triping over time. (deferred)
 * 5. Alarm auto clearing for auto-clear alarms. (deferred)
 *
 *  Created on: Apr 17, 2020
 *      Author: mstarch
 */
#include <ventilator/alarm.h>
#include <ventilator/sound.h>
#include <ventilator/types.h>
#include <ventilator/panel_public.h>
#include <swassert.h>

STATIC uint32_t ALARM_SOUND_COUNTDOWN = 0; //!< Countdown before alarm will redetect
STATIC uint32_t ALARM_BLINK_COUNTER = 0;

uint32_t alarm_run_state(Alarm* alarm, bool tripped) {
    SW_ASSERT(alarm);
    // Blinking state is highest priority, and is transitioned based on set conditions
    if (alarm->status == ALARM_BLINK_OFF || alarm->status == ALARM_BLINK_ON) {
        alarm->status = (ALARM_BLINK_COUNTER > DISPLAY_BLINK_OFF_CYCLES)? ALARM_BLINK_ON : ALARM_BLINK_OFF;
        return 1;
    }
    // Untripped latched alarm should emit a tone.  Note count latches too, to prevent overflow
    else if ((!tripped) && (alarm->status == ALARM_LATCH)) {
        return 1; // Tone because alarm is latched
    }
    // Untripped alarm that is not latched should auto-clear
    else if (!tripped) {
        alarm->count = 0;
        alarm->status = ALARM_OFF;
        return 0;
    }
    // Tripped alarm, run the alarm state machine
    alarm->count += 1;
    if (alarm->count >= alarm->latch_time || alarm->status == ALARM_LATCH) {
        alarm->status = ALARM_LATCH;
        return 1;
    } else if (alarm->count >= alarm->trip_time) {
        alarm->status = ALARM_SET;
        return 1;
    } else {
        alarm->status = ALARM_OFF;
        return 0;
    }
}

uint8_t alarm_detect(NumericalValues* values) {
    SW_ASSERT(values);
    // set up boolean for alarm tone
    uint8_t alarm_tone = 0;

    // Disconnect alarm
    bool tripped = (values->pressure.val < DISCONNECT_CMH20_CAP);
    alarm_tone = alarm_tone | alarm_run_state(&values->alarms.disconnect, tripped);

    // PEEP (lower) pressure alarm
    tripped = ((values->peep_pressure_average.val > (values->PEEP.setpoint + values->PEEP.thresh_upper)) ||
               (values->peep_pressure_average.val < (values->PEEP.setpoint + values->PEEP.thresh_lower)));
    alarm_tone = alarm_tone | alarm_run_state(&values->alarms.peep, tripped);

    // Tidal volume exceeds alarm value
    uint32_t ten_percent = (10*values->tidal_volume.setpoint)/100;
    tripped = (values->tidal_volume_last.val > (values->tidal_volume.setpoint + ten_percent) ||
               values->tidal_volume_last.val < (values->tidal_volume.setpoint - ten_percent));
    alarm_tone = alarm_tone | alarm_run_state(&values->alarms.tidal_vol, tripped);

    // Peak pressure exceeds alarm value
    tripped = ((values->peak_pressure_average.val > (values->peak_pressure.setpoint + values->peak_pressure.thresh_upper)));
    alarm_tone = alarm_tone | alarm_run_state(&values->alarms.peak_press, tripped);
    // Check for halt ventilation
    if (values->pressure.val > (values->peak_pressure.setpoint + values->peak_pressure.thresh_upper)) {
        p_haltVentilation = true;
        values->alarms.peep.status = ALARM_BLINK_ON;
    } else if (p_haltVentilation && (values->pressure.val >= (values->PEEP.setpoint + 5))) {
        p_haltVentilation = true;
    } else if (p_haltVentilation && (values->pressure.val < (values->PEEP.setpoint + 5))) {
        values->alarms.peep.status = ALARM_OFF;
        p_haltVentilation = false;
    }

    // Respiratory rate exceeds alarm value
    tripped = ((values->resp_rate.val > values->resp_rate.setpoint) ||
               (values->resp_rate.val < values->backup_rate.setpoint));
    alarm_tone = alarm_tone | alarm_run_state(&values->alarms.resp_rate, tripped);


    // FI02 exceeds alarm value
    tripped = (values->FIO2.val > (values->FIO2.setpoint + values->FIO2.thresh_upper) ||
               values->FIO2.val < (values->FIO2.setpoint + values->FIO2.thresh_lower));
    alarm_tone = alarm_tone | alarm_run_state(&values->alarms.fio2, tripped);

    // Check low battery, no need for state machine, this is driven directly from the GPIO
    tripped = (HAL_GPIO_ReadPin(GPIOB, LOW_BATTERY_Pin) == GPIO_PIN_RESET);
    alarm_tone = alarm_tone | alarm_run_state(&values->alarms.low_power, tripped);

    // Best attempt at machine fault
    tripped = SW_ASSERT_FLAG; // For parallelism with other alarms
    alarm_tone = alarm_tone | alarm_run_state(&values->alarms.machine_fault, tripped);
    return alarm_tone;
}

void alarm_clear(Alarms* alarms) {
    SW_ASSERT(alarms);
    alarm_silence(alarms); // Silence alarms first
    alarms->machine_fault.status = ALARM_OFF;
    alarms->power_off.status = ALARM_OFF;
    alarms->fio2.status = ALARM_OFF;
    alarms->resp_rate.status = ALARM_OFF;
    alarms->peak_press.status = ALARM_OFF;
    alarms->tidal_vol.status = ALARM_OFF;
    alarms->peep.status = ALARM_OFF;
    alarms->disconnect.status = ALARM_OFF;
    alarms->low_power.status = ALARM_OFF;
    // Clear counts too
    alarms->machine_fault.count = 0;
    alarms->power_off.count = 0;
    alarms->fio2.count = 0;
    alarms->resp_rate.count = 0;
    alarms->peak_press.count = 0;
    alarms->tidal_vol.count = 0;
    alarms->peep.count = 0;
    alarms->disconnect.count = 0;
    alarms->low_power.count = 0;
}

void alarm_silence(Alarms* alarms) {
    SW_ASSERT(alarms);
    // Only silence on any active alarms
    if (sound_is_alarming())
    {
        ALARM_SOUND_COUNTDOWN = ALARM_RESTART_DELAY; // Countdown until we will re-enable alarm sounds
        (void) sound_stop();
    }
}

void alarm_run(NumericalValues* values, PowerState state) {
    SW_ASSERT(values);
    // set up boolean for alarm tone
    uint8_t alarm_tone = alarm_detect(values);
    // Countdown on the alarm, and sound if the countdown is zero, and a tone should be generated
    ALARM_SOUND_COUNTDOWN = (ALARM_SOUND_COUNTDOWN == 0) ? 0 :   ALARM_SOUND_COUNTDOWN - 1;
    if ((0 == ALARM_SOUND_COUNTDOWN) && alarm_tone && (state == POWER_ON_STATE)) {
        (void) sound_start(SOUND_CONSTANT);
    }
    ALARM_BLINK_COUNTER = (ALARM_BLINK_COUNTER + 1) % DISPLAY_BLINK_CYCLES; // Need to blink at same 2Hz rate as display
}
