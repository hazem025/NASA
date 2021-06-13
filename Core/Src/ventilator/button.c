/*
 * button.c
 *
 *  Created on: Apr 9, 2020
 *      Author: tcanham
 */

#include <ventilator/button.h>
#include <ventilator/panel_public.h>
#include <ventilator/constants.h>
#include <ventilator/sound.h>
#include <ventilator/alarm.h>
#include <string.h>
#include <swassert.h>

STATIC ButtonState m_button_state[BUTTON_ID_NUM_BUTTONS];
STATIC ButtonId m_button_in_progress;
STATIC ButtonId m_adjust_button_in_progress;
STATIC uint32_t m_edit_timeout;

STATIC McpHandle m_button_mcp_handle;


void init_button_state(void) {
    SW_ASSERT(mcp23017_init(&m_button_mcp_handle, 0x24, 1) == HAL_OK);
    // initialize button state
    m_button_state[BUTTON_ID_SET_FIO_ALARM].type = BUTTON_TYPE_TOGGLE;
    m_button_state[BUTTON_ID_SET_FIO_ALARM].state = BUTTON_STATE_IDLE;
    m_button_state[BUTTON_ID_SET_FIO_ALARM].wait = 0;
    m_button_state[BUTTON_ID_SET_FIO_ALARM].count = 0;

    m_button_state[BUTTON_ID_SET_PEEP].type = BUTTON_TYPE_TOGGLE;
    m_button_state[BUTTON_ID_SET_PEEP].state = BUTTON_STATE_IDLE;
    m_button_state[BUTTON_ID_SET_PEEP].wait = 0;
    m_button_state[BUTTON_ID_SET_PEEP].count = 0;

    m_button_state[BUTTON_ID_SET_TV].type = BUTTON_TYPE_TOGGLE;
    m_button_state[BUTTON_ID_SET_TV].state = BUTTON_STATE_IDLE;
    m_button_state[BUTTON_ID_SET_TV].wait = 0;
    m_button_state[BUTTON_ID_SET_TV].count = 0;

    m_button_state[BUTTON_ID_SET_BUR].type = BUTTON_TYPE_TOGGLE_AND_PRESS_TO_HOLD;
    m_button_state[BUTTON_ID_SET_BUR].state = BUTTON_STATE_IDLE;
    m_button_state[BUTTON_ID_SET_BUR].wait = BUTTON_DELAY;
    m_button_state[BUTTON_ID_SET_BUR].count = 0;

    m_button_state[BUTTON_ID_SET_PEAK].type = BUTTON_TYPE_TOGGLE;
    m_button_state[BUTTON_ID_SET_PEAK].state = BUTTON_STATE_IDLE;
    m_button_state[BUTTON_ID_SET_PEAK].wait = 0;
    m_button_state[BUTTON_ID_SET_PEAK].count = 0;

    m_button_state[BUTTON_ID_SET_ITIME].type = BUTTON_TYPE_TOGGLE;
    m_button_state[BUTTON_ID_SET_ITIME].state = BUTTON_STATE_IDLE;
    m_button_state[BUTTON_ID_SET_ITIME].wait = 0;
    m_button_state[BUTTON_ID_SET_ITIME].count = 0;

    m_button_state[BUTTON_ID_POWER_DOWN].type = BUTTON_TYPE_TOGGLE_AND_PRESS_TO_HOLD;
    m_button_state[BUTTON_ID_POWER_DOWN].state = BUTTON_STATE_IDLE;
    m_button_state[BUTTON_ID_POWER_DOWN].wait = POWER_OFF_DELAY;
    m_button_state[BUTTON_ID_POWER_DOWN].count = 0;

    m_button_state[BUTTON_ID_ALRM_SILENCE].type = BUTTON_TYPE_TOGGLE_AND_PRESS_TO_HOLD;
    m_button_state[BUTTON_ID_ALRM_SILENCE].state = BUTTON_STATE_IDLE;
    m_button_state[BUTTON_ID_ALRM_SILENCE].wait = ALARM_CLEAR_DELAY;
    m_button_state[BUTTON_ID_ALRM_SILENCE].count = 0;

    m_button_state[BUTTON_ID_GET_PLAT].type = BUTTON_TYPE_MOMENTARY;
    m_button_state[BUTTON_ID_GET_PLAT].state = BUTTON_STATE_IDLE;
    m_button_state[BUTTON_ID_GET_PLAT].wait = 0;
    m_button_state[BUTTON_ID_GET_PLAT].count = 0;

    m_button_state[BUTTON_ID_ADJ_DWN].type = BUTTON_TYPE_MOMENTARY;
    m_button_state[BUTTON_ID_ADJ_DWN].state = BUTTON_STATE_IDLE;
    m_button_state[BUTTON_ID_ADJ_DWN].wait = 0;
    m_button_state[BUTTON_ID_ADJ_DWN].count = 0;

    m_button_state[BUTTON_ID_ADJ_UP].type = BUTTON_TYPE_MOMENTARY;
    m_button_state[BUTTON_ID_ADJ_UP].state = BUTTON_STATE_IDLE;
    m_button_state[BUTTON_ID_ADJ_UP].wait = 0;
    m_button_state[BUTTON_ID_ADJ_UP].count = 0;
    m_button_in_progress = BUTTON_NONE;
    m_adjust_button_in_progress = BUTTON_NONE;
    m_edit_timeout = 0;
}

