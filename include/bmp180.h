#ifndef __BMP180_H__
#define __BMP180_H__

#include <stdio.h>
#include <math.h>
#include "bmp180_i2c.h"

/* 
Data sheet can be found at https://www.alldatasheet.com/view.jsp?Searchword=Bmp180%20Datasheet&gclid=Cj0KCQiAj4ecBhD3ARIsAM4Q_jFxw-ka2tDoSAq-hRVmDPUqWJLJljY-YAPnbkRwIM0BGLI8VaWa158aArfZEALw_wcB
I will be referencing this as BMP180_DOC
Whenever a specific page is being referenced the code will be BMP180_DOC_<page_number>

This driver is meant to work on any micro controller using C.
The driver communicates through I2C
The bmp180_i2c.h header file is meant to be a generic wrapper function where one puts in a board specific I2C implementation.
The i2c_config.h header is meant to be a generic header where one defines I2C parameters and initialize functions.
More information can be found in the header itself.

Currently this driver is configured to work with the pico SDK.
*/

//BMP_180 Global Constants
#define BMP_180_CHIP_ID_ADDR _u(0xD0) //Given at BMP180_DOC_18
#define BMP_180_CHIP_ID _u(0x55) //Given at BMP180_DOC_18
#define BMP_180_ADDR _u(0xEE >> 1) //Given at BMP180_DOC_20. Bit shift to the left to remove LSB and make a 7-bit number
#define BMP_180_SEA_PRESSURE _u(101800) //Defines the relative pressure in Pa at sea level for the OR tambo station obtained from https://meteologix.com/za/observations/south-africa/pressure-qnh/20221204-0900z.html
#define BMP_180_CENTURION_HEIGHT _u(1453) //Height above sea level for Centurion. Obtained from https://elevation.maplogs.com/poi/centurion_south_africa.478211.html

// Lazy debug modes
#define BMP_180_DEBUG_MODE 0 //Defines if debug print statements are enabled. 0 for False 1>= for True. This will give feedback on each operational step.
#define BMP_180_INFO_MODE 1 //Defines if INFO print statements are enabled. 0 for False 1>= for True. Info is for init feedback.

//BMP_180 Global Register Values
//Given at BMP180_DOC_18
//Define the registers to be used for calibration parameters and amount of these
#define BMP_180_REG_A1_MSB _u(0xAA)
#define BMP_180_REG_A1_LSB _u(0xAB)
#define BMP_180_REG_A2_MSB _u(0xAC)
#define BMP_180_REG_A2_LSB _u(0xAD)
#define BMP_180_REG_A3_MSB _u(0xAE)
#define BMP_180_REG_A3_LSB _u(0xAF)
#define BMP_180_REG_A4_MSB _u(0xB0)
#define BMP_180_REG_A4_LSB _u(0xB1) 
#define BMP_180_REG_A5_MSB _u(0xB2)
#define BMP_180_REG_A5_LSB _u(0xB3)
#define BMP_180_REG_A6_MSB _u(0xB4)
#define BMP_180_REG_A6_LSB _u(0xB5)
#define BMP_180_REG_B1_MSB _u(0xB6)
#define BMP_180_REG_B1_LSB _u(0xB7)
#define BMP_180_REG_B2_MSB _u(0xB8)
#define BMP_180_REG_B2_LSB _u(0xB9)
#define BMP_180_REG_MB_MSB _u(0xBA)
#define BMP_180_REG_MB_LSB _u(0xBB)
#define BMP_180_REG_MC_MSB _u(0xBC)
#define BMP_180_REG_MC_LSB _u(0xBD)
#define BMP_180_REG_MD_MSB _u(0xBE)
#define BMP_180_REG_MD_LSB _u(0xBF)
#define BMP_180_N_CAL_PARAMS 22

//Soft Reset Register 
#define BMP_180_REG_SOFT_RST  _u(0xE0)
#define BMP_180_POWER_ON_RESET _u(0xB6)

