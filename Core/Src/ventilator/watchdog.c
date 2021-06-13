/*
 * watchdog.c
 *
 *  Created on: Apr 14, 2020
 *      Author: tcanham
 */

#include <stdint.h>
#include "stm32f0xx_hal.h"
#include <ventilator/constants.h>
#include <ventilator/watchdog.h>
#include <ventilator/panel_public.h>

TIM_HandleTypeDef* TIMER;

void init_fail_safe_timer(TIM_HandleTypeDef* timer) {
    SW_ASSERT(timer);
    TIMER = timer;
}


void stroke_outgoing_watchdog(void) {
    HAL_GPIO_TogglePin(GPIOB, WATCHDOG_GPIO);
}

void spin_on_incoming_watchdog() {
    // Spin waiting for the cycle.  Trip if the fail-safe clock enters failed state
    do {
        SW_ASSERT(p_doFail != FAIL_SAFE_CLOCK_FAILED);
    } while (p_doCycle == 0);
    // Reset the ISR flag
    p_doCycle = 0;
}

void reset_fail_safe_timer(void) {
    TIMER->Instance->CNT = 1; // Reset the fatal-timeout clock when a "good" packet is received
}
