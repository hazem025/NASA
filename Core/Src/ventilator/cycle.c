/*
 * ventilator/cycle.c:
 *
 * Implementation code for the cycle's single driven iteration.
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
#include <ventilator/controller.h>
#include <ventilator/watchdog.h>
#include <ventilator/eeprom.h>
#include <ventilator/alarm.h>

// TEST_MODE always has an attached controller
#ifndef TEST_MODE
static bool CONTROLLER_ATTACHED = false;
#else
static bool CONTROLLER_ATTACHED = true;
#endif

void cycle(void) {
    static uint32_t cycle_count = 1; // Start at one, so increments only happen at 1 minute
    static uint32_t powering_cycle_count = 0; // Count to stay in powering state
    HAL_StatusTypeDef status = HAL_OK;
    spin_on_incoming_watchdog();
    stroke_outgoing_watchdog(); // Note that we are still alive
// TEST_MODE performs basic hardware tests to validate the panel
#ifdef TEST_MODE
    doTestCycle();
    return;
#endif

    // Communicate with the SPI controller here. Wait to do the rest of the cycle until we get the first HAL_OK from the controller. That
    // represents it is online and we should de-blank.
    status = do_controller_cycle();
    if (status == HAL_OK) {
        CONTROLLER_ATTACHED = 1;
        reset_fail_safe_timer();
    }
    // Once the controller has been detected as online, we will begin normal operations
    if (CONTROLLER_ATTACHED) {
        // Update alive-time in minutes count
        if ((cycle_count % (CYCLES_PER_SECOND * 60)) == 0) {
            p_aliveMinutes += 1;
            SW_ASSERT(writeEeprom(EEPROM_ALIVE_MINUTES, p_aliveMinutes) == EEPROM_OK);
        }
        SW_ASSERT((p_aliveMinutes/60) < 2688);
        // Sound cycling should happen before any alarm setups or beeps
        SW_ASSERT(sound_cycle() == HAL_OK);  // Always stroke sound first
        // Detect and handle button presses
        run_buttons();
        // Run the detection for the alarm state as the last step before updating the display
        alarm_run(&p_numericalValues, p_powerState);
    }
    // Process display setup, blanking if we have not attached yet
    run_display(!CONTROLLER_ATTACHED, p_aliveMinutes/60);
    // Powering on state machine creates a powering-on time to display the hour count.
    // Power off state resets cycle count time
    if (p_powerState == POWER_OFF_STATE) {
        powering_cycle_count = 0;
        cycle_count = 1;
    }
    // Powering on before the power on time
    else if ((p_powerState == POWERING_STATE) && (powering_cycle_count < POWERING_ON_TIME)) {
        powering_cycle_count += 1;
        cycle_count += 1;
    }
    // Otherwise we need to transistion to power state
    else if ((p_powerState == POWERING_STATE) || (p_powerState == POWER_ON_STATE)) {
        p_powerState = POWER_ON_STATE;
        cycle_count += 1;
    }
}
