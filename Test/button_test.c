/*
 * button_test.c
 *
 *  Created on: Apr 11, 2020
 *      Author: tcanham
 */

#include "test.h"
#include <stdint.h>
#include <ventilator/button.h>
#include <ventilator/panel_public.h>
#include <ventilator/panel.h>
#include <string.h>

extern ButtonState m_button_state[BUTTON_ID_NUM_BUTTONS];
extern ButtonId m_button_in_progress;
extern ButtonId m_adjust_button_in_progress;



int test_is_actionable() {
    int i = 0;
    m_adjust_button_in_progress = BUTTON_NONE;
    for (i = 0; i < BUTTON_ID_ADJ_DWN; i++) {
        p_powerState = POWER_OFF_STATE;
        m_button_in_progress = BUTTON_NONE;
        // POWER on tests checks for no other buttons
        if (i == BUTTON_ID_ALRM_SILENCE || i == BUTTON_ID_POWER_DOWN) {
            TEST_ASSERT(isButtonActionable(i), "Button should be actionable");
        } else {
            TEST_ASSERT(!isButtonActionable(i), "Button should be locked-out from powered-off");
        }
        // Power on testin
        p_powerState = POWER_ON_STATE;

        int j = 0;
        for (j = 0; j < BUTTON_ID_ADJ_DWN; j++) {
            m_button_in_progress = j;
            // Check other buttons lock this one out
            if (j != i) {
                TEST_ASSERT(!isButtonActionable(i), "Button should be locked-out");
            } else {
                TEST_ASSERT(isButtonActionable(i), "Button should be actionable when it is pressed");
            }
        }
        m_button_in_progress = BUTTON_NONE;
        TEST_ASSERT(isButtonActionable(i), "Button should be actionable when none is active");
    }
    return 0;
}

int test_is_adj_actionable() {
    int i = 0;
    m_adjust_button_in_progress = BUTTON_NONE;
    for (i = BUTTON_ID_ADJ_DWN; i < BUTTON_ID_NUM_BUTTONS; i++) {
        m_button_in_progress = BUTTON_NONE;
        TEST_ASSERT(!isAdjustActionable(i), "Adj button should not be actionable when none is active");

        m_button_in_progress = BUTTON_ID_SET_ITIME;
        m_button_state[BUTTON_ID_SET_ITIME].state = BUTTON_STATE_MODIFY;
        m_adjust_button_in_progress = (i == BUTTON_ID_ADJ_DWN)? BUTTON_ID_ADJ_UP : BUTTON_ID_ADJ_DWN;
        TEST_ASSERT(!isAdjustActionable(i), "Adj button should not be actionable when another is active");

        m_adjust_button_in_progress = i;
        TEST_ASSERT(!isAdjustActionable(i), "Adj button should not be actionable when it is active");

        m_adjust_button_in_progress = BUTTON_NONE;
        TEST_ASSERT(isAdjustActionable(i), "Adj button should be actionable none is active");

        int j = 0;
        for (j = 0; j < BUTTON_ID_ADJ_DWN; j++) {
            m_button_in_progress = j;
            int k = 0;
            for (k = 0; k < BUTTON_STATE_NUM_STATES; k++) {
                m_button_state[j].state = k;
                // Check other buttons lock this one out
                if (((k == BUTTON_STATE_MODIFY) || (k == BUTTON_STATE_PRESS_TO_MODIFY)) &&
                        (j < BUTTON_ID_POWER_DOWN) && (j != BUTTON_ID_SET_FIO_ALARM)) {
                    TEST_ASSERT(isAdjustActionable(i), "Adjust should be actionable");
                } else {
                    TEST_ASSERT(!isAdjustActionable(i), "Adjust should not be actionable");
                }
            }
        }
    }
    return 0;
}

int test_fio2_state() {
    PanelButtons buttons;
    memset(&buttons, 0, sizeof(buttons));
    p_numericalValues.FIO2.setpoint = 0;
    p_numericalValues.FIO2.val = 32;
    TEST_ASSERT(p_numericalValues.FIO2.setpoint != p_numericalValues.FIO2.val, "Setpoint set to val");

    // Check that a press is ignored when another button is active
    m_button_in_progress = BUTTON_ID_SET_ITIME;
    m_button_state[BUTTON_ID_SET_ITIME].state = BUTTON_STATE_MODIFY;
    buttons.SET_FIO_ALRM_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_FIO_ALARM].state == BUTTON_STATE_IDLE, "Bad state transition");
    m_button_in_progress = BUTTON_NONE;
    m_button_state[BUTTON_ID_SET_ITIME].state = BUTTON_STATE_IDLE;

    // No press makes no transition
    buttons.SET_FIO_ALRM_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_FIO_ALARM].state == BUTTON_STATE_IDLE, "Bad state transition");

    // Walk the state machine through
    buttons.SET_FIO_ALRM_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_FIO_ALARM].state == BUTTON_STATE_WAITING_FOR_RELEASE_MODIFY, "Bad state transition");

    buttons.SET_FIO_ALRM_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_FIO_ALARM].state == BUTTON_STATE_MODIFY, "Bad state transition");

    buttons.SET_FIO_ALRM_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_FIO_ALARM].state == BUTTON_STATE_WAITING_FOR_RELEASE_IDLE, "Bad state transition");

    buttons.SET_FIO_ALRM_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_FIO_ALARM].state == BUTTON_STATE_IDLE, "Bad state transition");

    // Test edit-mode timeout
    buttons.SET_FIO_ALRM_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_FIO_ALARM].state == BUTTON_STATE_WAITING_FOR_RELEASE_MODIFY, "Bad state transition");

    buttons.SET_FIO_ALRM_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);

    int i = 0;
    for (i = 0; i < EDIT_TIMEOUT; i++) {
        TEST_ASSERT(m_button_state[BUTTON_ID_SET_FIO_ALARM].state == BUTTON_STATE_MODIFY, "Bad state transition");
        run_button_state_machines(&buttons);// Do nothing
    }
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_FIO_ALARM].state == BUTTON_STATE_IDLE, "Bad timeout transition");
    return 0;
}

