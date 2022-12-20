#include "../include/bmp180.h"
#include "../include/com_protocol.h"
// #include "com_protocol.c"

void bmp180_get_cal(struct bmp180_calib_param* params,struct bmp180_model* my_chip)
{
    //Read in the calibration parameters. 
    //Review BMP180_DOC_18
    uint8_t out_buff[BMP_180_N_CAL_PARAMS] = {0};
    uint8_t reg_start = BMP_180_REG_A1_MSB;

    bmp180_i2c_read(BMP_180_ADDR,&reg_start,out_buff,BMP_180_N_CAL_PARAMS,false);

    //Just some basic manipulations
    // Remember MSB is sent first from the stream.
    params->AC1 = (int16_t) (out_buff[0]<<8) | out_buff[1];
    params->AC2 = (int16_t) (out_buff[2]<<8) | out_buff[3];
    params->AC3 = (int16_t) (out_buff[4]<<8) | out_buff[5];

    params->AC4 = (uint16_t) (out_buff[6]<<8) | out_buff[7];
    params->AC5 = (uint16_t) (out_buff[8]<<8) | out_buff[9];
    params->AC6 = (uint16_t) (out_buff[10]<<8) | out_buff[11];

    params->B1 = (int16_t) (out_buff[12]<<8) | out_buff[13];
    params->B2 = (int16_t) (out_buff[14]<<8) | out_buff[15];

    params->MB = (int16_t) (out_buff[16]<<8) | out_buff[17];
    params->MC = (int16_t) (out_buff[18]<<8) | out_buff[19];
    params->MD = (int16_t) (out_buff[20]<<8) | out_buff[21];

    //Write it to our model
    my_chip->cal_params = params;

    //Print  out the callibrated data 
    print_cal_params(my_chip);

}

void bmp180_init(struct bmp180_model* my_chip, struct bmp180_calib_param* my_params, struct bmp180_measurements* measures) {
    // Define our arrays
    //Mappings for the OSS mode of Pressure to variable input 
    pressure_oss[0] = BMP_180_SET_PRESS_OSS_0;
    pressure_oss[1] = BMP_180_SET_PRESS_OSS_1;
    pressure_oss[2] = BMP_180_SET_PRESS_OSS_2;
    pressure_oss[3] = BMP_180_SET_PRESS_OSS_3;
    //Mappings for out wait time in pressure mode
    pressure_time[0] = BMP_180_PRES_OSS_0;
    pressure_time[1] = BMP_180_PRES_OSS_1;
    pressure_time[2] = BMP_180_PRES_OSS_2;
    pressure_time[3] = BMP_180_PRES_OSS_3;  

    //Basic check to see if the BMP180 is operational and in future configure the modes.
    // At BMP180_DOC_19 it states the device needs 10 ms to start up. Only needs to be more
    sleep_ms(1000); //it may say 10ms but that might be purely buffers. In practice needs 1s to boot.
    printf("Starting BMP180 setup\r\n");
    uint8_t chipID[1];
    uint8_t addr = BMP_180_CHIP_ID_ADDR;

    bmp180_i2c_read(BMP_180_ADDR,&addr,chipID,1,false);

    if (chipID[0] != BMP_180_CHIP_ID){
        while (true){
            printf("BMP180 was not configured correctly :(.\r\n");
            printf("Obtained values. chipID = %d, expected = %d \r\n",chipID[0],BMP_180_CHIP_ID);
            sleep_ms(5000);
        }   
    }
    else {
        printf("BMP180 was configured correctly :)\r\n");
        printf("Obtained values. chipID = %d, expected = %d \r\n",chipID[0],BMP_180_CHIP_ID);
    }
    //Write the ID to model
    my_chip->chipID = chipID[0];
    //Assign the input measurement structure to the chip structure
    my_chip->measurement_params = measures;
    //We read in the calibration parameters
    bmp180_get_cal(my_params,my_chip);
}

