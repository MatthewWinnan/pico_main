#include "../include/com_protocol.h"

void clean_rx_buff(){
    //Create state variables
    bool bflag = true;
    int result;

    // We first clean whatever might be in the RX
    while (bflag){
        result = getchar_timeout_us(1000);
        if ( result == PICO_ERROR_TIMEOUT) {
            // We are done cleaning
            bflag = false;
            #if COM_PROTO_DEBUG
            printf("RX buffer now clean :D.\n");
            #endif
        }
    }
}

// Read in stdin and return length of string
uint16_t read_stdin(char *buffer){
    //Create state variables
    bool bflag = true;
    int result;
    uint16_t index = 0;

    while (bflag){
        result = getchar_timeout_us((uint32_t) COM_PROTO_RX_WAIT);
        if ( result == PICO_ERROR_TIMEOUT) {
            // We are done cleaning
            bflag = false;
            #if COM_PROTO_DEBUG
            printf("RX buffer now clean :D.\n");
            #endif
        }
        else {
            if (index < COM_PROTO_RX_BUFFER_SIZE)
            {
                // Only add if smaller than buffer size
                buffer[index] = (char) result;
                index += 1;
            }
        }
    }

    return index;
}

// Init function
void com_protocol_init()
{
    queue_init(&call_queue, sizeof(queue_entry_t), 2);
    queue_init(&results_queue, sizeof(int32_t), 2);

    #if COM_PROTO_DEBUG
    printf("Spinning up communication interface.\n");
    #endif

    multicore_launch_core1(com_protocol_entry);
}

// Main function entry point
void com_protocol_entry(){
    // We create some buffer to store the inputs
    char stdin_buffer[COM_PROTO_RX_BUFFER_SIZE];
    uint16_t len_str;
    // First clean RX
    clean_rx_buff();    

    while(1){
        // If any user input read it
        len_str = read_stdin(stdin_buffer);
    }
}

// BMP_180 Print Functions Defines

// Print functions to be called by Serial queries.
void print_temp_results(struct bmp180_model* my_chip)
{
    #if USE_USB
    printf("==== Temperature Measurement Results ==== \r\n");
    printf("Obtained UT = %i \r\n",my_chip->measurement_params->ut);
    printf("Intermittent step X1 = %i \r\n",my_chip->measurement_params->X1_tmp);
    printf("Intermittent step X2 = %i \r\n",my_chip->measurement_params->X2_tmp);
    printf("Obtained B5 = %i \r\n",my_chip->measurement_params->B5);
    printf("Overall sample sum for %u samples = %i \r\n",BMP_180_SS,my_chip->measurement_params->T_sum);
    printf("Obtained TMP in 0.1C = %d \r\n",my_chip->measurement_params->T);
    #endif
}

void print_press_results(struct bmp180_model* my_chip){
    #if USE_USB
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
    #endif
}

void print_altitude_results(struct bmp180_model* my_chip){
    #if USE_USB
    printf("Currently the device is at %f (m) \r\n",my_chip->measurement_params->altitude);
    #endif
}

void print_relative_pressure_results(struct bmp180_model* my_chip){
    #if USE_USB
    printf("Relative pressure at sea level for device = %f Pa \r\n",my_chip->measurement_params->p_relative);
    #endif
}

void print_chip_ID(struct bmp180_model* my_chip){
    #if USE_USB
    printf("For BMP180 ChipID = %u \r\n",my_chip->chipID);
    #endif
}

void print_cal_params(struct bmp180_model* my_chip){
    #if USE_USB
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
    #endif
}

// 24LC16B Print functions defined

void print_eeprom_chip_ID(struct lcb16b_eeprom* my_eeprom){
    #if USE_USB
    printf("For 24LC16B eeprom ChipID = %u \r\n",my_eeprom->chipID);
    #endif
}