int test_itime_state() {
    PanelButtons buttons;
    memset(&buttons, 0, sizeof(buttons));

    // Check that a press is ignored when another button is active
    m_button_in_progress = BUTTON_ID_SET_FIO_ALARM;
    m_button_state[BUTTON_ID_SET_FIO_ALARM].state = BUTTON_STATE_MODIFY;
    buttons.SET_ITIME_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_ITIME].state == BUTTON_STATE_IDLE, "Bad state transition");
    m_button_in_progress = BUTTON_NONE;
    m_button_state[BUTTON_ID_SET_FIO_ALARM].state = BUTTON_STATE_IDLE;

    // No press makes no transition
    buttons.SET_ITIME_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_ITIME].state == BUTTON_STATE_IDLE, "Bad state transition");

    // Walk the state machine through
    buttons.SET_ITIME_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_ITIME].state == BUTTON_STATE_WAITING_FOR_RELEASE_MODIFY, "Bad state transition");

    buttons.SET_ITIME_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_ITIME].state == BUTTON_STATE_MODIFY, "Bad state transition");

    buttons.SET_ITIME_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_ITIME].state == BUTTON_STATE_WAITING_FOR_RELEASE_IDLE, "Bad state transition");

    buttons.SET_ITIME_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_ITIME].state == BUTTON_STATE_IDLE, "Bad state transition");

    // Test edit-mode timeout
    buttons.SET_ITIME_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_ITIME].state == BUTTON_STATE_WAITING_FOR_RELEASE_MODIFY, "Bad state transition");

    buttons.SET_ITIME_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);

    int i = 0;
    for (i = 0; i < EDIT_TIMEOUT; i++) {
        TEST_ASSERT(m_button_state[BUTTON_ID_SET_ITIME].state == BUTTON_STATE_MODIFY, "Bad state transition");
        run_button_state_machines(&buttons);// Do nothing
    }
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_ITIME].state == BUTTON_STATE_IDLE, "Bad timeout transition");

    return 0;
}


int test_peak_state() {
    PanelButtons buttons;
    memset(&buttons, 0, sizeof(buttons));

    // Check that a press is ignored when another button is active
    m_button_in_progress = BUTTON_ID_SET_ITIME;
    m_button_state[BUTTON_ID_SET_ITIME].state = BUTTON_STATE_MODIFY;
    buttons.SET_PEAK_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_PEAK].state == BUTTON_STATE_IDLE, "Bad state transition");
    m_button_in_progress = BUTTON_NONE;
    m_button_state[BUTTON_ID_SET_ITIME].state = BUTTON_STATE_IDLE;

    // No press makes no transition
    buttons.SET_PEAK_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_PEAK].state == BUTTON_STATE_IDLE, "Bad state transition");

    // Walk the state machine through
    buttons.SET_PEAK_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_PEAK].state == BUTTON_STATE_WAITING_FOR_RELEASE_MODIFY, "Bad state transition");

    buttons.SET_PEAK_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_PEAK].state == BUTTON_STATE_MODIFY, "Bad state transition");

    buttons.SET_PEAK_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_PEAK].state == BUTTON_STATE_WAITING_FOR_RELEASE_IDLE, "Bad state transition");

    buttons.SET_PEAK_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_PEAK].state == BUTTON_STATE_IDLE, "Bad state transition");

    // Test edit-mode timeout
    buttons.SET_PEAK_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_PEAK].state == BUTTON_STATE_WAITING_FOR_RELEASE_MODIFY, "Bad state transition");

    buttons.SET_PEAK_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);

    int i = 0;
    for (i = 0; i < EDIT_TIMEOUT; i++) {
        TEST_ASSERT(m_button_state[BUTTON_ID_SET_PEAK].state == BUTTON_STATE_MODIFY, "Bad state transition");
        run_button_state_machines(&buttons);// Do nothing
    }
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_PEAK].state == BUTTON_STATE_IDLE, "Bad timeout transition");

    return 0;
}


