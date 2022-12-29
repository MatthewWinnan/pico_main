#ifndef __BME280_H__
#define __BME280_H__

#include <stdio.h>
#include <math.h>
#include "bme280_i2c.h"

/*
Data sheet can be found at https://cdn-shop.adafruit.com/datasheets/BST-BME280_DS001-10.pdf

For reference I will be using the code BME280_DOC whenever the data sheet is being used as a reference.
Whenever a specific page is being referenced the convention is BME280_DOC_<Page_Number>
For example BME280_DOC_12 refers to page 12 of the data sheet.

The breakout board schematics can be found at https://www.mouser.com/datasheet/2/737/adafruit-bme280-humidity-barometric-pressure-tempe-740823.pdf

For reference I will be using the code BME280_CIRCUIT whenever the boardschematics are being used as reference.
Similarly when a page is referenced the format is BME280_CIRCUIT_<Page_Number>

This driver is meant to work on any micro controller using C.
The driver communicates through I2C
The bme280_i2c.h header file is meant to be a generic wrapper function where one puts in a board specific I2C implementation.
The i2c_config.h header is meant to be a generic header where one defines I2C parameters and initialize functions specific to ones board.
More information can be found in the header itself.

Currently this driver is configured to work with the pico SDK.
*/

//BME_280 Global Constants
#define BME_280_CHIP_ID_ADDR _u(0xD0) //Given at BME280_DOC_25
#define BME_280_CHIP_ID _u(0x60)
/*
From BME280_DOC_31 CSB must be connected to VDDIO to enable I2C.
Further the state of SDO sets the LSB of the device address.
The pinouts of each are given at BME280_CIRCUIT_5.
Further from BME280_CIRCUIT_25 VDD and VDDIO are being powered from the same source. 
(Pull up resistors, power regulator and capcitors are on the board already)

Further from BME280_CIRCUIT_5 3Vo is the output from the voltage regulator
that is being fed into VDDIO and VDD to power the chip.

Thus the setup has CSB connected to the output of 3Vo.
SDO is gonnected to GND.
From this the device address becomes 0x76 and the BME280 is in I2C mode.
If 0x77 address is wished connect SDO to 3Vo.
In my use case the bmp180 address is 0x77 thus I needed 0x76 instead 
*/
#define BME_280_ADDR _u(0x76)

#define BME_280_STARTUP_SCALE_SAFE 10 // Some safety scaler to make sure the chip has started
#define BME_280_STARTUP_T _u(BME_280_STARTUP_SCALE_SAFE * 2) // Startup time defined at BME280_DOC_7 as 2ms. Times it with scale safe to get total wait

// Lazy debug modes
#define BME_280_DEBUG_MODE 1 //Defines if debug print statements are enabled. 0 for False 1>= for True. This will give feedback on each operational step.
#define BME_280_INFO_MODE 1 //Defines if INFO print statements are enabled. 0 for False 1>= for True. Info is for init feedback.

//Register locations
//These can be obtained from BME280_DOC_22 and BME280_DOC_25 (shows a nice register map in the latter case)
#define BME_280_REG_T1_LSB _u(0x88)
#define BME_280_REG_T1_MSB _u(0x89)
#define BME_280_REG_T2_LSB _u(0x8A)
#define BME_280_REG_T2_MSB _u(0x8B)
#define BME_280_REG_T3_LSB _u(0x8C)
#define BME_280_REG_T3_MSB _u(0x8D)

#define BME_280_REG_P1_LSB _u(0x8E)
#define BME_280_REG_P1_MSB _u(0x8F)
#define BME_280_REG_P2_LSB _u(0x90)
#define BME_280_REG_P2_MSB _u(0x91)
#define BME_280_REG_P3_LSB _u(0x92)
#define BME_280_REG_P3_MSB _u(0x93)
#define BME_280_REG_P4_LSB _u(0x94)
#define BME_280_REG_P4_MSB _u(0x95)
#define BME_280_REG_P5_LSB _u(0x96)
#define BME_280_REG_P5_MSB _u(0x97)
#define BME_280_REG_P6_LSB _u(0x98)
#define BME_280_REG_P6_MSB _u(0x99)
#define BME_280_REG_P7_LSB _u(0x9A)
#define BME_280_REG_P7_MSB _u(0x9B)
#define BME_280_REG_P8_LSB _u(0x9C)
#define BME_280_REG_P8_MSB _u(0x9D)
#define BME_280_REG_P9_LSB _u(0x9E)
#define BME_280_REG_P9_MSB _u(0x9F)

#define BME_280_REG_H1 _u(0xA1)
#define BME_280_REG_H2_LSB _u(0xE1)
#define BME_280_REG_H2_MSB _u(0xE2)
#define BME_280_REG_H3 _u(0xE3)
#define BME_280_REG_H4_LSB _u(0xE4)
#define BME_280_REG_H4_MSB _u(0xE5)
#define BME_280_REG_H5_LSB _u(0xE5)
#define BME_280_REG_H5_MSB _u(0xE6)
#define BME_280_REG_H6 _u(0xE7)

#define BME_280_N_CAL_PARAMS _u(32)

// Important structure declerations
// Stores all the callibrated parameters shown at BME280_DOC_22 and BME280_DOC_23
// An important fact is that unsigned integer values are stored in two’s complement BME280_DOC_22
struct bme280_calib_param {
    // Temperature compensation values
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;

    // Pressure compensation values
    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;

    // Humidity compensation values
    uint8_t dig_H1; //BME280_DOC_22 does state unsigned char, convert if needed later, keep uint
    int16_t dig_H2;
    uint8_t dig_H3; //BME280_DOC_22 does state unsigned char, convert if needed later, keep uint
    int16_t dig_H4;
    int16_t dig_H5;
    int8_t dig_H6; //BME280_DOC_23 does state signed char ??, convert if needed later, keep int

};

// Structure to store the current state of the chip
struct bme280_model {
    struct bme280_calib_param *cal_params;
    uint8_t chipID;
};

// Main functions

// For initialization
void bme280_init(struct bme280_model *my_chip, struct bme280_calib_param *params);
void read_bme280_chip_id(struct bme280_model *my_chip);
void read_bme280_callibration_params(struct bme280_model *my_chip, struct bme280_calib_param *params);

#endif