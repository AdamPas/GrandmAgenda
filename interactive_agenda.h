//
// Created by adam on 20.04.21.
//

#define MAX_STRING_LENGTH 500          // limit for activity entries per line in file
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
    int hh_start;           // starting time: hours
    int mm_start;           // starting time: minutes
    int hh_end;             // ending time: hours
    int mm_end;             // ending time: minutes
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
clock_t printed_last = 0;   // timestamp that the program printed something or received input
clock_t now = 0;
struct Node *front = NULL; // front and rear element in the printer buffer queue
struct Node *rear = NULL;
int num_messages = 0;       // number of messages in the printing queue
Activity activities[MAX_ACTIVITIES];
int num_activities = 0;
int current_activity;
int activity_starts, activity_ends;


/* Utility functions */
void underscore_to_space(char *s);
void time_to_string(char* time_string, const int hh, const int mm);
int string_to_time(const char *time_string, int *hh, int *mm);
int time_to_minutes(const int hh, const int mm);
void display_help(void);


/* Input functions */
int handle_input(char* input);


/* Activity functions */
int load_Activities(const char *filename);
int find_activity(char *time_string);
void print_activity(const int index);


/* Printer functions */
void send_to_printer(char *in_string);
void print_next();


/* Time functions */
void reset_clock();
void time_now_string(char *time_string);
void time_now_int(int *hh, int *mm);


/* Thread functions */
void *printer_thread(void *arg);


#endif //INTERACTIVE_AGENDA_INTERACTIVE_AGENDA_H
