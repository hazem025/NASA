/*
 * mcp23017.c
 *
 *  Created on: Apr 9, 2020
 *      Author: tcanham
 */
#include <swassert.h>
#include <ventilator/mcp23017.h>
#include <ventilator/panel_public.h>

HAL_StatusTypeDef mcp23017_init(McpHandle* handle, uint8_t addr, bool is_input) {
    SW_ASSERT(handle);
    SW_ASSERT(is_input == 0 || is_input == 1);
    SW_ASSERT(addr < 0x80);
    HAL_StatusTypeDef stat = HAL_OK;
    // Setup basic handle properties
    handle->i2c = &hi2c1; // Same I2C is used on all devices
    handle->addr = addr << 1;
    handle->timeout = HAL_MAX_DELAY;

    // Send initialization commands
    uint8_t reg[2] = {0,0};
    // Configure switch MCP23017 input
    if (is_input) {
        // IOCON - default values are correct
        // IODIR A/B - all are inputs - default is input
        // IPOL A/B - normal polarity is correct - switches are pulled down when open
        // GPINTENA/A - interrupt on change to latch the value
        reg[0] = 0xFF;
        reg[1] = 0xFF;
        stat = mcp23017_write_reg(handle, REG_GPINTENA, reg, 2);
        if (stat != HAL_OK) {
            return stat;
        }
        stat = mcp23017_write_reg(handle, REG_INTCONA, reg, 2);
        if (stat != HAL_OK) {
            return stat;
        }
        // Pin direction "output"
        stat = mcp23017_write_reg(handle, REG_IODIRA, reg, 2);
        if (stat != HAL_OK) {
            return stat;
        }
    }
    // Output devices used for display
    else {
        // DEFVAL A/B - default is correct
        // INTCON A/B - default is correct. Want to compare against last value
        // so we can detect press and release
        // GPPU A/B - default is correct. Don't need pull-up since switch has pull-down.
        reg[0] = 0x00;
        reg[1] = 0x00;
        // reinforce state to off before making them outputs
        stat = mcp23017_write_reg(handle, REG_OLATA, reg, 2);
        if (stat != HAL_OK) {
            return stat;
        }
        stat = mcp23017_write_reg(handle, REG_GPIOA, reg, 2);
        if (stat != HAL_OK) {
            return stat;
        }
        stat = mcp23017_write_reg(handle, REG_IODIRA, reg, 2);
        if (stat != HAL_OK) {
            return stat;
        }
    }
    return stat;
}


HAL_StatusTypeDef mcp23017_read_reg(McpHandle* handle, const uint8_t device_reg_addr, uint8_t* val, const uint8_t num_addrs) {
    SW_ASSERT(handle);
    SW_ASSERT(val);
    SW_ASSERT2((device_reg_addr + num_addrs) <= REG_OLATB + 1, device_reg_addr, num_addrs);
    HAL_StatusTypeDef stat = HAL_I2C_Mem_Read (
            handle->i2c,
            handle->addr,
            device_reg_addr,
            1,
            val,
            num_addrs,
            handle->timeout);
    return stat;
}

HAL_StatusTypeDef mcp23017_write_reg(McpHandle* handle, const uint8_t device_reg_addr, uint8_t* val, const uint8_t num_addrs) {
    SW_ASSERT(handle);
    SW_ASSERT(val);
    SW_ASSERT2((device_reg_addr + num_addrs) <= REG_OLATB + 1, device_reg_addr, num_addrs);
    HAL_StatusTypeDef stat = HAL_I2C_Mem_Write (
            handle->i2c,
            handle->addr,
            device_reg_addr,
            1,
            val,
            num_addrs,
            handle->timeout);
    return stat;
}
