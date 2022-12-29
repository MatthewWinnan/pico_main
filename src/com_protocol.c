#include "../include/com_protocol.h"
#include "com_protocol.h"

// TODO find some generic way to initialize cmd by using struct declared in main
// TODO find out why I get the weird char malloc definitions. exe help)��@(kl

// Increments some value within a defined space
uint16_t safe_increment(uint16_t input, uint16_t max){
    if ( (input + 1) == max)
        return input;
    else
        return input + 1;
}

// Redumentary function to calculate 10^exponent. O(n) so eh?
uint16_t pow_10(uint8_t exponent){
    uint16_t output = 1;
    for (uint8_t i = 0; i < exponent; i++){
        output *= 10;
    }
    return output;
}

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
            #if COM_PROTO_INFO
            printf("RX buffer now clean :D.\r\n");
            #endif
        }
    }
}

void clean_stdin(char *buffer, uint16_t *len){
    // Just cleans the stdin after use
    for (uint16_t i = 0; i<*len; i++){
        buffer[i] = 0;
    }
}

void clean_cmd_line(struct cmd* cmd_line){
    // Clean the command
    for (uint16_t i = 0; i<cmd_line->cmd_len; i++){
        cmd_line->command[i] = (char) 0;
    }
    cmd_line->cmd_len = 0;

    // Clean the args
    for (uint16_t i = 0; i<cmd_line->arg_len; i++){
        cmd_line->args[i] = (char) 0;
    }

    // Clean the additional arg values
    for (uint16_t i = 0; i<cmd_line->arg_len; i++){
        cmd_line->int_arg[i] = 0;
    }

    // Clean the temp array
    for (uint8_t i = 0; i<cmd_line->int_tmp_len; i++){
        cmd_line->int_tmp[i] = 0;
    }

    cmd_line->int_tmp_len = 0;
    cmd_line->arg_len = 0;
    cmd_line->int_arg_len = 0;
}

// Read in stdin into the cmd holder
void read_stdin_to_cmd(char *std_in, uint16_t *len, struct cmd* cmd_line){
    /*
    State variable:
    0 - Reading in command
    1 - Reading in arguments
    2 - Done unless more arguments
    3 - Reading in argument values
    */
    uint8_t state = 0;

    // We have that last terminate character to read in so read until len + 1
    for (uint16_t i = 0; i<*len+1;i++){
        switch (state){
            case 0:
                if ((uint8_t)std_in[i] == 0){
                    // This is a null character thus we treat it as an escape to be done
                    cmd_line->cmd_len = i ;
                }
                else if ((uint8_t)std_in[i] == 32){
                    // This is a space thus the command read is done
                    // We can now go to state 2 to wait for the args
                    cmd_line->cmd_len = i ;
                    state = 2;
                }
                else{
                    cmd_line->command[i] = std_in[i];
                }
                break;
            
            case 1:
                if ((uint8_t)std_in[i] == 32){
                    // This is a space thus the arg read is done
                    state = 2;
                }
                else if ((uint8_t)std_in[i] != 0){
                    // Also ignore terminate char if found
                    // Here we read in the arguments
                    cmd_line->args[cmd_line->arg_len] = std_in[i];
                    cmd_line->arg_len = (uint8_t) safe_increment((uint16_t) cmd_line->arg_len, (uint16_t) COM_PROTO_ARG_ARRAY_SIZE);
                }
                break;
            
            case 2:
                
                if ((uint8_t)std_in[i] == 45){
                    // This is the character -, thus arguments are starting so switch over
                    state = 1;
                }
                else if (((uint8_t)std_in[i] != 32) && ((uint8_t)std_in[i] != 45)){
                    // Not a - or space. Assume value since we are here
                    // Here we move to value read and read in the first valid value if a value is expected
                    if ((cmd_line->int_arg_len < cmd_line->arg_len) && ((uint8_t) std_in[i] >= 48)){
                    state = 3;
                    // Add to the temp array
                    cmd_line->int_tmp[cmd_line->int_tmp_len] = (uint8_t) std_in[i] - 48;
                    cmd_line->int_tmp_len = (uint8_t) safe_increment((uint16_t) cmd_line->int_tmp_len, (uint16_t) COM_PROTO_ARG_ARRAY_SIZE);
                    } 
                }
                break;
            
            case 3:
                if ((uint8_t)std_in[i] == 0){
                    // This is a null character thus we treat it as an escape to be done
                    // Read in the temp 
                    for ( uint8_t v = 0; v<cmd_line->int_tmp_len; v++){
                        cmd_line->int_arg[cmd_line->int_arg_len] += cmd_line->int_tmp[v] * pow_10(cmd_line->int_tmp_len - 1 - v);
                    }
                    // Reset the temp
                    memset( cmd_line->int_tmp, 0 , cmd_line->int_tmp_len);
                    cmd_line->int_tmp_len = 0;
                    // Increment the int arg amount
                    cmd_line->int_arg_len = (uint8_t) safe_increment((uint16_t) cmd_line->int_arg_len, (uint16_t) COM_PROTO_ARG_ARRAY_SIZE);
                }
                else if ((uint8_t)std_in[i] == 32){
                    // This is a space thus the value read is done
                    state = 2;

                    // Read in the temp 
                    for ( uint8_t v = 0; v<cmd_line->int_tmp_len; v++){
                        cmd_line->int_arg[cmd_line->int_arg_len] += cmd_line->int_tmp[v] * pow_10(cmd_line->int_tmp_len - 1 - v);
                    }
                    // Reset the temp
                    memset( cmd_line->int_tmp, 0 , cmd_line->int_tmp_len);
                    cmd_line->int_tmp_len = 0;
                    // Increment the int arg amount
                    cmd_line->int_arg_len = (uint8_t) safe_increment((uint16_t) cmd_line->int_arg_len, (uint16_t) COM_PROTO_ARG_ARRAY_SIZE);
                }
                else if ((cmd_line->int_arg_len < cmd_line->arg_len) && ((uint8_t) std_in[i] >= 48)){
                    // Add to the temp array
                    cmd_line->int_tmp[cmd_line->int_tmp_len] = (uint8_t) std_in[i] - 48;
                    cmd_line->int_tmp_len = (uint8_t) safe_increment((uint16_t) cmd_line->int_tmp_len, (uint16_t) COM_PROTO_ARG_ARRAY_SIZE);
                } 
        }
    }
}

