#ifndef __COM_PROTOCOL_H__
#define __COM_PROTOCOL_H__

//Define major includes
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "boards/pico_w.h"
#include "bmp180.h"
#include "24LC16B_EEPROM.h"

/*
This will serve as the main interface to be used for general communication between the PICO and the outside world.
This will be ran on core1 in a super loop.
The general principle will be that it will read in some input, based on that make a decision as to what needs to be executed by main.
The function will be passed to main via FIFO as well as the needed structure.
Main will execute what is needed and pass the output print function and the main structure back to com_protcol to be print.
In this header there will be compile flags that decide what communication method the main TX and RX functions will be using.

Further DEBUG as defined in each driver will always depend on USB and will not pass instructions to core1.
The idea here is that it is DEBUG after all.

Additionally the I2C libraries have their own error messages for debugging.
*/

// Will tinyUSB be used as the main communications?
#define USE_USB 1


// Define bulk printing functions here

// Printing functions for the BMP180

void print_temp_results(struct bmp180_model* my_chip);
void print_press_results(struct bmp180_model* my_chip);
void print_altitude_results(struct bmp180_model* my_chip);
void print_relative_pressure_results(struct bmp180_model* my_chip);
void print_chip_ID(struct bmp180_model* my_chip);
void print_cal_params(struct bmp180_model* my_chip);

// Printing functions for the 24LC16B

void print_eeprom_chip_ID(struct lcb16b_eeprom* my_eeprom);

#endif