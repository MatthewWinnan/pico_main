#ifndef __MAIN_H__
#define __MAIN_H__

// Define our major includes since we needn't repeat if we include this to main.c
// main.c is example code on how to implement the driver on a PICO W using the provided PICO SDK
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "src/bmp180.c"
#include "include/bmp180_i2c.h"
#include "boards/pico.h"

#endif