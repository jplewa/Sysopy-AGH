#include <stdlib.h>
#include <string.h>
#include  <stdio.h>
#include  <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

int parse(int argc, char* argv[]);
void print_error(int error_code);
void children_processes();
void real_time_handler(int signum, siginfo_t* sig_info, void* sig_context);
void sigusr1_handler(int signum, siginfo_t* sig_info, void* sig_context);
void sigusr2_handler(int signum, siginfo_t* sig_info, void* sig_context);
void set_handlers();


int N = 0;
int K = 0;
int COUNT = 0;
pid_t* children;

int main(int argc, char* argv[]){
    int result = parse(argc, argv);
    if (result == 0){
        children_processes();
    }

    return 0;
}

int parse(int argc, char* argv[]){
    char * pEnd;
    N =  strtol (argv[1], &pEnd, 10);
    K =  strtol (argv[2], &pEnd, 10);
    if (N == 0 || K == 0) return -1;
    else return 0;
}

void print_error(int error_code){
    switch (error_code){
        case -1:
            printf("Error: Incorrect arguments\n");
            printf("Please provide: [number_of_processes] [number_of_requests]\n");
            break;
    }
}

void children_processes(){
    children = malloc(K*sizeof(pid_t));
    set_handlers();
    int pid;
    for (int i = 0; i < N; i++){
        if ((pid = fork()) == 0){
            printf ("Created process %d\n", getpid());
            int sleep_time = 1 + rand()%10;
            sleep(sleep_time);
            kill(getppid(), SIGUSR1);
            
            char* n = calloc(60, 1);
            sprintf(n, "Process %d received a signal from parent process\n", (int)getpid());
            write(1, n, 60);
            union sigval a;
            a.sival_int = 7;
            sigqueue(getppid(), rand()%(SIGRTMAX - SIGRTMIN) + SIGRTMIN, a);
            kill(SIGKILL, getpid());
        }
        else sleep(1);
    }
    while (wait(NULL) > 0) {}
}

void sigusr1_handler(int signum, siginfo_t* sig_info, void* sig_context){
    kill(sig_info -> si_pid, SIGSTOP);
    char* n = calloc(12, 1);
    write(1, "Received SIGUSR1 request from process ", 38);
    sprintf(n, "%d\n", (int)sig_info -> si_pid);
    write(1, n, 12);
    if (COUNT == K){
        for (int i = 0; i < K; i++){
            kill(SIGCONT, children[i]);
            write(1, "Sent SIGCONT permission to process ", 27);
            sprintf(n, "%d\n", (int)children[i]);
            write(1, n, 12);
        }
        kill(SIGCONT, sig_info -> si_pid);
        write(1, "Sent SIGCONT permission to process ", 27);
        sprintf(n, "%d\n", sig_info -> si_pid);
        write(1, n, 12);
    }
    else if (COUNT < K){
        children[COUNT] = sig_info -> si_pid;
    }
    else{
        kill(SIGCONT, sig_info -> si_pid);
        write(1, "Sent SIGCONT permission to process ", 27);
        sprintf(n, "%d\n", sig_info -> si_pid);
        write(1, n, 12);
    }
    COUNT++;
}

void sigusr2_handler(int signum, siginfo_t* sig_info, void* sig_context){
    char* n = calloc(60, 1);
    sprintf(n, "Process %d received a signal from parent process\n", (int)getpid());
    write(1, n, 60);
    union sigval a;
    a.sival_int = 7;
    sigqueue(getppid(), rand()%(SIGRTMAX - SIGRTMIN) + SIGRTMIN, a);
    kill(SIGKILL, getpid());
}

void real_time_handler(int signum, siginfo_t* sig_info, void* sig_context){
    char* n = calloc(12, 1);
    write(1, "Received SIGRTMIN+", 18);
    sprintf(n, "%d", (signum-SIGRTMIN));
    write(1, n, 12);
    write(1, " signal from ", 14);
    n = calloc(12, 1);
    sprintf(n, "%d", sig_info -> si_pid);
    write(1, n, 12);
    write(1, "\nFinished ", 11);
    n = calloc(12, 1);
    sprintf(n, "%d", sig_info -> si_pid);
    write(1, n, 12);
    write(1, " with return value ", 19);
    n = calloc(12, 1);
    sprintf(n, "%d", (sig_info -> si_value).sival_int);
    write(1, n, 12);
    write(1, "\n", 1);
}

void set_handlers(){
    struct sigaction act;
    sigfillset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO | SA_RESTART;
    act.sa_sigaction = &sigusr1_handler;
    sigaction(SIGUSR1, &act, NULL);

    act.sa_sigaction = &real_time_handler;
    for(int i = SIGRTMIN; i <= SIGRTMAX; i++){
      sigaction(i, &act, NULL);
    }
}