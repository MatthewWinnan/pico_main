#ifndef __COM_PROTOCOL_H__
#define __COM_PROTOCOL_H__

//Define major includes
#include <string.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "boards/pico_w.h"
#include "bmp180.h"
#include "24LC16B_EEPROM.h"
#include "pico/util/queue.h"
#include "/home/matthew/Dev/PICO/pico-sdk/src/host/pico_multicore/include/pico/multicore.h"

/*
TODO:
Fix multicore and pico_w includes not showing up through the linter.
Add checks to keep cmd's arrays within max and min array indices.
*/

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

The general structure of commands are modeled after Linux terminal commands.
Where one calls a key word (in the kernel case the binary), and feed options to it.
Options are fed by prefixing some str/char with -
If a str is prefixed by - then each char will be a separate option

For example:
bmp180 -h -> Executes bmp180 with argument h.
bmp180 -hs -> Executes bmp180 with argument h and s.
bmp180 -h -s -> Executes bmp180 with argument h and s.

For argument values this holds :
<command> -a -r 40 60 -> Executes command with argument a which has an input value of 40. And argument r with input 60.
<command> -ar 40 60 -> Executes command with argument a which has an input value of 40. And argument r with input 60.
<command> -a 40 -r 60 -> Executes command with argument a which has an input value of 40. And argument r with input 60.

On default of no option is given the help function will be printed.

NB as a word of caution, specific trumps general. The examples are the general case one would find and describes 
in general most commands' functionality.
If a callable function has some specific use cases or formatting requirements these will be stipulated at the corresponding 
function decleration and should be used instead.
The default help will also give the user a nudge.

The following will document the commands that can be sent to the pico.

help: Provides a basic list of key commands that can be sent.

*/

#define USE_USB 1 // Will tinyUSB be used as the main communications?
#define COM_PROTO_DEBUG 1 // Will USB debug be printed out?
#define COM_PROTO_RX_WAIT _u(1000000) //Wait for COM_PROTO_RX_WAIT us for some buffer input
#define COM_PROTO_RX_BUFFER_SIZE _u(1024) // Buffer size for stdin
#define COM_PROTO_ARG_ARRAY_SIZE _u(10) // How many arguments of str can I store at a time
#define COM_PROTO_COMMAND_SIZE _u(100) //max char size of a given command

// Main variables
// Declare a command structure
struct cmd{
    // Holds command
    char command[COM_PROTO_COMMAND_SIZE];
    // Holds command length
    uint8_t cmd_len;
    // Holds char arguments
    char args[COM_PROTO_ARG_ARRAY_SIZE];
    // Holds amount of arguments
    uint8_t arg_len;
    // Holds the integer arguments. The index of each corresponds to the index of the main arg.
    // For example command 24lc16b -a 50 -w 100 would write 100 to address 50
    uint32_t int_arg[COM_PROTO_ARG_ARRAY_SIZE];
    // Holds current list of argument values, must be equal or smaller than arg_len
    uint8_t int_arg_len;
    // Temp array to help the integer value to be converted to integer (weird sentence)
    uint8_t int_tmp[COM_PROTO_ARG_ARRAY_SIZE];
    // Hold the n amount of numbers present
    uint8_t int_tmp_len;
};

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

// Calculate powers of 10
uint16_t pow_10(uint8_t exponent);

// Reads characters to buffer
uint16_t read_stdin(char *buffer);
// Read characters to buffer based on the USB interface
uint16_t read_stdin_usb(char *buffer);

// Waits until the rx buffer is clean
void clean_rx_buff();
// Cleans the stdin buffer sent in as input
void clean_stdin(char *buffer, uint16_t *len);

// Reads string data and formats it into cmd terms
void read_stdin_to_cmd(char *std_in, uint16_t *len, struct cmd* cmd_line);
// Cleans the cmd_line structure
void clean_cmd_line(struct cmd* cmd_line);

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