// Read in stdin and return length of string. This is the RX USB implementation.
uint16_t read_stdin_usb(char *buffer){
    //Create state variables
    bool bflag = true;
    bool enter = true;
    int result;
    uint16_t index = 0;

    while (bflag){
        // I did not want some infinite wait. As such I read straight from stdin intead of using scanf and the likes.
        // PS I could play around with tinyUSB to trigger IRQ but nah that's a future project ;P
        result = getchar_timeout_us((uint32_t) COM_PROTO_RX_WAIT);
        if ( (result == PICO_ERROR_TIMEOUT) && (enter)) {
            // We are done cleaning
            bflag = false;
            #if COM_PROTO_DEBUG
            printf("Done reading :D.\r\n");
            #endif
        }
        else {
            // We did receive something, set enter to false so we can disable the exit condition until the user is done
            enter = false;
            // We need one free character for a termination point
            if (index < (COM_PROTO_RX_BUFFER_SIZE - 1))
            {
                #if COM_PROTO_DEBUG
                printf("Found result %i \r\n",result);
                #endif

                // Based on the character we find we do some generic operations
                // Special characters are treated first
                switch (result){
                    case 3:
                        // This is the text end character. Triggered by Ctrl+C
                        // Just escape
                        #if COM_PROTO_USER_FEEDBACK_SERIAL
                        printf("\r\n");
                        #endif
                        return index;
                        
                    case 8:
                        // This is the backspace. We treat it as if a character was deleted. 
                        // We move index back , so another character can be written over
                        // Only move back if no underflow will occur
                        if ( index > 0){
                        index -= 1;
                        buffer[index] = (char)0; // Clear this character
                        }
                        break;
                    
                    case 13:
                        // This is a CR. We are going to a new line thus we can assume ENTER was used.
                        // This can be triggered by ENTER, Ctrl+M
                        enter = true; // We can now escape the loop
                        break;

                    case 24:
                        // This is a CAN, here we assume the user has canceled their input. Return 0
                        // This is done by Ctrl+X
                        #if COM_PROTO_USER_FEEDBACK_SERIAL
                        printf("\r\n");
                        #endif
                        return 0;

                    default:
                        // Non special character, however we only accept [32,126] as useful input, thus here we filter useless out.
                        // Further we only accept a buffer size of COM_PROTO_RX_BUFFER_SIZE. If it so happen to be more exit the program 
                        if ((result > 31) && (result < 127)){
                            #if COM_PROTO_DEBUG
                            printf("Assigning %c = %i to index %i \r\n",(char) result,result, index);
                            #endif
                            // Only add if smaller than buffer size
                            buffer[index] = (char) result;
                            index += 1;
                        }
                        break;
                }
                // Print updated stdin feedback
                #if COM_PROTO_USER_FEEDBACK_SERIAL
                printf("\r[pico_w]:%s ",buffer);
                #endif
            }
            else {
                // Index is now overflowing return
                #if COM_PROTO_USER_FEEDBACK_SERIAL
                printf("\r\n");
                #endif
                buffer[index - 1] = (char) 0;
                return index;
            }
        }
    }
    #if COM_PROTO_USER_FEEDBACK_SERIAL
    // Normal return 
    if (index > 0){
        printf("\r\n");
    }
    else {
       printf("\r[pico_w]:"); 
    }
    #endif
    buffer[index] = (char) 0;
    return index;
}

