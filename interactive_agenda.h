//
// Created by adam on 20.04.21.
//

#define MAX_STRING_LENGTH 200          // limit for activity entries per line in file
#define MAX_ACTIVITIES 50              // maximum number of activities
#define PRINT_INTERVAL 3               // printing time interval in secs
#define MINUTES_DUE 10                 // the minutes to give a notification, before an activity ends

#ifndef INTERACTIVE_AGENDA_INTERACTIVE_AGENDA_H
#define INTERACTIVE_AGENDA_INTERACTIVE_AGENDA_H

/* Enums and structs */
typedef enum {undone, done} status;
typedef struct {
    /*
     * Represents and activity
     */
    status status;          // 0 undone, 1 done
    status start_notification; // done, if the start notification is printed
    int start;              // starting time in minutes format
    int end;                // ending time in minutes format
    char description[100];   // name of the activity
} Activity;
struct Node{
    /*
     * Represents a node in the messages queue of the printer
     */
    char message[MAX_STRING_LENGTH];
    struct Node *link;
};


/* Global Variables */
int speed_factor;           // input from user: how fast the simulated time moves (1 real time, 2 twice, etc.)
int t_simulation;           // the internal simulation time
clock_t last_t_sim;         // the last system time that t_simulation was advanced
clock_t last_t_printed = 0;   // timestamp that the program printed something or received input

struct Node *front = NULL; // front and rear element in the printer buffer queue
struct Node *rear = NULL;
int num_messages = 0;       // number of messages in the printing queue

Activity activities[MAX_ACTIVITIES];    // activities list
int num_activities = 0;                 // total number of activities
int current_activity;                   // index to activities[]
int activity_starts, activity_ends;     // start and end time of current activity

// mutexes for variables common to all threads
pthread_mutex_t mutex_printer = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_print_clock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_t_simulation = PTHREAD_MUTEX_INITIALIZER;


/* Utility functions */
void underscore_to_space(char *s);
void hm_to_string(char* time_string, int hh, int mm);
int str_to_hm(const char *time_string, int *hh, int *mm);
int hm_to_minutes(int hh, int mm);
void minutes_to_hm(int t_minutes, int *hh, int *mm);
void minutes_to_str(int t_minutes, char *t_string);
int str_to_minutes(const char *t_string);
void display_intro(void);


/* Input functions */
void user_input(char* input);
int process_input(char* input);


/* Activity functions */
int load_Activities(const char *filename);
int find_activity(char *time_string);
void print_activity(int index);


/* Printer functions */
void send_to_printer(char *in_string);
void print_next();


/* Time functions */
void reset_clock();
void now_string(char *time_string);

/* Thread functions */
void *thread_printer(void *arg);


#endif //INTERACTIVE_AGENDA_INTERACTIVE_AGENDA_H
