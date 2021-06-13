#include <stdint.h>
#include <string.h>
#include <test.h>
#include <ventilator/alarm.h>
#include <ventilator/initialize.h>

int sound_timer = 0;

// Needed module variables
extern uint32_t ALARM_SOUND_COUNTDOWN;
extern NumericalValues p_numericalValues;
extern uint8_t p_haltVentilation;

int test_alarm_run_state() {
    Alarm alarm_test;
    memset(&alarm_test, 0, sizeof(alarm_test));
    alarm_test.trip_time = 50;
    alarm_test.latch_time = 100;
    int ret = 0;
    // Walk through the alarm states
    for (int i = 0; i < 150; i++) {
        AlarmStatus stat = (i < 49) ? ALARM_OFF : ((i < 99) ? ALARM_SET : ALARM_LATCH);
        ret = alarm_run_state(&alarm_test, 1);  // Base test always tripped
        TEST_ASSERT(alarm_test.status == stat, "Status check failed");
        TEST_ASSERT(ret == (i >= 49), "Return check failed");
    }
    // Reset count to verify it stays latched after latching above
    alarm_test.count = 0;
    for (int i = 0; i < 150; i++) {
        ret = alarm_run_state(&alarm_test, 1); // Base test always tripped
        TEST_ASSERT(alarm_test.status == ALARM_LATCH, "Permanent latch failed");
        TEST_ASSERT(ret == 1, "Permanent fail failed");
    }
    // Not tripped tests: stays latched
    ret = alarm_run_state(&alarm_test, 0);
    TEST_ASSERT(alarm_test.status == ALARM_LATCH, "Permanent latch failed");
    TEST_ASSERT(ret == 1, "Permanent fail failed to tone");

    // Set alarm will clear
    alarm_test.status = ALARM_SET;
    ret = alarm_run_state(&alarm_test, 0);
    TEST_ASSERT(alarm_test.status == ALARM_OFF, "Auto clear set alarm failed");
    TEST_ASSERT(ret == 0, "Auto clear failed to clear tone");
    return 0;
}

int test_alarm_silence() {
    sound_init(&sound_timer);
    sound_start(SOUND_CONSTANT);
    TEST_ASSERT(sound_running, "Could not start sound. Check sound UT");
    ALARM_SOUND_COUNTDOWN = 0;

    // Start with no alarms, no reset should be performed
    Alarms alarms;
    memset(&alarms, 0, sizeof(alarms));
    sound_start(SOUND_OFF);
    alarm_silence(&alarms);
    TEST_ASSERT(ALARM_SOUND_COUNTDOWN == 0, "Alarm restart delay restarted without any latched alarms");

    // Must start alarm
    sound_start(SOUND_CONSTANT);
    alarms.disconnect.status = ALARM_SET;
    alarm_silence(&alarms);
    TEST_ASSERT(ALARM_SOUND_COUNTDOWN == ALARM_RESTART_DELAY, "Alarm restart delay not restarted");
    TEST_ASSERT(!sound_running, "Alarms did not silence sound");
    sound_start(SOUND_CONSTANT);
    ALARM_SOUND_COUNTDOWN = 0;

    alarms.disconnect.status = ALARM_LATCH;
    alarm_silence(&alarms);
    TEST_ASSERT(ALARM_SOUND_COUNTDOWN == ALARM_RESTART_DELAY, "Alarm restart delay not restarted");
    TEST_ASSERT(!sound_running, "Alarms did not silence sound");
    sound_start(SOUND_CONSTANT);
    ALARM_SOUND_COUNTDOWN = 0;

    return 0;
}