bool detect_button_state(PanelButtons* val) {
    static uint16_t last_reading = 0;  // Static tracking of last reading on the button.
    static uint16_t last_reading_same_count = 0;  // Count of reading the buttons exactly
    uint16_t buttonsm = 0; //Masked buttons readings
    uint16_t buttons1 = 0;
    uint16_t buttons2 = 0;
    uint16_t buttons3 = 0;
    SW_ASSERT(val != NULL);
    // Buttons are read three times sequentially in order to determine the truth of the state of the button. This is
    // the most basic button de-bounce. Every one of the three reads must return the same value, or an error is reported
    // and the buttons must be read again.
    SW_ASSERT(mcp23017_read_reg(&m_button_mcp_handle, REG_GPIOA, (uint8_t*)&buttons1, 2) == HAL_OK);
    SW_ASSERT(mcp23017_read_reg(&m_button_mcp_handle, REG_GPIOA, (uint8_t*)&buttons2, 2) == HAL_OK);
    SW_ASSERT(mcp23017_read_reg(&m_button_mcp_handle, REG_GPIOA, (uint8_t*)&buttons3, 2) == HAL_OK);
    buttonsm = (buttons1 & 0x3BFC); // Complete used button mask

    // Check for error status, or inconsistent reads
    if ((buttons1 != buttons2) || (buttons2 != buttons3) || (buttons1 != buttons3)) {
        p_uartDebug.fswStats.switchI2CErrors++;
        return false;
    }
    // Implement button-stuck count. If any series of button presses remains continuously pressed for the full set of cycles will set a fault.
    // This is considered rare unless: buttons are sticking (true positive) or a user is attempting to fault the machine (false positive). There
    // is no way to distinguish between a permanently stuck button and a user holding a button down for the detection window.
    else if ((last_reading & buttonsm) != 0) {
        last_reading_same_count += 1;
        SW_ASSERT(last_reading_same_count < BUTTON_STUCK_CYCLES); // Trip a machine fault if we have seen held down buttons for 60 seconds
    }
    // No sameness in readings.  Reset count.
    else {
        last_reading_same_count = 0;
    }
    last_reading = buttons1;
    (void) memset(val, 0, sizeof(PanelButtons));
    // Never trust ordering in bit fields
    val->SET_FIO_ALRM_B = (buttons1 & 0x0080)?BUTTON_POS_ON:BUTTON_POS_OFF;
    val->SET_PEEP_B =     (buttons1 & 0x0040)?BUTTON_POS_ON:BUTTON_POS_OFF;
    val->SET_TV_B =       (buttons1 & 0x0020)?BUTTON_POS_ON:BUTTON_POS_OFF;
    val->SET_BUR_B =      (buttons1 & 0x0010)?BUTTON_POS_ON:BUTTON_POS_OFF;
    val->SET_PEAK_B =     (buttons1 & 0x0008)?BUTTON_POS_ON:BUTTON_POS_OFF;
    val->SET_ITIME_B =    (buttons1 & 0x0004)?BUTTON_POS_ON:BUTTON_POS_OFF;
    val->POWER_DOWN_B =   (buttons1 & 0x2000)?BUTTON_POS_ON:BUTTON_POS_OFF;
    val->ALRM_SILENCE_B = (buttons1 & 0x1000)?BUTTON_POS_ON:BUTTON_POS_OFF;
    val->GET_PLAT_B =     (buttons1 & 0x0800)?BUTTON_POS_ON:BUTTON_POS_OFF;
    val->ADJ_DWN_B =      (buttons1 & 0x0200)?BUTTON_POS_ON:BUTTON_POS_OFF;
    val->ADJ_UP_B =       (buttons1 & 0x0100)?BUTTON_POS_ON:BUTTON_POS_OFF;
    // summary; BUTTON_POS_ON means at least one set
    val->ALL_BUTTONS = ((val->SET_FIO_ALRM_B) ||
                        (val->SET_PEEP_B) ||
                        (val->SET_TV_B) ||
                        (val->SET_BUR_B) ||
                        (val->SET_PEAK_B) ||
                        (val->SET_ITIME_B) ||
                        (val->POWER_DOWN_B) ||
                        (val->ALRM_SILENCE_B) ||
                        (val->GET_PLAT_B) ||
                        (val->ADJ_DWN_B) ||
                        (val->ADJ_UP_B)) ? BUTTON_POS_ON : BUTTON_POS_OFF;
    return true;
}

