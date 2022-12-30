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

void bme280_init(struct bme280_model *my_chip, struct bme280_calib_param *params, struct bme280_settings *settings){
    // Wait for startup
    sleep_ms(BME_280_STARTUP_T);

    // Read in chip id
    read_bme280_chip_id(my_chip);

    // Read in cal params
    read_bme280_callibration_params(my_chip,params);

    // Loading up the values for the lookup arrays
    // Debug Lines
    #if BME_280_INFO_MODE
    printf("[BME280]: Loading mode registers .... \r\n");
    #endif

    bme280_osrs_h_mode_array[0] = BME_280_OSRS_H_1;
    bme280_osrs_h_mode_array[1] = BME_280_OSRS_H_2;
    bme280_osrs_h_mode_array[2] = BME_280_OSRS_H_3;
    bme280_osrs_h_mode_array[3] = BME_280_OSRS_H_4;
    bme280_osrs_h_mode_array[4] = BME_280_OSRS_H_5;
    bme280_osrs_h_mode_array[5] = BME_280_OSRS_H_6;

    bme280_osrs_p_mode_array[0] = BME_280_OSRS_P_1;
    bme280_osrs_p_mode_array[1] = BME_280_OSRS_P_2;
    bme280_osrs_p_mode_array[2] = BME_280_OSRS_P_3;
    bme280_osrs_p_mode_array[3] = BME_280_OSRS_P_4;
    bme280_osrs_p_mode_array[4] = BME_280_OSRS_P_5;
    bme280_osrs_p_mode_array[5] = BME_280_OSRS_P_6;

    bme280_osrs_t_mode_array[0] = BME_280_OSRS_T_1;
    bme280_osrs_t_mode_array[1] = BME_280_OSRS_T_2;
    bme280_osrs_t_mode_array[2] = BME_280_OSRS_T_3;
    bme280_osrs_t_mode_array[3] = BME_280_OSRS_T_4;
    bme280_osrs_t_mode_array[4] = BME_280_OSRS_T_5;
    bme280_osrs_t_mode_array[5] = BME_280_OSRS_T_6;

    bme280_t_sb_mode_array[0] = BME_280_T_SB_1;
    bme280_t_sb_mode_array[1] = BME_280_T_SB_2;
    bme280_t_sb_mode_array[2] = BME_280_T_SB_3;
    bme280_t_sb_mode_array[3] = BME_280_T_SB_4;
    bme280_t_sb_mode_array[4] = BME_280_T_SB_5;
    bme280_t_sb_mode_array[5] = BME_280_T_SB_6;
    bme280_t_sb_mode_array[6] = BME_280_T_SB_7;
    bme280_t_sb_mode_array[7] = BME_280_T_SB_8;

    bme280_filter_mode_array[0] = BME_280_FILTER_1;
    bme280_filter_mode_array[1] = BME_280_FILTER_2;
    bme280_filter_mode_array[2] = BME_280_FILTER_3;
    bme280_filter_mode_array[3] = BME_280_FILTER_4;
    bme280_filter_mode_array[4] = BME_280_FILTER_5;

    bme280_t_sb_timing_array[0] = BME_280_T_SB_TIMING_1;
    bme280_t_sb_timing_array[1] = BME_280_T_SB_TIMING_2;
    bme280_t_sb_timing_array[2] = BME_280_T_SB_TIMING_3;
    bme280_t_sb_timing_array[3] = BME_280_T_SB_TIMING_4;
    bme280_t_sb_timing_array[4] = BME_280_T_SB_TIMING_5;
    bme280_t_sb_timing_array[5] = BME_280_T_SB_TIMING_6;
    bme280_t_sb_timing_array[6] = BME_280_T_SB_TIMING_7;
    bme280_t_sb_timing_array[7] = BME_280_T_SB_TIMING_8;

    // Debug Lines
    #if BME_280_INFO_MODE
    printf("[BME280]: Loading mode registers. Done :) \r\n");
    printf("[BME280]: Setting initial conditions ... \r\n");
    #endif

    // Set the mode settings to be called by setters
    settings->mode = BME_280_FILTER_MODE;
    settings->osrs_h = bme280_osrs_h_mode_array[BME_280_OSRS_H_MODE];
    settings->osrs_p = bme280_osrs_h_mode_array[BME_280_OSRS_P_MODE];
    settings->osrs_t = bme280_osrs_h_mode_array[BME_280_OSRS_T_MODE];
    settings->filter = bme280_filter_mode_array[BME_280_FILTER_MODE];
    settings->t_sb = bme280_t_sb_mode_array[BME_280_T_SB_MODE];
    settings->spi3w_en = BME_280_SPI3W_EN;

    // Save this
    my_chip->settings = settings;

    // Now set the initial conditions
    bme280_set_config(my_chip);
    bme280_set_ctrl_hum(my_chip); // because set_ctrl_hum also needs to set the ctrl_meas to take effect we only need to call this.

    // Debug lines
    #if BME_280_INFO_MODE
    printf("[BME280]: Initial conditions set :D \r\n");
    #endif
}

