/*
 * watchdog.h:
 *
 * Watchdog and timer control functions. This handles the necessary hardware signals to ensure "aliveness". There
 * are three signals:
 *
 * 1. outgoing watchdog, toggle every cycle, or be determined as dead.
 * 2. fail-safe timer: a timer that will detect a failure to get communication and fault the system. Reset count
 * each cycle.
 * 3. incoming watchdog: control signal. Spin until toggled to time "cycle start"
 *
 *  Created on: Apr 14, 2020
 *      Author: tcanham
 */

#ifndef INC_VENTILATOR_WATCHDOG_H_
#define INC_VENTILATOR_WATCHDOG_H_

/**
 * Initialize the fail-safe timer using supplied pointer
 * TIM_HandleTypeDef* timer: pointer to timer to initialize
 */
void init_fail_safe_timer(TIM_HandleTypeDef* timer);

/**
 * Stroke outgoing watchdog.
 */
void stroke_outgoing_watchdog(void);

/**
 * Spins on incoming watch dog. Will assert on a timeout of fail-safe timer. When incoming watchdog toggles, stop
 * spinning and release cycle.
 */
void spin_on_incoming_watchdog(void);
/**
 * Resets fail-safe timer. Call when comm is detected to reset the timer
 */
void reset_fail_safe_timer(void);

#endif /* INC_VENTILATOR_WATCHDOG_H_ */
