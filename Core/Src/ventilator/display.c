/*
 * display.c
 *
 *  Created on: Apr 10, 2020
 *      Author: mstarch
 */
#include <ventilator/display.h>
#include <ventilator/numerical.h>
#include <ventilator/bargraph.h>
#include <ventilator/mcp23017.h>
#include <ventilator/alarm.h>
#include <ventilator/panel_public.h>
#include <ventilator/types.h>
#include <swassert.h>

#include "stm32f0xx_hal.h"
#include <stdint.h>
#include <string.h>

STATIC uint32_t blink_cycle_count = 0;
STATIC Display m_display;

void display_init(void) {
    (void) memset(&m_display, 0, sizeof(Display));
    // Initialize each display output I2C
    SW_ASSERT(mcp23017_init(&m_display.mcp_lower, 0x20, 0) == HAL_OK);
    SW_ASSERT(mcp23017_init(&m_display.mcp_middle, 0x21, 0) == HAL_OK);
    SW_ASSERT(mcp23017_init(&m_display.mcp_upper, 0x22, 0) == HAL_OK);


    // SPI handle is for most of the display
    m_display.spi = &hspi2;
    // Both the latch and blank pins use GPIOB
    m_display.gpio_port = GPIOB;
    m_display.latch = SD_LATCH_Pin;
    m_display.blank = DISP_BLNK_Pin;
}

void display_machine_fault(void) {
    // Hail Mary machine fault to LED
    display_init();
    m_display.alarm  = 1 << DISPLAY_ALARM_MACH_FALT_SHIFT;
    (void) display_raw_send(); // Machine fault ignores failures
}

uint32_t display_get_value_helper(NumericalValue value) {
    if ((blink_cycle_count < DISPLAY_BLINK_OFF_CYCLES) &&
        ((value.mode == DISPLAY_EDIT_ALARM) ||
         (value.mode == DISPLAY_EDIT_SETPOINT) ||
         (value.mode == DISPLAY_EDIT_WITH_VALUE)
        )) {
        return BLANK_CONSTANT;
    } else if (value.mode == DISPLAY_SETPOINT) {
        return value.setpoint;
    } else if ((value.mode == DISPLAY_VALUE) || (value.mode == DISPLAY_EDIT_WITH_VALUE)) {
        return value.val;
    }
    return value.editval;
}

