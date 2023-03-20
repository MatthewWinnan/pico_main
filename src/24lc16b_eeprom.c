#include "../include/24LC16B_EEPROM.h"
// #include "com_protocol.c"

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
            #if LCB16B_DEBUG
            printf("The 24LC16B eeprom was not configured correctly :(\r\n");
            printf("Obtained values. chipID = %d, expected = %d \r\n",read_buff[0],LCB16B_CHIP_ID);
            #endif
            sleep_ms(5000);
        }
    }

    //Save the chip ID and init starting conditions
    my_eeprom->chipID = read_buff[0];
    my_eeprom->pointer = LCB16B_START_REG;
    my_eeprom->src_len = 0;
    my_eeprom->dst_len = 0;

    #if LCB16B_DEBUG
    print_eeprom_chip_ID(my_eeprom);
    #endif
}

void lcb16b_eeprom_random_write(struct lcb16b_eeprom* my_eeprom) {
    //Uses the pointer stored in my_eeprom to point to where to write
    //Additionally increments the internal pointer

    #if LCB16B_DEBUG
    printf("Writing %u bytes starting from address %i \r\n", my_eeprom->src_len, my_eeprom->pointer);
    #endif

    // We need to see if a wrap around will take place. If it does it needs to be shifted to fall back into the valid range of registers.
    // len is only 8 bits so we never need to worry about double wrapping. Thus overflow can only ever be max 8 bits too.
    // We only need to know the amount it overflowed
    uint8_t overflow; 
    if ( (my_eeprom->pointer + my_eeprom->src_len) > LCB16B_STOP_REG){
        overflow = (my_eeprom->pointer + my_eeprom->src_len) % (LCB16B_STOP_REG);
    }
    else {
        overflow = 0;
    }

    uint8_t device_addr = return_device_address(my_eeprom->pointer);
    uint8_t * write_buffer = (uint8_t *) malloc((1 + (my_eeprom->src_len - overflow)) * sizeof(uint8_t)); // Allocate some buffer in memory. If overflowed we need to offset by that amount
    const uint8_t a[] = {my_eeprom->pointer & 0x0FF};
    memcpy(write_buffer, a, sizeof(uint8_t)); // Copy the content into memory
    memcpy(write_buffer + sizeof(uint8_t), my_eeprom->src, sizeof(uint8_t) * (my_eeprom->src_len - overflow)); //Copy src to memory. If overflow we need to offset len by that amount

    // Write the contents to EEPROM
    // If overflow we need to offset len by that amount
    lc16b_eeprom_i2c_write(device_addr,write_buffer,(my_eeprom->src_len - overflow)+1,false); //Stop after write
    sleep_ms(LCB16B_PAGE_WRITE_TIME_SAFETY * LCB16B_PAGE_WRITE_TIME);

    //Free malloc memory
    free(write_buffer);

    // Move pointer up, we have to do it in modulo space in order for wrap around to work
    if ( overflow != 0)
    {
        //Overflowed so offset by start
        my_eeprom->pointer = LCB16B_START_REG + (my_eeprom->pointer + (my_eeprom->src_len - overflow)) % (LCB16B_STOP_REG);

        #if LCB16B_DEBUG
        printf("Writing process has overflowed with %u bytes. Pointer moved to %u and writing remaining bytes \r\n", overflow, my_eeprom->pointer);
        #endif

        // Additionally in this case we know some of the data did not write. Recursively call the function to finish that left overs
        // Shift src by what was written
        my_eeprom->src += ((my_eeprom->src_len - overflow) * sizeof(uint8_t));
        my_eeprom->src_len = overflow;
        lcb16b_eeprom_random_write(my_eeprom);
    }
    else {
        my_eeprom->pointer = (my_eeprom->pointer + (my_eeprom->src_len - overflow)) % (LCB16B_STOP_REG);
    }

}

void lcb16b_eeprom_point_write(struct lcb16b_eeprom* my_eeprom, uint16_t reg){
    //Uses reg to point to where to write
    //Additionally increments the internal pointer

    // Sanity check to see if register is in the desired range
    // Implementation choice is made to move it to the first valid number, aka start register
    if ( reg < LCB16B_START_REG || reg > LCB16B_STOP_REG){
        #if LCB16B_DEBUG
        printf("Address %i not in allowed range [%u, %u]. Setting it to first valid index %u.\r\n", reg, LCB16B_START_REG, LCB16B_STOP_REG, LCB16B_START_REG);
        #endif

        reg = LCB16B_START_REG;
    }

    // Assing the pointer to register
    my_eeprom->pointer = reg;

    //We can now use random write fot the remaining functionality
    lcb16b_eeprom_random_write(my_eeprom);

}

