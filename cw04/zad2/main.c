#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

int parse(int argc, char* argv[]);
void print_error(int error_code);
void children_processes();
void real_time_handler(int signum, siginfo_t* sig_info, void* sig_context);
void sigusr1_handler(int signum, siginfo_t* sig_info, void* sig_context);
void set_handlers();

int N = 0;
int K = 0;
int COUNT = 0;
int K_INDEX = 0;
pid_t* children;
int ppid;

int options[5];

int main(int argc, char* argv[]){
    int result = parse(argc, argv);
    if (result == 0){
        COUNT = N;
        children_processes();
    }
    print_error(result);
    return 0;
}

int parse(int argc, char* argv[]){
    if (argc < 3) return -1;
    char * pEnd;
    N =  strtol (argv[1], &pEnd, 10);
    K =  strtol (argv[2], &pEnd, 10);
    if (N == 0 || K == 0 || K > N) return -1;
    for (int i = 0; i < 5; i++) options[i] = 0;
    
    if (argc >= 4) {
        for (int j = 3; j < argc; j++){
            for (int i = 0; i < strlen(argv[j]); i++){
                switch(argv[j][i]){
                    case 'c':
                        options[0] = 1;
                        break;
                    case 'r':
                        options[1] = 1;
                        break;
                    case 'p':
                        options[2] = 1;
                        break;
                    case 's': 
                        options[3] = 1;
                        break;
                    case 'e':
                        options[4] = 1;
                        break;
                    case '-':
                        break;
                    default:
                        return -1;
                }
            }
        }
    }
    return 0;
}

void print_error(int error_code){
    switch (error_code){
        case 0:
            printf("Everything completed successfully\n");
            break;
        case -1:
            printf("Error: Incorrect arguments\n");
            printf("Please provide: [number_of_processes] [number_of_requests] [options]\n");
            printf("Available options are:\n");
            printf("\tc: output information about created processes\n");
            printf("\tr: output information about received requests\n");
            printf("\tp: output information about granted permissions\n");
            printf("\ts: output information about received real-time signals\n");
            printf("\te: output information about childrens' exit status\n");
            break;
    }
}

void children_processes(){
    children = malloc(N*sizeof(pid_t));
    set_handlers();
    ppid = getpid();
    int pid;
    for (int i = 0; i < N; i++){
        if ((pid = fork()) == 0){
            if (options[0]) printf ("Created process %d\n", getpid());
            srand(time(NULL) ^ (getpid()<<16));
            int sleep_time = 1000000 + rand()%10000000;
            usleep(sleep_time);
            kill(getppid(), SIGUSR1);
            kill(getpid(), SIGSTOP);
            union sigval a;
            a.sival_int = sleep_time/1000000;
            sigqueue(getppid(), rand()%(SIGRTMAX - SIGRTMIN) + SIGRTMIN, a);
            exit(sleep_time/1000000);
        }
        else sleep(1);
    }
    while(COUNT > 0){}
    print_error(0);
    free(children);
    exit(0);
}

void sigusr1_handler(int signum, siginfo_t* sig_info, void* sig_context){
    char* buffer;
    if (options[1]){
        buffer = calloc(100, 1);
        sprintf(buffer, "Received SIGUSR1 request from process %d\n", (int)sig_info -> si_pid);
        write(1, buffer, 100);
        free(buffer);
    }
    children[K_INDEX] = sig_info -> si_pid;
    K_INDEX++;
    if (K_INDEX == K){
        for (int i = 0; i < K; i++){
            kill(children[i], SIGCONT);
            if (options[2]){
                buffer = calloc(100, 1);
                sprintf(buffer, "Sent SIGCONT permission to process %d\n", (int)children[i]);
                write(1, buffer, 100);
                free(buffer);
            }
        }
    }
    else if (K_INDEX > K){
        kill(sig_info -> si_pid, SIGCONT);
        if (options[2]){
            buffer = calloc(100, 1);
            sprintf(buffer, "Sent SIGCONT permission to process %d\n", sig_info -> si_pid);
            write(1, buffer, 100);
            free(buffer);
        }
    }
}

void real_time_handler(int signum, siginfo_t* sig_info, void* sig_context){
    if (options[3]){
        char* buffer = calloc(100, 1);
        sprintf(buffer, "Received SIGRTMIN+%d signal from %d\n", (signum-SIGRTMIN), (int) sig_info -> si_pid);
        write(1, buffer, 100);
        free(buffer);
    }
}

void exit_handler(int signum, siginfo_t* sig_info, void* sig_context){
    char* buffer;
    pid_t pid;
    int status;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0){
        if (WIFEXITED(status)){
            if (options[4]){
                buffer = calloc(50, 1);
                sprintf(buffer, "Process %d exited with status %d\n", pid, WEXITSTATUS(status));
                write(1, buffer, 50);
                free(buffer);
            }
            COUNT--;
        }
    }
}

void sigint_handler(int signum, siginfo_t* sig_info, void* sig_context){
    if (getpid() == ppid){
        for (int i = 0; children[i] != 0; i++){
            kill(children[i], SIGKILL);
        }

        char* buffer = calloc(80, 1);
        sprintf(buffer, "\nSIGINT signal was received\nAll children have been terminated\n");
        write(1, buffer, 80);
        free(buffer);
        exit(0);
    }
}

void set_handlers(){
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    sigfillset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO | SA_RESTART;

    act.sa_sigaction = &real_time_handler;
    for(int i = SIGRTMIN; i <= SIGRTMAX; i++){
      sigaction(i, &act, NULL);
    }
    act.sa_flags |= SA_NOCLDSTOP;
    act.sa_sigaction = &exit_handler;
    sigaction(SIGCHLD, &act, NULL);
    
    act.sa_sigaction = &sigint_handler;
    sigaction(SIGINT, &act, NULL);
    
    act.sa_flags |= SA_NODEFER;
    act.sa_sigaction = &sigusr1_handler;
    sigaction(SIGUSR1, &act, NULL);
}