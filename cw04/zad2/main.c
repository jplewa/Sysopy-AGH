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
void print_number(int n);
void real_time_handler(int signum, siginfo_t* sig_info, void* sig_context);
void sigusr1_handler(int signum, siginfo_t* sig_info, void* sig_context);
void real_time_handler(int signum, siginfo_t* sig_info, void* sig_context);
void exit_handler(int signum, siginfo_t* sig_info, void* sig_context);
void sigint_handler(int signum, siginfo_t* sig_info, void* sig_context);
void set_handlers();

const char* msg1 = "Received SIGUSR1 request from process ";
const char* msg2 = "Sent SIGCONT permission to process ";
const char* msg3a = "Received SIGRTMIN+";
const char* msg3b = " signal from ";
const char* msg4a = "Process ";
const char* msg4b = " exited with status ";
const char* msg_sigint = "\nSIGINT signal was received\nAll children have been terminated\n";

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

void print_number (int n){
    char buffer[12];
    int digits = 0;
    int tmp = n;
    while (tmp > 0){
        digits++;
        tmp/=10;
    }
    buffer[digits] = '\0';
    while (digits > 0){
        digits--;
        buffer[digits] = 48 + n%10;
        n/=10;
    } 
    write(1, buffer, strlen(buffer));
}

void sigusr1_handler(int signum, siginfo_t* sig_info, void* sig_context){
    if (options[1]){
        write(1, msg1, 38);
        print_number((int)sig_info -> si_pid);
        write(1,"\n\0", 2);
    }
    children[K_INDEX] = sig_info -> si_pid;
    K_INDEX++;
    if (K_INDEX == K){
        for (int i = 0; i < K; i++){
            kill(children[i], SIGCONT);
            if (options[2]){
                write(1, msg2, 35);
                print_number((int)children[i]);
                write(1, "\n\0", 2);
            }
        }
    }
    else if (K_INDEX > K){
        kill(sig_info -> si_pid, SIGCONT);
        if (options[2]){
            write(1, msg2, 35);
            print_number((int)sig_info -> si_pid);
            write(1, "\n\0", 2);
        }
    }
}

void real_time_handler(int signum, siginfo_t* sig_info, void* sig_context){
    if (options[3]){
        write(1, msg3a, 18);
        print_number(signum-SIGRTMIN);
        write(1, msg3b, 13);
        print_number((int) sig_info -> si_pid);
        write(1, "\n\0", 2);
    }
}

void exit_handler(int signum, siginfo_t* sig_info, void* sig_context){
    pid_t pid;
    int status;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0){
        if (WIFEXITED(status)){
            if (options[4]){
                write(1, msg4a, 8);
                print_number(pid);
                write(1, msg4b, 20);
                print_number(WEXITSTATUS(status));
                write(1, "\n\0", 2);
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
        write(1, msg_sigint, 62);
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