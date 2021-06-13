/*
 * ventilator/cycle.h:
 *
 * File containing the API for calling the code cycler. This will contain the basic cycling code. This
 * code contains the main program loop. Basic cycle is follows:
 *
 * 1. Wait for watchdog to start cycle
 * 2. Communicate with controller
 * 3. Read buttons.
 * 4. Update display
 * 5. Block waiting for #1
 *
 * @author mstarch
 */

#ifndef SRC_VENTILATOR_CYCLE_H_
#define SRC_VENTILATOR_CYCLE_H_
/**
 * Runs a single cycle of the code. This is the "main" rate-driven loop's single iteration.
 */
void cycle(void);


#endif /* SRC_VENTILATOR_CYCLE_H_ */
