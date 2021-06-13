#include <ventilator/eeprom.h>
#include <ventilator/types.h>
#include <ventilator/panel_public.h>
#include <ventilator/constants.h>
#include <swassert.h>
#include <stm32f0xx_hal.h>
#include <assert.h>

EepromStatus readEeprom(const EepromRecordId id, uint32_t *val) {
    SW_ASSERT(val);
    SW_ASSERT1(id < EEPROM_NUM_RECORDS,id);

    uint16_t addr = little_to_big16(id * EEPROM_PAGE_SIZE);

    // first, do a dummy write to set the device internal address (spec page 10)
    HAL_StatusTypeDef stat = HAL_I2C_Master_Transmit(&hi2c1, EEPROM_I2C_ADDR, (uint8_t*)&addr, sizeof(uint16_t), HAL_MAX_DELAY);
    switch (stat) {
        case HAL_OK:
            break; // HAL OK is only valid status
        case HAL_BUSY: // fall through
        case HAL_TIMEOUT:
            return EEPROM_BUSY;
        case HAL_ERROR:
            return EEPROM_ERROR;
        default:
            SW_ASSERT1(0,stat);
            return EEPROM_ERROR;
    }

    // once the address is set, do a read to get the data
    stat = HAL_I2C_Master_Receive(&hi2c1, EEPROM_I2C_ADDR, (uint8_t*)val, sizeof(uint32_t), HAL_MAX_DELAY);
    switch (stat) {
        case HAL_OK:
            return EEPROM_OK;
        case HAL_BUSY: // fall through
        case HAL_TIMEOUT:
            return EEPROM_BUSY;
        case HAL_ERROR:
            return EEPROM_ERROR;
        default:
            SW_ASSERT1(0,stat);
            break;
    }
    return EEPROM_ERROR; // for code checkers, shouldn't get here
}

EepromStatus writeEeprom(const EepromRecordId id, const uint32_t val) {
    SW_ASSERT1(id < EEPROM_NUM_RECORDS,id);

    uint16_t data[3];
    data[0] = little_to_big16(id * EEPROM_PAGE_SIZE); // first 16-bit word is address
    data[1] = val;
    data[2] = val >> 16;

    // write data (spec page 8)
    HAL_StatusTypeDef stat = HAL_I2C_Master_Transmit(&hi2c1, EEPROM_I2C_ADDR, (uint8_t*)&data, sizeof(data), HAL_MAX_DELAY);

    switch (stat) {
        case HAL_OK:
            return EEPROM_OK;
        case HAL_BUSY: // fall through
        case HAL_TIMEOUT:
            return EEPROM_BUSY;
        case HAL_ERROR:
            return EEPROM_ERROR;
        default:
            SW_ASSERT1(0,stat);
            break;
    }
    return EEPROM_ERROR; // for code checkers, shouldn't get here
}