int test_alarm_clear() {
    Alarms alarms;
    memset(&alarms, 0, sizeof(alarms));

    AlarmStatus status = ALARM_OFF;
    // Loop through all statuses
    for (AlarmStatus status = ALARM_OFF; status < ALARM_LATCH; status++) {
        ALARM_SOUND_COUNTDOWN = 0;
        alarms.machine_fault.status = status;
        alarms.power_off.status = status;
        alarms.fio2.status = status;
        alarms.resp_rate.status = status;
        alarms.peak_press.status = status;
        alarms.tidal_vol.status = status;
        alarms.peep.status = status;
        alarms.disconnect.status = status;
        if (status == ALARM_OFF) {
            sound_start(SOUND_OFF);
        } else {
            sound_start(SOUND_CONSTANT);
        }
        alarm_clear(&alarms);
        TEST_ASSERT(alarms.machine_fault.status == ALARM_OFF, "Machine fault not cleared");
        TEST_ASSERT(alarms.power_off.status == ALARM_OFF, "Power off not cleared");
        TEST_ASSERT(alarms.fio2.status == ALARM_OFF, "FIO2 not cleared");
        TEST_ASSERT(alarms.resp_rate.status == ALARM_OFF, "Resp rate not cleared");
        TEST_ASSERT(alarms.peak_press.status == ALARM_OFF, "Peak press fault not cleared");
        TEST_ASSERT(alarms.tidal_vol.status == ALARM_OFF, "Tidal vol fault not cleared");
        TEST_ASSERT(alarms.peep.status == ALARM_OFF, "Peep not cleared");
        TEST_ASSERT(alarms.disconnect.status == ALARM_OFF, "Disconnect not cleared");
        uint32_t testable = (status == ALARM_OFF) ? (ALARM_SOUND_COUNTDOWN == 0) : (ALARM_SOUND_COUNTDOWN == ALARM_RESTART_DELAY);
        TEST_ASSERT(testable, "Alarms did not silence sound");
    }
    return 0;
}

int test_alarm_run() {
    NumericalValues values;
    memset(&values, 0, sizeof(values)); // All counts are set to zero.  One out-of-val will trigger
    PowerState state = POWER_OFF_STATE;
    ALARM_SOUND_COUNTDOWN = 0;
    alarm_run(&values, state); //Should not trip
    TEST_ASSERT(!sound_running, "Alarms should not trip in off state");

    state = POWER_ON_STATE;
    alarm_run(&values, state); //Should trip
    TEST_ASSERT(sound_running, "Alarms should be alarming");
    // Silence the alarms
    alarm_silence(&values.alarms);
    TEST_ASSERT(!sound_running, "Alarms should be silenced");
    int i = 0;
    for (i = 0; i < ALARM_RESTART_DELAY - 1; i++) {
        // Loop across  the whole time
        alarm_run(&values, state); //Should trip but stay quiet
        TEST_ASSERT(!sound_running, "Alarms should be silenced");
    }
    alarm_run(&values, state); //Should trip
    TEST_ASSERT(sound_running, "Alarms should be alarming again");
    return 0;
}

void initialize_alarm_values(NumericalValues* values, int alarm) {
    initialize_numeric_values(values);
    p_haltVentilation = false;
    // Some setpoints
    values->PEEP.setpoint = 50;
    values->tidal_volume.setpoint = 900;
    values->peak_pressure.setpoint = 50; // Over only
    values->resp_rate.setpoint = 30; // Over only
    values->backup_rate.setpoint = 20; // Under only
    values->FIO2.setpoint = 30;

    // Alarm values for both alarm, and non-alarm case
    values->pressure.val = alarm ? 0 : 49;
    values->FIO2.val = alarm ? 0 : 30;
    values->resp_rate.val = alarm ? 0 : 25;
    values->peak_pressure_average.val = alarm ? 99 : 0;
    values->tidal_volume_last.val = alarm ? 0 : 900;
    values->peep_pressure_average.val = alarm ? 0 : 50;

    SW_ASSERT_FLAG = alarm ? 1 : 0;
    GPIO_READ_TEST_VALUE = alarm ? 0 : 1;  // Good battery
    // Forced to alarm, set all counts for alarms to the latch value
    if (alarm) {
        Alarms* alarms = &values->alarms;
        alarms->disconnect.count = alarms->disconnect.latch_time;
        alarms->fio2.count = alarms->fio2.latch_time;
        alarms->low_power.count = alarms->low_power.latch_time;
        alarms->machine_fault.count = alarms->machine_fault.latch_time;
        alarms->peak_press.count = alarms->peak_press.latch_time;
        alarms->peep.count = alarms->peep.latch_time;
        alarms->resp_rate.count = alarms->resp_rate.latch_time;
        alarms->tidal_vol.count = alarms->tidal_vol.latch_time;
    }
}


