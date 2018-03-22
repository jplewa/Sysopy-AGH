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
    FILE* file;
    if ((file =  fopen(filename, mode)) == NULL){
        printf("Error: the file couldn't be opened or created\n");
        return;
    }
    for (int i = 0; i < size * record_size; i++){
        char c = (char)('!' + rand()%58);
        if(fwrite(&c, 1, 1, file) < 1){
            printf("Error: a problem occured while writing into the file.\n");
            return;
        }
    }
    fclose(file);
}
void sys_sort(char* filename, int size, int record_size){
    full_time start = time_stamp();
    int file = open(filename,O_RDWR|S_IRUSR|S_IWUSR);
    if (file < 0){
        printf("Error: the file couldn't be opened or created\n");
        return;
    }
    char* key = malloc(record_size);
    char* tmp = malloc(record_size);
    for (int i = 1; i < size; i++){
        if (lseek (file, (i*record_size), 0) == -1){
            printf("Error: a problem occured while accessing the file.\n");
            return;
        }
        if (read(file, key, record_size) < 0){
            printf("Error: a problem occured while accessing the file.\n");
            return;
        }
        int j = i - 1;
        if (lseek (file, (j*record_size), 0) == -1){
            printf("Error: a problem occured while accessing the file.\n");
            return;
        }
        if (read(file, tmp, record_size) < 0){
            printf("Error: a problem occured while accessing the file.\n");
            return;
        }
        while (j >= 0 && tmp[0] > key[0]){
            if (lseek (file, ((j+1)*record_size), 0)  == -1){
                printf("Error: a problem occured while accessing the file.\n");
                return;
            }
            if (write(file, tmp, record_size) < 0){
                printf("Error: a problem occured while accessing the file.\n");
                return;
            }
            j--;
            if (j > 0){
                if (lseek (file, (j*record_size), 0) == -1){
                    printf("Error: a problem occured while accessing the file.\n");
                    return;
                }
                if (read(file, tmp, record_size) < 0){
                    printf("Error: a problem occured while accessing the file.\n");
                    return;
                }
            }
        }
        if (lseek (file, (j+1)*record_size, 0) == -1){
            printf("Error: a problem occured while accessing the file.\n");
            return;
        }
        if (write(file, key, record_size) < 0){
            printf("Error: a problem occured while accessing the file.\n");
            return;
        }
    }
    free(key);
    free(tmp);
    if (close(file) != 0){
        printf("Error: a problem occured while closing the file.\n");
        return;
    }
    full_time end = time_stamp();
    time_output(start, end);
}
void lib_sort(char* filename, int size, int record_size){
    full_time start = time_stamp();
    FILE* file;
    if ((file = fopen(filename, "r+")) == NULL){
        printf("Error: the file couldn't be opened or created\n");
        return;
    }
    char* key = calloc(record_size, 1);
    char* tmp = calloc(record_size, 1);
    for (int i = 1; i < size; i++){
        if (fseek (file, (i*record_size), 0) != 0){
            printf("Error: a problem occured while accessing the file.\n");
            return;
        }
        if (fread(key, 1, record_size, file) != record_size){
            printf("Error: a problem occured while accessing the file.\n");
            return;
        }
        int j = i - 1;
        if (fseek (file, (j*record_size), 0) != 0){
            printf("Error: a problem occured while accessing the file.\n");
            return;
        }
        if (fread(tmp, 1, record_size, file) != record_size){
            printf("Error: a problem occured while accessing the file.\n");
            return;
        }
        while (j >= 0 && tmp[0] > key[0]){
            if (fseek (file, ((j+1)*record_size), 0) != 0){
                printf("Error: a problem occured while accessing the file.\n");
                return;
            }
            if (fwrite(tmp, 1, record_size, file) != record_size){
                printf("Error: a problem occured while accessing the file.\n");
                return;
            }
            j--;
            if (j > 0){
                if (fseek (file, (j*record_size), 0) != 0){
                    printf("Error: a problem occured while accessing the file.\n");
                    return;
                }
                if (fread(tmp, 1, record_size, file) != record_size){
                    printf("Error: a problem occured while accessing the file.\n");
                    return;
                }
            }
        }
        if (fseek (file, (j+1)*record_size, 0) != 0){
            printf("Error: a problem occured while accessing the file.\n");
            return;
        }
        if (fwrite(key, 1, record_size, file) != record_size){
            printf("Error: a problem occured while accessing the file.\n");
            return;
        }
    }
    free(key);
    free(tmp);
    if (fclose(file) != 0){
        printf("Error: a problem occured while closing the file.\n");
        return;
    }
    full_time end = time_stamp();
    time_output(start, end);
}
void sys_copy(char* filename1, char* filename2, int size, int record_size){
    int in_file = open(filename1, O_RDONLY);
    int out_file = open(filename2,O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
    if (in_file < 0 || out_file < 0){
        printf("Error: a problem occured while opening the files.\n");
        return;
    }
    int bytes_count = 0;
    int count = 0;
    char* buffer = malloc(record_size);
    full_time start = time_stamp();
    while((bytes_count = read(in_file, buffer, record_size)) > 0 && count < size){
        if (write(out_file, buffer, bytes_count) != bytes_count){
            printf("Error: a problem occured while accessing the files.\n");
            return;
        }
        count++;
    }
    free(buffer);
    if (close(in_file) != 0){
        printf("Error: a problem occured while closing the file.\n");
        return;
    }
    if (close(out_file) != 0){
        printf("Error: a problem occured while closing the file.\n");
        return;
    }
    full_time end = time_stamp();
    time_output(start, end);
}
void lib_copy(char* filename1, char* filename2, int size, int record_size){
    FILE* in_file = fopen(filename1, "r");
    FILE* out_file = fopen(filename2, "w");
    if (in_file == NULL || out_file == NULL){
        printf("Error: a problem occured while opening the files.\n");
        return;
    }
    char* buffer = malloc(record_size);
    int bytes_count = 0;
    int count = 0;
    full_time start = time_stamp();
    while (0 < (bytes_count = fread(buffer, 1, record_size, in_file)) && count < size){
        if (fwrite(buffer, 1, bytes_count, out_file) != bytes_count){
            printf("Error: a problem occured while accessing the files.\n");
            return;
        }
        count++;
    }
    free(buffer);
    if (fclose(in_file) != 0){
        printf("Error: a problem occured while closing the files.\n");
        return;
    }
    if (fclose(out_file) != 0){
        printf("Error: a problem occured while closing the files.\n");
        return;
    }
    full_time end = time_stamp();
    time_output(start, end);
}
void parse(int argc, char* argv[]){
    if (argc < 5){
        printf("Incorrect arguments!\n");
        printf("Operations available: generate, copy, sort.\n");
        printf("Arguments needed for generate: [file] [number of records] [record size] [type]\n");
        printf("Arguments needed for copy: [source file] [destination file] [number of records] [record size] [type]\n");
        printf("Arguments needed for sort: [file] [number of records] [record size] [type]\n");
        return;
    }
    char * pEnd = malloc(sizeof(char));
    FILE* file_test = malloc(sizeof(FILE));
    if (strncmp(argv[1], "generate", 8) == 0){
        int size = (int) strtol(argv[3],&pEnd,10);
        int record_size = (int) strtol(argv[4],&pEnd,10);
        if (size == 0 || record_size == 0){
            printf("Incorrect arguments!\n");
            printf("Arguments needed for generate: [file] [number of records] [record size] [type]\n");
            return; 
        }
        else generate(argv[2], size, record_size);
    }
    else if(strncmp(argv[1], "copy", 4) == 0){
        int size = (int) strtol(argv[4],&pEnd,10);
        int record_size = (int) strtol(argv[5],&pEnd,10);
        if (size == 0 || record_size == 0){
            printf("Incorrect arguments!\n");
            printf("Arguments needed for copy: [source file] [destination file] [number of records] [record size] [type]\n");
            return;
        }
        else if ((file_test = fopen(argv[2], "r")) == NULL){
            printf("Source file doesn't exist!\n");
            printf("Arguments needed for copy: [source file] [destination file] [number of records] [record size] [type]\n"); 
            return;
        }
        fclose(file_test);
        if(strncmp(argv[6], "sys", 3) == 0) sys_copy(argv[2], argv[3], size, record_size);
        else if(strncmp(argv[6], "lib", 3) == 0) lib_copy(argv[2], argv[3], size, record_size);
        else{
            printf("Incorrect arguments! Missing \n");
            printf("Arguments needed for copy: [source file] [destination file] [number of records] [record size] [type]\n");
            return;
        }
    }
    else if(strncmp(argv[1], "sort", 4) == 0){
        int size = (int) strtol(argv[3],&pEnd,10);
        int record_size = (int) strtol(argv[4],&pEnd,10);
        if (size == 0 || record_size == 0){
            printf("Incorrect arguments!\n");
            printf("Arguments needed for sort: [file] [number of records] [record size] [type]\n");
            return;
        }
        else if ((file_test = fopen(argv[2], "r")) == NULL){
            printf("Source file doesn't exist!\n");
            printf("Arguments needed for sort: [file] [number of records] [record size] [type]\n"); 
            return;
        }
        fclose(file_test);
        if(strncmp(argv[5], "sys", 3) == 0) sys_sort(argv[2], size, record_size);
        else if(strncmp(argv[5], "lib", 3) == 0) lib_sort(argv[2], size, record_size);
        else{
            printf("Incorrect arguments! Missing \n");
            printf("Arguments needed for sort: [file] [number of records] [record size] [type]\n");
            return;
        }
    }
}
int main(int argc, char* argv[]){
    parse(argc, argv);
}