//Here we follow the use case in BMP180_DOC_15. This function is meant to be called by the main temp processing function.
void bmp180_get_ut(struct bmp180_model* my_chip){
    //First write to begin temp sampling
    uint8_t write_buff[2];
    uint8_t read_buff[2];
    write_buff[0] = BMP_180_REG_CTRL_MEAS; //We first tell it to write to this register
    write_buff[1] = BMP_180_SET_TMP; //We tell it then to write this value to it
    //Tell the bmp180 to start sampling temperature
    bmp180_i2c_write(BMP_180_ADDR,write_buff,2,false); //No blocking
    //We wait the conversion time
    sleep_ms(BMP_180_TMP_TIME*2); //Wait twice as long for safety
    //Read the values now, optionally we should check if bit sco is still set BMP180_DOC_18
    uint8_t addr = BMP_180_REG_OUT_MSB;
    bmp180_i2c_read(BMP_180_ADDR,&addr,read_buff,2,false);//Release control
    //Assign our results
    my_chip->measurement_params->ut = (read_buff[0] << 8) | read_buff[1]; //Remember MSB first
}

void bmp180_get_up(struct bmp180_model* my_chip){
    //First write to begin temp sampling
    uint8_t write_buff[2];
    uint8_t read_buff[3];
    write_buff[0] = BMP_180_REG_CTRL_MEAS; //We first tell it to write to this register
    write_buff[1] = pressure_oss[BMP_180_OSS]; //We tell it then to write this value to it
    //Tell the bmp180 to start sampling pressure
    bmp180_i2c_write(BMP_180_ADDR,write_buff,2,false); //No blocking
    //We wait the conversion time based on the OSS sampling setting
    sleep_ms(pressure_time[BMP_180_OSS]*3); //Wait twice as long for safety
    //Read the values now, optionally we should check if bit sco is still set BMP180_DOC_18
    //Checking for the bit ensures full conversion is done.
    uint8_t addr = BMP_180_REG_OUT_MSB;
    bmp180_i2c_read(BMP_180_ADDR,&addr,read_buff,3,false);//We also read the XLSB 
    //Read in the final results
    //Equation is given at BMP180_DOC_15. 
    long up = ((read_buff[0] << 16) | (read_buff[1] << 8) | read_buff[2]) >> (8 - BMP_180_OSS);//Remember MSB first
    my_chip->measurement_params->up = ((read_buff[0] << 16) | (read_buff[1] << 8) | read_buff[2]) >> (8 - BMP_180_OSS);//Remember MSB first
}

void bmp180_get_temp(struct bmp180_model* my_chip){
    //First read in the raw value
    bmp180_get_ut(my_chip);
    //Calculation outlined in BMP180_DOC_15
    my_chip->measurement_params->X1_tmp = ((my_chip->measurement_params->ut - my_chip->cal_params->AC6) * my_chip->cal_params->AC5) >> 15; //Remember >>15 = /2^15
    my_chip->measurement_params->X2_tmp = (my_chip->cal_params->MC << 11)/(my_chip->measurement_params->X1_tmp+my_chip->cal_params->MD);
    my_chip->measurement_params->B5 = my_chip->measurement_params->X1_tmp + my_chip->measurement_params->X2_tmp;
    // Calculate final compensated temperature
    my_chip->measurement_params->T_sum += (my_chip->measurement_params->B5+8) >> 4;

}

