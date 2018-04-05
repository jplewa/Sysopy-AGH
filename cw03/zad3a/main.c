#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <limits.h>

struct rlimit* TIME_LIMIT;
struct rlimit* MEM_LIMIT;

int words(const char* sentence){
    int count = 0;
    char last_char;
    int len = strlen(sentence);
    if(len > 0) last_char = sentence[0];
    for(int i = 0; i <= len; i++){
        if((sentence[i] == ' ' || sentence[i] == '\0') && last_char != ' ') count++;
        last_char = sentence[i];
    }
    return count;
}

int parse_file(char* file_name){
    int line = 0;
    FILE* file = fopen(file_name, "r");
    if (file == NULL) return -2;
    char* buffer = malloc(1024);
    char* tmp;
    const char* delim = " \n";
    int pid;
    size_t n = 1024;
    while (getline(&buffer, &n, file) != -1){
        line++;
        int count = words(buffer);
        char** arguments = malloc((count+1)*sizeof(char*));
        int i = 0;
        tmp = strtok(buffer, delim);
        while (tmp != NULL){
            arguments[i] = malloc(strlen(tmp)+1);
            strncpy(arguments[i], tmp, strlen(tmp));
            arguments[i][strlen(tmp)] = '\0';
            tmp = strtok (NULL, delim);
            i++;
        }
        arguments[count] = NULL;
        pid = fork();
        if (pid == 0){
            if (setrlimit(RLIMIT_CPU, TIME_LIMIT) != 0) return -5;
            if (setrlimit(RLIMIT_AS, MEM_LIMIT) != 0) return -6;     
            if (execvp(arguments[0], arguments) < 0) exit(2);
        }
        else if (pid > 0){       
            int status = 0;
            struct rusage* rusage = malloc(sizeof(struct rusage));
            if (wait4(pid, &status, 0, rusage) == -1) return -7;
            else{
                if (WIFEXITED(status) && WEXITSTATUS(status) != 0){
                    return line;
                }
                else{
                    printf("***********************************************************\n");
                    printf("* pid: %d\n* command: %s\n* user CPU time: %ld.%06ld\n* system CPU time: %ld.%06ld\n", pid, buffer, \
                        rusage -> ru_utime.tv_sec, rusage -> ru_utime.tv_usec, rusage -> ru_stime.tv_sec, rusage -> ru_stime.tv_usec);
                    printf("***********************************************************\n");
                }   
            }
        }
        else return -8;
    } 
    fclose(file);
    return 0;
}

void print_errors(int error_code){
    if (error_code > 0){
        printf("Error executing command in line %d\n", error_code);
    }
    else{
        switch (error_code){
            case 0:
                printf("Commands executed successfully\n");
                break;
            case -1:
                printf("Error: Incorrect arguments\n");
                printf("Please provide: [batch file name] [time limit (s)] [memory limit (MB)]\n");
                break;
            case -2:
                perror("Error");
                printf("Requested batch file doesn't exist\n");
                break;
            case -3:
                printf("Incorrect argument: time_limit (s)\n");
                printf("Please provide: [batch file name] [time limit (s)] [memory limit (MB)]\n");
                break;
            case -4:
                printf("Incorrect argument: mem_limit (MB)\n");
                printf("Please provide: [batch file name] [time limit (s)] [memory limit (MB)]\n");
                break;
            case -5:
                printf("Error: couldn't set CPU time limit\n");
                break;
            case -6:
                printf("Error: couldn't set virtual memory size limit\n");
                break;
            case -7:
                printf("An unknown error occcured while waiting for the child proceess\n");
                break;
            case -8:
                perror("Error");
                printf("The child process couldn't be created\n");
                break;
        }
    }
}

int main(int argc, char* argv[], char* envp[]){
    int result = 0;
    if (argc < 4) result = -1;
    long time_limit = 0;
    long mem_limit = 0;
    if (result == 0){
        char * pEnd;
        time_limit =  strtol (argv[2], &pEnd, 10);
        if (time_limit == 0 || time_limit == LONG_MAX || time_limit == LONG_MIN) result = -3;
        mem_limit =  strtol (argv[3], &pEnd, 10);
        if (mem_limit == 0 || mem_limit == LONG_MAX || mem_limit == LONG_MIN) result = -4;
    }
    if (result == 0){
        TIME_LIMIT = malloc(sizeof(struct rlimit));
        TIME_LIMIT -> rlim_max = time_limit;
        TIME_LIMIT -> rlim_cur = time_limit;
        MEM_LIMIT = malloc(sizeof(struct rlimit));
        MEM_LIMIT -> rlim_max = mem_limit * 1024 * 1024;
        MEM_LIMIT -> rlim_cur = mem_limit * 1024 * 1024;        
    }
    if (result == 0) result = parse_file(argv[1]);
    print_errors(result);
    return 0;
}