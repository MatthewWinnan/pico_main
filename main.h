#ifndef __MAIN_H__
#define __MAIN_H__

// Define our major includes since we needn't repeat if we include this to main.c
// main.c is example code on how to implement the driver on a PICO W using the provided PICO SDK
// Standard C libraries
#include <stdio.h>
// Standard pico libraries
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "boards/pico_w.h"
// My driver libraries
#include "include/i2c_config.h"
#include "include/bmp180.h"
#include "include/bme280.h"
#include "include/24LC16B_EEPROM.h"
#include "include/com_protocol.h"
#include "include/bmp180_i2c.h"
#include "include/bme280_i2c.h"
#include "include/pico_rtc.h"

#define MAIN_DEBUG 0 // Should debug prints be done?

//In order to use the bmp180 library initialize an object instance of each of the following structs
extern struct bmp180_model my_bmp180; //used as variable to pass to save the current BMP state.
extern struct bmp180_calib_param my_bmp180_calib_params; //used as variable to pass to save calibration params. Used further in code.
extern struct bmp180_measurements my_bmp180_measurements; //used as a variable to store intermitent steps.

//In order to use the bme280 library initialize the object instance of each of the following
extern struct bme280_model my_bme280; //used as variable to pass to save the current BME280 state.
extern struct bme280_calib_param my_bme280_calib_params; //used as variable to pass to save calibration params. Used further in code.
extern struct bme280_settings my_bme280_settings; //Used to store the settings to be used be getters and setters

//In order to use the 24LC16B eeprom driver initialize the needed object
extern struct lcb16b_eeprom my_eeprom; //Used as structure to store ID and pointer

#endif