#include "../include/24LC16B_EEPROM.h"

uint8_t return_device_address(uint16_t register_address){
    /*
    From 24LC16B_DOC_7
    The next three bits of the controlbyte are the block-select bits (B2, B1, B0)
    Thus for some 11-bit register address we need to append the 3 MSB bits to the device control address

    This is done by shifting the 4-bit LCB16B_ADDR 3 spaces to the left then OR with MSB 11,10 and 9 of register_address
    */
    return LCB16B_ADDR << 3 | ((register_address & 0x70) >> 8); //Get the MSB
}

void lcb16b_eeprom_init(struct lcb16b_eeprom* my_eeprom){

    sleep_ms(1000); //Just standard thing to let everything settle after powering on the EEPROM
    uint8_t device_addr = return_device_address(LCB16B_CHIP_ID_ADDR);

    #if LCB16B_INIT
    //First write the needed chipID
    uint8_t write_buff[2];
    write_buff[0] = (LCB16B_CHIP_ID_ADDR & 0x0FF); //Only care about 8 LSB
    write_buff[1] = LCB16B_CHIP_ID;

    //Write the chip_ID
    lc16b_eeprom_i2c_write(device_addr,write_buff,2,false); //Stop after write
    sleep_ms(LCB16B_PAGE_WRITE_TIME_SAFETY * LCB16B_PAGE_WRITE_TIME);
    #endif

    //We now read the chip ID
    uint8_t read_buff[1];
    uint8_t addr = (LCB16B_CHIP_ID_ADDR & 0x0FF); //Only care about 8 LSB
    lc16b_eeprom_i2c_read(device_addr,&addr,read_buff,1,false);//Release control

    // Is chip ID correct
    if (read_buff[0] != LCB16B_CHIP_ID)
    {
        while(true){
            printf("The 24LC16B eeprom was not configured correctly :(\r\n");
            printf("Obtained values. chipID = %d, expected = %d \r\n",read_buff[0],LCB16B_CHIP_ID);
            sleep_ms(5000);
        }
    }

    //Save the chip ID and init starting conditions
    my_eeprom->chipID = read_buff[0];
    my_eeprom->pointer = LCB16B_START_REG;

    #if LCB16B_DEBUG
    print_eeprom_chip_ID(my_eeprom);
    #endif
}

void lcb16b_eeprom_random_write(struct lcb16b_eeprom* my_eeprom, const uint8_t *src, uint8_t len) {
    //Uses the pointer stored in my_eeprom to point to where to write
    //Additionally increments the internal pointer

    #if LCB16B_DEBUG
    printf("Writing %u bytes starting from address %i \r\n", len, my_eeprom->pointer);
    #endif

    uint8_t device_addr = return_device_address(my_eeprom->pointer);
    uint8_t * write_buffer = (uint8_t *) malloc((1 + len) * sizeof(uint8_t)); // Allocate some buffer in memory
    const uint8_t a[] = {my_eeprom->pointer & 0x0FF};
    memcpy(write_buffer, a, sizeof(uint8_t)); // Copy the content into memory
    memcpy(write_buffer + sizeof(uint8_t), src, sizeof(uint8_t) * len); //Copy src to memory

    //Write the contents to EEPROM
    lc16b_eeprom_i2c_write(device_addr,write_buffer,len+1,false); //Stop after write
    sleep_ms(LCB16B_PAGE_WRITE_TIME_SAFETY * LCB16B_PAGE_WRITE_TIME);

    //Free malloc memory
    free(write_buffer);

    // Move pointer up, we have to do it in modulo space in order for wrap around to work
    my_eeprom->pointer = (my_eeprom->pointer + len) % (LCB16B_STOP_REG - LCB16B_START_REG);

}

void lcb16b_eeprom_random_read(struct lcb16b_eeprom* my_eeprom, const uint8_t *dst, uint8_t len){
    // Uses the pointer stored in my_eeprom to point to where to start readin len amount of bytes
    //Additionally increments the internal pointer

    #if LCB16B_DEBUG
    printf("Reading %u bytes starting from address %i \r\n", len, my_eeprom->pointer);
    #endif

    uint8_t device_addr = return_device_address(my_eeprom->pointer);
    uint8_t read_buff[len];
    uint8_t addr = (my_eeprom->pointer & 0x0FF); //Only care about 8 LSB
    lc16b_eeprom_i2c_read(device_addr,&addr,read_buff,len,false);//Release control

    // Move pointer up, we have to do it in modulo space in order for wrap around to work
    my_eeprom->pointer = (my_eeprom->pointer + len) % (LCB16B_STOP_REG - LCB16B_START_REG);
}

void print_eeprom_chip_ID(struct lcb16b_eeprom* my_eeprom){
    printf("For 24LC16B eeprom ChipID = %u \r\n",my_eeprom->chipID);
}