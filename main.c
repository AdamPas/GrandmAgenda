#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>  //Header file for sleep()
#include "interactive_agenda.h"

// TODO: Use mutex for printing and reading: While there is stuff to be printed, the user cannot input
//       When the user needs to give input for specific stuff (like "doing an activity"), the console should not print
//       Make the messages more specific, containing the name of activities

// TODO: Notion of current time
// TODO: A thread that checks the starting time of tasks and prints message
// TODO: A thread (maybe the same) that checks the finishing time of tasks and prints 10 minutes before end

// TODO: Take filename and speed as cmd parameters
// TODO (optional) Use array of pointers, not of structs!

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

void display_help(void){
    /*
     * Display help message
     */
    char s[] = "\nWelcome to Grandmother Agenda ver.1.2!\n"
               "To check a timeslot, enter time in \"hh:mm\" format or simply type \"now\".\n"
               "I will notify you when it's time to start a task and 10 minutes before a task is due.\n"
               "To display this message again, type \"help\".\n"
               "To exit the program, type \"exit\". \n\n";
    print_job(s);

}


/* Input functions */
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
    fgets(input,15, stdin);
    reset_clock();

    // Exit program
    if(strncmp(input, "exit", 4 * sizeof(char)) == 0){
        //printf("Exiting program!\n\n");
        ret = 1;
    }
    // Display help message
    else if(strncmp(input, "help", 4 * sizeof(char)) == 0){
        display_help();
        ret = 2;
    }

    return ret;
}

int parse_time_input(char *time_string){
    /*
     * Check time input. Valid formats: "now" or %d%d:%d%d
     * Input:
     * time_string - The input string
     *
     * Output:
     * time_string - Contains valid time or rubbish
     * Return: 1 for invalid, 0 for valid
     */

    int ret;
    int hh, mm;

    // Input: now --> Get time into string format
    if(strncmp(time_string, "now", 3 * sizeof(char)) == 0){
        now(time_string);
        ret = 0;
    }
    // Input: Probably time in string format, but check it!
    else{
        // Check for valid time input
        if(string_to_time(time_string, &hh, &mm) == 0){   // isolate values from string
            if(hh > 23 || hh < 0 || mm > 59 || mm < 0)     //check provided input
            {
                print_job("Invalid input: 1 day = [0,23] hours, 1 hour = [0,59] minutes. Please try again.\n\n");
                ret = 1;
            }
            else
                ret = 0;
        }
        else{
            print_job("Invalid input! Please try again.\n\n");
            ret = 1;
        }
    }

    return ret;
}


/* Activity functions */

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

    char string[MAX_STRING_LENGTH];

    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        sprintf(string, "File \"%s\" not found.\n\n", filename);
        print_job(string);
        return 1;
    }

    *size = 0;
    // for each activity in the file
    while (fgets(string, MAX_STRING_LENGTH, f)) {

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
        underscore_to_space(token);     // replace "_" with " "
        strcpy(activities[*size].description, token); // Decode our word before copying it (remove underscores)

        (*size)++; // increase size of activities
    }

    return 0;
}

int find_activity(char *time_string, Activity *activities, const int size){
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

void print_activity(const int index, Activity activities[]){
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

    sprintf(temp_string, "%s (%s - %s). ", activities[index].description, start_string, end_string);
    print_job(temp_string);

    switch(activities[index].status){
        case undone:
            print_job("This activity is not done yet.\n");
            print_job("Should I check this activity as done? yes/no \n");
            // get user input
            char input[15];
            fgets(input,15, stdin);
            reset_clock();
            if(strncmp(input, "yes", 3 * sizeof(char)) == 0){
                print_job("Ok boss, good job! \n");
                activities[index].status = done;
            }
            else{
                print_job("Activity status remained: undone. \n");
            }
            break;
        case done:
            print_job("Chill, you already did this activity.\n");
    }
}


/* Printer thread */

void print_job(char *in_string){
    /*
     * Save the message to print in the linked list printer buffer
     *
     * Input:
     * in_string - The message to be printed
     */

    struct Node *node;

    node = (struct Node*)malloc(sizeof(struct Node));   // allocate memory in the heap for a new node
    strcpy(node->message, in_string);                   // place the message string in this new node
    node->link = NULL;                                  // initialize its linked node to NULL

    // If the linked list is empty
    if (rear == NULL)
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
}

void print_next(){
    /*
     * Print the next message in the linked list
     */
    struct Node *node;
    node = front;

    if (front == NULL)
    {
        //printf("No messages to print. The list is empty. \n");
        rear = NULL;
    }
    else
    {
        printf("%s\n", front->message);  // Print the content of front
        front = front->link;                    // Move the front pointer
        free(node);                             // Free previous front
    }
}


/* Time functions */
void reset_clock(){
    printed_last = clock();
}

void now(char *time_string){
    /*
     * Return now in string format "%d%d:%d%d"
     * Input:
     * time_string - A string to contain the time
     */
    time_t current_time = time(NULL);
    struct tm *tm = localtime(&current_time);
    time_to_string(time_string, tm->tm_hour, tm->tm_min);
}

/* Thread functions */
void *printer_thread(void *arg)
{
    clock_t now;

    while(1){
        now = clock();
        // Print every 3 seconds
        if((float)(now - printed_last) / CLOCKS_PER_SEC >= PRINT_INTERVAL){
            print_next();
            reset_clock();  // reset clock when something is printed
        }
    }

    return NULL;
}


int main(void){

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, printer_thread, NULL);

    print_job("Hello 1\n");
    print_job("Hello 2\n");
    print_job("Hello 3\n");

    // Load activities from file
    Activity activities[MAX_ACTIVITIES];
    int size = 0;
    char filename[] = "activities.txt";
    load_Activities(filename, activities, &size);

    display_help();

    char input[15];
    while(1){
        // Read user input, check for "help", "exit", invalid input
        switch(user_input(input)){
            case 1:
                // the user wants to exit
                exit(EXIT_SUCCESS);
            case 2:
                continue;
            default:
                if(parse_time_input(input) != 0){
                    continue;
                }
        }
        // Input contains valid time
        int ind_activity = find_activity(input, activities, size);
        if(ind_activity == -1){
            print_job("No activity planned, you are free as a bird!\n");
        }
        else{
            // Activity found
            print_activity(ind_activity, activities);
        }
    }

    return 0;
}