int test_peep_state() {
    PanelButtons buttons;
    memset(&buttons, 0, sizeof(buttons));

    // Check that a press is ignored when another button is active
    m_button_in_progress = BUTTON_ID_SET_ITIME;
    m_button_state[BUTTON_ID_SET_ITIME].state = BUTTON_STATE_MODIFY;
    buttons.SET_PEEP_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_PEEP].state == BUTTON_STATE_IDLE, "Bad state transition");
    m_button_in_progress = BUTTON_NONE;
    m_button_state[BUTTON_ID_SET_ITIME].state = BUTTON_STATE_IDLE;

    // No press makes no transition
    buttons.SET_PEEP_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_PEEP].state == BUTTON_STATE_IDLE, "Bad state transition");

    // Walk the state machine through
    buttons.SET_PEEP_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_PEEP].state == BUTTON_STATE_WAITING_FOR_RELEASE_MODIFY, "Bad state transition");

    buttons.SET_PEEP_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_PEEP].state == BUTTON_STATE_MODIFY, "Bad state transition");

    buttons.SET_PEEP_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_PEEP].state == BUTTON_STATE_WAITING_FOR_RELEASE_IDLE, "Bad state transition");

    buttons.SET_PEEP_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_PEEP].state == BUTTON_STATE_IDLE, "Bad state transition");

    // Test edit-mode timeout
    buttons.SET_PEEP_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_PEEP].state == BUTTON_STATE_WAITING_FOR_RELEASE_MODIFY, "Bad state transition");

    buttons.SET_PEEP_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);

    int i = 0;
    for (i = 0; i < EDIT_TIMEOUT; i++) {
        TEST_ASSERT(m_button_state[BUTTON_ID_SET_PEEP].state == BUTTON_STATE_MODIFY, "Bad state transition");
        run_button_state_machines(&buttons);// Do nothing
    }
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_PEEP].state == BUTTON_STATE_IDLE, "Bad timeout transition");

    return 0;
}

int test_tv_state() {
    PanelButtons buttons;
    memset(&buttons, 0, sizeof(buttons));

    // Check that a press is ignored when another button is active
    m_button_in_progress = BUTTON_ID_SET_ITIME;
    m_button_state[BUTTON_ID_SET_ITIME].state = BUTTON_STATE_MODIFY;
    buttons.SET_TV_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_TV].state == BUTTON_STATE_IDLE, "Bad state transition");
    m_button_in_progress = BUTTON_NONE;
    m_button_state[BUTTON_ID_SET_ITIME].state = BUTTON_STATE_IDLE;

    // No press makes no transition
    buttons.SET_TV_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_TV].state == BUTTON_STATE_IDLE, "Bad state transition");

    // Walk the state machine through
    buttons.SET_TV_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_TV].state == BUTTON_STATE_WAITING_FOR_RELEASE_MODIFY, "Bad state transition");

    buttons.SET_TV_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_TV].state == BUTTON_STATE_MODIFY, "Bad state transition");

    buttons.SET_TV_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_TV].state == BUTTON_STATE_WAITING_FOR_RELEASE_IDLE, "Bad state transition");

    buttons.SET_TV_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_TV].state == BUTTON_STATE_IDLE, "Bad state transition");

    // Test edit-mode timeout
    buttons.SET_TV_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_TV].state == BUTTON_STATE_WAITING_FOR_RELEASE_MODIFY, "Bad state transition");

    buttons.SET_TV_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);

    int i = 0;
    for (i = 0; i < EDIT_TIMEOUT; i++) {
        TEST_ASSERT(m_button_state[BUTTON_ID_SET_TV].state == BUTTON_STATE_MODIFY, "Bad state transition");
        run_button_state_machines(&buttons);// Do nothing
    }
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_TV].state == BUTTON_STATE_IDLE, "Bad timeout transition");

    return 0;
}

int test_plat_state() {
    PanelButtons buttons;
    memset(&buttons, 0, sizeof(buttons));

    // Check that a press is ignored when another button is active
    m_button_in_progress = BUTTON_ID_SET_ITIME;
    m_button_state[BUTTON_ID_SET_ITIME].state = BUTTON_STATE_MODIFY;
    buttons.GET_PLAT_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_GET_PLAT].state == BUTTON_STATE_IDLE, "Bad state transition");
    m_button_in_progress = BUTTON_NONE;
    m_button_state[BUTTON_ID_SET_ITIME].state = BUTTON_STATE_IDLE;

    // No press makes no transition
    buttons.GET_PLAT_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_GET_PLAT].state == BUTTON_STATE_IDLE, "Bad state transition");

    // Walk the state machine through
    buttons.GET_PLAT_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_GET_PLAT].state == BUTTON_STATE_WAITING_FOR_RELEASE_IDLE, "Bad state transition");

    buttons.GET_PLAT_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_GET_PLAT].state == BUTTON_STATE_IDLE, "Bad state transition");
    TEST_ASSERT(p_doPlateau == DO_PLATEAU_SENDS, "Failed to process momentary");

    return 0;
}