void display_fill_output_helper(NumericalValues* values) {
    SW_ASSERT(values != NULL);
    SW_ASSERT(m_display.spi); // Check display has been initialized
    // Assign all numerical displays
    numerical_set_two_digit(&m_display.minute_volume, display_get_value_helper(values->minute_volume));
    numerical_set_two_digit(&m_display.resp_rate, display_get_value_helper(values->resp_rate));
    numerical_set_two_digit(&m_display.ins_time, display_get_value_helper(values->ins_time));
    numerical_set_two_digit(&m_display.peak_pressure, display_get_value_helper(values->peak_pressure));
    numerical_set_two_digit(&m_display.backup_rate, display_get_value_helper(values->backup_rate));
    numerical_set_three_digit(&m_display.tidal_volume, display_get_value_helper(values->tidal_volume));
    numerical_set_two_digit(&m_display.PEEP, display_get_value_helper(values->PEEP));
    numerical_set_two_digit(&m_display.FIO2, display_get_value_helper(values->FIO2));
    // Inspiration time is always has decimal
    m_display.ins_time = m_display.ins_time | 1 << 12;

    // Assign normal green bargraphs
    uint32_t scaled_tidal = bargraph_scaled_value(values->tidal_volume.val, BARGRAPH_TIDAL_SHIFT, BARGRAPH_TIDAL_HEIGHT);
    uint32_t scaled_pressure = bargraph_scaled_value(values->pressure.val, BARGRAPH_PRESSURE_SHIFT, BARGRAPH_PRESSURE_HEIGHT);

    bargraph_assign_value(&m_display.pressure, scaled_pressure);
    bargraph_assign_value(&m_display.tidal, scaled_tidal);
    // Assign the triple point red-green bargraph points
    uint32_t scaled_pressure_lower = bargraph_scaled_value(values->pressure_min.val, BARGRAPH_PRESSURE_SHIFT, BARGRAPH_PRESSURE_HEIGHT);
    uint32_t scaled_pressure_middle = bargraph_scaled_value(values->pressure_mean.val, BARGRAPH_PRESSURE_SHIFT, BARGRAPH_PRESSURE_HEIGHT);
    uint32_t scaled_pressure_upper = bargraph_scaled_value(values->peak_pressure.val, BARGRAPH_PRESSURE_SHIFT, BARGRAPH_PRESSURE_HEIGHT);
    uint32_t scaled_pressure_plateau = bargraph_scaled_value(values->pressure_plat.val, BARGRAPH_PRESSURE_SHIFT, BARGRAPH_PRESSURE_HEIGHT);
    bargraph_assign_red_green_value(&m_display.red_green_green, &m_display.red_green_red, scaled_pressure_upper, scaled_pressure_middle, scaled_pressure_lower,
                                    scaled_pressure_plateau);
    //display->alarm = (uint16_t)values->alarms;
    m_display.alarm =
            ((values->alarms.disconnect.status != ALARM_OFF && values->alarms.disconnect.status != ALARM_BLINK_OFF) << DISPLAY_ALARM_DISCONNECT_SHIFT) |
            ((values->alarms.tidal_vol.status != ALARM_OFF  && values->alarms.tidal_vol.status != ALARM_BLINK_OFF)  << DISPLAY_ALARM_TIDAL_VOL_SHIFT) |
            ((values->alarms.peak_press.status != ALARM_OFF && values->alarms.peak_press.status != ALARM_BLINK_OFF) << DISPLAY_ALARM_PEAK_PRES_SHIFT) |
            ((values->alarms.resp_rate.status != ALARM_OFF  && values->alarms.resp_rate.status != ALARM_BLINK_OFF)  << DISPLAY_ALARM_RESP_RATE_SHIFT) |
            ((values->alarms.peep.status != ALARM_OFF       && values->alarms.peep.status != ALARM_BLINK_OFF)       << DISPLAY_ALARM_PEEP_PRES_SHIFT) |
            ((values->alarms.fio2.status != ALARM_OFF       && values->alarms.fio2.status != ALARM_BLINK_OFF)       << DISPLAY_ALARM_FIO2_PERC_SHIFT) |
            ((values->alarms.machine_fault.status != ALARM_OFF && values->alarms.machine_fault.status != ALARM_BLINK_OFF) << DISPLAY_ALARM_MACH_FALT_SHIFT) |
            ((values->alarms.low_power.status != ALARM_OFF  && values->alarms.low_power.status != ALARM_BLINK_OFF)  << DISPLAY_ALARM_LOW_POWER_SHIFT) |
            ((values->alarms.power_off.status != ALARM_OFF  && values->alarms.power_off.status != ALARM_BLINK_OFF)  << DISPLAY_ALARM_POWER_OFF_SHIFT);
}

