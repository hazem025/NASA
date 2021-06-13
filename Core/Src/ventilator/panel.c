/*
 * panel.c
 *
 *  Created on: Apr 9, 2020
 *      Author: tcanham
 */

#include "stm32f0xx_hal.h"
#include <ventilator/panel_public.h>
#include <ventilator/panel.h>
#include <ventilator/constants.h>
#include <ventilator/initialize.h>
#include <swassert.h>
#include <string.h>
#include <ventilator/watchdog.h>
#include <ventilator/eeprom.h>

const bool LOAD_FROM_EEPROM = true; // Set to 0 to use compile-time values and rewrite EEPROM to the defaults

int32_t panel_load_eeprom_value_or_default(EepromRecordId record, int32_t default_value) {
    int32_t eeprom = 0xFFFFFFFF;
    // On a compiled value of 0, we will write out to the EEPROM the default values. This *must* pass.
    if (!LOAD_FROM_EEPROM) {
        SW_ASSERT(writeEeprom(record, default_value) == EEPROM_OK);
        HAL_Delay(10);
    }
    EepromStatus status = readEeprom(record, (uint32_t*)(&eeprom));
    // On success, load the value, otherwise keep the previous value
    if (status == EEPROM_OK) {
        SW_ASSERT(LOAD_FROM_EEPROM || default_value == eeprom); //Check that the loading code reads back correctly
        return eeprom;
    }
    return default_value;
}


void panel_init(void) {
    // Global value initialization
    p_doCycle = 0;
    p_doPlateau = 0;
    p_powerState = POWER_OFF_STATE;
    p_haltVentilation = 0;
    // Initialize global numeric state
    initialize_numeric_values(&p_numericalValues);

    // Clear the panel packet first and then initialize the defaulted parameters
    (void) memset(&p_panel_packet, 0, sizeof(panel_packet_t));
    p_panel_packet.parameters.inhale_sensitivity = panel_load_eeprom_value_or_default(EEPROM_INIT_INHALE_SENSITIVITY, INITIAL_DEFAULT_SENSITIVITY);
    p_panel_packet.parameters.breath_detect_hold_off_time = panel_load_eeprom_value_or_default(EEPROM_INIT_BREATH_DETECT_HOLD_OFF, INITIAL_BREATH_DETECT_HOLD_OFF);
    p_panel_packet.parameters.plateau_sample_offset_time = panel_load_eeprom_value_or_default(EEPROM_INIT_PLATEAU_SAMPLE_OFFSET_TIME, INITIAL_PLATEAU_SAMPLE_OFFSET);

    // Initial pctrl parameters
    p_panel_packet.parameters.pctrl_kp = panel_load_eeprom_value_or_default(EEPROM_INIT_PCTRL_KP, 30591); //uV/sqrt(Pa)
    p_panel_packet.parameters.pctrl_ki = panel_load_eeprom_value_or_default(EEPROM_INIT_PCTRL_KI, 84127); //uV/sqrt(Pa)-s
    p_panel_packet.parameters.pctrl_kd = panel_load_eeprom_value_or_default(EEPROM_INIT_PCTRL_KD, 1835);  //uV/sqrt(Pa)/s
    p_panel_packet.parameters.pctrl_d_filt_cutoff = panel_load_eeprom_value_or_default(EEPROM_INIT_PCTRL_D_FILT_CUTOFF, 3183); //mHz
    p_panel_packet.parameters.pctrl_shape_filt_cutoff = panel_load_eeprom_value_or_default(EEPROM_INIT_PCTRL_SHAPE_FILT_CUTOFF, 1000); //mHz
    p_panel_packet.parameters.pctrl_int_l_limit = panel_load_eeprom_value_or_default(EEPROM_INIT_PCTRL_INT_L_LIMIT, -59); //sqrt(Pa)-s
    p_panel_packet.parameters.pctrl_int_u_limit = panel_load_eeprom_value_or_default(EEPROM_INIT_PCTRL_INT_U_LIMIT,  59); //sqrt(Pa)-s
    p_panel_packet.parameters.pctrl_delay = panel_load_eeprom_value_or_default(EEPROM_INIT_PCTRL_DELAY, 0);     //steps
    p_panel_packet.parameters.pctrl_sin_amp = panel_load_eeprom_value_or_default(EEPROM_INIT_PCTRL_SIN_AMP, 0); //mV
    p_panel_packet.parameters.pctrl_sin_f = panel_load_eeprom_value_or_default(EEPROM_INIT_PCTRL_SIN_F, 0);     //mHz

    p_aliveMinutes = panel_load_eeprom_value_or_default(EEPROM_ALIVE_MINUTES, 0);

    // Initialize the sound module
    sound_init(&htim1);
    // Initialize display and then set the blank pin
    display_init();
    display_blank();
    // Initialize the button state
    init_button_state();
    init_fail_safe_timer(&htim6);
}
