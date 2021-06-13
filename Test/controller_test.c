#include <stdint.h>
#include <string.h>
#include <test.h>
#include <ventilator/controller.h>


int test_pascal_to_cmh2O() {
    // These conversions need to be accurate from 0 to 100 cmH20, as it is the
    // valid range of the display.  We'll step by 10ths of a Pascal
    for (int i = 99; i < 10000; i++) {
        float correct = ((float)i)/98.0665f;
        int32_t returned = pascal_to_cmh2O(i);
        int32_t difference = returned - (int32_t) correct;

        // Ensure within 1 cmh20
        TEST_ASSERT(difference <= 1 && difference >= -1, "Pascal conversion failed.");
    }
    return 0;
}

int test_cmh2O_to_pascal() {
    // These conversions need to be accurate from 0 to 100 cmH20, as it is the
    // valid range of the display.  We'll step by 1cmH20 of a Pascal
    for (int i = 0; i < 100; i++) {
        float correct = ((float)i) * 98.0665f;
        int32_t returned = cmh2O_to_pascal(i);
        int32_t difference = (int32_t)correct - returned;

        // Make sure the difference between the *real* calculation and the efficient
        // one is less than 1 CMH20 in the result
        TEST_ASSERT(difference < 98  && difference > -98, "CMH20 conversion failed.");
    }
    return 0;
}

int test_bpm_to_ms_period() {
    // These conversions need to be accurate from 0 to 100 BPM, as it is the
    // valid range of the display. We'll step by ms period between the two bounds.

    // Normal range checks ms-bpm
    for (int i = 605; i < 60001; i++) {
        int bpm = 60000/i;
        int32_t returned = bpm_to_ms_period(i);
        int32_t difference = returned - bpm;

        TEST_ASSERT(difference <= 1 && difference >= -1, "MS to BPM Test failed");
    }
    // Reciprocal calculations must pass or assumptions made when coding no longer hold
    for (int i = 0; i < 100; i++) {
        TEST_ASSERT(bpm_to_ms_period(bpm_to_ms_period(i)) == i, "Reciprocal test failed");
    }
    return 0;
}

int test_panel_packet_helper(PowerState power, AlarmStatus machine_fault, uint8_t plateau_count, int sw_assert, int halt) {
    NumericalValues values;
    memset(&values, 0, sizeof(values));
    values.PEEP.setpoint = 20;
    values.peak_pressure.setpoint = 50;
    values.backup_rate.setpoint = 3;
    values.ins_time.setpoint = 31;
    values.tidal_volume.setpoint = 341;
    values.alarms.machine_fault.status = machine_fault;


    panel_packet_t expected;
    panel_packet_t tested;
    // Set memory to 1's so that we know it doesn't clear values initialized 1 time
    memset(&expected, 1, sizeof(expected));
    memset(&tested, 1, sizeof(tested));

    // Set expected values
    expected.parameters.peep_pressure = values.PEEP.setpoint * 98; // To pascal
    expected.parameters.pip_pressure = values.peak_pressure.setpoint * 98; // To pascal
    expected.parameters.breath_period = 20000; // 60/3 * 1000
    expected.parameters.inspiration_time = values.ins_time.setpoint * 100; // In deciseconds
    expected.parameters.tidal_volume = values.tidal_volume.setpoint;
    expected.do_poweroff = power != POWER_ON_STATE;
    expected.do_plateau = plateau_count != 0;
    expected.machine_fault = machine_fault != ALARM_OFF;
    expected.software_assert = sw_assert != 0;
    expected.stop_ventilation = halt != 0;
    SW_ASSERT_FLAG = sw_assert;

    prepare_panel_packet(&tested, &values, power, plateau_count, halt);
    SW_ASSERT_FLAG = 0; // To prevent crashing on TEST_ASSERT
    for (int i = 0; i < sizeof(expected); i++) {
        uint8_t expt_byte = ((uint8_t*)&expected)[i];
        uint8_t test_byte = ((uint8_t*)&tested)[i];
        TEST_ASSERT(expt_byte == test_byte, "Byte-per-byte check failed");
    }
    return 0;
}