int test_alarm_detect_disconnect() {
    NumericalValues values;
    initialize_alarm_values(&values, 0); // No forced alarms
    // Loop over valid range
    TEST_ASSERT(!p_haltVentilation, "Halt ventilation unexpectedly on");
    values.peak_pressure.setpoint = 100;
    int i = 0;
    for (i = 100; i >= 0; i--) {
        values.pressure.val = i;
        values.alarms.disconnect.status = ALARM_OFF;
        values.alarms.disconnect.count = 0;
        int count = 0;
        AlarmStatus expected = ALARM_OFF;
        for (count = 1; count < 200; count++) {
            int tone = alarm_detect(&values);
            TEST_ASSERT(!p_haltVentilation, "Halt ventilation unexpectedly on");
            // Alarm should latch
            if (i < DISCONNECT_CMH20_CAP && count >= (2 * 50)) { // 2 second kill switch
                TEST_ASSERT(values.alarms.disconnect.status == ALARM_LATCH, "Disconnect alarm didn't latch.");
                TEST_ASSERT(tone == 1, "Tone not set properly");
                expected = ALARM_LATCH;
            } else {
                TEST_ASSERT(values.alarms.disconnect.status == ALARM_OFF, "Disconnect alarm latched unexpectedly.");
                TEST_ASSERT(tone == 0, "Tone set improperly");
            }
        }
        values.pressure.val = 99;
        int tone = alarm_detect(&values);
        TEST_ASSERT(!p_haltVentilation, "Halt ventilation unexpectedly on");
        TEST_ASSERT(values.alarms.disconnect.status == expected, "Disconnect alarm didn't stay latched.");
        TEST_ASSERT(tone == (expected == ALARM_LATCH), "Tone not set properly");
    }
    return 0;
}

int test_alarm_detect_peep() {
    NumericalValues values;
    initialize_alarm_values(&values, 0); // No forced alarms
    TEST_ASSERT(!p_haltVentilation, "Halt ventilation unexpectedly on");
    // Loop over valid range
    int i = 0;
    for (i = 0; i < 100; i++) {
        // Loop over "count" bounds to test auto-clear.  Until bound is >= 750, the alarm should auto-clear
        int bound = 0;
        for (bound = 1; bound < (50 * 20); bound = bound + 5) {
            values.peep_pressure_average.val = i;
            values.alarms.peep.status = ALARM_OFF;
            values.alarms.peep.count = 0;
            AlarmStatus expected = ALARM_OFF;
            int count = 0;
            for (count = 1; count < bound; count++) {
                int tone = alarm_detect(&values);
                TEST_ASSERT(!p_haltVentilation, "Halt ventilation unexpectedly on");
                // Alarm should latch
                if ((i < (values.PEEP.setpoint - 3) || i > (values.PEEP.setpoint + 3)) && count >= (15*50)) {
                    TEST_ASSERT(values.alarms.peep.status == ALARM_LATCH, "peep alarm didn't latch.");
                    TEST_ASSERT(tone == 1, "Tone not set properly");
                    expected = ALARM_LATCH;
                }
                else if ((i < (values.PEEP.setpoint - 3) || i > (values.PEEP.setpoint + 3))) {
                    TEST_ASSERT(values.alarms.peep.status == ALARM_SET, "peep alarm didn't set.");
                    TEST_ASSERT(tone == 1, "Tone not set properly");
                    expected = ALARM_OFF;
                } else {
                    TEST_ASSERT(values.alarms.peep.status == ALARM_OFF, "peep alarm latched unexpectedly.");
                    TEST_ASSERT(tone == 0, "Tone set improperly");
                }
            }
            values.peep_pressure_average.val = values.PEEP.setpoint;
            int tone = alarm_detect(&values);
            TEST_ASSERT(!p_haltVentilation, "Halt ventilation unexpectedly on");
            TEST_ASSERT(values.alarms.peep.status == expected, "peep alarm didn't stay latched.");
            TEST_ASSERT(tone == (expected == ALARM_LATCH), "Tone not set properly");
        }
    }
    return 0;
}


