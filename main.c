#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>  //Header file for sleep(). man 3 sleep for deta

void display_help(void){
    printf("\nWelcome to Grandmother Agenda ver.1.2\n");
    printf("To check a timeslot, enter time in \"hh:mm\" format or simply type \"now\". \n");
    printf("I will notify you when it's time to start a task and 10 minutes before a task is due.\n");
    printf("To display this message again, type \"help\" \n");
    printf("To exit the program, type \"exit\" \n\n");
}


int user_input(void){

    static int hh, mm;
    char input[15];

    fgets(input,15, stdin);

    // Exit program
    if(strncmp(input, "exit", 4 * sizeof(char)) == 0){
        printf("Exiting program!\n\n");
        return 1;
    }
    // Display help message
    else if(strncmp(input, "help", 4 * sizeof(char)) == 0){
        display_help();
    }
    // Input time: NOW
    else if(strncmp(input, "now", 3 * sizeof(char)) == 0){
        char temp[100];
        time_t current_time = time(NULL);
        struct tm *tm = localtime(&current_time);
        printf("Time is %d:%d\n\n", tm->tm_hour, tm->tm_min);
    }
    // Check for valid time input
    else{
        if(sscanf(input, "%d:%d" , &hh,&mm) == 2){  // isolate values from string
            if(hh > 23 || hh < 0 || mm > 59 || mm < 0)     //check provided input
            {
                printf("Invalid input: 1 day = [0,23] hours, 1 hour = [0,59] minutes. Please try again.\n\n");
            }
            else{
                // Format time properly: Convert to digits and print in full format
                char h1 = (hh / 10) + '0';
                char h2 = (hh % 10) + '0';
                char m1 = (mm / 10) + '0';
                char m2 = (mm % 10) + '0';
                printf("The Time is: %c%c:%c%c\n\n", h1, h2, m1, m2);
            }
        }
        else{
            printf("Invalid input! Please try again.\n\n");
        }

    }

    return 0;
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

    // program loop
    while(1){
        // Check if a task is starting
        // Check if a task is 10 minutes from finishing

        if(user_input()){
           exit(EXIT_SUCCESS);
        }
    }

    return 0;
}
