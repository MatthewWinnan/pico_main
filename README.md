# pico_main
Main repository to develop and test drivers for various micro controllers and devices. The aim is to make each generic enough to be used in any C based project.
Examples of usage is written for the PICO-SDK

The following are good resources to help out:
PICO-SDK documents: https://raspberrypi.github.io/pico-sdk-doxygen/index.html

The following drivers have been made:
1) BMP-180: Data-Sheet -> https://cdn-shop.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf

For usage in vscode it is important to have the following defined in your .bashrc config file:
1) export PICO_SDK_PATH=/home/matthew/Dev/PICO/pico-sdk
2) export PICO_EXAMPLES_PATH=/home/matthew/Dev/PICO/pico-examples
3) export PICO_EXTRAS_PATH=/home/matthew/Dev/PICO/pico-extras
4) export PICO_PLAYGROUND_PATH=/home/matthew/Dev/PICO/pico-playground

Also install the cmake and c/c++ config extentions for vscode. 
A good help guide video can be found at https://www.youtube.com/watch?v=B5rQSoOmR5w