// Define setter functions
void bme280_set_config(struct bme280_model *my_chip){
    /*
    An important not is found in BME_280_DOC_27.
    "The “config” register sets the rate, filter and interface options of the device. Writes to the “config”
    register in normal mode may be ignored. In sleep mode writes are not ignored."
    As such for consistency we do the follow:
    1) Am I in sleep?
    2) If not store current mode and set to sleep
    3) Update the config
    4) If we were not in sleep, change the device mode back to what it was at 1)

    We also have the following consideration noted at BME_280_DOC_12
    "The supported mode transitions are shown in Figure 3. If the device is currently performing a
    measurement, execution of mode switching commands is delayed until the end of the currently
    running measurement period. Further mode change commands or other write commands to the
    register ctrl_hum are ignored until the mode change command has been executed."

    Thus in the mode switch function just wait for measurements to finish.
    */
    // Get the ctrl reg value
    bme280_read_ctrl_meas(my_chip);

    uint8_t tmp_mode = my_chip->settings->mode; // Store the current mode value
    // Case 1 mode is at [1:0] BME280_DOC_25
    if ( tmp_mode != 0 )
    {
        // Case 2 set to sleep
        my_chip->settings->mode = 0;
        bme280_set_ctrl_meas(my_chip);
    }

    // Case 3 update the set_config
    // Assing our write register and addresses
    uint8_t write_buffer[2];
    write_buffer[0] = BME_280_REG_CONFIG;
    /*
    From BME280_DOC_28:
    t_sb is bits [7:5]
    filter is bits [4:2]
    spi3w_en is bit 0
    */
    write_buffer[1] = (uint8_t) (((my_chip->settings->t_sb << 5) & 0xE0) | ((my_chip->settings->filter << 2) & 0x1C) | (my_chip->settings->spi3w_en & 0x01));
    // Now update the config
    bme280_i2c_write(BME_280_ADDR,write_buffer,2,false);

    if ( tmp_mode != 0 ){
        // Case 4 we need to switch back to the original mode
        my_chip->settings->mode = tmp_mode;
        bme280_set_ctrl_meas(my_chip);
    }
}

void bme280_set_ctrl_meas(struct bme280_model *my_chip){
    /*
    We have the following consideration noted at BME_280_DOC_12
    "The supported mode transitions are shown in Figure 3. If the device is currently performing a
    measurement, execution of mode switching commands is delayed until the end of the currently
    running measurement period. Further mode change commands or other write commands to the
    register ctrl_hum are ignored until the mode change command has been executed."

    The status register can be read to determine if the chip is busy doing conversions.
    */

    // Assing our write register and addresses
    uint8_t write_buffer[2];
    write_buffer[0] = BME_280_REG_CTRL_MEAS;
    write_buffer[1] = (uint8_t) ((my_chip->settings->mode & 0x03) | ((my_chip->settings->osrs_p << 2) & 0x1C) | ((my_chip->settings->osrs_t << 5) & 0xE0));

    // First wait for measurements to stop
    while (bme280_is_doing_conversion()){
    sleep_us(10);
    }

    // Now update the mode
    bme280_i2c_write(BME_280_ADDR,write_buffer,2,false);
}

void bme280_set_ctrl_hum(struct bme280_model *my_chip){
    /*
    Here the following should be considered from BME280_DOC_25
    "Changes to this register only become effective after a write operation to “ctrl_meas”"
    */

   // First write the new ctrl hum value 
   // Assing our write register and addresses
    uint8_t write_buffer[2];
    write_buffer[0] = BME_280_REG_CTRL_HUM;
    write_buffer[1] = (uint8_t) (my_chip->settings->osrs_h & 0x07);
    // Now update the mode
    bme280_i2c_write(BME_280_ADDR,write_buffer,2,false);

    // In order to make changes stick send write to ctrl_meas
    bme280_set_ctrl_meas(my_chip);
}