int test_bur_state() {
    PanelButtons buttons;
    memset(&buttons, 0, sizeof(buttons));

    // Check that a press is ignored when another button is active
    m_button_in_progress = BUTTON_ID_SET_ITIME;
    m_button_state[BUTTON_ID_SET_ITIME].state = BUTTON_STATE_MODIFY;
    buttons.SET_BUR_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_BUR].state == BUTTON_STATE_IDLE, "Bad state transition");
    m_button_in_progress = BUTTON_NONE;
    m_button_state[BUTTON_ID_SET_ITIME].state = BUTTON_STATE_IDLE;

    // No press makes no transition
    buttons.SET_BUR_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_BUR].state == BUTTON_STATE_IDLE, "Bad state transition");

    // Walk the state machine through
    buttons.SET_BUR_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_BUR].state == BUTTON_STATE_PRESS_TO_HOLD, "Bad state transition");

    buttons.SET_BUR_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_BUR].state == BUTTON_STATE_MODIFY, "Bad state transition");

    buttons.SET_BUR_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_BUR].state == BUTTON_STATE_WAITING_FOR_RELEASE_IDLE, "Bad state transition");

    buttons.SET_BUR_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_BUR].state == BUTTON_STATE_IDLE, "Bad state transition");

    // Test edit-mode timeout
    buttons.SET_BUR_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_BUR].state == BUTTON_STATE_PRESS_TO_HOLD, "Bad state transition");

    buttons.SET_BUR_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);
    int i = 0;
    for (i = 0; i < EDIT_TIMEOUT; i++) {
        TEST_ASSERT(m_button_state[BUTTON_ID_SET_BUR].state == BUTTON_STATE_MODIFY, "Bad state transition");
        run_button_state_machines(&buttons);// Do nothing
    }
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_BUR].state == BUTTON_STATE_IDLE, "Bad timeout transition");

    return 0;
}


int test_bur_hold_state() {
    PanelButtons buttons;
    memset(&buttons, 0, sizeof(buttons));

    // Check that a press is ignored when another button is active
    m_button_in_progress = BUTTON_ID_SET_ITIME;
    m_button_state[BUTTON_ID_SET_ITIME].state = BUTTON_STATE_MODIFY;
    buttons.SET_BUR_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_BUR].state == BUTTON_STATE_IDLE, "Bad state transition");
    m_button_in_progress = BUTTON_NONE;
    m_button_state[BUTTON_ID_SET_ITIME].state = BUTTON_STATE_IDLE;

    // No press makes no transition
    buttons.SET_BUR_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_BUR].state == BUTTON_STATE_IDLE, "Bad state transition");

    // Walk the state machine through
    int i = 0;
    for (i = 0; i < m_button_state[BUTTON_ID_SET_BUR].wait; i++) {
        buttons.SET_BUR_B = BUTTON_POS_ON;
        buttons.ALL_BUTTONS = BUTTON_POS_ON;
        run_button_state_machines(&buttons);
        TEST_ASSERT(m_button_state[BUTTON_ID_SET_BUR].state == BUTTON_STATE_PRESS_TO_HOLD, "Bad state transition");
    }
    buttons.SET_BUR_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_BUR].state == BUTTON_STATE_WAITING_FOR_RELEASE_PRESS_TO_MODIFY, "Bad state transition");


    buttons.SET_BUR_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_BUR].state == BUTTON_STATE_PRESS_TO_MODIFY, "Bad state transition");

    buttons.SET_BUR_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_BUR].state == BUTTON_STATE_WAITING_FOR_RELEASE_IDLE, "Bad state transition");

    buttons.SET_BUR_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_BUR].state == BUTTON_STATE_IDLE, "Bad state transition");

    // Walk the state machine to test timeout
    for (i = 0; i < m_button_state[BUTTON_ID_SET_BUR].wait; i++) {
        buttons.SET_BUR_B = BUTTON_POS_ON;
        buttons.ALL_BUTTONS = BUTTON_POS_ON;
        run_button_state_machines(&buttons);
        TEST_ASSERT(m_button_state[BUTTON_ID_SET_BUR].state == BUTTON_STATE_PRESS_TO_HOLD, "Bad state transition");
    }
    buttons.SET_BUR_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_BUR].state == BUTTON_STATE_WAITING_FOR_RELEASE_PRESS_TO_MODIFY, "Bad state transition");


    buttons.SET_BUR_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);

    for (i = 0; i < EDIT_TIMEOUT; i++) {
        TEST_ASSERT(m_button_state[BUTTON_ID_SET_BUR].state == BUTTON_STATE_PRESS_TO_MODIFY, "Bad state transition");
        run_button_state_machines(&buttons);// Do nothing
    }
    TEST_ASSERT(m_button_state[BUTTON_ID_SET_BUR].state == BUTTON_STATE_IDLE, "Bad timeout transition");

    return 0;
}


int test_power_press_state() {
    PanelButtons buttons;
    memset(&buttons, 0, sizeof(buttons));
    p_powerState = POWER_OFF_STATE;
    // Check that a press is ignored when another button is active
    m_button_in_progress = BUTTON_ID_SET_ITIME;
    m_button_state[BUTTON_ID_SET_ITIME].state = BUTTON_STATE_MODIFY;
    buttons.POWER_DOWN_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_POWER_DOWN].state == BUTTON_STATE_IDLE, "Bad state transition");
    m_button_in_progress = BUTTON_NONE;
    m_button_state[BUTTON_ID_SET_ITIME].state = BUTTON_STATE_IDLE;

    // No press makes no transition
    buttons.POWER_DOWN_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_POWER_DOWN].state == BUTTON_STATE_IDLE, "Bad state transition");

    // Walk the state machine through
    buttons.POWER_DOWN_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_POWER_DOWN].state == BUTTON_STATE_PRESS_TO_HOLD, "Bad state transition");

    buttons.POWER_DOWN_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_POWER_DOWN].state == BUTTON_STATE_IDLE, "Bad state transition");
    TEST_ASSERT(p_powerState == POWERING_STATE, "Didn't power on");

    return 0;
}

