/**
 * Controller update.
 */
#include <swassert.h>
#include <ventilator/panel_public.h>
#include <string.h>
#include <ventilator/constants.h>
#include <ventilator/types.h>


int32_t pascal_to_cmh2O(int32_t pascal) {
    return pascal/98; //Exact conversion "/ 98.0665" has less error then one digit across range
}

int32_t cmh2O_to_pascal(int32_t cmh2o) {
    return cmh2o * 98; //Exact conversion "* 98.0665" has less error then 1 digit across range
}

int32_t bpm_to_ms_period(int32_t bpm) {
    //Integer rounding doesn't work in reciprocals, so do floats, and round at the end.
    float bpmf = bpm;
    bpmf = (60.0f * 1000.0f)/bpmf;
    return (int32_t)(bpmf + 0.5f);
}

void prepare_panel_packet(panel_packet_t* packet, NumericalValues* values, PowerState power_state, uint8_t plateau_count, uint8_t halt_vent) {
    SW_ASSERT(packet);
    SW_ASSERT(values);

    // Note: do *NOT* clear the panel packet, it contains init-at-startup values
    // Unconverted values sent raw down to the controller
    packet->parameters.tidal_volume = values->tidal_volume.setpoint;

    // Converted values that must be remapped into the controller's unit space before being sent down
    packet->parameters.inspiration_time = values->ins_time.setpoint * 100;  // Deciseconds to milliseconds
    packet->parameters.breath_period =(uint16_t)bpm_to_ms_period(values->backup_rate.setpoint);
    packet->parameters.pip_pressure =  (int32_t) cmh2O_to_pascal(values->peak_pressure.setpoint);
    packet->parameters.peep_pressure = (int32_t) cmh2O_to_pascal(values->PEEP.setpoint);

    // Control flags used to signal state and communicate faults
    packet->do_poweroff = (POWER_OFF_STATE == power_state);
    packet->do_plateau = plateau_count > 0;
    packet->stop_ventilation = halt_vent;
    packet->software_assert = SW_ASSERT_FLAG;
    packet->machine_fault  = (values->alarms.machine_fault.status == ALARM_LATCH) ||
                             (values->alarms.machine_fault.status == ALARM_SET);
    // Pre-initialized values read from the initialization code or from the EEPROM
    // Note: this is done on initialization, not needed here
}

void process_control_packet(controller_packet_t* packet, NumericalValues* values) {
    SW_ASSERT(packet);
    SW_ASSERT(values);

    sensor_data_t sensors = packet->sensors;
    // Raw sensor values
    values->alarms.machine_fault.status = (packet->error_field != 0) ? ALARM_SET : ALARM_OFF;
    values->tidal_volume.val = sensors.tidal_volume;
    values->minute_volume.val = sensors.minute_volume;
    values->tidal_volume_last.val = sensors.last_breath_tidal_volume;

    // Remap converted values into our unit space
    values->FIO2.val = sensors.fio2/1000;  //1000ths of percent to percent
    values->peak_pressure.val = pascal_to_cmh2O(sensors.pressure_last_breath_max);
    values->pressure_min.val  = pascal_to_cmh2O(sensors.pressure_last_breath_min);
    values->pressure_mean.val = pascal_to_cmh2O(sensors.pressure_last_breath_mean);
    values->pressure_plat.val = pascal_to_cmh2O(sensors.pressure_plateau);
    values->resp_rate.val     = bpm_to_ms_period(sensors.breath_period_average - BREATH_PERIOD_ADJUSTMENT);
    values->pressure.val      = pascal_to_cmh2O(sensors.pressure_patient);
    values->peak_pressure_average.val = pascal_to_cmh2O(sensors.peak_pressure_average);
    values->peep_pressure_average.val = pascal_to_cmh2O(sensors.peep_pressure_average);
}

HAL_StatusTypeDef do_controller_cycle(void) {
    // Prepare for communication with the controller
    (void) memset(&p_controler_packet, 0, sizeof(controller_packet_t));
    prepare_panel_packet(&p_panel_packet, &p_numericalValues, p_powerState, p_doPlateau, p_haltVentilation);
    if (p_doPlateau > 0) {
        p_doPlateau = p_doPlateau - 1;
    }
    // Send packet and receive response
    HAL_StatusTypeDef status = if_txrx_packet(&hspi1, &p_panel_packet, &p_controler_packet, HAL_MAX_DELAY);
    // On good communication process the returned packet
    if (status == HAL_OK) {
        process_control_packet(&p_controler_packet, &p_numericalValues);
    }
    return status;
 }
