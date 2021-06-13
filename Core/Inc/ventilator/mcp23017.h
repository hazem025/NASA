/*
 * mcp23017.h:
 *
 * Functions and definitions needed to interact with the MCP23017 chip.
 *
 *  Created on: Apr 9, 2020
 *      Author: tcanham
 */

#ifndef INC_VENTILATOR_MCP23017_H_
#define INC_VENTILATOR_MCP23017_H_

#include <stdint.h>
#include <stdbool.h>
#include "stm32f0xx_hal.h"

// register addresses - use bank = 0 for sequential reads

#define REG_IODIRA 0x00
#define REG_IODIRB 0x01
#define REG_IPOLA 0x02
#define REG_IPOLB 0x03
#define REG_GPINTENA 0x04
#define REG_GPINTENB 0x05
#define REG_DEFVALA 0x06
#define REG_DEFVALB 0x07
#define REG_INTCONA 0x08
#define REG_INTCONB 0x09
#define REG_IOCON 0x0A
#define REG_IOCON2 0x0B
#define REG_GPPUA 0x0C
#define REG_GPPUB 0x0D
#define REG_INTFA 0x0E
#define REG_INTFB 0x0F
#define REG_INTCAPA 0x10
#define REG_INTCAPB 0x11
#define REG_GPIOA 0x12
#define REG_GPIOB 0x13
#define REG_OLATA 0x14
#define REG_OLATB 0x15

typedef struct {
    uint16_t addr;
    I2C_HandleTypeDef* i2c;
    uint32_t timeout;
} McpHandle;
/**
 * mcp23017_init:
 *
 * Initialize the chip for either reading as input or writing as output. This automates the reads
 * and writes to ensure the chip is configured.
 * McpHandle* handle: handle for device to init
 * uint8_t addr: address of device to initialize
 * bool is_input: true to make it an input chip, false for output
 */
HAL_StatusTypeDef mcp23017_init(McpHandle* handle, uint8_t addr, bool is_input);

/**
 * mcp23017_read_reg:
 *
 * Reads register memory from the device, starting at device_reg_addr and moving forward num_addrs.
 * McpHandle* handle: handle for device to read from
 * const uint8_t device_reg_addr: address of register on device to read from
 * uint8_t* val: data pointer to read into
 * const uint8_t num_addrs: numer of address to read in
 * return: HAL_OK (0) on success, something else on error
 */
HAL_StatusTypeDef mcp23017_read_reg(McpHandle* handle, const uint8_t device_reg_addr, uint8_t* val, const uint8_t num_addrs);

/**
 * mcp23017_write_reg:
 *
 * Writes register memory to the device, starting at device_reg_addr and moving forward num_addrs.
 * McpHandle* handle: handle for device to write to
 * const uint8_t device_reg_addr: address of register on device to write to
 * const uint8_t* val: data pointer to write out of
 * const uint8_t num_addrs: numer of address to write out
 * return: HAL_OK (0) on success, something else on error
 */
HAL_StatusTypeDef mcp23017_write_reg(McpHandle* handle, const uint8_t device_reg_addr, uint8_t* val, const uint8_t num_addrs);

#endif /* INC_VENTILATOR_MCP23017_H_ */
