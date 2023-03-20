#ifndef __BME280_H__
#define __BME280_H__

#include <stdio.h>
#include <math.h>
#include "bme280_i2c.h"
#include "com_protocol.h"

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
From BME280_DOC_30 CSB must be connected to VDDIO to enable I2C.
Further the state of SDO sets the LSB of the device address (BME280_DOC_31).
The pinouts of each are given at BME280_CIRCUIT_5.
Further from BME280_CIRCUIT_25 VDD and VDDIO are being powered from the same source. 
(Pull up resistors, power regulator and capcitors are on the board already)

Further from BME280_CIRCUIT_5 3Vo is the output from the voltage regulator
that is being fed into VDDIO and VDD to power the chip.

Thus the setup has CSB connected to the output of 3Vo.
SDO is gonnected to GND.
From this the device address becomes 0x76 and the BME280 is in I2C mode.
If 0x77 address is wished connect SDO to 3Vo.
In my use case the bmp180 address was 0x77 thus I needed 0x76 instead 
*/
#define BME_280_ADDR _u(0x76)

//Mode variables
#define BME_280_RESET_VALUE _u(0xB6) // Value to be written to reset reg to start power on process
/*
Values are given at BME280_DOC_27 and given here for reference
Sleep: 00
Forced: 10 or 01 
Normal: 11

A flow diagram and exlanations of the modes are given at BME280_DOC_12 and BME280_DOC_13 respectively
In essence normal mode does continious sampling.
Forced mode does a single sample and then the device enters sleep mode.
Sleep mode is an idle state. 

NB: Timing is not as important in normal mode but I have added constants to help with timing convention
The chip employs shadow registers and these are described in BME280_DOC_21
Only when measurement read is done will newer data be read to registers, so a simple guess of the frequency of new samples 
should be good enough.
*/
#define BME_280_MODE _u(0b00) // What mode should the init function set it to. My use case will be forced mode as such the init value is sleep mode
/*
Values are given at BME280_DOC_26 for osrs_h modes
*/
#define BME_280_OSRS_H_MODE _u(0) // Selects the mode
// Hardcoded mode values
#define BME_280_OSRS_H_1 _u(0b000)
#define BME_280_OSRS_H_2 _u(0b001)
#define BME_280_OSRS_H_3 _u(0b010)
#define BME_280_OSRS_H_4 _u(0b011)
#define BME_280_OSRS_H_5 _u(0b100)
#define BME_280_OSRS_H_6 _u(0b101)
// Lookup array
extern uint8_t bme280_osrs_h_mode_array[6];

/*
Values are given at BME280_DOC_27 for osrs_p modes
*/
#define BME_280_OSRS_P_MODE _u(0) // Selects the mode
// Hardcoded mode values
#define BME_280_OSRS_P_1 _u(0b000)
#define BME_280_OSRS_P_2 _u(0b001)
#define BME_280_OSRS_P_3 _u(0b010)
#define BME_280_OSRS_P_4 _u(0b011)
#define BME_280_OSRS_P_5 _u(0b100)
#define BME_280_OSRS_P_6 _u(0b101)
// Lookup array
extern uint8_t bme280_osrs_p_mode_array[6];

/*
Values are given at BME280_DOC_27 for osrs_t modes
*/
#define BME_280_OSRS_T_MODE _u(0) // Selects the mode
// Hardcoded mode values
#define BME_280_OSRS_T_1 _u(0b000)
#define BME_280_OSRS_T_2 _u(0b001)
#define BME_280_OSRS_T_3 _u(0b010)
#define BME_280_OSRS_T_4 _u(0b011)
#define BME_280_OSRS_T_5 _u(0b100)
#define BME_280_OSRS_T_6 _u(0b101)
// Lookup array
extern uint8_t bme280_osrs_t_mode_array[6];

/*
Values are given at BME280_DOC_28 for t_sb settings.
This defines the settling time in normal mode.
The impact it has on the timing of consecutive measurements is shown in the timing diagram at BME280_DOC_14
*/
#define BME_280_T_SB_MODE _u(0)
// Hardcoded mode values
#define BME_280_T_SB_1 _u(0b000)
#define BME_280_T_SB_2 _u(0b001)
#define BME_280_T_SB_3 _u(0b010)
#define BME_280_T_SB_4 _u(0b011)
#define BME_280_T_SB_5 _u(0b100)
#define BME_280_T_SB_6 _u(0b101)
#define BME_280_T_SB_7 _u(0b110)
#define BME_280_T_SB_8 _u(0b111)
// Lookup array
extern uint8_t bme280_t_sb_mode_array[8];