int test_power_hold_state() {
    PanelButtons buttons;
    memset(&buttons, 0, sizeof(buttons));

    // Check that a press is ignored when another button is active
    m_button_in_progress = BUTTON_ID_SET_ITIME;
    m_button_state[BUTTON_ID_SET_ITIME].state = BUTTON_STATE_MODIFY;
    buttons.POWER_DOWN_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_POWER_DOWN].state == BUTTON_STATE_IDLE, "Bad state transition");
    m_button_in_progress = BUTTON_NONE;
    m_button_state[BUTTON_ID_SET_ITIME].state = BUTTON_STATE_IDLE;

    // No press makes no transition
    buttons.POWER_DOWN_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_POWER_DOWN].state == BUTTON_STATE_IDLE, "Bad state transition");

    // Walk the state machine through
    int i = 0;
    for (i = 0; i < m_button_state[BUTTON_ID_POWER_DOWN].wait; i++) {
        buttons.POWER_DOWN_B = BUTTON_POS_ON;
        buttons.ALL_BUTTONS = BUTTON_POS_ON;
        run_button_state_machines(&buttons);
        TEST_ASSERT(m_button_state[BUTTON_ID_POWER_DOWN].state == BUTTON_STATE_PRESS_TO_HOLD, "Bad state transition");
    }
    buttons.POWER_DOWN_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_POWER_DOWN].state == BUTTON_STATE_WAITING_FOR_RELEASE_IDLE, "Bad state transition");
    TEST_ASSERT(p_powerState == POWER_OFF_STATE, "Powered off state");

    buttons.POWER_DOWN_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_POWER_DOWN].state == BUTTON_STATE_IDLE, "Bad state transition");

    return 0;
}


int test_alarm_press_state() {
    PanelButtons buttons;
    memset(&buttons, 0, sizeof(buttons));
    p_numericalValues.alarms.tidal_vol.status = ALARM_LATCH;
    sound_start(SOUND_CONSTANT);
    TEST_ASSERT(sound_running, "Sound not on");

    // Check that a press is ignored when another button is active
    m_button_in_progress = BUTTON_ID_SET_ITIME;
    m_button_state[BUTTON_ID_SET_ITIME].state = BUTTON_STATE_MODIFY;
    buttons.ALRM_SILENCE_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_ALRM_SILENCE].state == BUTTON_STATE_IDLE, "Bad state transition");
    m_button_in_progress = BUTTON_NONE;
    m_button_state[BUTTON_ID_SET_ITIME].state = BUTTON_STATE_IDLE;

    // No press makes no transition
    buttons.ALRM_SILENCE_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_ALRM_SILENCE].state == BUTTON_STATE_IDLE, "Bad state transition");

    // Walk the state machine through
    buttons.ALRM_SILENCE_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_ALRM_SILENCE].state == BUTTON_STATE_PRESS_TO_HOLD, "Bad state transition");


    buttons.ALRM_SILENCE_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_ALRM_SILENCE].state == BUTTON_STATE_IDLE, "Bad state transition");
    TEST_ASSERT(p_numericalValues.alarms.tidal_vol.status == ALARM_LATCH, "Not latched still");
    TEST_ASSERT(!sound_running, "Sound not off");

    return 0;
}

int test_alarm_hold_state() {
    PanelButtons buttons;
    memset(&buttons, 0, sizeof(buttons));
    p_numericalValues.alarms.tidal_vol.status = ALARM_LATCH;
    sound_start(SOUND_CONSTANT);
    TEST_ASSERT(sound_running, "Sound not on");

    // Check that a press is ignored when another button is active
    m_button_in_progress = BUTTON_ID_SET_ITIME;
    m_button_state[BUTTON_ID_SET_ITIME].state = BUTTON_STATE_MODIFY;
    buttons.ALRM_SILENCE_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_ALRM_SILENCE].state == BUTTON_STATE_IDLE, "Bad state transition");
    m_button_in_progress = BUTTON_NONE;
    m_button_state[BUTTON_ID_SET_ITIME].state = BUTTON_STATE_IDLE;

    // No press makes no transition
    buttons.ALRM_SILENCE_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_ALRM_SILENCE].state == BUTTON_STATE_IDLE, "Bad state transition");

    // Walk the state machine through
    int i = 0;
    for (i = 0; i < m_button_state[BUTTON_ID_ALRM_SILENCE].wait; i++) {
        buttons.ALRM_SILENCE_B = BUTTON_POS_ON;
        buttons.ALL_BUTTONS = BUTTON_POS_ON;
        run_button_state_machines(&buttons);
        TEST_ASSERT(m_button_state[BUTTON_ID_ALRM_SILENCE].state == BUTTON_STATE_PRESS_TO_HOLD, "Bad state transition");
    }
    buttons.ALRM_SILENCE_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_ALRM_SILENCE].state == BUTTON_STATE_WAITING_FOR_RELEASE_IDLE, "Bad state transition");
    TEST_ASSERT(!sound_running, "Sound not off");

    buttons.ALRM_SILENCE_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_ALRM_SILENCE].state == BUTTON_STATE_IDLE, "Bad state transition");
    TEST_ASSERT(p_numericalValues.alarms.tidal_vol.status == ALARM_OFF, "Alarm not off");

    return 0;
}