// Main entry for read. On compile time the correct function will be entered here.
uint16_t read_stdin(char *buffer){
    #if USE_USB
    return read_stdin_usb(buffer);
    #endif
}

// Init function
void com_protocol_init()
{
    queue_init(&call_queue, sizeof(queue_entry_t), COM_PROTO_QUEUE_LEN);
    queue_init(&results_queue, sizeof(queue_entry_t), COM_PROTO_QUEUE_LEN);

    #if COM_PROTO_INFO
    printf("Spinning up communication interface.\r\n");
    #endif

    multicore_launch_core1(com_protocol_entry);
}

// Init the cmd_line structure
void init_cmd_line(struct cmd* cmd_line){
    // Setting the defaults for cmd_line
    memset( cmd_line->command, '\0', sizeof( cmd_line->command ));
    memset( cmd_line->args, '\0', sizeof( cmd_line->args ));
    memset( cmd_line->int_arg, 0 , sizeof( cmd_line->int_arg ));
    memset( cmd_line->int_tmp, 0 , sizeof(cmd_line->int_tmp));
    cmd_line->arg_len = 0;
    cmd_line->cmd_len = 0;
    cmd_line->int_arg_len = 0;
    cmd_line->int_tmp_len = 0;

    // TODO find some nicer way to initialize the sensor state variables
    cmd_line->bmp_180 = &my_bmp180;
    cmd_line->eeprom = &my_eeprom;
}

// Initializes the bin_executable structure
void init_bin_executable(bin_executable *bin_array){
    // Define the first entry
    char * cmd_buffer_1 = (char *) malloc(4 * sizeof(char)); // Allocate some space in memory for the cmd char
    char command_1[4] = "help";
    memcpy(cmd_buffer_1, command_1, 4 * sizeof(char)); // Copy the content into memory
    bin_executable entry_1 = {&help_bin, cmd_buffer_1};
    bin_array[0] = entry_1;

    // Define the second entry
    char * cmd_buffer_2 = (char *) malloc(6 * sizeof(char)); // Allocate some space in memory for the cmd char
    char command_2[6] = "bmp180";
    memcpy(cmd_buffer_2, command_2, 6 * sizeof(char)); // Copy the content into memory
    bin_executable entry_2 = {&bmp180_bin, cmd_buffer_2};
    bin_array[1] = entry_2;

    #if COM_PROTO_INFO
    printf("init_bin_executable assigned bin string %s to index 0\r\n",bin_array[0].bin_string);
    printf("init_bin_executable assigned bin string %s to index 1\r\n",bin_array[1].bin_string);
    #endif
}

int execute_bin(struct cmd* cmd_line, bin_executable *bin_array){
    int result;
    for (uint8_t i = 0; i<COM_PROTO_N_BIN; i++){
        // Debug code
        #if COM_PROTO_DEBUG
        printf("Comparing string %s with string %s until index %i \r\n",cmd_line->command, bin_array[i].bin_string, cmd_line->cmd_len);
        #endif

        // Check if strings are equal.
        result = strncmp(cmd_line->command, bin_array[i].bin_string, cmd_line->cmd_len);
        if ( result == 0){
            // Execute the binary
            void (*func)() = (void(*)())(bin_array[i].func);
            (*func)(cmd_line);
            return i;
        }
    }
    return COM_PROTO_NO_BIN;
}

