#include "../include/pico_rtc.h"

// Initialize the variables here
datetime_t my_datetime;

void init_pico_rtc(datetime_t * my_date){
    /*
    We only really need some day,hour,min,sec basis.
    The year and day of week is not needed.
    TODO:
    Would be nice to have some way to automate this process.
    */

    my_date->year = 2022;
    my_date->month = 12;
    my_date->dotw = 6; // 0 is Sunday
    my_date->day = 24;
    my_date->hour = 12;
    my_date->min = 24;
    my_date->sec = 50;

    // Start the RTC
    rtc_init();
    bool res = rtc_set_datetime(my_date);

    // Debugging code
    #if RTC_DEBUG
    // Allocate string variables
    char datetime_buf[256];
    char *datetime_str = &datetime_buf[0];
    if (res){
        //datetime_to_str(datetime_str, sizeof(datetime_buf), &my_date);
        //printf("RTC has been set correctly to %s. \r\n",datetime_str);
        printf("TODO RTC updated chenge the datetime_to_string command.");
    }
    else {
        printf("Warning RTC could not be set.");
    }
    #endif
  
}

void sample_pico_rtc(datetime_t * my_date){
    // Just a nice wrapper :D
    rtc_get_datetime(my_date);
}