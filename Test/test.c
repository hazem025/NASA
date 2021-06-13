#include <stdio.h>
#include <stdint.h>
#include <test.h>
uint8_t SW_ASSERT_FLAG = 0;

#define EXTERN
#include <ventilator/panel_public.h>
#undef EXTERN

int GPIO_READ_TEST_VALUE = 1;

// To resolve symbols:

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;
I2C_HandleTypeDef hi2c1;
TIM_HandleTypeDef htim1;



void sw_assert(const char* file, int line) {
    printf("ASSERT: %s:%d\n", file, line);
    SW_ASSERT_FLAG = 1;
}

void sw_assert1(const char* file, int line, int arg1) {
    printf("ASSERT: %s:%d with argument %d\n", file, line, arg1);
    SW_ASSERT_FLAG = 1;
}

void sw_assert2(const char* file, int line, int arg1, int arg2) {
    printf("ASSERT: %s:%d with arguments %d, %d\n", file, line, arg1, arg2);
    SW_ASSERT_FLAG = 1;
}

int sound_running = 0;

HAL_StatusTypeDef sound_pwm_start(TIM_HandleTypeDef* tim1)  {
    sound_running = 1;
    return 0;
}

HAL_StatusTypeDef sound_pwm_stop(TIM_HandleTypeDef* tim1) {
    sound_running = 0;
    return 0;
}

HAL_StatusTypeDef if_txrx_packet(SPI_HandleTypeDef* spi_handle, panel_packet_t* outgoing, controller_packet_t* incoming, uint32_t timeout)
{
    return 0;
}
