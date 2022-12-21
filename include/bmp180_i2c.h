#ifndef __BMP180_I2C__
#define __BMP180_I2C__
// This consists of the wrapper functionality meant to hold any specific I2C implementation one might need.
// This example is based off of the PICO SDK
// Documentation can be found at https://raspberrypi.github.io/pico-sdk-doxygen/index.html

#include <stdio.h>
#include "pico/stdlib.h"
#include "i2c_config.h"

// Write function
int bmp180_i2c_write(uint8_t addr, const uint8_t *src, size_t len, bool nostop);

//Read function
int bmp180_i2c_read(uint8_t addr, const uint8_t *src, uint8_t *dst, size_t len, bool nostop);

#endif 