bool isAdjustActionable(ButtonId id) {
    // Ensure we are working with adjustments only
    SW_ASSERT((id == BUTTON_ID_ADJ_DWN) || (id == BUTTON_ID_ADJ_UP));
    // Adjustment buttons are only active if a button is in some MODIFY state or PRESS_TO_MODIFY state (with the exception of FIO2) and no adjustment button is active.
    bool actionable = 1;
    actionable = actionable && (m_button_in_progress != BUTTON_NONE); // Button (but not FIO) button must be active
    actionable = actionable && (m_button_in_progress == BUTTON_ID_SET_BUR || m_button_in_progress == BUTTON_ID_SET_ITIME ||
                                m_button_in_progress == BUTTON_ID_SET_PEAK || m_button_in_progress == BUTTON_ID_SET_PEEP ||
                                m_button_in_progress == BUTTON_ID_SET_TV);
    actionable = actionable && ((BUTTON_STATE_MODIFY == m_button_state[m_button_in_progress].state) ||         // The active button must be in MODIFY or
                                (BUTTON_STATE_PRESS_TO_MODIFY == m_button_state[m_button_in_progress].state)); // PRESS_TO_MODIFY state
    actionable = actionable && (m_adjust_button_in_progress == BUTTON_NONE);   // No adjustment button may be active
    return actionable;
}

bool isButtonActionable(ButtonId id) {
    // Check the current button id can be handled by this function
    switch (id) {
        // Cascade intended: for all serviceable buttons.
        // These buttons are actionable iff the following items are true. First, we are in POWER_ON_STATE state, or actioning power or alarm buttons. Next, there is no other active
        // button. Third, there is no other adjustment button (finish the adjustment first) and then the button can be finished.
        case BUTTON_ID_SET_FIO_ALARM:
        case BUTTON_ID_SET_PEEP:
        case BUTTON_ID_SET_TV:
        case BUTTON_ID_SET_BUR:
        case BUTTON_ID_SET_PEAK:
        case BUTTON_ID_SET_ITIME:
        case BUTTON_ID_POWER_DOWN:
        case BUTTON_ID_ALRM_SILENCE:
        case BUTTON_ID_GET_PLAT: {
            bool actionable = 1;
            actionable = actionable && ((POWER_ON_STATE == p_powerState) || (BUTTON_ID_ALRM_SILENCE == id) || (BUTTON_ID_POWER_DOWN == id));
            actionable = actionable && ((id == m_button_in_progress) || (BUTTON_NONE == m_button_in_progress)); // No active button, or currently the active button.
            actionable = actionable && (m_adjust_button_in_progress == BUTTON_NONE); // No adjustment button in progress
            return actionable;
        }
        // Unreal buttons and adjustments should never be the current button in this function
        case BUTTON_ID_ADJ_DWN:
        case BUTTON_ID_ADJ_UP:
        case BUTTON_ID_NUM_BUTTONS:
        case BUTTON_NONE:
        default:
            SW_ASSERT(0);
            break;
    }
    // For code analysis
    return 0;
}

