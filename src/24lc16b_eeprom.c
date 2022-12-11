#include "../include/24LC16B_EEPROM.h"

void lcb16b_eeprom_init(struct lcb16b_eeprom* my_eeprom){

    sleep_ms(1000); //Just standard thing to let everything settle
    uint8_t device_addr = LCB16B_ADDR << 3 | ((LCB16B_CHIP_ID_ADDR & 0x70) >> 8); //Get the MSB

    #if LCB16B_INIT
    //First write the needed chipID
    uint8_t write_buff[2];
    write_buff[0] = (LCB16B_CHIP_ID_ADDR & 0x0FF); //Only care about 8 LSB
    write_buff[1] = LCB16B_CHIP_ID;

    //Write the chip_ID
    lc16b_eeprom_i2c_write(device_addr,write_buff,2,false); //Stop after write
    sleep_ms(1000);
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

void print_eeprom_chip_ID(struct lcb16b_eeprom* my_eeprom){
    printf("For 24LC16B eeprom ChipID = %u \r\n",my_eeprom->chipID);
}