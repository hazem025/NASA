/*
 * sound.c
 *
 *  Created on: Apr 10, 2020
 *      Author: mstarch
 */
#include <ventilator/types.h>
#include <ventilator/sound.h>
#include <swassert.h>

STATIC uint8_t SOUND_INIT = 0;
STATIC Sound SOUND;

void sound_init(TIM_HandleTypeDef* timer) {
    SW_ASSERT(timer);
    SOUND.timer = timer;
    SOUND_INIT = 1;
}

bool sound_is_alarming() {
    return (SOUND.state == SOUND_CONSTANT);
}

HAL_StatusTypeDef sound_start(SoundState type) {
    SW_ASSERT(SOUND_INIT);
    SW_ASSERT1(type >= 0 && type <= MAX_SOUND_STATE, type);
    HAL_StatusTypeDef status = HAL_OK;
    // SOUND_OFF is equivalent to sound stopping
    if (type == SOUND_OFF) {
        status = sound_stop();
    }
    // Otherwise start the playback if  we are one state away, high-priority constant sound, or currently off
    else if ((SOUND.state == SOUND_OFF) || (SOUND.state == (type + 1)) || (type == SOUND_CONSTANT)) {
        SOUND.stop_count = SOUND_BEEP_DURATION_CYCLES;
        SOUND.current_count = 0;
        SOUND.state = type;
        // Delay beep is like sound, but turns off the PWM
        if (SOUND.state == SOUND_DELAY_BEEP) {
            status = sound_pwm_stop((TIM_HandleTypeDef*)SOUND.timer);
        } else {
            status = sound_pwm_start((TIM_HandleTypeDef*)SOUND.timer);
        }
    }
    return status;
}

HAL_StatusTypeDef sound_stop() {
    SW_ASSERT(SOUND_INIT);
    SOUND.stop_count = 0;
    SOUND.state = SOUND_OFF;
    return sound_pwm_stop((TIM_HandleTypeDef*)SOUND.timer);
}

HAL_StatusTypeDef sound_cycle() {
    SW_ASSERT(SOUND_INIT);
    SW_ASSERT1(SOUND.state >= 0 && SOUND.state <= MAX_SOUND_STATE, SOUND.state);
    // No updates on constant or off sounds
    if (SOUND.state == SOUND_CONSTANT || SOUND.state == SOUND_OFF) {
        return HAL_OK;
    }
    SOUND.current_count++;
    // When greater than stop count, we need to do something else
    if (SOUND.current_count >= SOUND.stop_count) {
        return sound_start(SOUND.state - 1); // Start the next sound down the ladder
    }
    return HAL_OK;
}
