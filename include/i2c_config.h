#ifndef __I2C_CONFIG_H__
#define __I2C_CONFIG_H__
// I am only defining some constant I2C functionality here.
// Will consist of an init and some constants for the program
// This is specific to the raspberry PI PICO SDK
// Use your own if needed.

#include <stdio.h>
#include "hardware/i2c.h"
#include "pico/stdlib.h"

//I2C variables
#define I2C_PORT i2c0
#define I2C_BAUDRATE 200000 //400KHZ

//GPIO Variables
#define GPIO_I2C0_SDA 4
#define GPIO_I2C0_SCL 5

void global_i2c_init(){
    //Initialize the I2C
    i2c_init(I2C_PORT,I2C_BAUDRATE);
    gpio_set_function(GPIO_I2C0_SDA, GPIO_FUNC_I2C);
    gpio_set_function(GPIO_I2C0_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(GPIO_I2C0_SDA);
    gpio_pull_up(GPIO_I2C0_SCL);
}

#endif 