void updateButtonPressSm(ButtonId id) {
    SW_ASSERT(id <= BUTTON_NONE); // Set to some valid enum value
    // Only process button if it is actionable
    if (isButtonActionable(id)) {
        switch (m_button_state[id].state) {
            case BUTTON_STATE_IDLE:
                switch (m_button_state[id].type) {
                    case BUTTON_TYPE_PRESS_TO_HOLD: // fall through
                    case BUTTON_TYPE_TOGGLE_AND_PRESS_TO_HOLD:
                        // copy the wait time to the countdown and switch state to in press progress
                        m_button_state[id].state = BUTTON_STATE_PRESS_TO_HOLD;
                        m_button_state[id].count = m_button_state[id].wait;
                        break;
                    case BUTTON_TYPE_TOGGLE:
                        // if state is idle, switch to waiting for release
                        m_button_state[id].state = BUTTON_STATE_WAITING_FOR_RELEASE_MODIFY;
                        break;
                    case BUTTON_TYPE_MOMENTARY:
                        m_button_state[id].state = BUTTON_STATE_WAITING_FOR_RELEASE_IDLE;
                        break;
                    default:
                        SW_ASSERT1(0,m_button_state[id].type);
                        return; // for code checkers
                }
                m_button_in_progress = id;
                break;
            case BUTTON_STATE_PRESS_TO_HOLD:
                // if state is in progress, count down time to hold
                if (0 == --m_button_state[id].count) {
                    if (BUTTON_TYPE_PRESS_TO_HOLD == m_button_state[id].type) {
                        // change to modify state
                        m_button_state[m_button_in_progress].state = BUTTON_STATE_WAITING_FOR_RELEASE_MODIFY;
                        // start beep
                        (void) sound_start(SOUND_BEEP);
                    } else {
                        // change to modify state
                        m_button_state[m_button_in_progress].state = BUTTON_STATE_WAITING_FOR_RELEASE_PRESS_TO_MODIFY;
                        // start beep
                        (void) sound_start(SOUND_BEEP);
                    }
                }
                break;
            case BUTTON_STATE_MODIFY:
                m_button_state[id].state = BUTTON_STATE_WAITING_FOR_RELEASE_IDLE;
                break;
            case BUTTON_STATE_PRESS_TO_MODIFY:
                m_button_state[id].state = BUTTON_STATE_WAITING_FOR_RELEASE_IDLE;
                break;
                // if waiting for button release, do nothing
            case BUTTON_STATE_WAITING_FOR_RELEASE_PRESS_TO_MODIFY:
            case BUTTON_STATE_WAITING_FOR_RELEASE_MODIFY:
            case BUTTON_STATE_WAITING_FOR_RELEASE_IDLE:
                break;
            default:
                SW_ASSERT1(0,m_button_state[id].state);
                break;
        }
    }
}

void updateAdjustPressSm(ButtonId id) {
    // Only process adjustment press if it is actionable.
    if (isAdjustActionable(id)) {
        switch (m_button_state[id].state) {
            case BUTTON_STATE_IDLE:
                m_button_state[id].state = BUTTON_STATE_WAITING_FOR_RELEASE_IDLE;
                m_adjust_button_in_progress = id;
                break;
            case BUTTON_STATE_WAITING_FOR_RELEASE_IDLE:
                break;
            default:
                SW_ASSERT1(0,m_button_state[id].state);
                return; // for code checkers
        }
    }
}

