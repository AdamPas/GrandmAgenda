#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "interactive_agenda.h"

// TODO: Initialization with the time the user inputs. DEBUG user input! DEBUG printing time e.g. 8:2
// TODO: Comments, try to substitute global variables

// TODO: Optional
// TODO: Variadic function for printer (to avoid sprintf() every time)



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

int str_to_time(const char *time_string, int *hh, int *mm){
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

void minutes_to_string(int t_minutes, char *t_string){
    /*
     * Convert time from minutes to string format
     * Input:
     * t_minutes - the time in total minutes format
     *
     * Output:
     * t_string
     *
     */

    time_to_string(t_string, (t_minutes / 60), (t_minutes % 60));
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
    return time_to_minutes(hh, mm);
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
     * input - A string of arbitrary length
     *
     * Output:
     * input - Contains valid time input or invalid input
     * return int - -1 invalid input, 0 valid time input, 1 exit program, 2 valid now
     */

    int ret;
    int hh, mm;

    // Exit program
    if(strncmp(input, "exit", 4 * sizeof(char)) == 0){
        ret = 1;
    }
    // Input: now --> Return current simulation time in string format
    else if(strncmp(input, "now", 3 * sizeof(char)) == 0){
        pthread_mutex_lock(&mutex_t_simulation);
        minutes_to_string(t_simulation, input);
        pthread_mutex_unlock(&mutex_t_simulation);

        ret = 2;
    }
    // Input: Probably time in string format, but check it!
    else{
        if(strlen(input) > 5){
            memset(input+5, '\0', 1);   // keep in input, only the first 5 characters
        }

        // Check for valid time input
        if(str_to_time(input, &hh, &mm) == 0){          // isolate values from string
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

    static int hh_start, mm_start, hh_end, mm_end;
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
        hh_start = atoi(token);

        token = strtok(NULL, " ");  // get start minute
        mm_start = atoi(token);

        token = strtok(NULL, " ");  // get ending hour
        hh_end = atoi(token);

        token = strtok(NULL, " ");  // get ending minute
        mm_end = atoi(token);

        activities[num_activities].start = time_to_minutes(hh_start, mm_start);
        activities[num_activities].end = time_to_minutes(hh_end, mm_end);

        token = strtok(NULL, " "); // get description
        token = strtok(token, "\n"); // strip "\n"
        underscore_to_space(token);     // replace "_" with " "
        strcpy(activities[num_activities].description, token);

        (num_activities)++; // increase size of activities
    }

    return 0;
}

int find_activity(char *t_string){
    /*
     * Check if the provided time corresponds to an existing activity
     * Input:
     * t_string - The time in string format
     *
     * Output:
     * Return: The activity index, if one exists or -1 if it doesn't
     */

    int ret = -1;
    int input_time = str_to_minutes(t_string);

    // Search all activities to find the one corresponding to the input time
    for(int i=0; i<num_activities; i++){
        if(activities[i].start<=input_time && input_time<=activities[i].end){
            // Activity found!
           ret = i;
           break;
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
    minutes_to_string(activities[index].start, start_string);
    minutes_to_string(activities[index].end, end_string);

    sprintf(temp_string, "%s (%s - %s)\n", activities[index].description, start_string, end_string);
    send_to_printer(temp_string);

    switch(activities[index].status){
        case undone:
            sprintf(temp_string, "Activity \"%s\" is not done yet.\nShould I check this activity as done? (yes/no)\n", activities[index].description);
            send_to_printer(temp_string);

            // get user input
            fgets(temp_string,MAX_STRING_LENGTH, stdin);
            // reset the printing clock
            pthread_mutex_lock(&mutex_print_clock);
            reset_clock();
            pthread_mutex_unlock(&mutex_print_clock);

            if(strncmp(temp_string, "yes", 3 * sizeof(char)) == 0){
                sprintf(temp_string, "Activity \"%s\" marked as done! \n", activities[index].description);
                send_to_printer(temp_string);
                activities[index].status = done;
            }
            else{
                sprintf(temp_string, "Status of \"%s\" remained: undone. \n", activities[index].description);
                send_to_printer(temp_string);
            }
            break;
        case done:
            sprintf(temp_string, "Chill, you already did \"%s\".\n", activities[index].description);
            send_to_printer(temp_string);
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
        // Message dropped, not critical. Unlock mutex and return
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

void now_string(char *time_string){
    /*
     * Return now in string format "%d%d:%d%d"
     * Input:
     * time_string - A string to contain the time
     */
    time_t current_time = time(NULL);
    struct tm *tm = localtime(&current_time);
    time_to_string(time_string, tm->tm_hour, tm->tm_min);
}

void now_int(int *hh, int *mm){
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

int now_minutes(){
    /*
    * Return now in minutes format
    *
    * Output:
    * now in minutes format
    */
    time_t current_time = time(NULL);
    struct tm *tm = localtime(&current_time);
    return(time_to_minutes(tm->tm_hour, tm->tm_min));
}

/* Thread functions */
void *thread_printer(void *arg)
{
    static clock_t now_clock; // for execution time
    static int hh, mm, now_min;
    char string[MAX_STRING_LENGTH];

    while(1){

        /* Advance Simulation Time */
        pthread_mutex_lock(&mutex_t_simulation);
        now_clock = clock();
        if((float)(now_clock - last_t_sim) / CLOCKS_PER_SEC >= (60 / speed_factor)) {
            last_t_sim = now_clock;
            t_simulation++; // increase the simulation time (minutes format)
        }
        pthread_mutex_unlock(&mutex_t_simulation);


        /* Print next available message */
        // atomic execution, for printed_last to be safe
        pthread_mutex_lock(&mutex_print_clock);
        now_clock = clock();
        if((float)(now_clock - printed_last) / CLOCKS_PER_SEC >= PRINT_INTERVAL){
            print_next();
            reset_clock();  // reset clock when something is printed
        }
        pthread_mutex_unlock(&mutex_print_clock);


        /* Check start and end notification of activities */

        // If start of current activity reached
        if(t_simulation == activity_starts){
            if(activities[current_activity].start_notification == undone){
                sprintf(string, "Activity \"%s\" starts now!\n", activities[current_activity].description);
                send_to_printer(string);
                activities[current_activity].start_notification = done;
            }
        }
        // The current activity ends soon
        else if((t_simulation + MINUTES_DUE) >= activity_ends){
            sprintf(string, "Activity \"%s\" ends in less than %d minutes!\n", activities[current_activity].description, MINUTES_DUE);
            send_to_printer(string);

            // The next activity becomes the current activity
            current_activity++;

            // If there is no next activity, exit
            if(current_activity >= num_activities){
                printf("Activity \"%s\" ends in less than %d minutes!\n", activities[current_activity-1].description, MINUTES_DUE);
                printf("End of day reached! Exiting.\n");
                exit(EXIT_SUCCESS);
            }

            // Store new activity starting and finishing times
            activity_starts = activities[current_activity].start;
            activity_ends = activities[current_activity].end;

        }
    }

    return NULL;
}


int main(int argc, char *argv[]){

    /* Command line arguments parsing */
    if( argc != 3 ) {
        printf("Please supply the following arguments:\n"
               " 1.filename 2.time_speed_factor\n");
        exit(EXIT_FAILURE);
    }
    char filename[20];
    strcpy(filename, argv[1]);
    speed_factor = atoi(argv[2]);


    /* Initialization */
    char string[MAX_STRING_LENGTH];    // for user input
    char t_string[20];                  //for valid time input
    static int i_activity;             // activity index

    // Load activities from file, if not, exit
    if(load_Activities(filename))
        exit(EXIT_FAILURE);

    // Initialize simulation time, according to user input
    printf("Initialization. What is the current time in the granny world?\n");
    fgets(t_string,MAX_STRING_LENGTH, stdin);
    switch(handle_input(t_string)){
        case 0:     // valid time input in string
            break;
        default:    // in any other case, use real world NOW
            now_string(t_string);
            break;
    }
    printf("Initialized to %s\n", t_string);
    t_simulation = str_to_minutes(t_string);    // initialize simulation time
    last_t_sim = clock();

    // Find current activity
    current_activity = find_activity(t_string);
    activity_starts = activities[current_activity].start;
    activity_ends = activities[current_activity].end;


    /* Launch thread for printing messages and checking activity notifications */
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, thread_printer, NULL);


    /* User input Loop */
    display_intro();
    while(1){
        // Read user input and reset printer clock
        fgets(t_string,MAX_STRING_LENGTH, stdin);
        pthread_mutex_lock(&mutex_print_clock);
        reset_clock();
        pthread_mutex_unlock(&mutex_print_clock);

        // Handle user input
        switch(handle_input(t_string)){
            case 0:     // valid time input in string
                break;
            case 2: // now
                printf("%s\n", t_string); // print time for convenience
                break;
            case 1:     // the user wants to exit
                exit(EXIT_SUCCESS);
            case -1:    // invalid input entered
            default:
                continue;
        }

        // String contains valid time in string format "%d%d:%d%d"
        i_activity = find_activity(t_string);
        if(i_activity == -1){
            // Something is wrong with the activities file
            printf("Activity not found. There should be no free slot in the activities file!Exiting.\n");
            exit(EXIT_FAILURE);
        }

        // Activity found
        print_activity(i_activity);
    }

    return 0;
}