int test_alarm_detect_tidal_volume() {
    NumericalValues values;
    initialize_alarm_values(&values, 0); // No forced alarms
    TEST_ASSERT(!p_haltVentilation, "Halt ventilation unexpectedly on");
    // Loop over valid range
    int i = 0;
    for (i = 0; i < 1000; i++) {
        // Loop over "count" bounds to test auto-clear.  Until bound is >= 750, the alarm should auto-clear
        int bound = 0;
        for (bound = 1; bound < (50 * 20); bound = bound + 5) {
            values.tidal_volume_last.val = i;
            values.alarms.tidal_vol.status = ALARM_OFF;
            values.alarms.tidal_vol.count = 0;
            AlarmStatus expected = ALARM_OFF;
            int count = 0;
            for (count = 1; count < bound; count++) {
                int tone = alarm_detect(&values);
                TEST_ASSERT(!p_haltVentilation, "Halt ventilation unexpectedly on");
                // Alarm should latch
                if ((i < ((int)((float)values.tidal_volume.setpoint *0.9f)) || (i > ((int)((float)values.tidal_volume.setpoint *1.10f)))) && count >= (12*50)) {
                    TEST_ASSERT(values.alarms.tidal_vol.status == ALARM_LATCH, "tidal alarm didn't latch.");
                    TEST_ASSERT(tone == 1, "Tone not set properly");
                    expected = ALARM_LATCH;
                }
                else if ((i < ((int)((float)values.tidal_volume.setpoint *0.9f)) || (i > ((int)((float)values.tidal_volume.setpoint *1.10f))))) {
                    TEST_ASSERT(values.alarms.tidal_vol.status == ALARM_SET, "tidal alarm didn't set.");
                    TEST_ASSERT(tone == 1, "Tone not set properly");
                    expected = ALARM_OFF;
                } else {
                    TEST_ASSERT(values.alarms.tidal_vol.status == ALARM_OFF, "tidal alarm tripped unexpectedly.");
                    TEST_ASSERT(tone == 0, "Tone set improperly");
                }
            }
            values.tidal_volume_last.val = values.tidal_volume.setpoint;
            int tone = alarm_detect(&values);
            TEST_ASSERT(!p_haltVentilation, "Halt ventilation unexpectedly on");
            TEST_ASSERT(values.alarms.tidal_vol.status == expected, "tidal alarm didn't stay latched.");
            TEST_ASSERT(tone == (expected == ALARM_LATCH), "Tone not set properly");
        }
    }
    return 0;
}