int test_adj_up() {
    PanelButtons buttons;
    memset(&buttons, 0, sizeof(buttons));

    m_button_in_progress = BUTTON_ID_SET_ITIME;
    m_button_state[BUTTON_ID_SET_ITIME].state = BUTTON_STATE_MODIFY;

    // Check that a press is ignored when another button is active
    m_adjust_button_in_progress = BUTTON_ID_ADJ_DWN;
    m_button_state[BUTTON_ID_ADJ_DWN].state = BUTTON_STATE_WAITING_FOR_RELEASE_MODIFY;

    buttons.ADJ_UP_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_ADJ_UP].state == BUTTON_STATE_IDLE, "Bad state transition");
    m_adjust_button_in_progress = BUTTON_NONE;
    m_button_state[BUTTON_ID_ADJ_DWN].state = BUTTON_STATE_IDLE;

    // No press makes no transition
    buttons.ADJ_UP_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_ADJ_UP].state == BUTTON_STATE_IDLE, "Bad state transition");


    buttons.ADJ_UP_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_ADJ_UP].state == BUTTON_STATE_WAITING_FOR_RELEASE_IDLE, "Bad state transition");

    buttons.ADJ_UP_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_ADJ_UP].state == BUTTON_STATE_MOMENTARY_DONE, "Bad state transition");

    return 0;
}


int test_adj_dn() {
    PanelButtons buttons;
    memset(&buttons, 0, sizeof(buttons));

    m_button_in_progress = BUTTON_ID_SET_ITIME;
    m_button_state[BUTTON_ID_SET_ITIME].state = BUTTON_STATE_MODIFY;

    // Check that a press is ignored when another button is active
    m_adjust_button_in_progress = BUTTON_ID_ADJ_UP;
    m_button_state[BUTTON_ID_ADJ_UP].state = BUTTON_STATE_WAITING_FOR_RELEASE_MODIFY;

    buttons.ADJ_DWN_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_ADJ_DWN].state == BUTTON_STATE_IDLE, "Bad state transition");
    m_adjust_button_in_progress = BUTTON_NONE;
    m_button_state[BUTTON_ID_ADJ_UP].state = BUTTON_STATE_IDLE;

    // No press makes no transition
    buttons.ADJ_DWN_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_ADJ_DWN].state == BUTTON_STATE_IDLE, "Bad state transition");


    buttons.ADJ_DWN_B = BUTTON_POS_ON;
    buttons.ALL_BUTTONS = BUTTON_POS_ON;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_ADJ_DWN].state == BUTTON_STATE_WAITING_FOR_RELEASE_IDLE, "Bad state transition");


    buttons.ADJ_DWN_B = BUTTON_POS_OFF;
    buttons.ALL_BUTTONS = BUTTON_POS_OFF;
    run_button_state_machines(&buttons);
    TEST_ASSERT(m_button_state[BUTTON_ID_ADJ_DWN].state == BUTTON_STATE_MOMENTARY_DONE, "Bad state transition");

    return 0;
}

void clear_value(NumericalValue* value) {
    memset(value, 0, sizeof(NumericalValue));
    value->upper = 90;
    value->lower = 10;
    value->editval = 50;
    value->setpoint = 0xFEED;
    value->val = 0xCAFE;
    value->step = 5; // A prime number, indivisible
    value->mode = DISPLAY_VALUE;
}

int stepped_value(int initial, NumericalValue* value, ButtonId adjust_active) {
    int temp = 0;
    if (adjust_active == BUTTON_ID_ADJ_DWN) {
        temp = initial - value->step;
        return (temp < value->lower)? value->lower: temp;
    } else if (adjust_active == BUTTON_ID_ADJ_UP) {
        temp = initial + value->step;
        return (temp > value->upper)? value->upper: temp;
    }
    return initial;
}

void reset_to_momentary(ButtonPressState state, ButtonId button) {
    // Adjusts only live in one of the modify states. This is tested above.
    if (state != BUTTON_STATE_MODIFY && state != BUTTON_STATE_PRESS_TO_MODIFY) {
        m_button_state[BUTTON_ID_ADJ_UP].state = BUTTON_STATE_IDLE;
        m_button_state[BUTTON_ID_ADJ_DWN].state = BUTTON_STATE_IDLE;
    }
    // Set exactly one adjust button
    else if (button == BUTTON_ID_ADJ_DWN) {
        m_button_state[BUTTON_ID_ADJ_DWN].state = BUTTON_STATE_MOMENTARY_DONE;
        m_button_state[BUTTON_ID_ADJ_UP].state = BUTTON_STATE_IDLE;
    }
    // Set exactly one adjust button
    else if (button == BUTTON_ID_ADJ_UP) {
        m_button_state[BUTTON_ID_ADJ_UP].state = BUTTON_STATE_MOMENTARY_DONE;
        m_button_state[BUTTON_ID_ADJ_DWN].state = BUTTON_STATE_IDLE;
    }
    // Set an idle adjust
    else {
        m_button_state[BUTTON_ID_ADJ_UP].state = BUTTON_STATE_IDLE;
        m_button_state[BUTTON_ID_ADJ_DWN].state = BUTTON_STATE_IDLE;
    }
}

