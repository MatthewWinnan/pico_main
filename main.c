#include "main.h"
//TODO
//Define better non blocking sleep function

void toggle_led(uint8_t* led_state) {

    //Update the passed state
    *led_state = (*led_state+1)%2;
    //Update the current LED
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, *led_state);

}

// Main init function to initialize all periphirals on program start
void program_init(){
    // Initialize all standard stdio types linked with binary.
    stdio_init_all();

    // Initialize cyw43 module to allow us to blink the onboard LED.
    if (cyw43_arch_init()) {
        printf("WiFi init failed");
        return;
    }
    printf("CYW43 WiFi init success");
    //Init I2C
    global_i2c_init();

    //Init BMP180
    bmp180_init(&my_bmp180, &calib_params, &my_bmp180_measurements);

    //Init the eeprom
    lcb16b_eeprom_init(&my_eeprom);

    //Init the com protocol
    com_protocol_init();

    //Init the RTC
    init_pico_rtc(&my_datetime);
}

int main() {

    //Init stuff
    program_init();

    // Holds the current state of the blink    
    uint8_t LED_STATE = 0;

    // Allocate some space to store a queue entry
    queue_entry_t call_queue_entry;


    // Main stuff....
    while (true) {
        sleep_ms(2000);
        toggle_led(&LED_STATE);

        // Execute anything that might be en the queue
        // Check if any entry exists in the result queue
        if (queue_is_empty(&call_queue))
        {
            // Empty. For debugging tell me
            #if MAIN_DEBUG
            printf("Call queue is empty.\r\n");
            #endif
        }
        else{
            // Read in the function and input from stdin
            queue_remove_blocking(&call_queue, &call_queue_entry);
            void (*stdin_func)() = (void(*)())(call_queue_entry.func);
            void *std_in_value = call_queue_entry.data ;
            // Execute the function
            (*stdin_func)(std_in_value);
            // Add response to the result queue (stdout)
            int res = stdout_selector(call_queue_entry.func);
        }
    }
}


