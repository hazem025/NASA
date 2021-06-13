/*
 * panel_public.h:
 *
 * Public (global) initializations for the panel code. This contains helpful definitions of pins,
 * non-external definitions of global state and more.
 *
 *  Created on: Apr 9, 2020
 *      Author: tcanham
 */

#ifndef INC_VENTILATOR_PANEL_PUBLIC_H_
#define INC_VENTILATOR_PANEL_PUBLIC_H_

#include "stm32f0xx_hal.h"
#include <ventilator/types.h>
#include <ventilator/display.h>
#include <ventilator/button.h>
#include <ventilator/mcp23017.h>
#include <ventilator/panel.h>
#include <ventilator/display.h>
#include <ventilator/sound.h>
#include <if_controller.h>
// Borrowed from main.h, redefinitions such that main.h need not be included everywhere
#define SD_LATCH_Pin GPIO_PIN_12
#define DISP_BLNK_Pin GPIO_PIN_14
#define LOW_BATTERY_Pin GPIO_PIN_10

// When defined run the hardware test code to check hardware status.  This is special hardware test firmware
//#define TEST_MODE

// Forces anything using EXTERN to be declared as "extern" -- except when included as follows:
//
// #define EXTERN
// #include <ventilator/panel_public.h>
// #undef EXTERN
//
// Which will automatically declare the variables in this case
#ifndef EXTERN
#define EXTERN extern
#endif

// STM GUI device handles defined by the auto-generated code
extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;
extern I2C_HandleTypeDef hi2c1;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim6;
extern UART_HandleTypeDef huart1;

// Software defined handles used for communication with controller
EXTERN panel_packet_t p_panel_packet;
EXTERN controller_packet_t p_controler_packet;

// Global system state used to determine global-driving state functions
EXTERN NumericalValues p_numericalValues; // Numerical state storing readings, edit values, and set point
EXTERN PowerState p_powerState; // Is the system ON or OFF
EXTERN uint8_t p_doPlateau; // Does a plateau measurement need to be taken
EXTERN uint8_t p_haltVentilation; // Halts the ventilation when peak pressure alarm violated
EXTERN uint32_t p_aliveMinutes; // Alive time in minutes, read and updated from eeprom
EXTERN UartDebug p_uartDebug; // Extra values to be sent to the debug UART

// ISR set variables used to indicate when ISR functions occur
EXTERN volatile uint32_t p_doCycle; // Set when the ISR detects a watchdog has been toggled and the system should cycle
extern volatile FailSafeClockState p_doFail; // Detects when the no-comm watchdog timer trips and the system should fail

#endif /* INC_VENTILATOR_PANEL_PUBLIC_H_ */
