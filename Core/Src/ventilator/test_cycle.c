/**
 * test_cycle:
 *
 * Test the cycle.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <ventilator/button.h>
#include <ventilator/cycle.h>
#include <string.h>
#include <ventilator/panel_public.h>
#include <swassert.h>
#include <string.h>
#include <ventilator/test_cycle.h>

//#define SPECIFIC_STATE TEST_VOLUME

typedef enum {
    TEST_FI02, // Test FIO2 display
    TEST_PEEP,
    TEST_VOLUME,
    TEST_BACKUP,
    TEST_PEAK,
    TEST_TIME,
    TEST_RESP,
    TEST_MINUTE,
    TEST_DISCONNECT_LED,
    TEST_PEEP_LED,
    TEST_TIDAL_LED,
    TEST_PEAK_LED,
    TEST_RESP_LED,
    TEST_FI02_LED,
    TEST_POWER_OFF,
    TEST_LO_PWER_LED,
    TEST_FAULT_LED,
    TEST_TIDAL_BAR,
    TEST_PRESSURE_BAR_L,
    TEST_PRESSURE_BAR_R_UP,
    TEST_PRESSURE_BAR_R_MID,
    TEST_PRESSURE_BAR_R_LOW,
    TEST_PRESSURE_BAR_R_PLAT,
    TEST_BUTTONS,
    TEST_BEEP,
    TEST_BEEP_BEEP,
    TEST_CONTINUOUS_10
} TestDisplayState;

TestDisplayState testState = TEST_FI02;

// a set of test values for the LED segments
uint32_t twoDigitMap[]  = {
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    10,
    20,
    30,
    40,
    50,
    60,
    70,
    80,
    90
};

uint32_t threeDigitMap[]  = {
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    10,
    20,
    30,
    40,
    50,
    60,
    70,
    80,
    90,
    100,
    200,
    300,
    400,
    500,
    600,
    700,
    800,
    900
};

uint32_t tidalMap[]  = {
    100,
    120,
    140,
    160,
    180,
    200,
    220,
    240,
    260,
    280,
    300,
    320,
    340,
    360,
    380,
    400,
    420,
    440,
    460,
    480,
    500,
    520,
    540,
    560,
    580,
    600,
    620,
    640,
    660,
    680,
    700,
    720,
    740,
    760,
    780,
    800,
    820,
    840,
    860,
    880,
    900
};

uint32_t pbrLMap[]  = {
    0,
    3,
    5,
    8,
    10,
    13,
    15,
    18,
    20,
    23,
    25,
    28,
    30,
    33,
    35,
    38,
    40,
    43,
    45,
    48,
    50,
    53,
    55,
    58,
    60,
    63,
    65,
    68,
    70,
    73,
    75,
    78,
    80,
    83,
    85,
    88,
    90,
    93,
    95,
    98,
    100
};

uint32_t currDigit = 0;

bool doTestCycle(void) {
    uint8_t inc = 0;
// If a specific state is being tested, drive the machine into that specific state
#ifdef SPECIFIC_STATE
    testState = SPECIFIC_STATE;
#endif
    // Sound cycling should happen first
    sound_cycle();
    switch (testState) {
        case TEST_FI02:
            p_numericalValues.FIO2.val = twoDigitMap[currDigit];
            p_numericalValues.FIO2.setpoint = twoDigitMap[currDigit];
            p_numericalValues.FIO2.editval = twoDigitMap[currDigit];
            inc = 1;
            if (currDigit == ARRAY_LEN(twoDigitMap)-1) {
                testState = TEST_PEEP;
                currDigit = 0;
                inc = 0;
            }
            // write display value here
            break;
        case TEST_PEEP:
            p_numericalValues.PEEP.val = twoDigitMap[currDigit];
            p_numericalValues.PEEP.setpoint = twoDigitMap[currDigit];
            p_numericalValues.PEEP.editval = twoDigitMap[currDigit];
            inc = 1;
            if (currDigit == ARRAY_LEN(twoDigitMap)-1) {
                testState = TEST_VOLUME;
                currDigit = 0;
                inc = 0;
            }
            break;
        case TEST_VOLUME:
            p_numericalValues.tidal_volume.val = threeDigitMap[currDigit];
            p_numericalValues.tidal_volume.setpoint = threeDigitMap[currDigit];
            p_numericalValues.tidal_volume.editval = threeDigitMap[currDigit];
            inc = 1;
            if (currDigit == ARRAY_LEN(threeDigitMap)-1) {
                testState = TEST_BACKUP;
                currDigit = 0;
                inc = 0;
            }
            break;
        case TEST_BACKUP:
            p_numericalValues.backup_rate.val = twoDigitMap[currDigit];
            p_numericalValues.backup_rate.setpoint = twoDigitMap[currDigit];
            p_numericalValues.backup_rate.editval = twoDigitMap[currDigit];
            inc = 1;
            if (currDigit == ARRAY_LEN(twoDigitMap)-1) {
                testState = TEST_PEAK;
                currDigit = 0;
                inc = 0;
            }
            break;
        case TEST_PEAK:
            p_numericalValues.peak_pressure.val = twoDigitMap[currDigit];
            p_numericalValues.peak_pressure.setpoint = twoDigitMap[currDigit];
            p_numericalValues.peak_pressure.editval = twoDigitMap[currDigit];
            inc = 1;
            if (currDigit == ARRAY_LEN(twoDigitMap)-1) {
                testState = TEST_TIME;
                currDigit = 0;
                inc = 0;
            }
            break;
        case TEST_TIME:
            p_numericalValues.ins_time.val = twoDigitMap[currDigit];
            p_numericalValues.ins_time.setpoint = twoDigitMap[currDigit];
            p_numericalValues.ins_time.editval = twoDigitMap[currDigit];
            inc = 1;
            if (currDigit == ARRAY_LEN(twoDigitMap)-1) {
                testState = TEST_RESP;
                currDigit = 0;
                inc = 0;
            }
            break;
        case TEST_RESP:
            p_numericalValues.resp_rate.val = twoDigitMap[currDigit];
            p_numericalValues.resp_rate.setpoint = twoDigitMap[currDigit];
            p_numericalValues.resp_rate.editval = twoDigitMap[currDigit];
            inc = 1;
            if (currDigit == ARRAY_LEN(twoDigitMap)-1) {
                testState = TEST_MINUTE;
                currDigit = 0;
                inc = 0;
            }
            break;
        case TEST_MINUTE:
            p_numericalValues.minute_volume.val = twoDigitMap[currDigit];
            p_numericalValues.minute_volume.setpoint = twoDigitMap[currDigit];
            p_numericalValues.minute_volume.editval = twoDigitMap[currDigit];
            inc = 1;
            if (currDigit == ARRAY_LEN(twoDigitMap)-1) {
                testState = TEST_DISCONNECT_LED;
                currDigit = 0;
                inc = 0;
            }
            break;
        case TEST_DISCONNECT_LED:
            p_numericalValues.alarms.disconnect.status = ALARM_LATCH;
            testState = TEST_PEEP_LED;
            break;
        case TEST_PEEP_LED:
            p_numericalValues.alarms.peep.status = ALARM_LATCH;
            testState = TEST_TIDAL_LED;
            break;
        case TEST_TIDAL_LED:
            p_numericalValues.alarms.tidal_vol.status = ALARM_LATCH;
            testState = TEST_PEAK_LED;
            break;
        case TEST_PEAK_LED:
            p_numericalValues.alarms.peak_press.status = ALARM_LATCH;
            testState = TEST_RESP_LED;
            break;
        case TEST_RESP_LED:
            p_numericalValues.alarms.resp_rate.status = ALARM_LATCH;
            testState = TEST_FI02_LED;
            break;
        case TEST_FI02_LED:
            p_numericalValues.alarms.fio2.status = ALARM_LATCH;
            testState = TEST_POWER_OFF;
            break;
        case TEST_POWER_OFF:
            p_numericalValues.alarms.power_off.status = ALARM_LATCH;
            testState = TEST_LO_PWER_LED;
            break;
        case TEST_LO_PWER_LED:
            p_numericalValues.alarms.low_power.status  = ALARM_LATCH;
            testState = TEST_FAULT_LED;
            break;
        case TEST_FAULT_LED:
            p_numericalValues.alarms.machine_fault.status  = ALARM_LATCH;
            testState = TEST_TIDAL_BAR;
            currDigit = 0;
            inc = 0;
            break;
        case TEST_TIDAL_BAR:
            p_numericalValues.tidal_volume.val = tidalMap[currDigit];
            p_numericalValues.tidal_volume.setpoint = tidalMap[currDigit];
            p_numericalValues.tidal_volume.editval = tidalMap[currDigit];
            inc = 1;
            if (currDigit == ARRAY_LEN(tidalMap)-1) {
                testState = TEST_PRESSURE_BAR_L;
                currDigit = 0;
                inc = 0;
            }
            break;
        case TEST_PRESSURE_BAR_L:
            p_numericalValues.pressure.val = pbrLMap[currDigit];
            p_numericalValues.pressure.setpoint = pbrLMap[currDigit];
            inc = 1;
            if (currDigit == ARRAY_LEN(pbrLMap)-1) {
                testState = TEST_PRESSURE_BAR_R_UP;
                currDigit = 0;
                inc = 0;
            }
            break;
        case TEST_PRESSURE_BAR_R_UP:
            p_numericalValues.pressure_min.val = 0;
            p_numericalValues.pressure_mean.val = 0;
            p_numericalValues.peak_pressure.val = pbrLMap[currDigit];
            p_numericalValues.pressure_plat.val = 0;
            inc = 1;
            if (currDigit == ARRAY_LEN(pbrLMap)-1) {
                testState = TEST_PRESSURE_BAR_R_MID;
                currDigit = 0;
                inc = 0;
            }
            break;
        case TEST_PRESSURE_BAR_R_MID:
            p_numericalValues.pressure_min.val = 0;
            p_numericalValues.pressure_mean.val = pbrLMap[currDigit];
            p_numericalValues.peak_pressure.val = 100;
            p_numericalValues.pressure_plat.val = 0;
            inc = 1;
            if (currDigit == ARRAY_LEN(pbrLMap)-1) {
                testState = TEST_PRESSURE_BAR_R_LOW;
                currDigit = 0;
                inc = 0;
            }
            break;
        case TEST_PRESSURE_BAR_R_LOW:
            p_numericalValues.pressure_min.val = pbrLMap[currDigit];
            p_numericalValues.pressure_mean.val = 100;
            p_numericalValues.peak_pressure.val = 100;
            p_numericalValues.pressure_plat.val = 0;
            inc = 1;
            if (currDigit == ARRAY_LEN(pbrLMap)-1) {
                testState = TEST_PRESSURE_BAR_R_PLAT;
                currDigit = 0;
                inc = 0;
            }
            break;
        case TEST_PRESSURE_BAR_R_PLAT:
            p_numericalValues.pressure_min.val = 100;
            p_numericalValues.pressure_mean.val = 100;
            p_numericalValues.peak_pressure.val = 100;
            p_numericalValues.pressure_plat.val = pbrLMap[currDigit];
            inc = 1;
            if (currDigit == ARRAY_LEN(pbrLMap)-1) {
                testState = TEST_BEEP;
                currDigit = 0;
                inc = 0;
            }
            break;
        case TEST_BEEP: {
                inc = 1;
                if (currDigit == 0) {
                    sound_start(SOUND_BEEP);
                } else if (currDigit >= SOUND_BEEP_DURATION_CYCLES) {
                    testState = TEST_BEEP_BEEP;
                    inc = 0;
                    currDigit = 0;
                }
            }
            break;
        case TEST_BEEP_BEEP: {
                inc = 1;
                if (currDigit == 0) {
                    sound_start(SOUND_TWO_BEEP);
                } else if (currDigit >= (SOUND_BEEP_DURATION_CYCLES * 3)) {
                    testState = TEST_CONTINUOUS_10;
                    inc = 0;
                    currDigit = 0;
                }
            }
            break;
        case TEST_CONTINUOUS_10: {
                inc = 1;
                if (currDigit == 0) {
                    sound_start(SOUND_CONSTANT);
                } else if (currDigit > (SOUND_BEEP_DURATION_CYCLES*5 + 1)) {
                    testState = TEST_BUTTONS;
                    inc = 0;
                    currDigit = 1;
                } else if (currDigit >= SOUND_BEEP_DURATION_CYCLES*5) {
                    sound_stop();
                }
            }
            break;
        case TEST_BUTTONS: {

                // read buttons
                PanelButtons buttons;
                // we'll borrow the FIO display to display which button
                // was pushed
                uint32_t dispVal = 0;
                bool status = detect_button_state(&buttons);
                if (!status) {
                    break;
                }
                if (buttons.SET_FIO_ALRM_B) {
                    dispVal = 1;
                }
                if (buttons.SET_PEEP_B) {
                    dispVal = 2;
                }
                if (buttons.SET_TV_B) {
                    dispVal = 3;
                }
                if (buttons.SET_BUR_B) {
                    dispVal = 4;
                }
                if (buttons.SET_PEAK_B) {
                    dispVal = 5;
                }
                if (buttons.SET_ITIME_B) {
                    dispVal = 6;
                }
                if (buttons.POWER_DOWN_B) {
                    dispVal = 7;
                }
                if (buttons.ALRM_SILENCE_B) {
                    dispVal = 8;
                }
                if (buttons.GET_PLAT_B) {
                    dispVal = 9;
                }
                if (buttons.ADJ_UP_B) {
                    dispVal = 11;
                }
                if (buttons.ADJ_DWN_B) {
                    dispVal = 10;
                    testState = TEST_FI02;
                    // exit back to test state
                    currDigit = 0;
                }
                p_numericalValues.FIO2.val = dispVal;
            }
            break;
    }
    display_send_update(&p_numericalValues);
    if (inc) {
        currDigit++;
    }
    return testState == TEST_BUTTONS; // Done when we hit buttons

}