int test_alarm_detect_peak_pressure() {
    NumericalValues values;
    initialize_alarm_values(&values, 0); // No forced alarms
    TEST_ASSERT(!p_haltVentilation, "Halt ventilation unexpectedly on");
    // Loop over valid range
    int i = 0;
    for (i = 0; i < 100; i++) {
        values.peak_pressure_average.val = i;
        values.alarms.peak_press.status = ALARM_OFF;
        values.alarms.peak_press.count = 0;
        AlarmStatus expected = ALARM_OFF;
        int count = 0;
        for (count = 1; count < 100; count++) {
            int tone = alarm_detect(&values);
            // Alarm should latch
            if ((i > (values.peak_pressure.setpoint + 5)) && count >= 0) {
                TEST_ASSERT(values.alarms.peak_press.status == ALARM_LATCH, "peak pressure alarm didn't latch.");
                TEST_ASSERT(tone == 1, "Tone not set properly");
                expected = ALARM_LATCH;
                TEST_ASSERT(!p_haltVentilation, "Halt ventilation unexpectedly on");
            }
            else {
                TEST_ASSERT(values.alarms.peak_press.status == ALARM_OFF, "peak pressure tripped unexpectedly.");
                TEST_ASSERT(tone == 0, "Tone set improperly");
                TEST_ASSERT(!p_haltVentilation, "Halt ventilation unexpectedly on");
            }
        }
        values.peak_pressure_average.val = values.peak_pressure.setpoint;
        int tone = alarm_detect(&values);
        TEST_ASSERT(!p_haltVentilation, "Halt ventilation unexpectedly on");
        TEST_ASSERT(values.alarms.peak_press.status == expected, "peak pressure alarm didn't stay latched.");
        TEST_ASSERT(tone == (expected == ALARM_LATCH), "Tone not set properly");
    }
    return 0;
}

int test_alarm_detect_peak_pressure_halt() {
    NumericalValues values;
    initialize_alarm_values(&values, 0); // No forced alarms
    TEST_ASSERT(!p_haltVentilation, "Halt ventilation unexpectedly on");
    // Loop over valid range
    int i = 0;
    for (i = 0; i < 100; i++) {
        values.pressure.val = i;
        (void) alarm_detect(&values);
        // Alarm should latch
        if (i > (values.peak_pressure.setpoint + 5)) {
            TEST_ASSERT(p_haltVentilation, "Halt ventilation unexpectedly on");
        }
        else {
            TEST_ASSERT(!p_haltVentilation, "Halt ventilation unexpectedly on");
        }
    }
    // Loop over valid range
    for (i = 100; i >= 0; i--) {
        values.pressure.val = i;
        (void) alarm_detect(&values);
        // Alarm should latch
        if (i >= (values.PEEP.setpoint + 5)) {
            TEST_ASSERT(p_haltVentilation, "Halt ventilation unexpectedly on");
        }
        else {
            TEST_ASSERT(!p_haltVentilation, "Halt ventilation unexpectedly on");
        }
    }
    return 0;
}

int test_alarm_detect_resp_rate() {
    NumericalValues values;
    initialize_alarm_values(&values, 0); // No forced alarms
    TEST_ASSERT(!p_haltVentilation, "Halt ventilation unexpectedly on");
    // Loop over valid range
    int i = 0;
    for (i = 0; i < 100; i++) {
        values.resp_rate.val = i;
        values.alarms.resp_rate.status = ALARM_OFF;
        values.alarms.resp_rate.count = 0;
        AlarmStatus expected = ALARM_OFF;
        int count = 0;
        for (count = 1; count < 100; count++) {
            int tone = alarm_detect(&values);
            TEST_ASSERT(!p_haltVentilation, "Halt ventilation unexpectedly on");
            // Alarm should latch
            if (i < (values.backup_rate.setpoint) || i > (values.resp_rate.setpoint)) {
                TEST_ASSERT(values.alarms.resp_rate.status == ALARM_LATCH, "resp_rate alarm didn't latch.");
                TEST_ASSERT(tone == 1, "Tone not set properly");
                expected = ALARM_LATCH;
            } else {
                TEST_ASSERT(values.alarms.resp_rate.status == ALARM_OFF, "resp_rate alarm latched unexpectedly.");
                TEST_ASSERT(tone == 0, "Tone set improperly");
            }
        }
        values.resp_rate.val = values.resp_rate.setpoint;
        int tone = alarm_detect(&values);
        TEST_ASSERT(!p_haltVentilation, "Halt ventilation unexpectedly on");
        TEST_ASSERT(values.alarms.resp_rate.status == expected, "resp_rate alarm didn't stay latched.");
        TEST_ASSERT(tone == (expected == ALARM_LATCH), "Tone not set properly");
    }
    return 0;
}

