#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <ventilator/display.h>
#include <ventilator/types.h>
#include <ventilator/panel_public.h>
#include <ventilator/controller.h>
#include <main.h>

uint8_t SW_ASSERT_FLAG = 0;

void generic_assert_helper(void) {
    // If the assert is already set, then bail to prevent reentrant or infinitely recursing assert failures.
    if (SW_ASSERT_FLAG) {
        return;
    }
    // When an assert arises, we attempt to communicate to the controller that we have asserted.
    while (1) {
        // Assert happens
        SW_ASSERT_FLAG = 1;
        HAL_GPIO_WritePin(GPIOB, MTR_SHTDN_Pin, GPIO_PIN_SET);  //Shutdown motor
        // Hail Mary machine fault to LED
        display_machine_fault();
        // Hail Mary send values to controller, don't react to returned values, just continue to inform controller
        p_numericalValues.alarms.machine_fault.status = ALARM_LATCH;
        (void) do_controller_cycle();
    }
    assert(0);
}

void sw_hard_fault(const char* file, int line) {
    // If the assert is already set, then bail to prevent reentrant or infinitely recursing assert failures.
    if (SW_ASSERT_FLAG) {
        return;
    }
    // When a hard fault arises, we just attempt to display the machine fault LED. We *do not* attempt communication
    // as that communication could be erroneous.
    while (1) {
        // Assert happens
        SW_ASSERT_FLAG = 1;
        HAL_GPIO_WritePin(GPIOB, MTR_SHTDN_Pin, GPIO_PIN_SET);  //Shutdown motor
        // Hail Mary machine fault to LED
        display_machine_fault();
    }
}

void sw_assert(const char* file, int line) {
    if (file != 0 && line > 0) {
        (void) printf("ASSERT: %s:%d\n", file, line);
    }
    generic_assert_helper();
}

void sw_assert1(const char* file, int line, int arg1) {
    if (file != 0 && line > 0) {
        (void) printf("ASSERT: %s:%d with argument %d\n", file, line, arg1);
    }
    generic_assert_helper();
}

void sw_assert2(const char* file, int line, int arg1, int arg2) {
    if (file != 0 && line > 0) {
        (void) printf("ASSERT: %s:%d with arguments %d, %d\n", file, line, arg1, arg2);
    }
    generic_assert_helper();
}