int test_prepare_panel_packet() {
    int status = 0;
    for (int plat = 0; plat < 256; plat++) {
        for (int halt = 0; halt < 2; halt++) {
            for (int swas = 0; swas < 2; swas++) {
                status |= test_panel_packet_helper(POWER_ON_STATE,  ALARM_OFF, plat, swas, halt);
                status |= test_panel_packet_helper(POWER_OFF_STATE, ALARM_OFF, plat, swas, halt);

                status |= test_panel_packet_helper(POWER_ON_STATE,  ALARM_SET, plat, swas, halt);
                status |= test_panel_packet_helper(POWER_OFF_STATE, ALARM_SET, plat, swas, halt);

                status |= test_panel_packet_helper(POWER_ON_STATE,  ALARM_LATCH, plat, swas, halt);
                status |= test_panel_packet_helper(POWER_OFF_STATE, ALARM_LATCH, plat, swas, halt);
            }
        }
    }
    return status;
}

int test_process_control_packet() {
    //ERROR fields

    controller_packet_t packet;
    memset(&packet, 0, sizeof(packet));
    packet.sensors.tidal_volume = 123.0f;
    packet.sensors.minute_volume = 456.1f;
    packet.sensors.last_breath_tidal_volume = 789.3f;
    packet.sensors.fio2 = 9900;

    packet.sensors.pressure_last_breath_max = 4321;
    packet.sensors.pressure_last_breath_min = 8765;
    packet.sensors.pressure_last_breath_mean = 2109;
    packet.sensors.pressure_patient = 5674;
    packet.sensors.peak_pressure_average = 9922;
    packet.sensors.peep_pressure_average = 31411;
    packet.sensors.breath_period_average = 32675;
    packet.sensors.pressure_plateau = 11223;
    packet.error_field = 0;


    NumericalValues expected;
    NumericalValues tested;

    memset(&expected, 0, sizeof(expected));
    memset(&tested, 0, sizeof(tested));

    expected.tidal_volume.val = packet.sensors.tidal_volume;
    expected.minute_volume.val = packet.sensors.minute_volume;
    expected.tidal_volume_last.val = packet.sensors.last_breath_tidal_volume;
    expected.FIO2.val = packet.sensors.fio2/1000;  // 1000ths of % to %

    expected.peak_pressure.val = packet.sensors.pressure_last_breath_max/98;
    expected.pressure_min.val = packet.sensors.pressure_last_breath_min/98;
    expected.pressure_mean.val = packet.sensors.pressure_last_breath_mean/98;
    expected.pressure_plat.val = packet.sensors.pressure_plateau/98;
    expected.resp_rate.val = 2; // 60000/(32675 - 10) rounded up
    expected.pressure.val = packet.sensors.pressure_patient/98;
    expected.peak_pressure_average.val = packet.sensors.peak_pressure_average/98;
    expected.peep_pressure_average.val = packet.sensors.peep_pressure_average/98;


    expected.alarms.machine_fault.status = ALARM_OFF;


    process_control_packet(&packet, &tested);

    for (int i = 0; i < sizeof(expected); i++) {
        uint8_t expt_byte = ((uint8_t*)&expected)[i];
        uint8_t test_byte = ((uint8_t*)&tested)[i];
        if (expt_byte != test_byte) {
            printf("I: %d Ex: %x Ts: %x\n", i, expt_byte, test_byte);
        }
        TEST_ASSERT(expt_byte == test_byte, "Byte-per-byte check failed");
    }
    return 0;
}

int main(int argc, char** argv) {
    TEST(test_pascal_to_cmh2O);
    TEST(test_cmh2O_to_pascal);
    TEST(test_bpm_to_ms_period);
    TEST(test_prepare_panel_packet);
    TEST(test_process_control_packet);
}
