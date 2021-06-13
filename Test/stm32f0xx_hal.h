/**
 * stm32f0xx_hal.h:
 *
 * Faked stm32f0xx_hal.h header for testing purposes.
 */
extern int GPIO_READ_TEST_VALUE;

// Override the timer type to become void*
#define TIM_HandleTypeDef int
#define I2C_HandleTypeDef int
#define SPI_HandleTypeDef int


#define HAL_StatusTypeDef int
#define GPIO_TypeDef int
#define UART_HandleTypeDef int
#define ADC_HandleTypeDef int

#define HAL_OK 0
#define HAL_ERROR 1

#define HAL_MAX_DELAY 0

#define HAL_SPI_Transmit(...) HAL_OK
#define HAL_I2C_Master_Transmit(...) HAL_OK
#define HAL_I2C_Master_Receive(...) HAL_OK

#define HAL_I2C_Mem_Read(...) HAL_OK
#define HAL_I2C_Mem_Write(...) HAL_OK

#define HAL_GPIO_ReadPin(...) GPIO_READ_TEST_VALUE
#define HAL_GPIO_WritePin(...) HAL_OK
#define HAL_GPIO_TogglePin(...) HAL_OK

#define HAL_TIM_PWM_Start(...) HAL_OK
#define HAL_TIM_PWM_Stop(...) HAL_OK

#define GPIOB 0
#define GPIO_PIN_SET 1
#define GPIO_PIN_UNSET 0
#define GPIO_PIN_12 1
#define GPIO_PIN_14 1
#define GPIO_PIN_5 1
#define GPIO_PIN_RESET 0
