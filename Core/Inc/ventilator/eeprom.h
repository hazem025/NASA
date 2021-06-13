/*
 * eeprom.h:
 *
 * Handles the interaction with the eeprom chip.  The eeprom stores records of 4-byte length. Each record
 * is stored to its own page. This simplifies reading and writing to ensure that no rewrite or erases cause
 * issue. This is possible because the number of records < number of pages.
 *
 *  Created on: Apr 21, 2020
 *      Author: tcanham
 */

#ifndef INC_VENTILATOR_EEPROM_H_
#define INC_VENTILATOR_EEPROM_H_

#include <assert.h>
#include <stdint.h>
#include <ventilator/bargraph.h>

//!< Define EEPROM records. Each enumeration entry
//!< corresponds to a 64 byte page on the device.
//!< There are 512 records available
//!< The device will store a 32-bit int on each page.

typedef enum {
    EEPROM_ALIVE_MINUTES,
    EEPROM_INIT_INHALE_SENSITIVITY,
    EEPROM_INIT_BREATH_DETECT_HOLD_OFF,
    EEPROM_INIT_PLATEAU_SAMPLE_OFFSET_TIME,
    EEPROM_INIT_PCTRL_KP,
    EEPROM_INIT_PCTRL_KI,
    EEPROM_INIT_PCTRL_KD,
    EEPROM_INIT_PCTRL_D_FILT_CUTOFF,
    EEPROM_INIT_PCTRL_SHAPE_FILT_CUTOFF,
    EEPROM_INIT_PCTRL_INT_L_LIMIT,
    EEPROM_INIT_PCTRL_INT_U_LIMIT,
    EEPROM_INIT_PCTRL_DELAY,
    EEPROM_INIT_PCTRL_SIN_AMP,
    EEPROM_INIT_PCTRL_SIN_F,
    EEPROM_NUM_RECORDS
} EepromRecordId;
/**
 * Enum status returned from read/write.
 */
typedef enum {
    EEPROM_OK,
    EEPROM_ERROR,
    EEPROM_BUSY
} EepromStatus;
/**
 * Eeprom constants.
 */
typedef enum {
    EEPROM_PAGE_SIZE = 64,       // Page size in bytes
    EEPROM_I2C_ADDR = 0x50 << 1, // EEPROM address on I2C bus
} EepromConst;


// Protects from over-using EEPROM
static_assert(EEPROM_NUM_RECORDS < 512, "Too many EEPROM records defined");

/**
 * Read from the eeprom.  Reads a record (of given ID) into the supplied val pointer.
 * const EepromRecordId id: ID to read
 * uint32_t *val: location to read to
 */
EepromStatus readEeprom(const EepromRecordId id, uint32_t *val);
/**
 * Write to the eeprom.  Writes a record (of given ID) from the supplied val.
 * const EepromRecordId id: ID to write
 * uint32_t val: value to write out
 */
EepromStatus writeEeprom(const EepromRecordId id, const uint32_t val);

#endif /* INC_VENTILATOR_EEPROM_H_ */
