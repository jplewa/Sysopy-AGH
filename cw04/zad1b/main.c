#include <stdlib.h>
#include <time.h>
#include <signal.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void start_handler(int signum);
void stop_handler(int signum);
void sigint_handler(int signum);
void infinite_time();

const void* buf = "\nOczekuję na CTRL+Z - kontynuacja albo CTR+C - zakonczenie programu\n\0";
const void* buf2 = "\nOdebrano sygnał SIGINT\n\0";
int child_pid = 0;
int run = 1;

void stop_handler(int number){
    kill(child_pid, SIGINT); 
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

void infinite_time(){
    signal(SIGTSTP, stop_handler);
    signal(SIGINT, sigint_handler);
    do{
        if (run){
            child_pid = fork();
            run = 0;
        }
    } while (child_pid > 0);
    
    execl("script", "script", NULL);
}

int main(void){
    infinite_time();
    return 0;
}