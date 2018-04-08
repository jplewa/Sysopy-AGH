#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int L = 0;
int TYPE = 0;
int S_COUNT = 0;
int R_COUNT = 0;

void p_sigusr1_handler(int signum, siginfo_t* sig_info, void* sig_context){
    S_COUNT++;
}

void c_sigusr1_handler(int signum, siginfo_t* sig_info, void* sig_context){
    R_COUNT++;
    kill(getppid(), SIGUSR1);
}

void c_sigusr2_handler(int signum, siginfo_t* sig_info, void* sig_context){
    printf("SIGUSR1 signals received by child: %d\n", R_COUNT);
    exit(0);
}

void p_rt1_handler(int signum, siginfo_t* sig_info, void* sig_context){
    S_COUNT++;
}

void c_rt1_handler(int signum, siginfo_t* sig_info, void* sig_context){
    R_COUNT++;
    kill(getppid(), SIGRTMIN);
}

void c_rt2_handler(int signum, siginfo_t* sig_info, void* sig_context){
    printf("SIGRTMAX signals received by child: %d\n", R_COUNT);
    exit(0);
}

int parse(int argc, char* argv[]){
    char * pEnd;
    L =  strtol (argv[1], &pEnd, 10);
    TYPE =  strtol (argv[2], &pEnd, 10);
    if (L == 0 || TYPE == 0) return -1;
    else if (TYPE != 1 && TYPE != 2 && TYPE != 3) return -1;
    else return 0;
}

void print_error(int error_code){
    switch (error_code){
        case 0:
            break;
        case -1:
            printf("Error: incorrect arguments\n");
            printf("Please provide: [number_of_signals] [type]\n");
            printf("Available types: 1, 2, 3\n");
            break;
    }
}

void set_child_handlers(){
    if (TYPE == 1 || TYPE == 2){
        struct sigaction act;
        sigfillset(&act.sa_mask);
        act.sa_flags = SA_SIGINFO | SA_RESTART;
        act.sa_sigaction = &c_sigusr1_handler;
        sigaction(SIGUSR1, &act, NULL);
        act.sa_sigaction = &c_sigusr2_handler;
        sigaction(SIGUSR2, &act, NULL);
    }
    else{
        struct sigaction act;
        sigfillset(&act.sa_mask);
        act.sa_flags = SA_SIGINFO | SA_RESTART;
        act.sa_sigaction = &c_rt1_handler;
        sigaction(SIGRTMIN, &act, NULL);
        act.sa_sigaction = &c_rt2_handler;
        sigaction(SIGRTMAX, &act, NULL); 
    }
}

void set_parent_handlers(){
    if (TYPE == 1 || TYPE == 2){
        struct sigaction act;
        sigfillset(&act.sa_mask);
        act.sa_flags = SA_SIGINFO | SA_RESTART;
        act.sa_sigaction = &p_sigusr1_handler;
        sigaction(SIGUSR1, &act, NULL);
    }
    else{
        struct sigaction act;
        sigfillset(&act.sa_mask);
        act.sa_flags = SA_SIGINFO | SA_RESTART;
        act.sa_sigaction = &p_rt1_handler;
        sigaction(SIGRTMIN, &act, NULL);
    }
}

void send_signals1(){
    int pid;
    set_child_handlers();
    if ((pid = fork()) > 0){
        set_parent_handlers();
        for (int i = 0; i < L; i++){
            if (TYPE == 1) kill(pid, SIGUSR1);
            else if (TYPE == 2){
                kill(pid, SIGUSR1);
                pause();
            }
            else{
                kill(pid, SIGRTMIN);
            }
        }
        if (TYPE == 1 || TYPE == 2){
            printf("SIGUSR1 signals sent to child: %d\n", L);
            kill(pid, SIGUSR2);
            while (wait(NULL) > 0) {}
            printf("SIGUSR1 signal responses received by parent: %d\n", S_COUNT);
        }
        else{
            printf("SIGRTMIN signals sent to child: %d\n", L);
            kill(pid, SIGRTMAX);
            while (wait(NULL) > 0) {}
            printf("SIGRTMIN signal responses received by parent: %d\n", S_COUNT);
        }
    }
    else if (pid == 0){
        for (int i = 0; i < L; i++) pause();
    }    
}

int main(int argc, char* argv[]){
    int result = parse(argc, argv);
    if (result == 0) send_signals1();
    print_error(result);
    return 0;
}