// Main function entry point
void com_protocol_entry(){
    // We create some buffer to store the inputs
    char stdin_buffer[COM_PROTO_RX_BUFFER_SIZE] = {0};
    // Initialize the command holder
    struct cmd cmd_line;
    init_cmd_line(&cmd_line);
    // Declare our bin name array
    bin_executable bin_array[COM_PROTO_N_BIN];
    // Init bin executables
    init_bin_executable(bin_array);
    // Store the length of the command
    uint16_t len_str;
    // First clean RX
    clean_rx_buff();    
    // Result of executables
    int res;
    // Allocate some space to store a queue entry
    queue_entry_t result_queue_entry;

    while(1){
        // If any user input read it
        len_str = read_stdin(stdin_buffer);

        // Read in the command if len != 0
        if ( len_str != 0 ){
            read_stdin_to_cmd(stdin_buffer,&len_str,&cmd_line);

            // Debugging lines
            #if COM_PROTO_DEBUG
            printf("Obtained command %s with char length %i.\r\nArguments were %s with length %i \r\n",cmd_line.command,cmd_line.cmd_len,cmd_line.args,cmd_line.arg_len);
            for (uint8_t i = 0; i<cmd_line.int_arg_len; i++)
            {
                printf("At index %i the value is %i :\r\n",i,cmd_line.int_arg[i]);
            }
            #endif

            // Execute the binary
            res = execute_bin(&cmd_line,bin_array);

            // Debugging lines
            #if COM_PROTO_DEBUG
            if (res == COM_PROTO_NO_BIN){
                printf("Command %s does not exist.\r\n",cmd_line.command);
            }
            #endif
        }
        else{
            // Debugging lines
            #if COM_PROTO_DEBUG
            printf("No input was obtained :( \r\n");
            #endif
        }

        // Check if any entry exists in the result queue
        if (queue_is_empty(&results_queue))
        {
            // Empty. For debugging tell me
            #if COM_PROTO_DEBUG
            printf("Result queue is empty.\r\n");
            #endif
        }
        else{
            // Read in the function and input
            queue_remove_blocking(&results_queue, &result_queue_entry);
            void (*stdout_func)() = (void(*)())(result_queue_entry.func);
            void *std_out_value = result_queue_entry.data ;
            // Execute the function
            // We always assume stdout will be of some form of this.
            (*stdout_func)(std_out_value);
        }
        
        // Clean stdin after reading
        clean_stdin(stdin_buffer, &len_str);
        // Clean cmd_line after reading
        clean_cmd_line(&cmd_line);
    }
}

/*
Define out 'binary' functions here.
These act as linux like binary executables.
Each is associated with a string in bin_executable.
struct cmd is passed to it to tell it what to do.
*/

void help_bin(struct cmd* cmd_line){
    // We check if the command received any args
    switch (cmd_line->arg_len){
        case 0:
            // No args received print generic help
            print_help_bin_help();
            break;
        default:
            // Print generic help
            print_help_bin_help();
            break;
    }
}

void print_help_bin_help(){
    // USB communications based implementation
    #if USE_USB
    printf("Usage for help:\r\n-h: Displays this help message.\r\nDefault: Displays this message and entire list of defined binaries.\r\n");
    printf("List of binaries:\r\n1) help\r\n2) bmp180\r\n");
    #endif
}