HAL_StatusTypeDef display_raw_send(void) {
    SW_ASSERT(m_display.spi); // Check display has been initialized
    HAL_StatusTypeDef timstat = HAL_OK;
    // Write out the the SPI, and all three I2C devices. If any of these fail, the device could be displaying incorrect or miss-leading values.
    HAL_StatusTypeDef stat0 = HAL_SPI_Transmit(m_display.spi, (uint8_t*)(&m_display), DISPLAY_U16_COUNT, HAL_MAX_DELAY);
    HAL_StatusTypeDef stat1 = mcp23017_write_reg(&m_display.mcp_lower, REG_GPIOA, (uint8_t*)(&m_display.red_green_red.lower), sizeof(uint16_t));
    HAL_StatusTypeDef stat2 = mcp23017_write_reg(&m_display.mcp_middle, REG_GPIOA, (uint8_t*)(&m_display.red_green_red.middle), sizeof(uint16_t));
    HAL_StatusTypeDef stat3 = mcp23017_write_reg(&m_display.mcp_upper, REG_GPIOA, (uint8_t*)(&m_display.red_green_red.upper), sizeof(uint16_t));
    // Start the alarm-bright LED iff any alarm LED is on, otherwise the PWM should be stopped. In this way, the light is only on when
    // 1+ lesser LEDs is illuminated.  Flash it as the inverse of the screen blink (mostly off, short on)
    if (((m_display.alarm & ~(1 << DISPLAY_ALARM_POWER_OFF_SHIFT)) != 0) && (blink_cycle_count < DISPLAY_BLINK_OFF_CYCLES)) {
        timstat = HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    } else {
        timstat = HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
    }
    // Commit display by setting the latch pins on and off to latch the data, and then ensure that the blank pin is low so the
    // data doesn't get stopped at the output gate.
    HAL_GPIO_WritePin(GPIOB, m_display.latch, GPIO_PIN_SET);   //ON LATCH
    HAL_GPIO_WritePin(GPIOB, m_display.latch, GPIO_PIN_RESET);  // OFF LATCH
    HAL_GPIO_WritePin(GPIOB, m_display.blank, GPIO_PIN_RESET); // BLANK OFF
    return (timstat == HAL_OK && stat0 == HAL_OK && stat1 == HAL_OK && stat2 == HAL_OK && stat3 == HAL_OK)? HAL_OK : HAL_ERROR;
}

void display_send_update(NumericalValues *values) {
    SW_ASSERT(values != NULL);
    display_fill_output_helper(values);
    // Cycles the blink count so that the displays blink
    blink_cycle_count = (blink_cycle_count + 1) % DISPLAY_BLINK_CYCLES;
    SW_ASSERT(display_raw_send() == HAL_OK);
}

void run_display(bool force_blank, uint32_t alive_hours) {
    if (force_blank) {
        display_blank();
    } else if (p_powerState == POWER_OFF_STATE) {
        display_standby(BLANK_CONSTANT, BLANK_CONSTANT);
    } else if (p_powerState == POWERING_STATE) {
        uint32_t upper = alive_hours/100;
        uint32_t lower = alive_hours % 100;
        display_standby(upper, lower);
    } else {
        display_send_update(&p_numericalValues);
    }
}

void display_blank(void) {
    HAL_GPIO_WritePin(m_display.gpio_port, m_display.blank, GPIO_PIN_SET);
}

void display_standby(uint32_t upper, uint32_t lower) {
    // Assign all numerical displays to blank
    numerical_set_two_digit(&m_display.minute_volume, lower);
    numerical_set_two_digit(&m_display.resp_rate, upper);
    numerical_set_two_digit(&m_display.ins_time, BLANK_CONSTANT);
    numerical_set_two_digit(&m_display.peak_pressure, BLANK_CONSTANT);
    numerical_set_two_digit(&m_display.backup_rate, BLANK_CONSTANT);
    numerical_set_three_digit(&m_display.tidal_volume, BLANK_CONSTANT);
    numerical_set_two_digit(&m_display.PEEP, BLANK_CONSTANT);
    numerical_set_two_digit(&m_display.FIO2, BLANK_CONSTANT);
    //  Assign the  bargraphs to be zero
    bargraph_assign_value(&m_display.tidal, 0);
    bargraph_assign_value(&m_display.pressure, 0);
    bargraph_assign_value(&m_display.red_green_green, 0);
    bargraph_assign_value(&m_display.red_green_red, 0);
    if (upper == BLANK_CONSTANT && lower == BLANK_CONSTANT) {
        m_display.alarm = 1 << DISPLAY_ALARM_POWER_OFF_SHIFT;
    } else {
        m_display.alarm = 0;
    }
    SW_ASSERT(display_raw_send() == HAL_OK); // A failure to display must assert, as the display could be in a miss-leading state
}