void updateButtonReleaseSm(void) {
    // first check to see if a adjustment button was in progress
    if (m_adjust_button_in_progress != BUTTON_NONE) {
        m_button_state[m_adjust_button_in_progress].state = BUTTON_STATE_MOMENTARY_DONE;
        m_adjust_button_in_progress = BUTTON_NONE;
    }
    // Only deal with button if another one is not active
    else if (m_button_in_progress != BUTTON_NONE) {
        switch (m_button_state[m_button_in_progress].state) {
            case BUTTON_STATE_PRESS_TO_HOLD:
                if (BUTTON_TYPE_TOGGLE_AND_PRESS_TO_HOLD == m_button_state[m_button_in_progress].type) {
                    // button is toggled instead of held, so go to modify state
                    m_button_state[m_button_in_progress].state = BUTTON_STATE_MODIFY;
                    (void) sound_start(SOUND_BEEP);
                } else {
                    // let go of button early, so quit
                    m_button_state[m_button_in_progress].state = BUTTON_STATE_IDLE;
                    m_button_in_progress = BUTTON_NONE;
                }
                break;
            case BUTTON_STATE_WAITING_FOR_RELEASE_MODIFY:
                // if the button was waiting to be released, go to modify state
                m_button_state[m_button_in_progress].state = BUTTON_STATE_MODIFY;
                // start beep
                (void) sound_start(SOUND_BEEP);
                break;
            case BUTTON_STATE_WAITING_FOR_RELEASE_PRESS_TO_MODIFY:
                // if the button was waiting to be released, go to press to modify state
                m_button_state[m_button_in_progress].state = BUTTON_STATE_PRESS_TO_MODIFY;
                (void) sound_start(SOUND_BEEP);
                break;
            case BUTTON_STATE_WAITING_FOR_RELEASE_IDLE:
                 // if the button was waiting to be released, go to idle state
                 if (BUTTON_TYPE_MOMENTARY == m_button_state[m_button_in_progress].type) {
                     m_button_state[m_button_in_progress].state = BUTTON_STATE_MOMENTARY_DONE;
                 } else {
                     m_button_state[m_button_in_progress].state = BUTTON_STATE_IDLE;
                 }
                 (void) sound_start(SOUND_BEEP);
                 m_button_in_progress = BUTTON_NONE;
                 break;
            case BUTTON_STATE_MODIFY: // In modification state: handle-edit timeout
            case BUTTON_STATE_PRESS_TO_MODIFY:
                // We are in one of the modify states, check and run timeout
                if (m_edit_timeout >= EDIT_TIMEOUT) {
                    m_button_state[m_button_in_progress].state = BUTTON_STATE_IDLE;
                    m_button_in_progress = BUTTON_NONE;
                    (void) sound_start(SOUND_TWO_BEEP); // Double beep on timeout
                }
                break;
            default:
                // BUTTON_STATE_IDLE should never happen or corrupted value
                SW_ASSERT1(0, m_button_state[m_button_in_progress].state);
                break; // for code checkers
         }
    } else {
        // Asserts expectation that the button state is always idle for all buttons if no button is active
        // and a release is being processed. This will trip if any buttons become stuck.
        uint32_t i = 0;
        for (i = 0; i < BUTTON_ID_NUM_BUTTONS; i++) {
            SW_ASSERT1(m_button_state[i].state == BUTTON_STATE_IDLE, m_button_state[i].state);
        }
    }
}

