#ifndef __COM_PROTOCOL_H__
#define __COM_PROTOCOL_H__

//Define major includes
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "boards/pico_w.h"
#include "bmp180.h"
#include "24LC16B_EEPROM.h"
#include "pico/util/queue.h"
#include "/home/matthew/Dev/PICO/pico-sdk/src/host/pico_multicore/include/pico/multicore.h"

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

#define USE_USB 1 // Will tinyUSB be used as the main communications?
#define COM_PROTO_DEBUG 1 // Will USB debug be printed out?
#define COM_PROTO_RX_WAIT _u(10000) //Wait for COM_PROTO_RX_WAIT us for some buffer input
#define COM_PROTO_RX_BUFFER_SIZE _u(1024) // Buffer size for stdin

// Main variables
// Declare a queue entry
typedef struct
{
    void *func;
    int32_t data;
} queue_entry_t;

// Define our queues to be used

// com_protocol will add the needed entry to be used by main
queue_t call_queue;
// Here results are added by main to be printed by com_protocol
queue_t results_queue;

// Define helpers

// Reads characters to buffer
uint16_t read_stdin(char *buffer);

// Waits until the rx buffer is clean
void clean_rx_buff();

// Define main functions

// Initializer
void com_protocol_init();

// Main entry loop
void com_protocol_entry();

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