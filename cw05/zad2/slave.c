#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <time.h> 

void print_error(int result){
    switch(result){
        case -1:
            printf("Error: incorrect arguments\n");
            printf("Please provide: [number_of_passes] [pipe_file_name]\n");
            break;
        case -3:
            perror("Error");
            printf("Slave: A problem occured while trying to open pipe\n");
            break;
    }
}

int process_fifo(int argc, char* argv[]){
    int N = 0;
    if (argc < 3) return -1;    
    FILE* fd;
    char* pEnd;
    N =  strtol (argv[1], &pEnd, 10);
    if (N == 0) return -1;
    printf("<%d>\n", getpid());
    char* line;
    for (int i = 0; i < N; i++){
        if ((fd = fopen(argv[2], "w")) == NULL) return -3;
        line = calloc(100, 1);
        sprintf(line, "<%d> ", getpid());
        fwrite(line, 1, strlen(line), fd);
        FILE* date = popen("date", "r");
        char* line = NULL;
        size_t len = 0;
        getline(&line, &len, date) ;
        pclose(date);
        fwrite(line, 1, strlen(line), fd);
        fclose(fd);
        srand(time(NULL));
        sleep(2+rand()%4);
    }
    return 0;
}

int main(int argc, char* argv[]){
    print_error(process_fifo(argc, argv));
    return 0;
}