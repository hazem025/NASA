#include <swassert.h>
#include <ventilator/types.h>
#include <ventilator/initialize.h>
#include <string.h>

void initialize_numeric_values(NumericalValues* values) {
    SW_ASSERT(values);
    (void) memset(values, 0, sizeof(NumericalValues));
    values->FIO2.editval = 0;
    values->FIO2.setpoint = 21; //Atmospheric O2
    values->FIO2.thresh_lower = INITIAL_FIO2_THRESHOLD_LOWER;
    values->FIO2.thresh_upper = INITIAL_FIO2_THRESHOLD_UPPER;

    values->PEEP.thresh_upper = INITIAL_PEEP_THRESHOLD_UPPER;
    values->PEEP.thresh_lower = INITIAL_PEEP_THRESHOLD_LOWER;
    values->PEEP.upper = INITIAL_PEEP_UPPER;
    values->PEEP.lower = INITIAL_PEEP_LOWER;
    values->PEEP.step = INITIAL_PEEP_STEP;
    values->PEEP.setpoint = INITIAL_PEEP_INIT;
    values->PEEP.editval = INITIAL_PEEP_INIT;
    values->PEEP.mode = DISPLAY_SETPOINT;

    values->tidal_volume.thresh_upper = INITIAL_TIDAL_THRESHOLD_UPPER;
    values->tidal_volume.thresh_lower = INITIAL_TIDAL_THRESHOLD_LOWER;
    values->tidal_volume.upper = INITIAL_TIDAL_UPPER;
    values->tidal_volume.lower = INITIAL_TIDAL_LOWER;
    values->tidal_volume.step = INITIAL_TIDAL_STEP;
    values->tidal_volume.setpoint = INITIAL_TIDAL_INIT;
    values->tidal_volume.editval = INITIAL_TIDAL_INIT;
    values->tidal_volume.mode = DISPLAY_SETPOINT;

    values->backup_rate.thresh_upper = INITIAL_BACKUP_THRESHOLD_UPPER;
    values->backup_rate.thresh_lower = INITIAL_BACKUP_THRESHOLD_LOWER;
    values->backup_rate.upper = INITIAL_BACKUP_UPPER;
    values->backup_rate.lower = INITIAL_BACKUP_LOWER;
    values->backup_rate.step = INITIAL_BACKUP_STEP;
    values->backup_rate.setpoint = INITIAL_BACKUP_INIT;
    values->backup_rate.editval = INITIAL_BACKUP_INIT;
    values->backup_rate.mode = DISPLAY_SETPOINT;

    values->peak_pressure.thresh_upper = INITIAL_PEAK_THRESHOLD_UPPER;
    values->peak_pressure.thresh_lower = INITIAL_PEAK_THRESHOLD_LOWER;
    values->peak_pressure.upper = INITIAL_PEAK_UPPER; // Dependent on peep
    values->peak_pressure.lower = INITIAL_PEAK_LOWER;  // Dependent on peep
    values->peak_pressure.step = INITIAL_PEAK_STEP;
    values->peak_pressure.setpoint = INITIAL_PEAK_INIT;
    values->peak_pressure.editval = INITIAL_PEAK_INIT;
    values->peak_pressure.mode = DISPLAY_SETPOINT;

    values->ins_time.thresh_upper = INITIAL_ITIME_THRESHOLD_UPPER;
    values->ins_time.thresh_lower = INITIAL_ITIME_THRESHOLD_LOWER;
    values->ins_time.upper = INITIAL_ITIME_UPPER; // Deciseconds
    values->ins_time.lower = INITIAL_ITIME_LOWER;
    values->ins_time.step = INITIAL_ITIME_STEP;
    values->ins_time.setpoint = INITIAL_ITIME_INIT;
    values->ins_time.editval = INITIAL_ITIME_INIT;
    values->ins_time.mode = DISPLAY_SETPOINT;

    values->resp_rate.thresh_upper = INITIAL_RESPR_THRESHOLD_UPPER;
    values->resp_rate.thresh_lower = INITIAL_RESPR_THRESHOLD_LOWER;
    values->resp_rate.upper = INITIAL_RESPR_UPPER;
    values->resp_rate.lower = INITIAL_RESPR_LOWER;
    values->resp_rate.step = INITIAL_RESPR_STEP;
    values->resp_rate.setpoint = INITIAL_RESPR_INIT;
    values->resp_rate.editval = INITIAL_RESPR_INIT;
    values->resp_rate.mode = DISPLAY_VALUE;

    values->alarms.disconnect.status = ALARM_OFF;
    values->alarms.disconnect.count = 0;
    values->alarms.disconnect.latch_time = INITIAL_DISCONNECT_LATCH;
    values->alarms.disconnect.trip_time = INITIAL_DISCONNECT_TRIP;

    values->alarms.fio2.status = ALARM_OFF;
    values->alarms.fio2.count = 0;
    values->alarms.fio2.latch_time = INITIAL_FIO2_LATCH;
    values->alarms.fio2.trip_time = INITIAL_FIO2_TRIP;

    values->alarms.low_power.status = ALARM_OFF;
    values->alarms.low_power.count = 0;
    values->alarms.low_power.latch_time = INITIAL_LOWPOW_LATCH;
    values->alarms.low_power.trip_time = INITIAL_LOWPOW_TRIP;

    values->alarms.machine_fault.status = ALARM_OFF;
    values->alarms.machine_fault.count = 0;
    values->alarms.machine_fault.latch_time = INITIAL_MACHINEFAULT_LATCH;
    values->alarms.machine_fault.trip_time = INITIAL_MACHINEFAULT_TRIP;

    values->alarms.peak_press.status = ALARM_OFF;
    values->alarms.peak_press.count = 0;
    values->alarms.peak_press.latch_time = INITIAL_PEAK_LATCH;
    values->alarms.peak_press.trip_time = INITIAL_PEAK_TRIP;

    values->alarms.peep.status = ALARM_OFF;
    values->alarms.peep.count = 0;
    values->alarms.peep.latch_time = INITIAL_PEEP_LATCH;
    values->alarms.peep.trip_time = INITIAL_PEEP_TRIP;

    values->alarms.power_off.status = ALARM_OFF;
    values->alarms.power_off.count = 0;
    values->alarms.power_off.latch_time = INITIAL_POWEROFF_LATCH;
    values->alarms.power_off.trip_time = INITIAL_POWEROFF_TRIP;

    values->alarms.resp_rate.status = ALARM_OFF;
    values->alarms.resp_rate.count = 0;
    values->alarms.resp_rate.latch_time = INITIAL_RESPRATE_LATCH;
    values->alarms.resp_rate.trip_time = INITIAL_RESPRATE_TRIP;

    values->alarms.tidal_vol.status = ALARM_OFF;
    values->alarms.tidal_vol.count = 0;
    values->alarms.tidal_vol.latch_time = INITIAL_TIDALVOL_LATCH;
    values->alarms.tidal_vol.trip_time = INITIAL_TIDALVOL_TRIP;
}
