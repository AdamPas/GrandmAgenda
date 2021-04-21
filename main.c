#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>  //Header file for sleep()
#include "interactive_agenda.h"

pthread_mutex_t mutex_printer = PTHREAD_MUTEX_INITIALIZER;

// TODO: Debug the printer time scheme
//       Use mutex for printing and reading: While there is stuff to be printed, the user cannot input
//       When the user needs to give input for specific stuff (like "doing an activity"), the console should not print
//       Make the messages more specific, containing the name of activities

// TODO: Notion of current time
//       A thread that checks the starting time of tasks and prints message
//       A thread (maybe the same) that checks the finishing time of tasks and prints 10 minutes before end

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
    char s[] = "Welcome to Grandmother Agenda ver.1.2!\n"
               "To check a timeslot, enter time in \"hh:mm\" format or simply type \"now\".\n"
               "I will notify you when it's time to start a task and 10 minutes before a task is due.\n"
               "To display this message again, type \"help\".\n"
               "To exit the program, type \"exit\". \n\n";
    send_to_printer(s);

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
     * return int - -1 invalid input, 0 valid time input, 1 exit program, 2 display help message
     */

    int ret;

    // Exit program
    if(strncmp(input, "exit", 4 * sizeof(char)) == 0){
        ret = 1;
    }
    // Display help message
    else if(strncmp(input, "help", 4 * sizeof(char)) == 0){
        display_help();
        ret = 2;
    }
    // Input: now --> Get time into string format
    else if(strncmp(input, "now", 3 * sizeof(char)) == 0){
        time_now(input);
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
        send_to_printer(string);
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

    sprintf(temp_string, "%s (%s - %s).\n", activities[index].description, start_string, end_string);
    send_to_printer(temp_string);

    switch(activities[index].status){
        case undone:
            sprintf(temp_string, "Activity \"%s\" is not done yet.\nShould I check this activity as done? yes/no\n", activities[index].description);
            send_to_printer(temp_string);

            // get user input, lock screen until they give it!
            //pthread_mutex_lock(&mutex_printer);
            fgets(temp_string,15, stdin);
            //reset_clock();
            //pthread_mutex_unlock(&mutex_printer);

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

void time_now(char *time_string){
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
    static clock_t now;

    while(1){
        now = clock();
        // Print every 3 seconds
        if((float)(now - printed_last) / CLOCKS_PER_SEC >= PRINT_INTERVAL){
            //printf("Num messages: %d\n", num_messages);
            print_next();
            reset_clock();  // reset clock when something is printed
        }
    }

    return NULL;
}


int main(void){

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, printer_thread, NULL);

    char string[MAX_STRING_LENGTH];                // for user input
    char time_string[10];            //for valid time input

    // Load activities from file
    Activity activities[MAX_ACTIVITIES];
    int size = 0;
    strcpy(string, "activities.txt");
    load_Activities(string, activities, &size);

    display_help();

    while(1){
        // Read and process user input
        fgets(string,15, stdin);
        switch(handle_input(string)){
            case 0:     // valid time input in string
                break;
            case 1:     // the user wants to exit
                exit(EXIT_SUCCESS);
            case -1:    // invalid input entered
            case 2:     // help message printed
                continue;
        }

        // String contains valid time in string format "%d%d:%d%d"
        strncpy(time_string, string, 5);
        int ind_activity = find_activity(time_string, activities, size);
        if(ind_activity == -1){
            sprintf(string, "No activity planned for time %s, you are free as a bird!\n", time_string);
            send_to_printer(string);
        }
        else{
            // Activity found
            print_activity(ind_activity, activities);
        }
    }

    return 0;
}