void run_button_state_machines(PanelButtons* val) {
    SW_ASSERT(val);
    // If button is released, update any waiting button presses otherwise processes each button that could
    // be set, looking for which one was pressed.
    if (BUTTON_POS_OFF == val->ALL_BUTTONS) {
        updateButtonReleaseSm();
        SW_ASSERT(m_edit_timeout < 0xFFFFFFFF); // Fail if about to overflow
        m_edit_timeout += 1; // Increase in-action timeout
    } else {
        m_edit_timeout = 0; // A button is currently pressed, this resets the timeout
        // SET_FIO_ALRM_B
        if (BUTTON_POS_ON == val->SET_FIO_ALRM_B) {
            updateButtonPressSm(BUTTON_ID_SET_FIO_ALARM);
        }
        // SET_PEEP_B
        else if (BUTTON_POS_ON == val->SET_PEEP_B) {
            updateButtonPressSm(BUTTON_ID_SET_PEEP);
        }
        // SET_TV_B
        else if (BUTTON_POS_ON == val->SET_TV_B) {
            updateButtonPressSm(BUTTON_ID_SET_TV);
        }
        // SET_BUR_B
        else if (BUTTON_POS_ON == val->SET_BUR_B) {
            updateButtonPressSm(BUTTON_ID_SET_BUR);
        }
        // SET_PEAK_B
        else if (BUTTON_POS_ON == val->SET_PEAK_B) {
            updateButtonPressSm(BUTTON_ID_SET_PEAK);
        }
        // SET_ITIME_B
        else if (BUTTON_POS_ON == val->SET_ITIME_B) {
            updateButtonPressSm(BUTTON_ID_SET_ITIME);
        }
        // POWER_DOWN_B
        else if (BUTTON_POS_ON == val->POWER_DOWN_B) {
            updateButtonPressSm(BUTTON_ID_POWER_DOWN);
        }
        // ALRM_SILENCE_B
        else if (BUTTON_POS_ON == val->ALRM_SILENCE_B) {
            updateButtonPressSm(BUTTON_ID_ALRM_SILENCE);
        }
        // GET_PLAT_B
        else if (BUTTON_POS_ON == val->GET_PLAT_B) {
            updateButtonPressSm(BUTTON_ID_GET_PLAT);
        }
        // ADJ_DWN_B
        else if (BUTTON_POS_ON == val->ADJ_DWN_B) {
            updateAdjustPressSm(BUTTON_ID_ADJ_DWN);
        }
        // ADJ_UP_B
        else if (BUTTON_POS_ON == val->ADJ_UP_B) {
            updateAdjustPressSm(BUTTON_ID_ADJ_UP);
        }
        // Expected some press
        else {
            SW_ASSERT(0);
        }
    }
    run_special_button_states();
}

void run_special_button_states() {
    // act on non-display related buttons

    // check power button state.
    // Momentary (equivalent to modify state) is on,
    // Hold is to turn off
    switch (m_button_state[BUTTON_ID_POWER_DOWN].state) {
        case BUTTON_STATE_MODIFY: // This is push to turn on
            if (POWER_OFF_STATE == p_powerState) {
                p_powerState = POWERING_STATE;
                (void) sound_start(SOUND_BEEP);
            }
            m_button_state[BUTTON_ID_POWER_DOWN].state = BUTTON_STATE_IDLE;
            m_button_in_progress = BUTTON_NONE;
            break;
        case BUTTON_STATE_WAITING_FOR_RELEASE_PRESS_TO_MODIFY:
            if (POWER_ON_STATE == p_powerState) {
                p_powerState = POWER_OFF_STATE; // will be cleared when power bit is sent to controller
                (void) sound_start(SOUND_CONSTANT);
            }
            m_button_state[BUTTON_ID_POWER_DOWN].state = BUTTON_STATE_WAITING_FOR_RELEASE_IDLE;
            break;
        case BUTTON_STATE_PRESS_TO_MODIFY: // This is push to wait to turn off
        case BUTTON_STATE_IDLE: // fall through - don't care about these states
        case BUTTON_STATE_PRESS_TO_HOLD:
        case BUTTON_STATE_WAITING_FOR_RELEASE_MODIFY:
        case BUTTON_STATE_WAITING_FOR_RELEASE_IDLE:
            break;
        default: // shouldn't get here
            SW_ASSERT1(0,m_button_state[BUTTON_ID_POWER_DOWN].state);
            break;
    }

    // act on non-display related buttons

    // Alarms:
    //  1. Press (BUTTON_STATE_MODIFY) silences alarm for X amount of time
    //  2. Press and hold: resets all alarms
    switch (m_button_state[BUTTON_ID_ALRM_SILENCE].state) {
        case BUTTON_STATE_MODIFY: // Pushed once will silence, but not de-latch the alarm condition
            alarm_silence(&p_numericalValues.alarms);
            m_button_state[BUTTON_ID_ALRM_SILENCE].state = BUTTON_STATE_IDLE;
            m_button_in_progress = BUTTON_NONE;
            break;
        case BUTTON_STATE_WAITING_FOR_RELEASE_PRESS_TO_MODIFY: // Held long enough
            alarm_clear(&p_numericalValues.alarms);
            m_button_state[BUTTON_ID_ALRM_SILENCE].state = BUTTON_STATE_WAITING_FOR_RELEASE_IDLE;
            break;
        case BUTTON_STATE_PRESS_TO_MODIFY: // Pushing and held with de-latch the alarms, and silence
        case BUTTON_STATE_IDLE: // fall through - don't care about these states
        case BUTTON_STATE_PRESS_TO_HOLD:
        case BUTTON_STATE_WAITING_FOR_RELEASE_MODIFY:
        case BUTTON_STATE_WAITING_FOR_RELEASE_IDLE:
            break;
        default: // shouldn't get here
            SW_ASSERT1(0,m_button_state[BUTTON_ID_ALRM_SILENCE].state);
            break;
    }

    // check plateau button
    if (BUTTON_STATE_MOMENTARY_DONE == m_button_state[BUTTON_ID_GET_PLAT].state) {
        p_doPlateau = DO_PLATEAU_SENDS;
        m_button_state[BUTTON_ID_GET_PLAT].state = BUTTON_STATE_IDLE;
    }
}