int check_value(ButtonPressState state, DisplayMode dmode, NumericalValue* value, int initial) {
    DisplayMode expected_display = (dmode == DISPLAY_VALUE) ? DISPLAY_EDIT_ALARM : DISPLAY_EDIT_SETPOINT;
    int expected_editval = stepped_value(initial, value, m_adjust_button_in_progress);

    // In modify state, the the display should be flashing (or in one of the edit states) and
    // the editvale should change in response to
    if (state == BUTTON_STATE_MODIFY) {
        TEST_ASSERT(value->mode == expected_display, "Numerical update not flashing when in modify");
        TEST_ASSERT(value->editval == expected_editval, "Edit value changed incorrectly");
    }
    // In waiting for release modify, we copy into editval from the setpoint is being modified.
    // Thus editval should change
    else if (state == BUTTON_STATE_WAITING_FOR_RELEASE_MODIFY) {
        TEST_ASSERT(value->mode == dmode, "Numerical update did not return to expected state");
        TEST_ASSERT(value->editval == 0xfeed, "Edit value not updated from setpoint");
        TEST_ASSERT(value->setpoint == 0xfeed, "Setpoint changed improperly");
    }
    // In wating for release idle state, the editvalue should be copied back over into
    // the setpoint.
    else if (state == BUTTON_STATE_WAITING_FOR_RELEASE_IDLE) {
        TEST_ASSERT(value->mode == dmode, "Numerical update did not return to expected state");
        TEST_ASSERT(value->editval == 50, "Edit value changed outside of MODIFY state");
        TEST_ASSERT(value->setpoint == 50, "Edit value not updating setpoint when finished");
    }
    // In all other states, no modifications should happen.
    else {
        TEST_ASSERT(value->mode == dmode, "Numerical update did not return to expected state");
        TEST_ASSERT(value->editval == 50, "Edit value changed outside of MODIFY state");
        TEST_ASSERT(value->setpoint == 0xfeed, "Setpoint changed improperly");
    }
    TEST_ASSERT(value->val == 0xcafe, "Value changed -- this CANNOT happen");
    // Check the momentary done state was consumed.
    TEST_ASSERT(m_button_state[BUTTON_ID_ADJ_UP].state == BUTTON_STATE_IDLE, "Momentary up not consumed");
    TEST_ASSERT(m_button_state[BUTTON_ID_ADJ_DWN].state == BUTTON_STATE_IDLE, "Momentary down not consumed");

    return 0;
}

