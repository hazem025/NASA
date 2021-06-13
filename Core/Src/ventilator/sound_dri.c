/*
 * sound_dri.c
 *
 *  Created on: Apr 10, 2020
 *      Author: mstarch
 */
#include <stm32f0xx_hal.h>
#include <swassert.h>

HAL_StatusTypeDef sound_pwm_start(TIM_HandleTypeDef* tim1)  {
    SW_ASSERT(tim1 != NULL);
    return HAL_TIM_PWM_Start(tim1, TIM_CHANNEL_1);
}

HAL_StatusTypeDef sound_pwm_stop(TIM_HandleTypeDef* tim1) {
    SW_ASSERT(tim1 != NULL);
    return HAL_TIM_PWM_Stop(tim1, TIM_CHANNEL_1);
}
