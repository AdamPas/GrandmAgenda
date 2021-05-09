/**
 *  @file main.h 
 *  @brief  Main functionality functions
 * 
 */ 

#ifndef GRANDMAGENDA_H
#define GRANDMAGENDA_H


/**
 * @brief  DIsplay intro message
 */
extern void display_intro(void);

/* Input functions */

/**
 * @brief  Read user input from the terminal
 * @param input   a string of sufficient size to read the input
 */
extern void user_input(char* input);

/**
 * @brief  Process user input
 * @param input  A string of arbitrary length containing user input, stripped of \n in its end
 *                             When return:  Contains valid time input or invalid input, as interpreted by the returned value
 * @return  -1 invalid input, 0 valid time input, 1 exit program, 2 valid now
 */
extern int process_input(char* input);


/* Activity functions */

/**
 * @brief  Load activities from a txt file (specific format, see activities.txt)
 * @param filename   The name of the file containing the activities
 * @return  0 for success, 1 in case the file is not found
 */
extern int load_activities(const char *filename);

/**
 * @brief  Check if the provided time corresponds to an existing activity
 * @param time_string  The time in string format
 * @return  The activity index, if one exists or -1 if it doesn't
 */
extern int find_activity(char *time_string);

/**
 * @brief  Print activity details. In case an activity is not done, ask for an update.
 * @param index  The index of the activity in the array activities
 */
extern void print_activity(int index);


/* Printer functions */

/**
 * @brief   Save the message to print in the printer buffer
 * @param in_string  The message to be printed as formated string
 * @param ... The necessary variables for the formated string
 * 
 * --------------------------------------------------------------
 * Perform simple printing of formatted data
 * Supported conversion specifiers: 
 *      d, i     signed int
 *      u        unsigned int
 *      ld, li   signed long
 *      lu       unsigned long
 *      f        double
 *      c        char
 *      s        string
 *      %        '%'
 * Usage: %[conversion specifier]
 * Note: This function does not support these format specifiers: 
 *      [flag][min width][precision][length modifier]
 * --------------------------------------------------------------
 */
extern void send_to_printer(const char *in_string, ...);

/**
 * @brief   Print the next message in the linked list
 */
extern void print_next(void);


/* Time functions */

/**
 * @brief  Resets the global printer clock
 */
extern void reset_print_clock();

/* Thread functions */

/**
 * @brief  Thread function: Check deadlines(printing messages, issuing activity messages)
 */
extern void *thread_printer(void *arg);


#endif //GRANDMAGENDA_H
