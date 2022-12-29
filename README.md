# PICO_MAIN
Main repository to develop and test drivers for various micro controllers and devices. The aim is to make each generic enough to be used in any C based project.
Future plans for the project include:
1) Statistical fusion of multiple sensors.
2) Direct communication using a PS/2 Keyboard. (This will use the built COM_PROTOCOL framework)
3) Direct communication using a USB2.0 Keyboard. (This will use the built COM_PROTOCOL framework)
4) Generic UART serial communication to send data out for external analysis. (This will use the built COM_PROTOCOL framework)
5) Incremental sample storage using the eeprom drivers.
6) Sending live status of the sensors to an external device via the WIFI adapter. (Will need to see if this can be built into COM_PROTOCOL, or only separate)
7) Perform meteorological calculations. (Exact definitions and explination will follow in the WEATHER section)

# DRIVERS
Examples of usage is written for the PICO-SDK.

The following are good resources to help out:
PICO-SDK documents: https://raspberrypi.github.io/pico-sdk-doxygen/index.html

The following drivers have been made:
1) BMP-180: Data-Sheet -> https://cdn-shop.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf
2) 24LC16B: Data-Sheet -> https://pdf1.alldatasheet.com/datasheet-pdf/view/515802/MICROCHIP/24LC16B.html
3) BME-280: Data-Sheet -> https://cdn-shop.adafruit.com/datasheets/BST-BME280_DS001-10.pdf  
   Breakout Board -> https://www.mouser.com/datasheet/2/737/adafruit-bme280-humidity-barometric-pressure-tempe-740823.pdf
            

It is important to install the cmake and c/c++ config extentions for vscode. 
A good help guide video can be found at https://www.youtube.com/watch?v=B5rQSoOmR5w

For PICO-SDK usage in vscode it is important to have the following defined in your .bashrc config file:
1) export PICO_SDK_PATH=/home/matthew/Dev/PICO/pico-sdk
2) export PICO_EXAMPLES_PATH=/home/matthew/Dev/PICO/pico-examples
3) export PICO_EXTRAS_PATH=/home/matthew/Dev/PICO/pico-extras
4) export PICO_PLAYGROUND_PATH=/home/matthew/Dev/PICO/pico-playground

If you did that then you should be able to use the onboard Cmake file.

If one is only interested in using the drivers keep the following in mind:
1) Add any board specific I2C implementations to the corresponding device's *_i2c.h header implementation files (for example bmp180_i2c.c).
2) Add any board specific I2C initialization functions and values to i2c_config.c and i2c_config.h.
3) main.h shows the structure that need to be declared in order to start using the drivers, further they always need to be initialized.
4) The drivers are a package deal. As such in order to use a driver you need to include its .c, .h and all i2c specific files to your project.

# COM_PROTOCOL
This is designed to simulate the feel of working on a linux terminal. 
It provides a generic way to interact with the PICO and allows one to execute functions on demand.
Further documentation is provided in the header com_protocol.h.

Unlike the drivers this is PICO specific and will only work with the recommended PICO-SDK.

So far the following communication modes have been implemented:
1) USB - via the tinyUSB library and enabling pico_enable_stdio_usb(${PROJECT_NAME} 1) in the CMakeLists.txt in order to overwrite the stdin methods of C.
