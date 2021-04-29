/**
 *  @file utils.c 
 *  @brief  Utility functions for handling time formats
 * 
 */ 


#include <stdio.h>
#include <string.h>
#include <time.h>

#include "utils.h"


/* Utility functions */
void underscore_to_space(char *s) {
  
    for (int i = 0; i < strlen(s); i++) {
        if (s[i] == '_') {
            s[i] = ' ';
        }
    }
}

void hm_to_string(char* time_string, int hh, int mm){
  
    int h1 = hh / 10;
    int h2 = hh % 10;
    int m1 = mm / 10;
    int m2 = mm % 10;

    sprintf(time_string, "%d%d:%d%d", h1, h2, m1, m2);
}

int str_to_hm(const char *time_string, int *hh, int *mm){

    int args_num = sscanf(time_string, "%d:%d", hh, mm);
    if (args_num == 2)
        return 0;

    return 1;
}

int hm_to_minutes(int hh, int mm){

    return (hh * 60 + mm);
}

void minutes_to_hm(int t_minutes, int *hh, int *mm){

    *hh = t_minutes / 60;
    *mm = t_minutes % 60;
}

void minutes_to_str(int t_minutes, char *t_string){
    
    int h, m;
    minutes_to_hm(t_minutes, &h, &m);
    hm_to_string(t_string, h, m);
}

int str_to_minutes(const char *t_string){

    int hh, mm;
    sscanf(t_string, "%d:%d", &hh, &mm);
    return hm_to_minutes(hh, mm);
}

void now_in_string(char *time_string){

    time_t current_time = time(NULL);
    struct tm *tm = localtime(&current_time);
    hm_to_string(time_string, tm->tm_hour, tm->tm_min);
}