// Define getter functions
void bme280_read_ctrl_meas(struct bme280_model *my_chip){
    /*
    Allocates the correspondings values to the model settings.
    Each bit in the register represents some setting value,
    these are obtained from BME280_DOC_27
    */
    uint8_t addr = BME_280_REG_CTRL_MEAS;
    uint8_t reg[0];
    bme280_i2c_read(BME_280_ADDR,&addr,reg,1,false);

    // Debug lines
    #if BME_280_DEBUG_MODE
    printf("CTRL_REG = %u \r\n",reg[0]);
    #endif

    // Mode is [1:0]
    my_chip->settings->mode = (uint8_t) reg[0] & 0x03;
    // osrs_p is [4:2]
    my_chip->settings->osrs_p = (uint8_t) ((reg[0] >> 2) & 0x07); // 0x07 = 0b00000111
    // osrs_t is [7:5]
    my_chip->settings->osrs_t = (uint8_t) ((reg[0] >> 5) & 0x07); // 0x07 = 0b00000111

    // Debug lines
    #if BME_280_DEBUG_MODE
    printf("BME280 mode = %u\r\n",my_chip->settings->mode);
    printf("BME280 osrs_p = %u\r\n",my_chip->settings->osrs_p);
    printf("BME280 osrs_t = %u\r\n",my_chip->settings->osrs_t);
    #endif
}

void bme280_read_config(struct bme280_model *my_chip){
    /*
    Allocates the correspondings values to the model settings.
    Each bit in the register represents some config value,
    these are obtained from BME280_DOC_28
    */
    uint8_t addr = BME_280_REG_CONFIG;
    uint8_t reg[0];
    bme280_i2c_read(BME_280_ADDR,&addr,reg,1,false);

    // Debug lines
    #if BME_280_DEBUG_MODE
    printf("CONFIG_REG = %u \r\n",reg[0]);
    #endif

    // spi3w_en is [0]
    my_chip->settings->spi3w_en = (uint8_t) reg[0] & 0x01;
    // filter is [4:2]
    my_chip->settings->filter = (uint8_t) ((reg[0] >> 2) & 0x07); // 0x07 = 0b00000111
    // t_sb is [7:5]
    my_chip->settings->t_sb = (uint8_t) ((reg[0] >> 5) & 0x07); // 0x07 = 0b00000111

    // Debug lines
    #if BME_280_DEBUG_MODE
    printf("BME280 spi3w_en = %u\r\n",my_chip->settings->spi3w_en);
    printf("BME280 filter = %u\r\n",my_chip->settings->filter);
    printf("BME280 t_sb = %u\r\n",my_chip->settings->t_sb);
    #endif
}

void bme280_read_ctrl_hum(struct bme280_model *my_chip){
    /*
    Allocates the correspondings values to the model settings.
    Each bit in the register represents some config value,
    these are obtained from BME280_DOC_26
    */
    uint8_t addr = BME_280_REG_CTRL_HUM;
    uint8_t reg[0];
    bme280_i2c_read(BME_280_ADDR,&addr,reg,1,false);

    // Debug lines
    #if BME_280_DEBUG_MODE
    printf("CTRL_HUM_REG = %u \r\n",reg[0]);
    #endif

    // osrs_h is [2:0]
    my_chip->settings->osrs_h = (uint8_t) reg[0] & 0x07; // 0x07 = 0b00000111

    // Debug lines
    #if BME_280_DEBUG_MODE
    printf("BME280 osrs_h = %u\r\n",my_chip->settings->osrs_h);
    #endif
}

void bme280_read_status(uint8_t *reg){
    // Simple read of CTRL_MEAS reg
    uint8_t addr = BME_280_REG_STATUS;
    bme280_i2c_read(BME_280_ADDR,&addr,reg,1,false);
}

bool bme280_is_doing_conversion(){
    uint8_t status[0];
    // Read status
    bme280_read_status(status);

    // Debug lines
    #if BME_280_DEBUG_MODE
    printf("STATUS_REG = %u \r\n",status[0]);
    #endif

    // Looking if bit 3 is set (BME280_DOC_26)
    if (((uint8_t) (status[0] >> 2) & 0x01 ) == 1){
        return true;
    }
    // TODO check if there is a time when bit 0 and 3 is low and rather use that since that is perfect inbetween
    return false;
}