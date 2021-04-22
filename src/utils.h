//
// Created by adam on 22.04.21.
//

#ifndef UTILS_H
#define UTILS_H


/* Utility functions */
void underscore_to_space(char *s) {
    /*
     * Replace underscores in a string with spaces
     * Input/Output:
     * s - The string to be converted
     */
    for (int i = 0; i < strlen(s); i++) {
        if (s[i] == '_') {
            s[i] = ' ';
        }
    }
}

void hm_to_string(char* time_string, int hh, int mm){
    /*
     * Converts time from integers to string format
     * Input:
     * hh - hours
     * mm - minutes
     *
     * Output:
     * time_string - A pointer to a string of sufficient length, time in hours and minutes
     */
    int h1 = hh / 10;
    int h2 = hh % 10;
    int m1 = mm / 10;
    int m2 = mm % 10;

    sprintf(time_string, "%d%d:%d%d", h1, h2, m1, m2);
}

int str_to_hm(const char *time_string, int *hh, int *mm){
    /*
     * Converts time from string to integers format
     *
     * Input:
     * time_string - A pointer to the string containing the time
     *
     * Output:
     * hh: Hours
     * mm: Minutes
     * Return int - 0 for success, 1 for failure
     */
    int args_num = sscanf(time_string, "%d:%d", hh, mm);
    if (args_num == 2)
        return 0;

    return 1;
}

int hm_to_minutes(int hh, int mm){
    /*
     * Convert a timestamp to minutes for easy comparisons
     * Input:
     * hh - hours
     * mm - minutes
     *
     * Output:
     * int - The same time in minutes
     */

    return (hh * 60 + mm);
}

void minutes_to_hm(int t_minutes, int *hh, int *mm){
    /*
     * Converts time from minutes to h:m format
     *
     * Input:
     * t_minutes - Time in minutes
     *
     * Output:
     * hh: Hours
     * mm: Minutes
     */

    *hh = t_minutes / 60;
    *mm = t_minutes % 60;
}

void minutes_to_str(int t_minutes, char *t_string){
    /*
     * Convert time from minutes to string format
     * Input:
     * t_minutes - the time in total minutes format
     *
     * Output:
     * t_string
     *
     */
    int h, m;
    minutes_to_hm(t_minutes, &h, &m);
    hm_to_string(t_string, h, m);
}

int str_to_minutes(const char *t_string){
    /*
     * Convert time from string format to minutes
     * Input: t_string - the time to be converted
     *
     * Output: The time in minutes
     */

    int hh, mm;
    sscanf(t_string, "%d:%d", &hh, &mm);
    return hm_to_minutes(hh, mm);
}

void now_in_string(char *time_string){
    /*
     * Return now in string format "%d%d:%d%d"
     * Input:
     * time_string - A string to contain the time
     */
    time_t current_time = time(NULL);
    struct tm *tm = localtime(&current_time);
    hm_to_string(time_string, tm->tm_hour, tm->tm_min);
}


#endif //UTILS_H
