#ifndef __24LC16B_EEPROM__
#define __24LC16B_EEPROM__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "24LC16B_EEPROM_i2c.h"

/*
The datasheet for the 24LC16B eeprom can be found at https://pdf1.alldatasheet.com/datasheet-pdf/view/515802/MICROCHIP/24LC16B.html
This document will be referenced as 24LC16B_DOC throughout inplementation
Whenever a specific page is referenced I will refer to it as 24LC16B_DOC_x where x is the page number of the datasheet

This driver is meant to work on any micro controller using C.
The driver communicates to the eeprom through I2C
The eeprom address is non configurable :(
The 24LC16B_EEPROM_i2c.h header file is meant to be a generic wrapper function where one puts in a board specific I2C implementation.
The i2c_config.h header is meant to be a generic header where one defines I2C parameters and initialize functions.
More information can be found in the header itself.

Currently this driver is configured to work with the pico SDK.
*/

// 24LC16B global constants
#define LCB16B_CHIP_ID_ADDR _u(0x000) //Configured myself for future
#define LCB16B_CHIP_ID _u(0xAA) //Configured myself for future
#define LCB16B_ADDR _u(0xA) //First nibble of the address 24LC16B_DOC_7
#define LCB16B_START_REG _u(0x001) //First register that can be W/R to 24LC16B_DOC_8 (Excludes chip ID register)
#define LCB16B_STOP_REG _u(0x7FF) //Last register that can be W/R to 24LC16B_DOC_8

#define LCB16B_PAGE_WRITE_TIME_SAFETY _u(4) // Defines the scalar to scale LCB16B_PAGE_WRITE_TIME to form a safety margin
#define LCB16B_PAGE_WRITE_TIME _u(5) // From 24LC16B_DOC_8 there is a max page write time of 5ms, make it wait 4X for safety

#define LCB16B_INIT 1 //Flag to use to determine if new chipID should be written. If set to 0 will see if chipID can be read
#define LCB16B_DEBUG 1 //Flag to determine if USB debut statements should be printed

// Objects used to decribe the chip

//Declare our chip model
//The model follows a circular methodology where once the pointer is higer than 
//LCB16B_STOP_REG then it starts back at LCB16B_START_REG  

struct lcb16b_eeprom {
    uint8_t chipID; //Stores the chipID for reference
    uint16_t pointer; //Points to what register we are currently at
};

//Helper functions

// Returns control byte
uint8_t return_device_address(uint16_t register_address);

//Main Functions

//Initializes the EEPROM
void lcb16b_eeprom_init(struct lcb16b_eeprom* my_eeprom);

//Writing functions

//Performs a random write operation
void lcb16b_eeprom_random_write(struct lcb16b_eeprom* my_eeprom, const uint8_t *src, uint8_t len);
//Performs a write to the address pointed to by register
void lcb16b_eeprom_point_write(struct lcb16b_eeprom* my_eeprom, const uint8_t *src, uint8_t len, uint16_t reg);

//Performs a random read operation
void lcb16b_eeprom_random_read(struct lcb16b_eeprom* my_eeprom, uint8_t *dst, uint8_t len);
//Performs a read to the address pointed to by register
void lcb16b_eeprom_point_read(struct lcb16b_eeprom* my_eeprom, uint8_t *dst, uint8_t len, uint16_t reg);

//Print functions to be grabbed by serial queries
void print_eeprom_chip_ID(struct lcb16b_eeprom* my_eeprom);

#endif