/*
 * button.h:
 *
 * Controls the button state machines and the detection for buttons pushed. These buttons follow a state machine
 * that looks for "pressed" (momentary) buttons, modify buttons, and press hold and modify buttons. This file handles
 * both the generic state machines, and the specifc handling of each button.
 *
 *  Created on: Apr 9, 2020
 *      Author: tcanham
 */

#ifndef INC_VENTILATOR_BUTTON_H_
#define INC_VENTILATOR_BUTTON_H_

#include <stdint.h>
#include <stdbool.h>
#include "stm32f0xx_hal.h"
#include <ventilator/mcp23017.h>
#include <ventilator/types.h>

/**
 * ButtonPosState:
 *
 * Button position state (on or off).
 */
typedef enum {
    BUTTON_POS_OFF, // Button is off
    BUTTON_POS_ON   // Button is on
} ButtonPosState;

/**
 * ButtonPosState:
 *
 * Set of all button positions to track which buttons are pressed, and what are not.
 */
typedef struct {
    ButtonPosState SET_FIO_ALRM_B;
    ButtonPosState SET_PEEP_B;
    ButtonPosState SET_TV_B;
    ButtonPosState SET_BUR_B;
    ButtonPosState SET_PEAK_B;
    ButtonPosState SET_ITIME_B;
    ButtonPosState POWER_DOWN_B;
    ButtonPosState ALRM_SILENCE_B;
    ButtonPosState GET_PLAT_B;
    ButtonPosState ADJ_DWN_B;
    ButtonPosState ADJ_UP_B;
    ButtonPosState ALL_BUTTONS; //!< borrow for quick overall check
} PanelButtons;

/**
 * ButtonId:
 *
 * Individual button ids to track which button state machine is active and in-progress.
 */
typedef enum {
    BUTTON_ID_SET_FIO_ALARM,
    BUTTON_ID_SET_PEEP,
    BUTTON_ID_SET_TV,
    BUTTON_ID_SET_BUR,
    BUTTON_ID_SET_PEAK,
    BUTTON_ID_SET_ITIME,
    BUTTON_ID_POWER_DOWN,
    BUTTON_ID_ALRM_SILENCE,
    BUTTON_ID_GET_PLAT,
    BUTTON_ID_ADJ_DWN,
    BUTTON_ID_ADJ_UP,
    BUTTON_ID_NUM_BUTTONS,
    BUTTON_NONE
} ButtonId;
/**
 * ButtonType:
 *
 * Button types available.
 */
typedef enum {
    BUTTON_TYPE_TOGGLE,        // Toggles to modify state and then again to idle
    BUTTON_TYPE_PRESS_TO_HOLD, // Held down trigger behavior
    BUTTON_TYPE_MOMENTARY,     // Press once to capture a pres
    BUTTON_TYPE_TOGGLE_AND_PRESS_TO_HOLD // Dual-function.  Press for toggle behavior, hold for hold behavior.
} ButtonType;

/**
 * ButtonPressState:
 *
 * States of the button state machines. Only one state machine (one button) can leave IDLE at a time.
 */
typedef enum {
    BUTTON_STATE_IDLE,           // IDLE state, nothing happening
    BUTTON_STATE_PRESS_TO_HOLD,  // Button looking to see if it was held-down or merely pressed
    BUTTON_STATE_MODIFY,         // MODIFY actions allowed as a button was pressed
    BUTTON_STATE_PRESS_TO_MODIFY,// MODIFY actions allowed but this button was held to modify
    BUTTON_STATE_WAITING_FOR_RELEASE_MODIFY,          // Waiting for release into the MODIFY state
    BUTTON_STATE_WAITING_FOR_RELEASE_PRESS_TO_MODIFY, // Waiting for release into MODIFY after being held
    BUTTON_STATE_WAITING_FOR_RELEASE_IDLE,            // Waiting for release to return to IDLE
    BUTTON_STATE_MOMENTARY_DONE, // A momentary button was pressed and released **manually clear to IDLE**
    BUTTON_STATE_TIMEOUT,        // Button has timed-out and is returning to IDLE
    BUTTON_STATE_NUM_STATES      // Assertiion check
} ButtonPressState ;
/**
 * ButtonState:
 *
 * Button state data including type, machine state, counts (for hold and timeout) and wait information.
 */
typedef struct {
    ButtonType type;
    ButtonPressState state;
    uint32_t count; // countdown for press to hold, in cycles
    uint32_t wait; // wait time for press to hold to complete, in cycles
} ButtonState;
/**
 * Initializes button module's state variables.
 */
void init_button_state(void);

/**
 * Detect if a button (any button) has been pressed and fill-out the state for which one was pressed.
 * PanelButtons* val: value to record button presses in
 * return: true if *any* button is pressed
 */
bool detect_button_state(PanelButtons* val);

/**
 * Using the pressed button states detected in "detect_button_state", run all state machines. Typically
 * only one state machine is active, but this will run all of them anyway.
 * PanelButtons* val: detected button states. Use detect_button_state to fill first.
 */
void run_button_state_machines(PanelButtons* val);

/**
 * Update the button numerical states. This handles MODIFY + adjust up or down MOMENTARTY states to edit
 * the editpoint up or down. This applies to all non-held buttons exclusing FIO2 and is called internally
 * by the HELD backup rate/resp rate button. Call once for each button.
 * ButtonPressState state: button's state to handle
 * NumericalValue* value: value to adjust up or down.
 */
void update_button_numerical_state(ButtonPressState state, NumericalValue* value);
/**
 * Update the button numerical states for backup rate/resp rate. Multiplexes backup rate/resp rate based
 * on MODIFY or PRESS_TO_MODIFY state. Then passes off to "update_button_numerical_state" to run the code
 * on the correct numerical value.
 * ButtonPressState state: button's state to handle
 * NumericalValue* backup_rate: value to adjust up or down when pressed
 * NumericalValue* resp_rate: value to adjust up or down when pressed and held long enough
 */
void update_backup_rate_numerical_states(ButtonPressState state, NumericalValue* backup_rate,  NumericalValue* resp_rate);
/**
 * FIO2 is only recording it's sensor val into setpoint when transitioning in and out of modify. FIO2 does not
 * allow adjusts.
 * ButtonPressState state: button's state to handle
 * NumericalValue* backup_rate: value to adjust up or down when pressed
 * NumericalValue* fio2: value to update
 */
void update_fio2_numerical_states(ButtonPressState state, NumericalValue* fio2);
/**
 * Run the button module looking for presses and transitioning state machines.
 */
void run_buttons();

/**
 * Runs special state processing for special case buttons (non-editing). Called from
 * "run_button_state_machines".
 */
void run_special_button_states();

/**
 * Determine if a adjustment (edit up or down) button can run. Otherwise the press will be ignored.
 * Is only actionable if another button is in MODIFY, and the other adjust button is not pressed.
 */
bool isAdjustActionable(ButtonId id);
/**
 * See if a non-adjust button is actionable. This is actionable if all other buttons is IDLE.
 */
bool isButtonActionable(ButtonId id);


#endif /* INC_VENTILATOR_BUTTON_H_ */
