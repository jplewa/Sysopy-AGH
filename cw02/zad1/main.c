#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h> 
#include <time.h>
#include <sys/times.h>

struct full_time{
    __clock_t real_time;
    __clock_t user_time;
    __clock_t system_time;
} typedef full_time;

full_time time_stamp(){
    full_time time_stamp;
    struct timespec time_r;
    clock_gettime(CLOCK_REALTIME, &time_r);
    time_stamp.real_time = time_r.tv_nsec + time_r.tv_sec * 1000000000;
    struct tms time_s_u;
    times(&time_s_u);
    time_stamp.system_time = time_s_u.tms_stime;
    time_stamp.user_time = time_s_u.tms_utime;
    return  time_stamp;
}
void time_output(full_time start, full_time end){
    printf(" real time: %f\n user time: %f\n system time: %f\n", 
        (float)(end.real_time - start.real_time) / 1000000000.0, 
        (float) (end.user_time - start.user_time) / (float) sysconf(_SC_CLK_TCK), 
        (float)(end.system_time - start.system_time) / (float) sysconf(_SC_CLK_TCK));
}

void generate (char* filename, int size, int record_size){
    const char* mode = "w";
    FILE* file =  fopen(filename, mode);
    for (int i = 0; i < size; i++){
        for (int j = 0; j < record_size - 1; j++){
            char c = (char)(' ' + rand()%58);
            fwrite(&c, 1, 1, file);
        }
        char c = (char) 10;
        fwrite(&c, 1, 1, file);
    }
    fclose(file);
}
void sys_sort (){

}
void lib_sort(){

}
void sys_copy(char* filename1, char* filename2, int size, int record_size){
    int in_file = open(filename1, O_RDONLY);
    int out_file = open(filename2,O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
    int bytes_count = 0;
    int count = 0;
    int* buffer = malloc(record_size);
    full_time start = time_stamp();
    while((bytes_count = read(in_file, buffer, record_size)) > 0 && count < size){
        write(out_file, buffer, bytes_count);
        count++;
    }
    full_time end = time_stamp();
    time_output(start, end);
}
void lib_copy(){

}

void parse(int argc, char* argv[]){
    if (argc < 5) return;
    char * pEnd;
    if (strncmp(argv[1], "generate", 8) == 0){
        int size = (int) strtol(argv[3],&pEnd,10);
        int record_size = (int) strtol(argv[4],&pEnd,10);
        if (size == 0 || record_size == 0) return;
        else generate(argv[2], size, record_size);
    }
    else if(strncmp(argv[1], "copy", 4) == 0){
        int size = (int) strtol(argv[4],&pEnd,10);
        int record_size = (int) strtol(argv[5],&pEnd,10);
        if (size == 0 || record_size == 0) return;
        else if(strncmp(argv[6], "sys", 3) == 0) sys_copy(argv[2], argv[3], size, record_size);
    }

}


int main(int argc, char* argv[]){
    parse(argc, argv);
}