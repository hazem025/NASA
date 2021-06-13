/*
 * sound.h:
 *
 * Sound module for playing alarms and running sound-feedback on button presses and other actions take in the system.
 * This allows for 3 sounds:
 *
 * CONSTANT: alarm sound, steady tone
 * BEEP: a single quick beep
 * TWO_BEEP: two beeps in succession.
 *
 *  Created on: Apr 10, 2020
 *      Author: mstarch
 */

#ifndef INC_VENTILATOR_SOUND_H_
#define INC_VENTILATOR_SOUND_H_
#include <stm32f0xx_hal.h>
#include <stdint.h>
#include <ventilator/constants.h>

/**
 * SoundState:
 * Current playback state of the sound module. Designed that the states (except constant) can walk-back
 * to off using state = state - 1. i.e. two beeps walks to delay to beep to off.
 */
typedef enum {
    SOUND_OFF = 0,  // NO sound
    SOUND_BEEP = 1, // Single beep
    SOUND_DELAY_BEEP = 2,// Delay between beeps
    SOUND_TWO_BEEP = 3,  // Two beeps
    SOUND_CONSTANT = 4,  // Constant sound
    MAX_SOUND_STATE = 5  //Bounds-checking constant
} SoundState;
/**
 * Sound:
 *
 * Timing and state information for sound playback.
 */
typedef struct {
    uint32_t current_count;   // Count to time beeps
    uint32_t stop_count;      // Count to stop at for delays and beeps
    SoundState state;         // Current playback state
    TIM_HandleTypeDef* timer; // PWM timer to use for playback.
} Sound;

/**
 * sound_init:
 *
 * Sound initialization code. Send it a timer to run PWM and it will be happy.
 * TIM_HandleTypeDef* timer: timer to run start and stop for PWM code.
 */
void sound_init(TIM_HandleTypeDef* timer);

/**
 * sound_is_alarming:
 *
 * Returns if the sound module is returning an alarm.
 * \return: true if alarming, false otherwise
 */
bool sound_is_alarming();

/**
 * sound_start:
 *
 * Starts a sound output with the corresponding type. Updates the sound module to play the sound
 * SoundState type - type of sound to start. SOUND_BEEP: single beep. SOUND_TWO_BEEP: two beeps.
 *                   SOUND_CONSTANT: continuously playing alarm
 * return: HAL_OK on success, error otherwise
 */
HAL_StatusTypeDef sound_start(SoundState type);

/**
 * sound_stop:
 *
 * Stop the output playing sound and set the state to SOUND_OFF.
 * return: HAL_OK on success, error otherwise
 */
HAL_StatusTypeDef sound_stop();

/**
 * sound_cycle:
 *
 * Must be called every cycle to ensure that the state is being tracked. For precise timing, do NOT call during the same cycle as a previous start.
 * return: HAL_OK on success, error otherwise
 */
HAL_StatusTypeDef sound_cycle();

// ******** Internal Implementation ********

/**
 * sound_pwm_start:
 *
 * Start the PWM driver for the sound.
 * TIM_HandleTypeDef* timer: timer to run PWM sound.
 * return: HAL_OK on success, error otherwise
 */
HAL_StatusTypeDef sound_pwm_start(TIM_HandleTypeDef* timer);

/**
 * sound_pwm_stop:
 *
 * Stop the PWM driver for the sound.
 * TIM_HandleTypeDef* timer: timer to run PWM sound.
 * return: HAL_OK on success, error otherwise
 */

HAL_StatusTypeDef sound_pwm_stop(TIM_HandleTypeDef* timer);

#endif /* INC_VENTILATOR_SOUND_H_ */