int test_alarm_detect_fi02() {
    NumericalValues values;
    initialize_alarm_values(&values, 0); // No forced alarms
    TEST_ASSERT(!p_haltVentilation, "Halt ventilation unexpectedly on");
    // Loop over valid range
    int i = 0;
    for (i = 0; i < 100; i++) {
        values.FIO2.val = i;
        values.alarms.fio2.status = ALARM_OFF;
        values.alarms.fio2.count = 0;
        AlarmStatus expected = ALARM_OFF;
        int count = 0;
        for (count = 1; count < (300 * 50); count++) {
            int tone = alarm_detect(&values);
            TEST_ASSERT(!p_haltVentilation, "Halt ventilation unexpectedly on");
            // Alarm should latch
            if ((i < (values.FIO2.setpoint - 10) || i > (values.FIO2.setpoint + 10)) && (count >= (200 * 50))) {
                TEST_ASSERT(values.alarms.fio2.status == ALARM_LATCH, "fio2 alarm didn't latch.");
                TEST_ASSERT(tone == 1, "Tone not set properly");
                expected = ALARM_LATCH;
            } else {
                TEST_ASSERT(values.alarms.fio2.status == ALARM_OFF, "fio2 alarm latched unexpectedly.");
                TEST_ASSERT(tone == 0, "Tone set improperly");
            }
        }
        values.FIO2.val = values.FIO2.setpoint;
        int tone = alarm_detect(&values);
        TEST_ASSERT(!p_haltVentilation, "Halt ventilation unexpectedly on");
        TEST_ASSERT(values.alarms.fio2.status == expected, "fio2 alarm didn't stay latched.");
        TEST_ASSERT(tone == (expected == ALARM_LATCH), "Tone not set properly");
    }
    return 0;
}
int test_alarm_detect_low_power() {
    NumericalValues values;
    initialize_alarm_values(&values, 0); // No forced alarms
    TEST_ASSERT(!p_haltVentilation, "Halt ventilation unexpectedly on");
    // Loop over valid range
    int i = 0;
    for (i = 1; i >= 0; i--) {
        GPIO_READ_TEST_VALUE = i;
        values.alarms.low_power.status = ALARM_OFF;
        values.alarms.low_power.count = 0;
        AlarmStatus expected = ALARM_OFF;
        int count = 0;
        for (count = 1; count < 5; count++) {
            int tone = alarm_detect(&values);
            TEST_ASSERT(!p_haltVentilation, "Halt ventilation unexpectedly on");
            // Alarm should latch
            if (i == 0) {
                TEST_ASSERT(values.alarms.low_power.status == ALARM_LATCH, "fio2 alarm didn't latch.");
                TEST_ASSERT(tone == 1, "Tone not set properly");
                expected = ALARM_LATCH;
            } else {
                TEST_ASSERT(values.alarms.low_power.status == ALARM_OFF, "fio2 alarm latched unexpectedly.");
                TEST_ASSERT(tone == 0, "Tone set improperly");
            }
        }
        GPIO_READ_TEST_VALUE = 1;
        int tone = alarm_detect(&values);
        TEST_ASSERT(!p_haltVentilation, "Halt ventilation unexpectedly on");
        TEST_ASSERT(values.alarms.low_power.status == expected, "fio2 alarm didn't stay latched.");
        TEST_ASSERT(tone == (expected == ALARM_LATCH), "Tone not set properly");
    }
    SW_ASSERT_FLAG = 0; // Prevent cascading assert up
    return 0;
}
int test_alarm_detect_sw_assert() {
    NumericalValues values;
    initialize_alarm_values(&values, 0); // No forced alarms
    TEST_ASSERT(!p_haltVentilation, "Halt ventilation unexpectedly on");
    // Loop over valid range
    int i = 0;
    for (i = 0; i <= 1; i++) {
        SW_ASSERT_FLAG = i;
        values.alarms.machine_fault.status = ALARM_OFF;
        values.alarms.machine_fault.count = 0;
        AlarmStatus expected = ALARM_OFF;
        int count = 0;
        for (count = 1; count < 5; count++) {
            int tone = alarm_detect(&values);
            SW_ASSERT_FLAG = 0; // Romve flag to prevent cascading failure
            TEST_ASSERT(!p_haltVentilation, "Halt ventilation unexpectedly on");
            // Alarm should latch
            if (i == 1) {
                TEST_ASSERT(values.alarms.machine_fault.status == ALARM_LATCH, "fio2 alarm didn't latch.");
                TEST_ASSERT(tone == 1, "Tone not set properly");
                expected = ALARM_LATCH;
            } else {
                TEST_ASSERT(values.alarms.machine_fault.status == ALARM_OFF, "fio2 alarm latched unexpectedly.");
                TEST_ASSERT(tone == 0, "Tone set improperly");
            }
        }
        SW_ASSERT_FLAG = 0;
        int tone = alarm_detect(&values);
        TEST_ASSERT(!p_haltVentilation, "Halt ventilation unexpectedly on");
        TEST_ASSERT(values.alarms.machine_fault.status == expected, "fio2 alarm didn't stay latched.");
        TEST_ASSERT(tone == (expected == ALARM_LATCH), "Tone not set properly");
    }
    SW_ASSERT_FLAG = 0; // Prevent cascading assert up
    return 0;
}

