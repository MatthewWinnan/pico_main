#include "../include/bmp180_i2c.h"

int bmp180_i2c_write(uint8_t addr, const uint8_t *src, size_t len, bool nostop){
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
        printf("BMP180 Write to addr %i FAILED with PICO_ERROR_GENERIC.",addr);
    }
    return answer;
}

int bmp180_i2c_read(uint8_t addr, const uint8_t *src, uint8_t *dst, size_t len, bool nostop){
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
    int answer = bmp180_i2c_write(addr,src,1,true); //In general we want to keep control since we need to write next. Also in general address would be 1 byte.
   //One should put any generic error handling here.
    if (answer == PICO_ERROR_GENERIC){
        printf("BMP180 Write to addr %i FAILED with PICO_ERROR_GENERIC.",addr);
        return answer;
    }

    //Now we read the response
    answer = i2c_read_blocking(I2C_PORT,addr,dst,len,nostop);
    //One should put any generic error handling here.
    if (answer == PICO_ERROR_GENERIC){
        printf("BMP180 Read from addr %i FAILED with PICO_ERROR_GENERIC.",*src);
    }
    return answer;
}