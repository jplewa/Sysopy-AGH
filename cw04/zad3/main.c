#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int CONF = 0;
int L = 0;
int TYPE = 0;
int P_COUNT = 0;
int C_COUNT = 0;

void p_sig_handler(int signum, siginfo_t* sig_info, void* sig_context){
    P_COUNT++;
    CONF = 1;
}

void c_sig_handler(int signum, siginfo_t* sig_info, void* sig_context){
    C_COUNT++;
    if (TYPE < 3) kill(getppid(), SIGUSR1);
    else kill(getppid(), SIGRTMIN);
}

void c_exitsig_handler(int signum, siginfo_t* sig_info, void* sig_context){
    char* buffer = calloc(100, 1);
    if (TYPE < 3){
        sprintf(buffer, "SIGUSR1 signals received by child: %d\n", C_COUNT);
        write(1, buffer, 100);
    }
    else{
        sprintf(buffer, "SIGRTMAX signals received by child: %d\n", C_COUNT);
        write(1, buffer, 100);
    }
    free(buffer);
    exit(0);
}

int parse(int argc, char* argv[]){
    if (argc < 3) return -1;
    char * pEnd;
    L =  strtol (argv[1], &pEnd, 10);
    TYPE =  strtol (argv[2], &pEnd, 10);
    if (L == 0) return -1;
    else if (TYPE <= 0 || TYPE > 3) return -1;
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
    struct sigaction act;
    sigfillset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO | SA_RESTART | SA_NODEFER;
    act.sa_sigaction = &c_sig_handler;
    if (TYPE < 3){
        sigaction(SIGUSR1, &act, NULL);
        act.sa_sigaction = &c_exitsig_handler;
        sigaction(SIGUSR2, &act, NULL);
    }
    else{
        sigaction(SIGRTMIN, &act, NULL);
        act.sa_sigaction = &c_exitsig_handler;
        sigaction(SIGRTMAX, &act, NULL); 
    }
}

void set_parent_handlers(){
    struct sigaction act;
    sigfillset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO | SA_RESTART | SA_NODEFER;
    act.sa_sigaction = &p_sig_handler;
    if (TYPE < 3) sigaction(SIGUSR1, &act, NULL);
    else sigaction(SIGRTMIN, &act, NULL);
}

void send_signals1(){
    int pid;
    set_child_handlers();
    if ((pid = fork()) > 0){
        set_parent_handlers();
        for (int i = 0; i < L; i++){
            if (TYPE == 3) kill(pid, SIGRTMIN);
            else {
                if (TYPE == 2) CONF = 0;
                kill(pid, SIGUSR1);
                if (TYPE == 2){
                    while (CONF == 0){}
                }
            }
        }
        if (TYPE < 3){
            printf("SIGUSR1 signals sent to child: %d\n", L);
            kill(pid, SIGUSR2);
            while (wait(NULL) > 0) {}
            printf("SIGUSR1 signal responses received by parent: %d\n", P_COUNT);
        }
        else{
            printf("SIGRTMIN signals sent to child: %d\n", L);
            kill(pid, SIGRTMAX);
            while (wait(NULL) > 0) {}
            printf("SIGRTMIN signal responses received by parent: %d\n", P_COUNT);
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