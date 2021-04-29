/**
 *  @file utils.h 
 *  @brief  Utility functions for handling time formats
 * 
 */ 


#ifndef UTILS_H
#define UTILS_H


/* Utility functions */

/**
 * @brief  Replace underscores in a string with spaces
 * @param The string to be converted
 */
extern void underscore_to_space(char *s);


/**
 * @brief  Convert time from integers to string format
 * @param time_string  A pointer to a string of sufficient length, holds the result
 * @param hh  Hours
 * @param mm  Minutes
 */
extern void hm_to_string(char* time_string, int hh, int mm);


/**
 * @brief  Convert time from string to integers format
 * @param time_string  A pointer to the string containing the time
 * @param hh  Hours
 * @param mm  Minutes
 * @return   0 for success, 1 for failure
 */
extern int str_to_hm(const char *time_string, int *hh, int *mm);


/**
 * @brief  Convert a timestamp to minutes for easy comparisons
 * @param hh  Hours
 * @param mm  Minutes
 * @return   Time in minutes format
 */
extern int hm_to_minutes(int hh, int mm);


/**
 * @brief  Convert time from nimutes to h:m format
 * @param t_minutes  Time in minutes format
 * @param hh  Hours
 * @param mm  Minutes
 */
extern void minutes_to_hm(int t_minutes, int *hh, int *mm);


/**
 * @brief  Convert time from minutes to string format
 * @param t_minutes  the time in total minutes format
 * @param t_string  String to hold the result
 */
extern void minutes_to_str(int t_minutes, char *t_string);


/**
 * @brief   Convert time from string format to minutes
 * @param t_string  The time to be converted in string format
 * @return  Time in minutes format
 */
extern int str_to_minutes(const char *t_string);


/**
 * @brief Return real-time "now" in string format "%d%d:%d%d"
 * @param time_string  A string to hold the result
 */
extern void now_in_string(char *time_string);


#endif //UTILS_H
