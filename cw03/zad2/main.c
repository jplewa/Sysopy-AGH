#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>

#define MAX_ARGS 10

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
            if (execvp(arguments[0], arguments) < 0) exit(2);
        }
        else if (pid > 0){
            int status = 0;
            if (waitpid(pid, &status, 0) == -1) return -7;
            if (WIFEXITED(status) && WEXITSTATUS(status) != 0) return line;
        }
        else return -8;
    } 
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
                printf("Incorrect arguments\n");
                printf("Please provide batch file name\n");
                break;
            case -2:
                perror("Error");
                printf("Requested batch file doesn't exist\n");
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
    if (argc < 2) result = -1;
    if (result == 0){
        result = parse_file(argv[1]);
    }
    print_errors(result);
    return 0;
}