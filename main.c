#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "interactive_agenda.h"


// TODO: Check mutexes
// TODO: Time:
//       Use a variable to hold the simulation's time in minutes
//       Also represent time in minutes in struct Activity and in the program
//       Advance the simulation time in the parallel thread, according to argument passed
//       A function now() should return the simulation time in minutes
// TODO: Comments, remove unnecessary stuff
// TODO: Check the lengths of strings I copy around
// TODO: Use array of pointers to activities! Do malloc() in Load_Activities()
// TODO: Draw execution diagram

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

void time_to_string(char* time_string, int hh, int mm){
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

int time_to_minutes(int hh, int mm){
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

void display_intro(void){
    /*
     * Display intro message
     */
    printf("Welcome to Grandmother Agenda ver.1.2!\n"
               "To check a timeslot, enter time in \"hh:mm\" format or simply type \"now\".\n"
               "I will notify you when it's time to start an activity and 10 minutes before an activity is due.\n"
               "To exit the program, type \"exit\". \n\n");
}


/* Input functions */
int handle_input(char* input){
    /*
     * Process user input
     * Input:
     * input - A string of sufficient length, to read the input in
     *
     * Output:
     * input - Contains valid time string or useless stuff
     * return int - -1 invalid input, 0 valid time input, 1 exit program
     */

    int ret;

    // Exit program
    if(strncmp(input, "exit", 4 * sizeof(char)) == 0){
        ret = 1;
    }
    // Input: now --> Get time into string format
    else if(strncmp(input, "now", 3 * sizeof(char)) == 0){
        time_now_string(input);
        ret = 0;
    }
    // Input: Probably time in string format, but check it!
    else{
        int hh, mm;
        // Check for valid time input
        if(string_to_time(input, &hh, &mm) == 0){          // isolate values from string
            // Invalid time input
            if(hh > 23 || hh < 0 || mm > 59 || mm < 0)
            {
                send_to_printer("Invalid input: 1 day = [0,23] hours, 1 hour = [0,59] minutes. Please try again.\n");
                ret = -1;
            }
            // Valid time input
            else
                ret = 0;
        }
        // Any other invalid input
        else{
            send_to_printer("Invalid input! Please try again.\n");
            ret = -1;
        }
    }

    return ret;
}


/* Activity functions */

int load_Activities(const char *filename){
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

    char string[MAX_STRING_LENGTH];

    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        printf("File \"%s\" not found.\n\n", filename);
        return 1;
    }

    num_activities = 0;
    // for each activity in the file
    while (fgets(string, MAX_STRING_LENGTH, f)) {

        activities[num_activities].status = undone;
        activities[num_activities].start_notification = undone;

        char *token = strtok(string, " "); // get start hour
        activities[num_activities].hh_start = atoi(token);

        token = strtok(NULL, " ");  // get start minute
        activities[num_activities].mm_start = atoi(token);

        token = strtok(NULL, " ");  // get ending hour
        activities[num_activities].hh_end = atoi(token);

        token = strtok(NULL, " ");  // get ending minute
        activities[num_activities].mm_end = atoi(token);

        token = strtok(NULL, " "); // get description
        token = strtok(token, "\n"); // strip "\n"
        underscore_to_space(token);     // replace "_" with " "
        strcpy(activities[num_activities].description, token); // Decode our word before copying it (remove underscores)

        (num_activities)++; // increase size of activities
    }

    return 0;
}

int find_activity(char *time_string){
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

    for(int i=0; i<num_activities; i++){
        start = time_to_minutes(activities[i].hh_start, activities[i].mm_start);
        end = time_to_minutes(activities[i].hh_end, activities[i].mm_end);

        // First check if the ending time and the starting time
        if(start<=input_time && input_time<=end){
           ret = i;
        }
    }

    return ret;
}

void print_activity(int index){
    /*
     * Print activity details. In case an activity is not done, ask for an update.
     *
     * Input:
     * index - The index of the activity in the array activities
     * activities - An array of type Activity, contains the planned activities
     */

    // Get start and end time in string format
    char temp_string[MAX_STRING_LENGTH];
    char start_string[6];
    char end_string[6];
    time_to_string(start_string, activities[index].hh_start, activities[index].mm_start);
    time_to_string(end_string, activities[index].hh_end, activities[index].mm_end);

    sprintf(temp_string, "%s (%s - %s).\n", activities[index].description, start_string, end_string);
    send_to_printer(temp_string);

    switch(activities[index].status){
        case undone:
            sprintf(temp_string, "Activity \"%s\" is not done yet.\nShould I check this activity as done? yes/no\n", activities[index].description);
            send_to_printer(temp_string);

            // get user input
            fgets(temp_string,15, stdin);
            // reset the printing clock
            pthread_mutex_lock(&mutex_clock);
            reset_clock();
            pthread_mutex_unlock(&mutex_clock);

            if(strncmp(temp_string, "yes", 3 * sizeof(char)) == 0){
                send_to_printer("Ok boss, good job! \n");
                activities[index].status = done;
            }
            else{
                send_to_printer("Activity status remained: undone. \n");
            }
            break;
        case done:
            send_to_printer("Chill, you already did this activity.\n");
    }
}


/* Printer functions */

void send_to_printer(char *in_string){
    /*
     * Save the message to print in the linked list printer buffer
     *
     * Input:
     * in_string - The message to be printed
     */

    pthread_mutex_lock(&mutex_printer); // lock the printer_mutex

    struct Node *node;
    node = (struct Node*)malloc(sizeof(struct Node));   // allocate memory in the heap for a new node
    if(node == NULL){
        // Message dropped, not critical
        printf("Memory allocation failed! Message dropped.\n");
        pthread_mutex_unlock(&mutex_printer);
        return;
    }

    strcpy(node->message, in_string);                   // place the message string in this new node
    node->link = NULL;                                  // initialize its linked node to NULL

    // If the linked list is empty
    if (num_messages == 0)
    {
        front = rear = node;                            // both front and rear should point to the only node
    }
    // Else if there are some entries in the list
    else
    {
        // Add the node to the list
        rear->link = node;
        rear = node;
    }

    num_messages++;
    pthread_mutex_unlock(&mutex_printer); // unlock mutex
}

void print_next(){
    /*
     * Print the next message in the linked list
     */

    pthread_mutex_lock(&mutex_printer);

    struct Node *node;
    node = front;

    if (num_messages > 0)
    {
        printf("%s", front->message);  // Print the content of front
        front = front->link;                    // Move the front pointer
        free(node);                             // Free previous front
        num_messages--;
    }

    pthread_mutex_unlock(&mutex_printer);
}


/* Time functions */
void reset_clock(){
    printed_last = clock();
}

void time_now_string(char *time_string){
    /*
     * Return now in string format "%d%d:%d%d"
     * Input:
     * time_string - A string to contain the time
     */
    time_t current_time = time(NULL);
    struct tm *tm = localtime(&current_time);
    time_to_string(time_string, tm->tm_hour, tm->tm_min);
}

void time_now_int(int *hh, int *mm){
    /*
    * Return now in format hh, mm
    * Input:
    * hh - Pointer to an int to contain the hours
    * mm - Pointer to an int to contain the minutes
    */
    time_t current_time = time(NULL);
    struct tm *tm = localtime(&current_time);
    *hh = tm->tm_hour;
    *mm = tm->tm_min;
}

/* Thread functions */
void *printer_thread(void *arg)
{
    while(1){
        // Print next available message
        now = clock();

        // atomic execution
        pthread_mutex_lock(&mutex_clock);
        if((float)(now - printed_last) / CLOCKS_PER_SEC >= PRINT_INTERVAL){
            print_next();
            reset_clock();  // reset clock when something is printed
        }
        pthread_mutex_unlock(&mutex_clock);

        int hh, mm, now_minutes;
        char string[MAX_STRING_LENGTH];


        // Get the time now in minutes
        time_now_int(&hh, &mm);
        now_minutes = time_to_minutes(hh, mm);

        // If start of current activity reached
        if(now_minutes == activity_starts){
            if(activities[current_activity].start_notification == undone){
                sprintf(string, "Activity \"%s\" starts now!\n", activities[current_activity].description);
                send_to_printer(string);
                activities[current_activity].start_notification = done;
            }
        }
        // The current activity ends soon
        else if((now_minutes + MINUTES_DUE) >= activity_ends){

            sprintf(string, "Activity \"%s\" ends in less than 10 minutes!\n", activities[current_activity].description);
            send_to_printer(string);

            // Move to next activity
            current_activity++;

            // If there is no next activity, exit
            if(current_activity >= num_activities){
                printf("Activity \"%s\" ends in less than 10 minutes!\n", activities[current_activity-1].description);
                printf("End of day reached! Exiting.\n");
                exit(EXIT_SUCCESS);
            }

            // Store new activity starting and finishing times
            activity_starts = time_to_minutes(activities[current_activity].hh_start,
                                              activities[current_activity].mm_start);
            activity_ends = time_to_minutes(activities[current_activity].hh_end,
                                            activities[current_activity].mm_end);
        }
    }

    return NULL;
}


int main(int argc, char *argv[]){

    // Command line arguments parsing
    if( argc != 3 ) {
        printf("Please supply the following arguments:\n"
               " 1.filename 2.time_speed_factor\n");
        exit(EXIT_FAILURE);
    }

    printf("Chosen speed factor: %s\n", argv[2]);

    char string[MAX_STRING_LENGTH];     // for user input
    char time_string[10];               //for valid time input
    int index;

    // Load activities from file
    if(load_Activities(argv[1]))
        exit(EXIT_FAILURE);

    // Find current activity
    time_now_string(time_string);
    current_activity = find_activity(time_string);
    activity_starts = time_to_minutes(activities[current_activity].hh_start,
                                      activities[current_activity].mm_start);
    activity_ends = time_to_minutes(activities[current_activity].hh_end,
                                    activities[current_activity].mm_end);

    printf("Current activity: %d", current_activity);

    // Create thread for printing and checking times
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, printer_thread, NULL);

    // Display intro message
    display_intro();

    // Main loop
    while(1){
        // Read user input and reset printing clock
        fgets(string,15, stdin);
        pthread_mutex_lock(&mutex_clock);
        reset_clock();
        pthread_mutex_unlock(&mutex_clock);

        // Handle user input
        switch(handle_input(string)){
            case 0:     // valid time input in string
                break;
            case 1:     // the user wants to exit
                exit(EXIT_SUCCESS);
            case -1:    // invalid input entered
            default:
                continue;
        }

        // String contains valid time in string format "%d%d:%d%d"
        strncpy(time_string, string, 5);
        index = find_activity(time_string);
        if(index == -1){
            // Something is wrong with the activities file
            printf("Activity not found. There should be no free slot in the activities file!Exiting.\n");
            exit(EXIT_FAILURE);
        }

        // Activity found
        print_activity(index);
    }

    return 0;
}
