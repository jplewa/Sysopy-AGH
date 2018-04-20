#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>

#define MAX_ARGS 10
#define MAX_PIPES 10

int words(const char* sentence){
    int count = 0;
    char last_char;
    int len = strlen(sentence);
    if(len > 0) last_char = sentence[0];
    for(int i = 0; i <= len; i++){
        if((sentence[i] == ' ' || sentence[i] == '\0') && last_char != ' ' && last_char != '|') count++;
        last_char = sentence[i];
    }
    return count;
}

int commands(const char* sentence){
    int count = 1;
    char last_char;
    int len = strlen(sentence);
    if(len > 0) last_char = sentence[0];
    for(int i = 0; i <= len; i++){
        if (last_char == '|' && sentence[i] == '|') return -1;
        else if(sentence[i] == '|') count++;
        last_char = sentence[i];
    }
    return count;
}

int parse_file(char* file_name){
    int** fd;
    int line = 0;
    FILE* file = fopen(file_name, "r");
    if (file == NULL) return -2;
    char* buffer = malloc(1024);
    char* words_tmp;
    char* cmd_tmp;
    const char* words_delim = " \n";
    const char* cmd_delim = "|\n";
    int pid;
    size_t n = 1024;
    int i, j;
    char* c;
    char* c2;
    while (getline(&buffer, &n, file) != -1){        
        line++;
        printf("Line %d: %s", line, buffer);
        int cmd_count = commands(buffer);
        if (cmd_count >= MAX_PIPES+1) return -9;
        char*** cmd = malloc((cmd_count)*sizeof(char**));
        i = 0;
        cmd_tmp = strtok_r(buffer, cmd_delim, &c);
        while (cmd_tmp != NULL){
            int words_count = words(cmd_tmp);
            if (words_count >= MAX_ARGS-1) return -10;
            if (words_count != 0){
                cmd[i] = malloc((words_count+1)*sizeof(char*));
                j = 0;
                words_tmp = strtok_r(cmd_tmp, words_delim, &c2);
                while(words_tmp != NULL){
                    cmd[i][j] = malloc(strlen(words_tmp)+1);
                    strncpy(cmd[i][j], words_tmp, strlen(words_tmp));
                    cmd[i][j][strlen(words_tmp)] = '\0';
                    words_tmp = strtok_r(NULL, words_delim, &c2);
                    j++;
                }
                cmd[i][j] = NULL;
                cmd_tmp = strtok_r(NULL, cmd_delim, &c);
                i++;
            }
        }
        fd = malloc((i-1)*sizeof(int*));
        for (int l = 0; l < i-1; l++){
            fd[l] = malloc(2*sizeof(int));
            if (pipe(fd[l]) == -1) return -4;
        }
        printf("Line %d: ", line);
        for (int l = 0; l < i; l++){
            if ((pid = fork()) < 0) return -3;
            else if (pid == 0){
                if (i != 1){
                    if (l == 0){
                        if (dup2(fd[l][1], STDOUT_FILENO) == -1) return -5;
                    }
                    else if (l < i-1){
                        if (dup2(fd[l][1], STDOUT_FILENO) == -1) return -5;    
                        if (dup2(fd[l-1][0], STDIN_FILENO) == -1) return -5;  
                    }
                    else{
                        if (dup2(fd[l-1][0], STDIN_FILENO) == -1) return -5;
                    }
                }
                for (int k = 0; k < i-1; k++){
                    close(fd[k][0]);
                    close(fd[k][1]);
                }
                execvp(cmd[l][0], cmd[l]);
                exit(2);
            }
            else if (pid > 0){
                printf("<%d> ", pid);
                if (l == i-1){
                    for (int k = 0; k < i-1; k++){
                        close(fd[k][0]);
                        close(fd[k][1]);
                    }
                }
            }
        }
        printf("\n");
        int status = 0;
        while ((pid = waitpid(-1, &status, 0)) > 0){
            if (!WIFEXITED(status)) return line;
            if (WIFEXITED(status) && WEXITSTATUS(status) != 0) return line;
            else printf("<%d> finished\n", pid);
        }
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
                printf("Error: Requested batch file doesn't exist\n");
                break;
            case -3:
                perror("Error");
                printf("Unable to fork process\n");
                break;
            case -4:
                perror("Error");
                printf("Unable to create a pipe\n");
                break;
            case -5:
                perror("Error");
                printf("Unable to duplicate descriptor\n");
                break;
            case -6:
                perror("Error");
                printf("Unable to duplicate descriptor\n");
                break;
            case -7:
                printf("An unknown error occcured while waiting for the child proceess\n");
                break;
            case -8:
                perror("Error");
                printf("The child process couldn't be created\n");
                break;
            case -9:
                printf("Error: the limit of pipes is %d\n", MAX_PIPES);
                break;
            case -10:
                printf("Error: the limit of arguments is %d\n", MAX_ARGS);
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