//Register for the Measurements
#define BMP_180_REG_CTRL_MEAS _u(0xF4)
//The following values are precalculated at BMP180_DOC_21
#define BMP_180_SET_TMP _u(0x2E) //Passed to ctrl_meas register to init the tmp measurements.
#define BMP_180_SET_PRESS_OSS_0 _u(0x34) //Pressure measurements at OSS 0
#define BMP_180_SET_PRESS_OSS_1 _u(0x74) //Pressure measurements at OSS 1
#define BMP_180_SET_PRESS_OSS_2 _u(0xB4) //Pressure measurements at OSS 2
#define BMP_180_SET_PRESS_OSS_3 _u(0xF4) //Pressure measurements at OSS 3
#define BMP_180_OSS 1 //Defines the mode we want to work in
#define BMP_180_SS 3 //Defines the amount of samples we take and average out over per measurement index.

//Output Registers
#define BMP_180_REG_OUT_MSB _u(0xF6)
#define BMP_180_REG_OUT_LSB _u(0xF7)
#define BMP_180_REG_OUT_XLSB _u(0xF8) //Remember to bitshift left 3 since these are all 0

//BMP_180_Global Variables
//Timing control variables given at BMP180_DOC_21. I added 0.5 to make them all ints for neatness
#define BMP_180_TMP_TIME 5
#define BMP_180_PRES_OSS_0 5
#define BMP_180_PRES_OSS_1 8
#define BMP_180_PRES_OSS_2 14
#define BMP_180_PRES_OSS_3 26

// Calibration parameters given at BMP180_DOC_18
// Nice struct decleration for each of the params can be found at the BOSCH site https://rbdevportal.secure.footprint.net/Documentation/html/structbmp180__calib__param__t.html
struct bmp180_calib_param {
    int16_t AC1;
    int16_t AC2;
    int16_t AC3;
    uint16_t AC4;
    uint16_t AC5;
    uint16_t AC6;
    int16_t B1;
    int16_t B2;
    int16_t MB;
    int16_t MC;
    int16_t MD;
};

//Structure used to store and save the parameters used during temperature and pressure calculations.
//This is used for debugging
struct bmp180_measurements{
     long ut;
     long up;
     // Intermittent temperature values
     long X1_tmp;
     long X2_tmp;
     // Intermittent Pressure values (There is a lot. Ideally you would reuse the variable, however I wanted to break each step appart)
     long X1_p_1;
     long X2_p_1;
     long X3_p_1;

     long X1_p_2;
     long X2_p_2;
     long X3_p_2;

     long X1_p_3;
     long X2_p_3;

     long X1_p_4;

     // These seem to be special derived compensation parameters
     long B3;
     unsigned long B4;
     long B5;
     long B6;
     unsigned long B7;

     // Sum variables that get stores until we average again
     long T_sum;
     long p_sum;

     //Final calculated values
     long T;
     long p_inter;
     long p;

     //Altitude value
     float altitude;

     //Relative pressure to sea-level
     float p_relative;
};

//Declare our chip model
struct bmp180_model {
    struct bmp180_calib_param* cal_params;
    struct bmp180_measurements* measurement_params;
    uint8_t chipID;
};

//Mappings for the OSS mode of Pressure to variable input 
uint16_t pressure_oss[4] ;
//Mappings for out wait time in pressure mode
uint16_t pressure_time[4] ;  

void bmp180_get_cal(struct bmp180_calib_param* params,struct bmp180_model* my_chip);

void bmp180_init(struct bmp180_model* my_chip, struct bmp180_calib_param* my_params, struct bmp180_measurements* measures);

//Here UT and UP stands for unprocessed temperature and -pressure respectively.
void bmp180_get_ut(struct bmp180_model* my_chip);
void bmp180_get_temp(struct bmp180_model* my_chip);

void bmp180_get_up(struct bmp180_model* my_chip);
void bmp180_get_pressure(struct bmp180_model* my_chip);

//Main wrapper to obtain measurements and average over :)
void bmp180_get_measurement(struct bmp180_model* my_chip);

// Get the altitude
void bmp180_get_altitude(struct bmp180_model* my_chip);
// Get relative sea pressure
void bmp180_get_sea_pressure(struct bmp180_model* my_chip);


#endif