void bmp180_get_pressure(struct bmp180_model* my_chip){
    //First read in the raw data
    bmp180_get_up(my_chip);
    //Calculation outlined in BMP180_DOC_15.
    //Example code can be seen at https://github.com/BoschSensortec/BMP180_driver
    //Unfortunately the reasoning behind the calculations seem to be proprietary https://community.bosch-sensortec.com/t5/MEMS-sensors-forum/BMP180-datasheet/m-p/7503#M454
    //Normal bitshifting does not work like how I thought. I need to divide by the actual float/double value of 2^x and not >> x. Further intermediate steps must keep accuracy (else if below 1 it reports 0)
    // To do this I used some sickening conversions :(
    my_chip->measurement_params->B6 = my_chip->measurement_params->B5 - 4000;
    long B6_int =(long) ((float) my_chip->measurement_params->B6 * (float) (((float) my_chip->measurement_params->B6)/powf((float)2,(float)12)));
    my_chip->measurement_params->X1_p_1 = (my_chip->cal_params->B2*(B6_int)) >> 11;
    my_chip->measurement_params->X2_p_1 = (long)((float) my_chip->cal_params->AC2 * (((float) my_chip->measurement_params->B6)/(powf((float) 2, (float) 11)))); //Ugly conversion tricks to keep accuracy :(
    my_chip->measurement_params->X3_p_1 = my_chip->measurement_params->X1_p_1 + my_chip->measurement_params->X2_p_1;
    my_chip->measurement_params->B3 = ((((long) (my_chip->cal_params->AC1 << 2) + my_chip->measurement_params->X3_p_1) << BMP_180_OSS) + 2) >> 2;
    my_chip->measurement_params->X1_p_2 = (long) ((float) my_chip->cal_params->AC3 * (float) (((float) my_chip->measurement_params->B6)/(powf((float) 2, (float) 13))));//Ugly conversion tricks to keep accuracy :(
    my_chip->measurement_params->X2_p_2 = (long) ((float) ((float) my_chip->cal_params->B1* B6_int)/(powf((float) 2, (float) 16)));//Ugly conversion tricks to keep accuracy :(
    my_chip->measurement_params->X3_p_2 = ((my_chip->measurement_params->X1_p_2 + my_chip->measurement_params->X2_p_2) + 2) >> 2;
    my_chip->measurement_params->B4 = (unsigned long)((float) my_chip->cal_params->AC4 * (((float)((unsigned long)(my_chip->measurement_params->X3_p_2 + 32768)))/ (powf((float) 2,(float) 15))));//Ugly conversion tricks to keep accuracy :(
    my_chip->measurement_params->B7 = (unsigned long)((float) ((unsigned long) my_chip->measurement_params->up - my_chip->measurement_params->B3) * (((float) 50000)/(powf((float) 2, (float) BMP_180_OSS))));

    if (my_chip->measurement_params->B7 < 0x80000000){
        my_chip->measurement_params->p_inter = (my_chip->measurement_params->B7 << 1)/my_chip->measurement_params->B4;
    }
    else{
        my_chip->measurement_params->p_inter = (my_chip->measurement_params->B7/my_chip->measurement_params->B4) << 1;
    }

    my_chip->measurement_params->X1_p_3 = (long)((((float) my_chip->measurement_params->p_inter)/(powf((float)2, (float) 8))) * (((float) my_chip->measurement_params->p_inter)/(powf((float)2, (float) 8))));
    my_chip->measurement_params->X1_p_4 = (long)(((float) my_chip->measurement_params->X1_p_3 * 3038))/(powf((float)2, (float) 16));
    my_chip->measurement_params->X2_p_3 = (long)(((float) -7357 * my_chip->measurement_params->p_inter))/(powf((float)2, (float) 16));
    my_chip->measurement_params->p_sum += my_chip->measurement_params->p_inter + (long)(((float)(my_chip->measurement_params->X1_p_4 + my_chip->measurement_params->X2_p_3 + 3791))/powf((float)2, (float) 4));
}

void bmp180_get_altitude(struct bmp180_model* my_chip)
{
    //The following altitude calculations are defined in BMP180_DOC_16
    float p_ratio = (float) ( (float) my_chip->measurement_params->p/BMP_180_SEA_PRESSURE);
    float inter_term = (float) (1- powf(p_ratio,(float) (1/5.255)));
    // Assign the altitude
    my_chip->measurement_params->altitude = (float) ( (float) 44330 *inter_term);
}

void bmp180_get_sea_pressure(struct bmp180_model* my_chip){
    //The following calculations are defined in BMP180_DOC_17
    my_chip->measurement_params->p_relative = (float) ((float) my_chip->measurement_params->p/(powf((float) (1 - ((float) BMP_180_CENTURION_HEIGHT/(float) 44330)),5.255)));
}

void bmp180_get_measurement(struct bmp180_model* my_chip){
    // Big note here, besides causing the BMP180 to take longer per sample. This will also cause it to be less responsive to sudden changes.
    // It is recopmmended to keep the BMP_180_SS value to [1,3]. However one may go as high as 256 if conditions are completely stable
    // Set sum variables to 0
    my_chip->measurement_params->p_sum = 0;
    my_chip->measurement_params->T_sum = 0;
    for (uint8_t i = 0; i < BMP_180_SS; i++)
    {
        //Sample N times
        bmp180_get_temp(my_chip);
        bmp180_get_pressure(my_chip);
    }
    // We then divide by the sampling size variable
    my_chip->measurement_params->p = (long ) (my_chip->measurement_params->p_sum/BMP_180_SS);
    my_chip->measurement_params->T = (long ) (my_chip->measurement_params->T_sum/BMP_180_SS);

    //Calculate additional derived measurements
    bmp180_get_altitude(my_chip);
    bmp180_get_sea_pressure(my_chip);

    //Print the results
    #if BMP_180_DEBUG_MODE 
    print_temp_results(my_chip);
    print_press_results(my_chip);
    print_altitude_results(my_chip);
    print_relative_pressure_results(my_chip);
    #endif
}

