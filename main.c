#include "main.h"
//TODO
//Define better non blocking sleep function
//Rather than store as file structures we define a struct to be called by serial queries.

void toggle_led(uint8_t* led_state) {

    //Update the passed state
    *led_state = (*led_state+1)%2;
    //Update the current LED
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, *led_state);

}


void program_init(){
    // Initialize all standard stdio types linked with binary.
    stdio_init_all();

    // Initialize cyw43 module to allow us to blink the onboard LED.
    if (cyw43_arch_init()) {
        printf("WiFi init failed");
        return;
    }
    //Init I2C
    global_i2c_init();

    //Init BMP180
    bmp180_init(&my_bmp180, &calib_params, &my_bmp180_measurements);

    //Init the eeprom
    lcb16b_eeprom_init(&my_eeprom);
}

int main() {

    //Init stuff
    program_init();

    // Holds the current state of the blink    
    uint8_t LED_STATE = 0;

    // Main stuff....
    while (true) {
        sleep_ms(2000);
        toggle_led(&LED_STATE);
        // Take measurements
        bmp180_get_measurement(&my_bmp180);
    }
}