int test_updates() {

    NumericalValue value;
    NumericalValue value1;
    NumericalValue value2;
    NumericalValue valuef;

    // Checks the response for every available button state.  In most cases, nothing should happen except in
    // BUTTON_STATE_MODIFY or BUTTON_STATE_PRESS_TO_MODIFY and this asserts that.
    int i = 0;
    for (i = 0; i < BUTTON_STATE_NUM_STATES; i++) {
        // Checks the response for each adjust button (and no adjust button) to ensure that each was performed as
        // expected. This will ensure that the buttons work.
        int l = 0;
        for (l = BUTTON_ID_ADJ_DWN; l <= BUTTON_ID_NUM_BUTTONS; l++) {
            m_adjust_button_in_progress = (l == BUTTON_ID_NUM_BUTTONS) ? BUTTON_NONE : l;
            clear_value(&value);
            clear_value(&value1);
            clear_value(&value2);
            clear_value(&valuef);
            // Checks that enought button presses are performed to hit the upper, or lower limit of the setpoint.
            // This will ensure that we stay within-bounds for the signal and don't pass hard limits with many
            // presses.
            int j = 0;
            for (j = 0; j < 1000; j++) {
                // Loop over setpoint modes
                int k = 0;
                for (k = 0; k <= DISPLAY_SETPOINT; k++) {
                    int inital = value.editval;
                    // This state machine doesn't handle BUTTON_STATE_PRESS_TO_MODIFY as an actionable state
                    reset_to_momentary((i != BUTTON_STATE_PRESS_TO_MODIFY)? i : BUTTON_STATE_IDLE, l);
                    value.mode = k;
                    update_button_numerical_state(i, &value);
                    TEST_ASSERT(check_value(i, k, &value, inital) == 0, "Value check failed");
                }
                reset_to_momentary(i, l);
                value1.mode = DISPLAY_SETPOINT;
                value2.mode = DISPLAY_VALUE;
                int initial1 = value1.editval;
                int initial2 = value2.editval;
                update_backup_rate_numerical_states(i, &value1,  &value2);
                // In modify state only the backup rate is changed, the resp rate acts as if it were idle
                if (i == BUTTON_STATE_MODIFY) {
                    TEST_ASSERT(check_value(BUTTON_STATE_MODIFY, DISPLAY_SETPOINT, &value1, initial1) == 0, "BUR not modified as expected");
                    TEST_ASSERT(check_value(BUTTON_STATE_IDLE, DISPLAY_VALUE, &value2, initial2) == 0, "Resp rate modified unexpectedly");
                }
                // In press to modify state, the rolls are reversed. Back-up rate should be idle.
                else if (i == BUTTON_STATE_PRESS_TO_MODIFY) {
                    TEST_ASSERT(check_value(BUTTON_STATE_IDLE, DISPLAY_SETPOINT, &value1, initial1) == 0, "BUR modified as unexpectedly");
                    TEST_ASSERT(check_value(BUTTON_STATE_MODIFY, DISPLAY_VALUE, &value2, initial2) == 0, "Resp rate not modified as expected");
                }
                // In BUTTON_STATE_WAITING_FOR_RELEASE_MODIFY
                else if (i == BUTTON_STATE_WAITING_FOR_RELEASE_MODIFY) {
                    TEST_ASSERT(check_value(BUTTON_STATE_WAITING_FOR_RELEASE_MODIFY, DISPLAY_SETPOINT, &value1, initial1) == 0, "BUR not waiting for release");
                    TEST_ASSERT(check_value(BUTTON_STATE_IDLE, DISPLAY_VALUE, &value2, initial2) == 0, "Resp rate waiting for release inappropriately");
                }
                // In BUTTON_STATE_WAITING_FOR_RELEASE_PRESS_TO_MODIFY
                else if (i == BUTTON_STATE_WAITING_FOR_RELEASE_PRESS_TO_MODIFY) {
                    TEST_ASSERT(check_value(BUTTON_STATE_IDLE, DISPLAY_SETPOINT, &value1, initial1) == 0, "BUR waiting for release inappropriately");
                    TEST_ASSERT(check_value(BUTTON_STATE_WAITING_FOR_RELEASE_MODIFY, DISPLAY_VALUE, &value2, initial2) == 0, "Resp rate not waiting for release");
                }
                // Otherwise act as normal for the state for both values
                else {
                    TEST_ASSERT(check_value(i, DISPLAY_SETPOINT, &value1, initial1) == 0, "BUR not following state protocol");
                    TEST_ASSERT(check_value(i, DISPLAY_VALUE, &value2, initial2) == 0, "Resp rate following state protocol");
                }
                reset_to_momentary(i, l);

                // Check out FIO2
                update_fio2_numerical_states(i, &valuef);
                if (i == BUTTON_STATE_MODIFY) {
                    TEST_ASSERT(valuef.mode == DISPLAY_EDIT_WITH_VALUE, "FIO2 not flashing when in modify");
                    TEST_ASSERT(valuef.editval == 50, "FIO2 wdit value not updated from setpoint");
                    TEST_ASSERT(valuef.setpoint == 0xfeed, "FIO2 setpoint changed improperly");
                } else if (i == BUTTON_STATE_WAITING_FOR_RELEASE_IDLE) {
                    TEST_ASSERT(valuef.mode == DISPLAY_VALUE, "FIO2 not displaying value normally");
                    TEST_ASSERT(valuef.editval == 50, "FIO2 edit value not updated from setpoint");
                    TEST_ASSERT(valuef.setpoint == 0xcafe, "FIO2 setpoint not changed properly");
                } else {
                    TEST_ASSERT(valuef.mode == DISPLAY_VALUE, "FIO2 not displaying value normally");
                    TEST_ASSERT(valuef.editval == 50, "FIO2 edit value not updated from setpoint");
                    TEST_ASSERT(valuef.setpoint == 0xfeed, "FIO2 setpoint changed improperly");
                }
                TEST_ASSERT(valuef.val == 0xcafe, "Value changed -- this CANNOT happen");
            }
        }
    }
    return 0;
}

int main(int argc, char** argv) {
    sound_init(&htim1);
    init_button_state();
    TEST(test_is_actionable);
    init_button_state();
    TEST(test_is_adj_actionable);
    init_button_state();

    // Individual state machines
    TEST(test_fio2_state);
    init_button_state();
    TEST(test_itime_state);
    init_button_state();
    TEST(test_peak_state);
    init_button_state();
    TEST(test_peep_state);
    init_button_state();
    TEST(test_tv_state);
    init_button_state();
    TEST(test_plat_state);
    init_button_state();
    // Special state machines
    TEST(test_bur_state);
    init_button_state();
    TEST(test_bur_hold_state);

    // Power button tests
    p_powerState = POWER_OFF_STATE;
    init_button_state();
    TEST(test_power_press_state);

    p_powerState = POWER_ON_STATE;
    init_button_state();
    TEST(test_power_press_state);
    init_button_state();


    init_button_state();
    TEST(test_alarm_press_state);

    init_button_state();
    TEST(test_alarm_hold_state);
    // Adjust buttons
    init_button_state();
    TEST(test_adj_up);
    init_button_state();
    TEST(test_adj_dn);
    init_button_state();
    // Update state values
    TEST(test_updates);
}

