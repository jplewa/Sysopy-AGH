#define _GNU_SOURCE 1

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>

void start_handler(int signum);
void stop_handler(int signum);
void sigint_handler(int signum);
void print_time();
void infinite_time();

const void* buf = "\nOczekuję na CTRL+Z - kontynuacja albo CTR+C - zakonczenie programu\n\0";
const void* buf2 = "\nOdebrano sygnał SIGINT\n\0";
int run = 1;

void stop_handler(int number){
    run = 0;
    write(1, buf, 70);
    signal(SIGTSTP, start_handler);
}

void start_handler(int signum){
    run = 1;
    signal(SIGTSTP, stop_handler);
}

void sigint_handler(int signum){
    write(1, buf2, 25);
    exit(0);
}

void print_time(){
    time_t raw_time;
    struct tm *tm_info;
    time(&raw_time);
    tm_info = localtime(&raw_time);
    printf("%s", asctime(tm_info));
}

void infinite_time(){
    signal(SIGTSTP, stop_handler);
    struct sigaction act;
    act.sa_handler = sigint_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);
    while(1){
        if (run) print_time();
        sleep(1);
    }
}

int main(void){
    infinite_time();
    return 0;
}