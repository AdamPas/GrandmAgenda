/*
 * author: Adam Pasvatis
 */

#define MAX_STRING_LENGTH 200          // limit for activity entries per line in file
#define MAX_ACTIVITIES 50              // maximum number of activities
#define PRINT_INTERVAL 3               // printing time interval in secs
#define MINUTES_DUE 10                 // the minutes to give a notification, before an activity ends

#include <pthread.h>

#ifndef GRANDMAGENDA_H
#define GRANDMAGENDA_H

/* Enums and structs */
typedef enum {undone, done} status;     // status of an activity
typedef struct {
    /*
     * Represents an activity
     */
    status status;                  // 0 undone, 1 done
    status start_notification;      // done, if the start notification is printed
    int start;                      // starting time in minutes format
    int end;                        // ending time in minutes format
    char description[100];          // name of the activity
} Activity;
struct Node{
    /*
     * Represents a node in the messages queue of the printer
     */
    char message[MAX_STRING_LENGTH];
    struct Node *link;
};


/* Global Variables */
int speed_factor;               // input from user: how fast the simulated time moves (1 real time, 2 twice, etc.)
int t_simulation;               // the internal simulation time
clock_t last_t_sim;             // the last system time that t_simulation was advanced
clock_t last_t_printed = 0;     // the last system time that the program printed something or received input

struct Node *front = NULL;      // front and rear element in the printer buffer queue
struct Node *rear = NULL;
int num_messages = 0;           // number of messages in the printing queue

Activity activities[MAX_ACTIVITIES];    // activities list
int num_activities = 0;                 // total number of activities
int current_activity;                   // index to activities[]
int activity_starts, activity_ends;     // start and end time of current activity

// mutexes for variables common to all threads
pthread_mutex_t mutex_printer = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_print_clock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_t_simulation = PTHREAD_MUTEX_INITIALIZER;


/* Functions */
void display_intro(void);

/* Input functions */
void user_input(char* input);
int process_input(char* input);


/* Activity functions */
int load_activities(const char *filename);
int find_activity(char *time_string);
void print_activity(int index);


/* Printer functions */
void send_to_printer(char *in_string);
void print_next(void);


/* Time functions */
void reset_print_clock();
void now_in_string(char *time_string);

/* Thread functions */
void *thread_printer(void *arg);


#endif //GRANDMAGENDA_H
