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
    __clock_t user_time;
    __clock_t system_time;
} typedef full_time;

full_time time_stamp(){
    full_time time_stamp;
    struct tms time_s_u;
    times(&time_s_u);
    time_stamp.system_time = time_s_u.tms_stime;
    time_stamp.user_time = time_s_u.tms_utime;
    return  time_stamp;
}
void time_output(full_time start, full_time end){
    printf("\tuser time: %9.6f\t| system time: %9.6f |\n", 
        (float) (end.user_time - start.user_time) / (float) sysconf(_SC_CLK_TCK), 
        (float)(end.system_time - start.system_time) / (float) sysconf(_SC_CLK_TCK));
}
void generate (char* filename, int size, int record_size){
    const char* mode = "w";
    FILE* file =  fopen(filename, mode);
    for (int i = 0; i < size; i++){
        for (int j = 0; j < record_size; j++){
            char c = (char)('!' + rand()%58);
            fwrite(&c, 1, 1, file);
        }
    }
    fclose(file);
}
void sys_sort(char* filename, int size, int record_size){
    full_time start = time_stamp();
    int file = open(filename,O_RDWR|S_IRUSR|S_IWUSR);
    char* key = malloc(record_size);
    char* tmp = malloc(record_size);
    for (int i = 1; i < size; i++){
        lseek (file, (i*record_size), 0);
        read(file, key, record_size);
        int j = i - 1;
        lseek (file, (j*record_size), 0);
        read(file, tmp, record_size);
        while (j >= 0 && tmp[0] > key[0]){
            lseek (file, ((j+1)*record_size), 0);
            write(file, tmp, record_size);
            j--;
            lseek (file, (j*record_size), 0);
            read(file, tmp, record_size);
        }
        lseek (file, (j+1)*record_size, 0);
        write(file, key, record_size);
    }
    close(file);
    full_time end = time_stamp();
    time_output(start, end);
}
void lib_sort(char* filename, int size, int record_size){
    full_time start = time_stamp();
    FILE* file = fopen(filename, "r+");
    char* key = calloc(record_size, 1);
    char* tmp = calloc(record_size, 1);
    for (int i = 1; i < size; i++){
        fseek (file, (i*record_size), 0);
        fread(key, 1, record_size, file);
        int j = i - 1;
        fseek (file, (j*record_size), 0);
        fread(tmp, 1, record_size, file);
        while (j >= 0 && tmp[0] > key[0]){
            fseek (file, ((j+1)*record_size), 0);
            fwrite(tmp, 1, record_size, file);
            j--;
            fseek (file, (j*record_size), 0);
            fread(tmp, 1, record_size, file);
            
        }
        fseek (file, (j+1)*record_size, 0);
        fwrite(key, 1, record_size, file);
    }
    fclose(file);
    full_time end = time_stamp();
    time_output(start, end);
}
void sys_copy(char* filename1, char* filename2, int size, int record_size){
    int in_file = open(filename1, O_RDONLY);
    int out_file = open(filename2,O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
    int bytes_count = 0;
    int count = 0;
    char* buffer = malloc(record_size);
    full_time start = time_stamp();
    while((bytes_count = read(in_file, buffer, record_size)) > 0 && count < size){
        write(out_file, buffer, bytes_count);
        count++;
    }
    full_time end = time_stamp();
    time_output(start, end);
}
void lib_copy(char* filename1, char* filename2, int size, int record_size){
    FILE* in_file = fopen(filename1, "r");
    FILE* out_file = fopen(filename2, "w");
    char* buffer = malloc(record_size);
    int bytes_count = 0;
    int count = 0;
    full_time start = time_stamp();
    while (0 < (bytes_count = fread(buffer, 1, record_size, in_file)) && count < size){
        fwrite(buffer, 1, bytes_count, out_file);
        count++;
    }
    full_time end = time_stamp();
    time_output(start, end);
    fclose(in_file);
    fclose(out_file);
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
        else if(strncmp(argv[6], "lib", 3) == 0) lib_copy(argv[2], argv[3], size, record_size);
    }
    else if(strncmp(argv[1], "sort", 4) == 0){
        int size = (int) strtol(argv[3],&pEnd,10);
        int record_size = (int) strtol(argv[4],&pEnd,10);
        if (size == 0 || record_size == 0) return;
        else if(strncmp(argv[5], "sys", 3) == 0) sys_sort(argv[2], size, record_size);
        else if(strncmp(argv[5], "lib", 3) == 0) lib_sort(argv[2], size, record_size);
    }
}
int main(int argc, char* argv[]){
    parse(argc, argv);
}