#ifndef __MAIN_H__
#define __MAIN_H__

// Define our major includes since we needn't repeat if we include this to main.c
// main.c is example code on how to implement the driver on a PICO W using the provided PICO SDK
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "src/bmp180.c"
#include "src/24lc16b_eeprom.c"
#include "include/bmp180_i2c.h"
#include "boards/pico_w.h"

//In order to use the bmp180 library initialize on object instance of each of the following structs
struct bmp180_model my_bmp180; //used as variable to pass to save the current BMP state.
struct bmp180_calib_param calib_params; //used as variable to pass to save calibration params. Used further in code.
struct bmp180_measurements my_bmp180_measurements; //used as a variable to store intermitent steps.

//In order to use the 24LC16B eeprom driver initialize the needed object
struct lcb16b_eeprom my_eeprom; //Used as structure to store ID and pointer

#endif