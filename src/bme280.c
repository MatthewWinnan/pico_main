#include "../include/bme280.h"
#include "../include/com_protocol.h"

void read_bme280_chip_id(struct bme280_model *my_chip){
    uint8_t chipID[1];
    uint8_t addr = BME_280_CHIP_ID_ADDR;

    bme280_i2c_read(BME_280_ADDR,&addr,chipID,1,false);

    if (chipID[0] != BME_280_CHIP_ID){
        while (true){
            #if BME_280_INFO_MODE
            printf("BME280 was not configured correctly :(.\r\n");
            printf("Obtained values. chipID = %d, expected = %d \r\n",chipID[0],BME_280_CHIP_ID);
            #endif
            sleep_ms(5000);
        }   
    }
    else {
        #if BME_280_INFO_MODE
        printf("BME280 was configured correctly :)\r\n");
        printf("Obtained values. chipID = %d, expected = %d \r\n",chipID[0],BME_280_CHIP_ID);
        #endif
    }

    // Assign the ID
    my_chip->chipID = chipID[0];
}

void read_bme280_callibration_params(struct bme280_model *my_chip, struct bme280_calib_param *params){
    // Reading in the callibration data as defined in BME280_DOC_22 and BME280_DOC_23
    // There is no reason to do it like this but I like to separate logic.....
    // First read in temperature values
    uint8_t rx_temp_buffer[6] = {0};
    uint8_t addr = BME_280_REG_T1_LSB;
    bme280_i2c_read(BME_280_ADDR,&addr,rx_temp_buffer,6,false);
    // The values are split [7:0]/[15:8] thus LSB is first
    params->dig_T1 = (uint16_t) (rx_temp_buffer[1] << 8) | rx_temp_buffer[0];
    params->dig_T2 = (int16_t) (rx_temp_buffer[3] << 8) | rx_temp_buffer[2]; 
    params->dig_T3 = (int16_t) (rx_temp_buffer[5] << 8) | rx_temp_buffer[4];  

    // Read pressure values
    uint8_t rx_pressure_buff[18];
    addr = BME_280_REG_P1_LSB;
    bme280_i2c_read(BME_280_ADDR,&addr,rx_pressure_buff,18,false);
    // The values are split [7:0]/[15:8] thus LSB is first
    params->dig_P1 = (uint16_t) (rx_pressure_buff[1] << 8) | rx_pressure_buff[0];
    params->dig_P2 = (int16_t) (rx_pressure_buff[3] << 8) | rx_pressure_buff[2];
    params->dig_P3 = (int16_t) (rx_pressure_buff[5] << 8) | rx_pressure_buff[4];
    params->dig_P4 = (int16_t) (rx_pressure_buff[7] << 8) | rx_pressure_buff[6];
    params->dig_P5 = (int16_t) (rx_pressure_buff[9] << 8) | rx_pressure_buff[8];
    params->dig_P6 = (int16_t) (rx_pressure_buff[11] << 8) | rx_pressure_buff[10];
    params->dig_P7 = (int16_t) (rx_pressure_buff[13] << 8) | rx_pressure_buff[12];
    params->dig_P8 = (int16_t) (rx_pressure_buff[15] << 8) | rx_pressure_buff[14];
    params->dig_P9 = (int16_t) (rx_pressure_buff[17] << 8) | rx_pressure_buff[16];

    // Read humidity values
    // This one is rather weird....
    uint8_t reg_h1[1];
    addr = BME_280_REG_H1;
    bme280_i2c_read(BME_280_ADDR,&addr,reg_h1,1,false);
    // First value is splt [7:0]
    params->dig_H1 = (uint8_t) reg_h1[0];
    // We then need to go to another part of storage and read the rest
    // The weird split rules are defined at BME280_DOC_23
    uint8_t rx_humidity_buff[7];
    addr = BME_280_REG_H2_LSB;
    bme280_i2c_read(BME_280_ADDR,&addr,rx_humidity_buff,8,false);
    // Split [7:0]/[15:8]
    params->dig_H2 = (int16_t) (rx_humidity_buff[1] << 8) | rx_humidity_buff[0];
    // Split [7:0]
    params->dig_H3 = (uint8_t) rx_humidity_buff[2];
    // Split 0xE4/0xE5[3:0] or [11:4]/[3:0] so here the MSB is first and the LSB nibble second
    // Use 0000 1111 as a bit mask to get the [3:0] nibble
    params->dig_H4 = (int16_t) (rx_humidity_buff[3] << 4) | (0x0F & rx_humidity_buff[4]);
    // Split 0xE5[7:4]/0xE6 or [3:0]/[11:4] so here the LSB nibble is first
    // Use 1111 0000 as a bit mask to get the [7:4] nibble
    params->dig_H5 = (int16_t) ((rx_humidity_buff[4] & 0xF0) << 4) | rx_humidity_buff[5];
    // Split [7:0]
    params->dig_H6 = (int8_t) rx_humidity_buff[6];

    // Assign to the chip model
    my_chip->cal_params = params;

    // Debug Lines
    #if BME_280_INFO_MODE
    print_cal_params_bme280(my_chip);
    #endif
}

void bme280_init(struct bme280_model *my_chip, struct bme280_calib_param *params){
    // Wait for startup
    sleep_ms(BME_280_STARTUP_T);

    // Read in chip id
    read_bme280_chip_id(my_chip);

    // Read in cal params
    read_bme280_callibration_params(my_chip,params);
}