#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>  //Header file for sleep(). man 3 sleep for deta

#define MAX_ACTIVITY_DESCRIPTION 50    // limit for activity entries in file
#define MAX_ACTIVITIES 50              // maximum number of activities

typedef enum {undone, done} status;
typedef struct {
    status status;          // 0 undone, 1 done
    int hh_start;           // starting time: hours
    int mm_start;           // starting time: minutes
    int hh_end;             // ending time: hours
    int mm_end;             // ending time: minutes
    char description[15];   // name of the activity
} Activity;


void convert(char *s) {
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

    return;
}



void time_to_string(char* time_string, const int hh, const int mm){
    /*
     * Converts time from integers to string format
     * Input:
     * time_string - A pointer to a string of sufficient length, time in hours and minutes
     * hh - hours
     * mm - minutes
     *
     * Output:
     * In time_string
     */
    int h1 = hh / 10;
    int h2 = hh % 10;
    int m1 = mm / 10;
    int m2 = mm % 10;

    sprintf(time_string, "%d%d:%d%d", h1, h2, m1, m2);
}

int string_to_time(const char *time_string, int *hh, int *mm){
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

void display_help(void){
    /*
     * Display help message
     */
    printf("\nWelcome to Grandmother Agenda ver.1.2!\n");
    printf("To check a timeslot, enter time in \"hh:mm\" format or simply type \"now\". \n");
    printf("I will notify you when it's time to start a task and 10 minutes before a task is due.\n");
    printf("To display this message again, type \"help\". \n");
    printf("To exit the program, type \"exit\". \n\n");
}

int user_input(char* input){
    /*
     * Read user input from the command line
     * Input:
     * input - A string of sufficient length, to read the input in
     *
     * Output:
     * input - Contains the input read from command line
     * return int - 0 time input, 1 exit program, 2 display help message
     */

    int ret = 0;
    printf("Waiting for your input.\n");
    fgets(input,15, stdin);

    // Exit program
    if(strncmp(input, "exit", 4 * sizeof(char)) == 0){
        printf("Exiting program!\n\n");
        ret = 1;
    }
    // Display help message
    else if(strncmp(input, "help", 4 * sizeof(char)) == 0){
        display_help();
        ret = 2;
    }

    return ret;
}

int handle_time_input(char *time_string){
    /*
     * Check time input. Valid formats: now or %d%d:%d%d
     * Input:
     * time_string - The input string
     *
     * Output:
     * time_string - Contains valid time or rubbish
     * Return: 1 for invalid, 0 for valid
     */

    int ret = 0;
    int hh, mm;

    // Input: now --> Get time into string format
    if(strncmp(time_string, "now", 3 * sizeof(char)) == 0){
        time_t current_time = time(NULL);
        struct tm *tm = localtime(&current_time);
        time_to_string(time_string, tm->tm_hour, tm->tm_min);
    }
    else{
        // Check for valid time input
        if(string_to_time(time_string, &hh, &mm) == 0){   // isolate values from string
            if(hh > 23 || hh < 0 || mm > 59 || mm < 0)     //check provided input
            {
                printf("Invalid input: 1 day = [0,23] hours, 1 hour = [0,59] minutes. Please try again.\n\n");
                ret = 1;
            }
        }
        else{
            printf("Invalid input! Please try again.\n\n");
            ret = 1;
        }
    }

    return ret;
}

int load_Activities(char *filename, Activity activities[], int *size){
    /*
     * Load activities from a txt file (specific format, see activities.txt)
     *
     * Input:
     * filename: The name of the file containing the activities
     * activities: An empty activities for the activities to be loaded in
     * size: After loading, this will contain the number of activities in the activities
     *
     * Output:
     * int - 0 for success, 1 in case the file is not found
     */

    char string[MAX_ACTIVITY_DESCRIPTION];
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        printf("File \"%s\" not found.\n\n", filename);
        return 1;
    }

    *size = 0;
    // for each activity in the file
    while (fgets(string, MAX_ACTIVITY_DESCRIPTION, f)) {

        activities[*size].status = undone;

        char *token = strtok(string, " "); // get start hour
        activities[*size].hh_start = atoi(token);

        token = strtok(NULL, " ");  // get start minute
        activities[*size].mm_start = atoi(token);

        token = strtok(NULL, " ");  // get ending hour
        activities[*size].hh_end = atoi(token);

        token = strtok(NULL, " ");  // get ending minute
        activities[*size].mm_end = atoi(token);

        token = strtok(NULL, " "); // get description
        token = strtok(token, "\n"); // strip "\n"
        convert(token);     // replace "_" with " "
        strcpy(activities[*size].description, token); // Decode our word before copying it (remove underscores)

        (*size)++; // increase size of activities
    }

    return 0;
}

int time_to_minutes(const int hh, const int mm){
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

int check_for_activity(char *time_string, Activity activities[], const int size){
    /*
     * Check if the provided time corresponds to an existing activity
     * Input:
     * time_string - The time in string format
     * activities - A list of activities of type Activity
     * size - The size of the activities list
     *
     * Output:
     * Return: The activity index, if one exists or -1 if it doesn't
     */

    int ret = -1;
    int hh, mm;
    int start, end;
    string_to_time(time_string, &hh, &mm);
    int input_time = time_to_minutes(hh, mm);

    for(int i=0; i<size; i++){
        start = time_to_minutes(activities[i].hh_start, activities[i].mm_start);
        end = time_to_minutes(activities[i].hh_end, activities[i].mm_end);

        // First check if the ending time and the starting time
        if(start<=input_time && input_time<=end){
           ret = i;
        }
    }

    return ret;
}

void *thread1(void *vargp)
{
    // Print message every 10 minutes
    while(1){
        sleep(10);
        printf("\nHi from thread! \n");
    }

    return NULL;
}


int main(void) {

    display_help();
//
//    pthread_t thread_id;
//    printf("Before Thread\n");
//    pthread_create(&thread_id, NULL, thread1, NULL);

    Activity activities[MAX_ACTIVITIES];
    int size = 0;
    char filename[] = "activities.txt";
    load_Activities(filename, activities, &size);

    char input[15];
    while(1){
        // Read user input
        switch(user_input(input)){
            case 0:
                // check if the input is time format valid
                if(handle_time_input(input) == 0){
                    // TODO: Handle the int I get from check_for_activity()
                    // input contains valid time
                    printf("%d\n", check_for_activity(input, activities, size));
                }
                break;
            case 1:
                // the user wants to exit
                exit(EXIT_SUCCESS);
        }
    }

    return 0;
}
