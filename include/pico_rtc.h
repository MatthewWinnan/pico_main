#ifndef __PICO_RTC_H__
#define __PICO_RTC_H__

#include <stdio.h>
#include "hardware/rtc.h"
#include "pico/stdlib.h"
#include "pico/util/datetime.h"

/*
Basic implementation of the pico RTC 
*/

#define RTC_DEBUG 1 // Set to 1 to allow for debug messages

//In order to use the pico RTC initialize the needed object
extern datetime_t my_datetime;

void init_pico_rtc(datetime_t * my_date);

void sample_pico_rtc(datetime_t * my_date);

#endif