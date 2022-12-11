#ifndef __24LC16B_EEPROM_I2C__
#define __24LC16B_EEPROM_I2C__
// This consists of the wrapper functionality meant to hold any specific I2C implementation one might need.
// This example is based off of the PICO SDK
// Documentation can be found at https://raspberrypi.github.io/pico-sdk-doxygen/index.html

#include <stdio.h>
#include "pico/stdlib.h"
#include "i2c_config.h"

// Define macros
#define LCB16B_WRITE_WAIT _u(20) //According to 24LC16B_DOC_1 page write time is 5 ms Max, so add lots of padding

int lc16b_eeprom_i2c_write(uint8_t addr, const uint8_t *src, size_t len, bool nostop){
    /*
    Attempt to write specified number of bytes to address, blocking
        Parameters:
        addr – 7-bit address of device to write to
        src – Pointer to data to send
        len – Length of data in bytes to send
        nostop – If true, master retains control of the bus at the end of the transfer (no Stop is issued), and the next transfer will begin with a Restart rather than a Start.
    */
    int answer =  i2c_write_blocking(I2C_PORT,addr,src,len,nostop); //Here I2C_PORT references the object that is used to implement I2C0 for the PICO W.
    //One should put any generic error handling here.
    if (answer == PICO_ERROR_GENERIC){
        printf("24LC16B_EEPROM Write to addr %i FAILED with PICO_ERROR_GENERIC 23.\r\n",addr);
    }
    else{
        printf("24LC16B_EEPROM Wrote %i to addr %i.\r\n",*src,addr);
        // Wait for everything to write properly
        sleep_ms(LCB16B_WRITE_WAIT);
    }
    return answer;
}

int lc16b_eeprom_i2c_read(uint8_t addr, const uint8_t *src, uint8_t *dst, size_t len, bool nostop){
    /*
    Attempt to read specified number of bytes from address, blocking
        Parameters:
        addr – 7-bit address of device to read from
        src – Pointer to data to send to initialize read
        dst – Pointer to buffer to receive data
        len – Length of data in bytes to receive
        nostop – If true, master retains control of the bus at the end of the transfer (no Stop is issued), and the next transfer will begin with a Restart rather than a Start.
    */

   //Remember for reading we first need to write addr we wish to read
    int answer = lc16b_eeprom_i2c_write(addr,src,1,true); //In general we want to keep control since we need to write next. Also in general address would be 1 byte.
   //One should put any generic error handling here.
    if (answer == PICO_ERROR_GENERIC){
        printf("24LC16B_EEPROM Write to addr %i FAILED with PICO_ERROR_GENERIC  46.\r\n",addr);
        return answer;
    }

    //Now we read the response
    answer = i2c_read_blocking(I2C_PORT,addr,dst,len,nostop);
    //One should put any generic error handling here.
    if (answer == PICO_ERROR_GENERIC){
        printf("24LC16B_EEPROM Read from addr %i FAILED with PICO_ERROR_GENERIC.\r\n",*src);
    }
    return answer;
}

#endif