void update_button_numerical_state(ButtonPressState state, NumericalValue* value) {
    SW_ASSERT(value != NULL);
    if (BUTTON_STATE_MODIFY == state) {
        value->mode = ((value->mode == DISPLAY_VALUE) || (value->mode == DISPLAY_EDIT_ALARM)) ? DISPLAY_EDIT_ALARM : DISPLAY_EDIT_SETPOINT;
        // check up arrow
        if (BUTTON_STATE_MOMENTARY_DONE == m_button_state[BUTTON_ID_ADJ_UP].state) {
            // increment value if not past max
            if ((value->editval + value->step) <= value->upper) {
                value->editval += value->step;
            }
            // clear up button to idle
            m_button_state[BUTTON_ID_ADJ_UP].state = BUTTON_STATE_IDLE;
        } else if (BUTTON_STATE_MOMENTARY_DONE == m_button_state[BUTTON_ID_ADJ_DWN].state) {
            // do something for down
            if ((value->editval - value->step) >= value->lower) {
                value->editval -= value->step;
            }
            // clear down button to idle
            m_button_state[BUTTON_ID_ADJ_DWN].state = BUTTON_STATE_IDLE;

        }
    }
    // Handle the ability to fallback by copying the setpoint into editval just before detecting
    // up and down presses, and copying the editval back to the setpoint when done.
    else if (BUTTON_STATE_WAITING_FOR_RELEASE_MODIFY == state) {
        value->editval = value->setpoint;
    }
    else if (BUTTON_STATE_WAITING_FOR_RELEASE_IDLE == state) {
        // Revert to the correct display mode when done
        value->mode = ((value->mode == DISPLAY_VALUE) || (value->mode == DISPLAY_EDIT_ALARM)) ? DISPLAY_VALUE : DISPLAY_SETPOINT;
        value->setpoint = value->editval;
    }
}