void lcb16b_eeprom_random_read(struct lcb16b_eeprom* my_eeprom){
    // Uses the pointer stored in my_eeprom to point to where to start readin len amount of bytes
    // Additionally increments the internal pointer

    #if LCB16B_DEBUG
    printf("Reading %u bytes starting from address %i \r\n", my_eeprom->dst_len, my_eeprom->pointer);
    #endif

    // We need to see if a wrap around will take place. If it does it needs to be shifted to fall back into the valid range of registers.
    // len is only 8 bits so we never need to worry about double wrapping. Thus overflow can only ever be max 8 bits too.
    // We only need to know the amount it overflowed
    uint8_t overflow; 
    if ( (my_eeprom->pointer + my_eeprom->dst_len) > LCB16B_STOP_REG){
        overflow = (my_eeprom->pointer + my_eeprom->dst_len) % (LCB16B_STOP_REG);
    }
    else {
        overflow = 0;
    }

    uint8_t device_addr = return_device_address(my_eeprom->pointer);
    uint8_t addr = (my_eeprom->pointer & 0x0FF); //Only care about 8 LSB
    lc16b_eeprom_i2c_read(device_addr,&addr,my_eeprom->dst,(my_eeprom->dst_len - overflow),false);//Release control

    // Move pointer up, we have to do it in modulo space in order for wrap around to work
    if ( overflow != 0)
    {
        //Overflowed so offset by start
        my_eeprom->pointer = LCB16B_START_REG + (my_eeprom->pointer + (my_eeprom->dst_len - overflow)) % (LCB16B_STOP_REG);

        #if LCB16B_DEBUG
        printf("Reading process has overflowed with %u bytes. Pointer moved to %u and writing remaining bytes \r\n", overflow, my_eeprom->pointer);
        #endif

        // Addtionally in this case we know some of the data did not write. Recursively call the function to finish that left overs.
        // Shift dst by what was read.
        my_eeprom->dst = my_eeprom->dst + ((my_eeprom->dst_len - overflow) * sizeof(uint8_t));
        my_eeprom->dst_len = overflow;
        lcb16b_eeprom_random_read(my_eeprom);
    }
    else {
        my_eeprom->pointer = (my_eeprom->pointer + (my_eeprom->dst_len - overflow)) % (LCB16B_STOP_REG);
    }
}

void lcb16b_eeprom_point_read(struct lcb16b_eeprom* my_eeprom, uint16_t reg){
    //Uses reg to point to where to read
    //Additionally increments the internal pointer

    // Sanity check to see if register is in the desired range
    // Implementation choice is made to move it to the first valid number, aka start register
    if ( reg < LCB16B_START_REG || reg > LCB16B_STOP_REG){
        #if LCB16B_DEBUG
        printf("Address %i not in allowed range [%u, %u]. Setting it to first valid index %u.\r\n", reg, LCB16B_START_REG, LCB16B_STOP_REG, LCB16B_START_REG);
        #endif

        reg = LCB16B_START_REG;
    }

    // Point the pointer to where to read
    my_eeprom->pointer = reg;

    // We can then use random read to complete the functionality
    lcb16b_eeprom_random_read(my_eeprom);

}

// Creates space in memory and assigns the src value to an 8-bit buffer
void lcb16b_set_src(struct lcb16b_eeprom* my_eeprom, uint32_t src_value, bool time_stamp){
    // Check if some malloc has been used.
    // We know on init len is 0. Thus is malloc was used some memory was assigned thus not 0....
    // If time stamp is on. samples from RTC and adds a simple time stamp to the data
    if (my_eeprom->src_len != 0){
        // Free the pointer
        free(my_eeprom->src);
    }

    if (time_stamp){
        my_eeprom->src_len = 8;
    }
    else {
        my_eeprom->src_len = 4;
    }

    // Allocate memory
    my_eeprom->src =  malloc((1 + my_eeprom->src_len) * sizeof(uint8_t));

    // Add the values
    uint8_t offset = 0;

    if (time_stamp){
        // Get the RTC date and add to the src buffer
        sample_pico_rtc(&my_datetime);
        // Only add day, hour, minute and second
        my_eeprom->src[0] = my_datetime.day;
        my_eeprom->src[1] = my_datetime.hour;
        my_eeprom->src[2] = my_datetime.min;
        my_eeprom->src[3] = my_datetime.sec;
        offset = 4;
    }

    // Add the src data to src buffer
    // MSB first
    my_eeprom->src[offset] = src_value >> 24;
    my_eeprom->src[offset+1] = (src_value >> 16) & 0x00FF;
    my_eeprom->src[offset+2] = (src_value >> 8) & 0x0000FF;
    my_eeprom->src[offset+3] = src_value & 0x000000FF;
}

//Reads in some value to the 8-bit dst buffer in my_eeprom.
void lcb16b_set_dst(struct lcb16b_eeprom* my_eeprom, bool time_stamp){
    if (time_stamp){
        // We know the value will have some time stamp
        my_eeprom->dst_len = 8;
    }
    else {
        // Just a 32 bit number
        my_eeprom->dst_len = 4;
    }

    // Read
    lcb16b_eeprom_random_read(my_eeprom);
}