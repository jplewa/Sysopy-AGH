#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h> 

void print_error(int result){
    switch(result){
        case -1:
            printf("Error: incorrect arguments\n");
            printf("Please provide FIFO file path\n");
            break;
        case -2:
            perror("Error");
            printf("Master: A problem occured while trying to create pipe\n");
            break;
        case -3:
            perror("Error");
            printf("Master: A problem occured while trying to open pipe\n");
            break;
    }
}

int process_fifo(int argc, char* argv[]){
    if (argc < 2) return -1;
    if (mkfifo(argv[1], 0777) != 0) return -2;

    FILE* fd;
    char* line = NULL;
    ssize_t amount_read = 0;
    size_t len = 0;
    while(1){
        if ((fd = fopen(argv[1], "r")) == NULL) return -3;
        while ((amount_read = getline(&line, &len, fd)) > 0) printf("%s", line);
        sleep(1);
    }
    return 0;
}

int main(int argc, char* argv[]){
    print_error(process_fifo(argc, argv));
    return 0;
}