void update_backup_rate_numerical_states(ButtonPressState state, NumericalValue* backup_rate,  NumericalValue* resp_rate) {
    SW_ASSERT(backup_rate != NULL);
    SW_ASSERT(resp_rate != NULL);
    // Normal state for displaying these values.  Will update in update_numerical_state when editing
    resp_rate->mode = DISPLAY_VALUE;
    backup_rate->mode = DISPLAY_SETPOINT;
    // Backup rate is a special case - when toggled, adjusts backup rate
    // when press-to-hold, adjusts max respiratory rate
    if (BUTTON_STATE_MODIFY == state) {
        update_button_numerical_state(BUTTON_STATE_MODIFY, backup_rate);
    } else if (BUTTON_STATE_PRESS_TO_MODIFY == state) {
        update_button_numerical_state(BUTTON_STATE_MODIFY, resp_rate);
    }
    else if (BUTTON_STATE_WAITING_FOR_RELEASE_MODIFY == state) {
        backup_rate->editval = backup_rate->setpoint;
    }
    else if (BUTTON_STATE_WAITING_FOR_RELEASE_PRESS_TO_MODIFY == state) {
        resp_rate->editval = resp_rate->setpoint;
    }
    // Copy the edited values to the setpoint values as we are leaving edit mode
    else if (BUTTON_STATE_WAITING_FOR_RELEASE_IDLE == state) {
        backup_rate->setpoint = backup_rate->editval;
        resp_rate->setpoint = resp_rate->editval;
    }
}

void update_fio2_numerical_states(ButtonPressState state, NumericalValue* fio2) {
    SW_ASSERT(fio2);
    // FI02 is a special case button, it only starts edit and save the value back, no need for up and
    // down arrows.  FIO2 edit timeout has no effect except to reset the display blink.
    fio2->mode = DISPLAY_VALUE;
    if (state == BUTTON_STATE_MODIFY) {
        fio2->mode = DISPLAY_EDIT_WITH_VALUE;
    } else if (state == BUTTON_STATE_WAITING_FOR_RELEASE_IDLE) {
        fio2->setpoint = fio2->val;
    }
}

void run_buttons() {
    PanelButtons buttons;
    if (detect_button_state(&buttons)) {
        // Process the button state and detect the
        run_button_state_machines(&buttons);
        // Update the button state for the 4 buttons that all operate similarly
        // Must reset display here for non-handled cases
        p_numericalValues.PEEP.mode = DISPLAY_SETPOINT;
        p_numericalValues.ins_time.mode = DISPLAY_SETPOINT;
        p_numericalValues.tidal_volume.mode = DISPLAY_SETPOINT;
        p_numericalValues.peak_pressure.mode = DISPLAY_SETPOINT;
        update_button_numerical_state(m_button_state[BUTTON_ID_SET_PEEP].state, &p_numericalValues.PEEP);
        update_button_numerical_state(m_button_state[BUTTON_ID_SET_ITIME].state,&p_numericalValues.ins_time);
        update_button_numerical_state(m_button_state[BUTTON_ID_SET_TV].state,   &p_numericalValues.tidal_volume);
        update_button_numerical_state(m_button_state[BUTTON_ID_SET_PEAK].state, &p_numericalValues.peak_pressure);
        update_backup_rate_numerical_states(m_button_state[BUTTON_ID_SET_BUR].state, &p_numericalValues.backup_rate,  &p_numericalValues.resp_rate);
        update_fio2_numerical_states(m_button_state[BUTTON_ID_SET_FIO_ALARM].state, &p_numericalValues.FIO2);
        // Extra safety checks amount to PEEP / PEAK inversion. This forces the values to prevent inversion on the active button.
        // If PEEP is active, its edit-point *may not* go above peak pressure's current setpoint
        if ((m_button_state[BUTTON_ID_SET_PEEP].state == BUTTON_STATE_MODIFY) &&
            (p_numericalValues.PEEP.editval > p_numericalValues.peak_pressure.setpoint)) {
            p_numericalValues.PEEP.editval = p_numericalValues.peak_pressure.setpoint;
        }
        // If peak pressure is active, its edit-point *may not* go above peak pressure's current setpoint
        else if ((m_button_state[BUTTON_ID_SET_PEAK].state == BUTTON_STATE_MODIFY) &&
                 (p_numericalValues.peak_pressure.editval < p_numericalValues.PEEP.setpoint)) {
            p_numericalValues.peak_pressure.editval = p_numericalValues.PEEP.setpoint;
        }
    }
}