void bmp180_bin(struct cmd* cmd_line){
    // For the rationale behind my weird empty switch case label. Consult the discussion at 
    // https://stackoverflow.com/questions/18496282/why-do-i-get-a-label-can-only-be-part-of-a-statement-and-a-declaration-is-not-a
    // We check if the command received any args

    // Start by setting verbose back to 0
    cmd_line->bmp_180->v = 0;
    // Initialize some space for undefined amount of queue entries
    // We do this so we can set the unified state of the bmp180 struct if needed
    queue_entry_t entry_array[COM_PROTO_QUEUE_LEN]; // Can only be max this
    uint8_t entry_array_index = 0;
    // Now we need an arror flag incase bogus inputs were made
    bool valid_case = true;
    switch (cmd_line->arg_len){
        case 0:
            // No args received print generic help
            print_help_bmp180_help();
            valid_case = false;
            break;
        default: ; // This empty label is so we can use declerations
            // We cycle through the arguments and add each to the call queue.
            for (uint16_t i = 0; i<cmd_line->arg_len; i++){
                switch ((uint8_t) cmd_line->args[i]){
                    case 97: ;
                        // The a case.
                        if (entry_array_index < COM_PROTO_QUEUE_LEN){
                        // Add it to our 'to enter entries'
                        entry_array[entry_array_index].func = &bmp180_get_altitude;
                        entry_array[entry_array_index].data = cmd_line->bmp_180;
                        // Step index up
                        entry_array_index+=1;
                        }
                        break;
                    case 99: ;
                        // The c case.
                        print_cal_params_bmp180(cmd_line->bmp_180);
                        break;
                    case 104:
                        // The h case. We also break out of the for loop
                        valid_case = false; // Whenever -h is used we only print no execute.
                        print_help_bmp180_help();
                        i = cmd_line->arg_len;
                        break;
                    case 109: ; // This empty label is so we can use declerations
                        // The m case
                        bmp180_inter_m(entry_array, &entry_array_index, cmd_line,i);

                        break; // This should still correctly break out of the switch
                    case 115: ;
                        // The s case
                        if (entry_array_index < COM_PROTO_QUEUE_LEN){
                        // Add it to our 'to enter entries'
                        entry_array[entry_array_index].func = &bmp180_get_sea_pressure;
                        entry_array[entry_array_index].data = cmd_line->bmp_180;
                        // Step index up
                        entry_array_index+=1;
                        }
                        break;
                    case 118: ;
                        // The v case so just set verbose on
                        cmd_line->bmp_180->v = 1;
                        break;
                    default:
                        // Invalid input
                        bmp180_error(cmd_line->args[i]);
                        valid_case = false;
                        // Also exit the for loop
                        i = cmd_line->arg_len;
                        break;
                }
            }
            break; // This should still correctly break out of the switch
    }
    if (valid_case){
        // If no errors occurred we now add everything to the main queue :)
        for (uint8_t loc=0; loc<entry_array_index; loc++){
        // Add to the queue
        queue_add_blocking(&call_queue, &entry_array[loc]);
    }
    }
}

void print_help_bmp180_help(){
    // USB communications based implementation
    #if USE_USB
    printf("Usage for bmp180:\r\n-a: Performs altitude estimation.\r\n");
    printf("-c: Displays the bmp180's calibration parameters.\r\n");
    printf("-h: Displays this help message.\r\n");
    printf("-m: Performs full temperature and pressure sampling. Takes in additional integer arguments if one wishes to repeat the process.\r\n");
    printf("-s: Performs relative sea pressure estimation.\r\n");
    printf("-v: Prints results in verbose mode. Default this option is turned off.\r\n");
    printf("Default: Displays this help message.\r\n");
    #endif
}

void bmp180_error(char argument){
    // USB communications based implementation
    #if USE_USB
    printf("Recieved invalid character %c with value %u.\r\nThe usage is defined as: \r\n\r\n",argument,argument);
    #endif
    // Print generic helper
    print_help_bmp180_help();
}

void bmp180_inter_m(queue_entry_t *entry_queue, uint8_t *entry_len, struct cmd* cmd_line, uint8_t index)
{
    if (cmd_line->int_arg_len>index){
        // We have an entry for this case
        cmd_line->bmp_180->m = cmd_line->int_arg[index];
    }
    else {
        // Set to 1
        cmd_line->bmp_180->m = 1;
    }
    // Go ahead and add entries
    for (uint8_t loc=*entry_len; loc<cmd_line->bmp_180->m; loc++){
        if (*entry_len < COM_PROTO_QUEUE_LEN){
        entry_queue[loc].func = &bmp180_get_measurement;
        entry_queue[loc].data = cmd_line->bmp_180;
        *entry_len += 1;
        }
    }
}

// Defines STDOUT selection and enques it to the result queue. This should be called by main. Makes sense to me to keep it here
int stdout_selector(void *func_pointer){
    // Can't use a switch statement since pointer is not a constant value....
    if ((uint32_t) func_pointer == (uint32_t) &bmp180_get_measurement){
        // Queue the temp results
        queue_entry_t result_queue_entry_temp = {&print_temp_results_bmp180,&my_bmp180};
        queue_add_blocking(&results_queue,&result_queue_entry_temp);
        // Queue the pressure results
        queue_entry_t result_queue_entry_press = {&print_press_results_bmp180,&my_bmp180};
        queue_add_blocking(&results_queue,&result_queue_entry_press);
        return 0;
    }
    else if ((uint32_t) func_pointer == (uint32_t) &bmp180_get_altitude)
    {
       // Queue the alt results
        queue_entry_t result_queue_entry_altitude = {&print_altitude_results_bmp180,&my_bmp180};
        queue_add_blocking(&results_queue,&result_queue_entry_altitude); 
    }
    else if ((uint32_t) func_pointer == (uint32_t) &bmp180_get_sea_pressure)
    {
       // Queue the alt results
        queue_entry_t result_queue_entry_sea_pressure = {&print_relative_pressure_results_bmp180,&my_bmp180};
        queue_add_blocking(&results_queue,&result_queue_entry_sea_pressure); 
    }
    else {
        #if COM_PROTO_DEBUG
        printf("Invalid function entered to stdout_selector.");
        #endif
        return -1;
    }
}