int test_alarm_all() {
    NumericalValues values;
    initialize_alarm_values(&values, 1); // Forced alarms
    int tone = alarm_detect(&values);
    SW_ASSERT_FLAG = 0;
    TEST_ASSERT(tone, "Tone not set for all forced alarms");
    Alarms* alarms = &values.alarms;
    TEST_ASSERT(alarms->disconnect.status != ALARM_OFF, "Alarm is still off.");
    TEST_ASSERT(alarms->fio2.status != ALARM_OFF, "Alarm is still off.");
    TEST_ASSERT(alarms->low_power.status != ALARM_OFF, "Alarm is still off.");
    TEST_ASSERT(alarms->machine_fault.status != ALARM_OFF, "Alarm is still off.");
    TEST_ASSERT(alarms->peak_press.status != ALARM_OFF, "Alarm is still off.");
    TEST_ASSERT(alarms->peep.status != ALARM_OFF, "Alarm is still off.");
    TEST_ASSERT(alarms->resp_rate.status != ALARM_OFF, "Alarm is still off.");
    TEST_ASSERT(alarms->tidal_vol.status != ALARM_OFF, "Alarm is still off.");
    TEST_ASSERT(!p_haltVentilation, "Halt ventilation did not come on"); // Conflicts with disconnect
    return 0;
}

int main(int argc, char** argv) {
    TEST(test_alarm_run_state);
    TEST(test_alarm_silence);
    TEST(test_alarm_clear);
    TEST(test_alarm_run);
    TEST(test_alarm_detect_disconnect);
    TEST(test_alarm_detect_peep);
    TEST(test_alarm_detect_tidal_volume);
    TEST(test_alarm_detect_peak_pressure);
    TEST(test_alarm_detect_peak_pressure_halt);
    TEST(test_alarm_detect_resp_rate);
    TEST(test_alarm_detect_fi02);
    TEST(test_alarm_detect_low_power);
    TEST(test_alarm_detect_sw_assert);
    TEST(test_alarm_all);

}
