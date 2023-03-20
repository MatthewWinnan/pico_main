# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.12)

# Include build functions from Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Set name of project (as PROJECT_NAME) and C/C%2B%2B standards
project(pico_drivers C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Creates a pico-sdk subdirectory in our project for the libraries
pico_sdk_init()

#Create libraries
include_directories(include
src)

# Tell CMake where to find the executable source file

add_executable(${PROJECT_NAME} 
    main.c
    src/bmp180.c
    src/24lc16b_eeprom.c
    src/com_protocol.c
    src/bmp180_i2c.c
    src/24LC16B_EEPROM_i2c.c
    src/i2c_config.c
    src/pico_rtc.c
    src/bme280.c
    src/bme280_i2c.c
)

#Create libraries
# target_include_directories(${PROJECT_NAME} PUBLIC
# include
# src)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(${PROJECT_NAME})

# add url via pico_set_program_url
#example_auto_set_url(${PROJECT_NAME})

# Link to pico_stdlib (gpio, time, etc. functions)
# Link to pico_cyw43_arch_none (for the cyw43 module)
# Link to pico_multicore 
# Link to hardware_i2c (for i2c communications)
# Link to hardware_rtc for the RTC functionality
target_link_libraries(${PROJECT_NAME} 
    pico_stdlib
    pico_cyw43_arch_none
    pico_multicore
    hardware_i2c
    hardware_rtc
)

# Enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