// BMP_180 Print Functions Defines

// Print functions to be called by Serial queries.
void print_temp_results_bmp180(struct bmp180_model* my_chip)
{
    #if USE_USB
    printf("\r==== Temperature Measurement Results ==== \r\n");
    // Only print these if verbose
    if (my_chip->v == 1){
    printf("Obtained UT = %i \r\n",my_chip->measurement_params->ut);
    printf("Intermittent step X1 = %i \r\n",my_chip->measurement_params->X1_tmp);
    printf("Intermittent step X2 = %i \r\n",my_chip->measurement_params->X2_tmp);
    printf("Obtained B5 = %i \r\n",my_chip->measurement_params->B5);
    printf("Overall sample sum for %u samples = %i \r\n",BMP_180_SS,my_chip->measurement_params->T_sum);
    }
    printf("Obtained TMP in 0.1C = %d \r\n",my_chip->measurement_params->T);
    #endif
}

void print_press_results_bmp180(struct bmp180_model* my_chip){
    #if USE_USB
    printf("\r==== Pressure Measurement Results ==== \r\n");
    // Only print these if verbose
    if (my_chip->v == 1){
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
    }
    printf("Obtained Pressure in 1Pa = %d \r\n",my_chip->measurement_params->p);
    #endif
}

void print_altitude_results_bmp180(struct bmp180_model* my_chip){
    #if USE_USB
    printf("\rCurrently the device is at %f (m) \r\n",my_chip->measurement_params->altitude);
    #endif
}

void print_relative_pressure_results_bmp180(struct bmp180_model* my_chip){
    #if USE_USB
    printf("\rRelative pressure at sea level for device = %f Pa \r\n",my_chip->measurement_params->p_relative);
    #endif
}

void print_chip_ID_bmp180(struct bmp180_model* my_chip){
    #if USE_USB
    printf("\rFor BMP180 ChipID = %u \r\n",my_chip->chipID);
    #endif
}

void print_cal_params_bmp180(struct bmp180_model* my_chip){
    #if USE_USB
    printf("\r==== BMP180 Obtained Calibration Parameters ====\r\n");
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
    printf("\rFor 24LC16B eeprom ChipID = %u \r\n",my_eeprom->chipID);
    #endif
}

// BME280 print functions defines 
void print_cal_params_bme280(struct bme280_model* my_chip){
    #if USE_USB
    printf("\r==== BME280 Obtained Calibration Parameters ====\r\n");
    printf("Obtained data for T1: %u \r\n",my_chip->cal_params->dig_T1);
    printf("Obtained data for T2: %i \r\n",my_chip->cal_params->dig_T2);
    printf("Obtained data for T2: %i \r\n",my_chip->cal_params->dig_T3);
    printf("Obtained data for P1: %u \r\n",my_chip->cal_params->dig_P1);
    printf("Obtained data for P2: %i \r\n",my_chip->cal_params->dig_P2);
    printf("Obtained data for P3: %i \r\n",my_chip->cal_params->dig_P3);
    printf("Obtained data for P4: %i \r\n",my_chip->cal_params->dig_P4);
    printf("Obtained data for P5: %i \r\n",my_chip->cal_params->dig_P5);
    printf("Obtained data for P6: %i \r\n",my_chip->cal_params->dig_P6);
    printf("Obtained data for P7: %i \r\n",my_chip->cal_params->dig_P7);
    printf("Obtained data for P8: %i \r\n",my_chip->cal_params->dig_P8);
    printf("Obtained data for P9: %i \r\n",my_chip->cal_params->dig_P9);
    printf("Obtained data for H1: %u Unsigned char: %c \r\n",my_chip->cal_params->dig_H1);
    printf("Obtained data for H2: %i \r\n",my_chip->cal_params->dig_H2);
    printf("Obtained data for H3: %u Unsigned char: %c \r\n",my_chip->cal_params->dig_H3);
    printf("Obtained data for H4: %i \r\n",my_chip->cal_params->dig_H4);
    printf("Obtained data for H5: %i \r\n",my_chip->cal_params->dig_H5);
    printf("Obtained data for H6: %i Signed char: %c \r\n",my_chip->cal_params->dig_H6);
    #endif
}