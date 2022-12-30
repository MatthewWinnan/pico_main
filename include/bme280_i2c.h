#ifndef __BME280_I2C__
#define __BME280_I2C__
// This consists of the wrapper functionality meant to hold any specific I2C implementation one might need.
// This example is based off of the PICO SDK
// Documentation can be found at https://raspberrypi.github.io/pico-sdk-doxygen/index.html

/*
A small comment on the I2C can be found at BME280_DOC_30.
Of note "multiple byte write (using pairs of register addresses and register data)".
Thus multiple address writing does not use an autoincremented target address and must be assigned each time.
Further for reading " multiple byte read (using a single register address which is auto-incremented)".
Thus a typical burst read is supported.
*/

#include <stdio.h>
#include "pico/stdlib.h"
#include "i2c_config.h"

// Write function
int bme280_i2c_write(uint8_t addr, const uint8_t *src, size_t len, bool nostop);

//Read function
int bme280_i2c_read(uint8_t addr, const uint8_t *src, uint8_t *dst, size_t len, bool nostop);

#endif 