/*
Values are given at BME280_DOC_28 for filter settings.
For the full explenation of this look at BME280_DOC_15 - BME280_DOC_16
*/
#define BME_280_FILTER_MODE _u(0)
// Hardcoded mode values
#define BME_280_FILTER_1 _u(0b000)
#define BME_280_FILTER_2 _u(0b001)
#define BME_280_FILTER_3 _u(0b010)
#define BME_280_FILTER_4 _u(0b011)
#define BME_280_FILTER_5 _u(0b100)
// Lookup array
extern uint8_t bme280_filter_mode_array[5];

/*
This enables the SPI interface 3 wire when set to 1. I have included it for completeness sake, since the example uses I2C it is not needed.
But feel free to change this to use SPI. 
Some documentation is found at BME280_DOC_28 for the value and BME280_DOC_32 (Section 6.2) for details on the SPI.
BME280_DOC_30 (Section 6.1) explains the interface selection process based on hardware (the one I followed).
*/
#define BME_280_SPI3W_EN _u(0)

// Constant timing variables
#define BME_280_STARTUP_SCALE_SAFE 10 // Some safety scaler to make sure the chip has started
#define BME_280_STARTUP_T _u(BME_280_STARTUP_SCALE_SAFE * 2) // Startup time defined at BME280_DOC_7 as 2ms. Times it with scale safe to get total wait

/*
Standby timing values can be viewed at BME280_DOC_28
Timing is done in us to avoid float values.
Selection is done based on BME_280_T_SB_MODE
*/
#define BME_280_T_SB_TIMING_1 500
#define BME_280_T_SB_TIMING_2 62500
#define BME_280_T_SB_TIMING_3 125000
#define BME_280_T_SB_TIMING_4 250000
#define BME_280_T_SB_TIMING_5 500000
#define BME_280_T_SB_TIMING_6 1000000
#define BME_280_T_SB_TIMING_7 10000
#define BME_280_T_SB_TIMING_8 20000
// Lookup array
extern uint32_t bme280_t_sb_timing_array[8];

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

// Register locations for configuration values. Obtained from BME280_DOC_25
#define BME_280_REG_CONFIG _u(0xF5)
#define BME_280_REG_CTRL_MEAS _u(0xF4)
#define BME_280_REG_STATUS _u(0xF3)
#define BME_280_REG_CTRL_HUM _u(0xF2) // NB Changes to this register only become effective after a write operation to “ctrl_meas”
#define BME_280_REG_RESET _u(0xE0) // Write 0xB6 for reset using the complete power-on-reset procedure

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

// Stores the configured mode values for the bme280. Ideally one would use the macros and API to redefine it in runtime :)
struct bme280_settings {
    uint8_t mode;
    uint8_t osrs_h;
    uint8_t osrs_t;
    uint8_t osrs_p;
    uint8_t filter;
    uint8_t spi3w_en;
    uint8_t t_sb;
};

// Stores the sampled readings
struct bme280_measurements {
    // Variable sizes are defined at BME280_DOC_23
    int32_t adc_T;
    int32_t adc_P;
    int32_t adc_H;

    int32_t t_fine;

    int32_t T;
    uint32_t P;
    uint32_t H; 
};

// Structure to store the current state of the chip
struct bme280_model {
    struct bme280_calib_param *cal_params;
    struct bme280_settings *settings;
    uint8_t chipID;
};

// Main functions

// For initialization
void bme280_init(struct bme280_model *my_chip, struct bme280_calib_param *params, struct bme280_settings *settings);
void read_bme280_chip_id(struct bme280_model *my_chip);
void read_bme280_callibration_params(struct bme280_model *my_chip, struct bme280_calib_param *params);

// Define configuration functions
void bme280_set_config(struct bme280_model *my_chip);
void bme280_set_ctrl_meas(struct bme280_model *my_chip);
void bme280_set_ctrl_hum(struct bme280_model *my_chip);

// Define getter functions
void bme280_read_ctrl_meas(struct bme280_model *my_chip);
void bme280_read_config(struct bme280_model *my_chip);
void bme280_read_ctrl_hum(struct bme280_model *my_chip);
void bme280_read_status(uint8_t *reg);
bool bme280_is_doing_conversion();

#endif