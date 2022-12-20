#include "../include/com_protocol.h"

// BMP_180 Print Functions Defines

// Print functions to be called by Serial queries.
void print_temp_results(struct bmp180_model* my_chip)
{
    printf("==== Temperature Measurement Results ==== \r\n");
    printf("Obtained UT = %i \r\n",my_chip->measurement_params->ut);
    printf("Intermittent step X1 = %i \r\n",my_chip->measurement_params->X1_tmp);
    printf("Intermittent step X2 = %i \r\n",my_chip->measurement_params->X2_tmp);
    printf("Obtained B5 = %i \r\n",my_chip->measurement_params->B5);
    printf("Overall sample sum for %u samples = %i \r\n",BMP_180_SS,my_chip->measurement_params->T_sum);
    printf("Obtained TMP in 0.1C = %d \r\n",my_chip->measurement_params->T);
}

void print_press_results(struct bmp180_model* my_chip){
    printf("==== Pressure Measurement Results ==== \r\n");
    printf("Obtained UP = %i \r\n",my_chip->measurement_params->up);

    printf("Obtained B6 = %i \r\n",my_chip->measurement_params->B6);
    printf("Intermittent step X1_1 = %i \r\n",my_chip->measurement_params->X1_p_1);
    printf("Intermittent step X2_1 = %i \r\n",my_chip->measurement_params->X2_p_1);
    printf("Intermittent step X3_1 = %i \r\n",my_chip->measurement_params->X3_p_1);

    printf("Intermittent step X1_2 = %i \r\n",my_chip->measurement_params->X1_p_2);
    printf("Intermittent step X2_2 = %i \r\n",my_chip->measurement_params->X2_p_2);
    printf("Intermittent step X3_2 = %i \r\n",my_chip->measurement_params->X3_p_2);

    printf("Obtained B3 = %u \r\n",my_chip->measurement_params->B3);
    printf("Obtained B4 = %u \r\n",my_chip->measurement_params->B4);
    printf("Obtained B7 = %u \r\n",my_chip->measurement_params->B7);
    printf("Compensated pressure (P_1) before tuning = %i \r\n",my_chip->measurement_params->p_inter);

    printf("Intermittent step X1_3 = %i \r\n",my_chip->measurement_params->X1_p_3);

    printf("Intermittent step X1_4 = %i \r\n",my_chip->measurement_params->X1_p_4);
    printf("Intermittent step X2_3 = %i \r\n",my_chip->measurement_params->X2_p_3);

    printf("Overall sample sum for %u samples = %i \r\n",BMP_180_SS,my_chip->measurement_params->p_sum);
    printf("Obtained Pressure in 1Pa = %d \r\n",my_chip->measurement_params->p);
}

void print_altitude_results(struct bmp180_model* my_chip){
    printf("Currently the device is at %f (m) \r\n",my_chip->measurement_params->altitude);
}

void print_relative_pressure_results(struct bmp180_model* my_chip){
    printf("Relative pressure at sea level for device = %f Pa \r\n",my_chip->measurement_params->p_relative);
}

void print_chip_ID(struct bmp180_model* my_chip){
    printf("For BMP180 ChipID = %u \r\n",my_chip->chipID);
}

void print_cal_params(struct bmp180_model* my_chip){
    printf("==== Obtained Calibration Parameters ====");
    printf("Obtained data for A1: %i \r\n",my_chip->cal_params->AC1);
    printf("Obtained data for A2: %i \r\n",my_chip->cal_params->AC2);
    printf("Obtained data for A3: %i \r\n",my_chip->cal_params->AC3);
    printf("Obtained data for A4: %u \r\n",my_chip->cal_params->AC4);
    printf("Obtained data for A5: %u \r\n",my_chip->cal_params->AC5);
    printf("Obtained data for A6: %u \r\n",my_chip->cal_params->AC6);
    printf("Obtained data for B1: %i \r\n",my_chip->cal_params->B1);
    printf("Obtained data for B2: %i \r\n",my_chip->cal_params->B2);
    printf("Obtained data for MB: %i \r\n",my_chip->cal_params->MB);
    printf("Obtained data for MC: %i \r\n",my_chip->cal_params->MC);
    printf("Obtained data for MD: %i \r\n",my_chip->cal_params->MD);
}

// 24LC16B Print functions defined

void print_eeprom_chip_ID(struct lcb16b_eeprom* my_eeprom){
    printf("For 24LC16B eeprom ChipID = %u \r\n",my_eeprom->chipID);
}