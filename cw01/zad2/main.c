#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <sys/times.h>
#include <unistd.h> 
#include "library.h"

char* frame = "----------------------------------------------\n"; 

char* random_char_block (int block_size) {
    char *block = malloc(block_size * sizeof(char));
    for (int i = 0; i < block_size - 1; i++) block[i] = (char) 33 + rand() % 90;
    block[block_size - 1] = '\0';
    return block;
}
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
int main( int argc, char *argv[] )  {
    if (argc == 0){
        printf("You have to specify at least the following options: filename create_table [s/d] [array_size] [block_size]\n");
        return 0;
    }
    char* filename;
    if((filename = malloc((strlen(argv[1])+7)*sizeof(char))) != NULL){
        sprintf(filename, "./%s.txt",argv[1]);
    }
    else return 0;
    FILE *file;
    file = fopen(filename, "a");

    int array_size = 0;
    int block_size = 0;
    char array_type = '\0';
    bool argument_found = false;

    for (int i = 0; i < argc; i++){
        if (strcmp(argv[i], "create_table") == 0 && i+3 < argc){
            argument_found = true;
            array_type = argv[i+1][0];
            char * pEnd;
            array_size = (int) strtol (argv[i+2], &pEnd, 10);
            block_size = (int) strtol (argv[i+3], &pEnd, 10);
        }
    }
    if (!argument_found){
        printf("You have to specify the following option: create_table [s/d] [array_size] [block_size]\n");
        return 0;
    }
    if (array_size <= 0 || block_size <= 0) {
        printf("create_table: Incorrect array size\n");
        return 0;
    }
    if (array_type != 'd' && array_type != 's'){
        printf("create_table: Incorrect type of memory allocation\n");
        return 0;
    }
    if (array_type == 's'){
        printf("%s", frame);
        fprintf(file, "%s", frame);
        printf("STATIC ALLOCATION\n");
        fprintf(file, "STATIC ALLOCATION\n");
        printf("%s", frame);
        fprintf(file, "%s", frame);

        full_time start = time_stamp();
        s_block_array* test_array = s_create_array(array_size, block_size);
        full_time end = time_stamp();

        printf("CREATE ARRAY: \n real time: %f\n user time: %f\n system time: %f\n", 
                (float)(end.real_time - start.real_time) / 1000000000.0, 
                (float) (end.user_time - start.user_time) / (float) sysconf(_SC_CLK_TCK), 
                (float)(end.system_time - start.system_time) / (float) sysconf(_SC_CLK_TCK));
        fprintf(file, "CREATE ARRAY: \n real time: %f\n user time: %f\n system time: %f\n", 
                (float)(end.real_time - start.real_time) / 1000000000.0, 
                (float) (end.user_time - start.user_time) / (float) sysconf(_SC_CLK_TCK), 
                (float)(end.system_time - start.system_time) / (float) sysconf(_SC_CLK_TCK));

        for (int i = 0; i < argc; i++){
            if (strcmp(argv[i], "add") == 0){
                if (i+1 >= argc){
                    printf("ADD: You need to specify the number of blocks to be added");
                    return 0;
                }
                char * pEnd;

                int n = (int) strtol (argv[i+1], &pEnd, 10);
                char** random_blocks = malloc(n*sizeof(char*));
                for (int i = 0; i < array_size; i++){
                    random_blocks[i] = malloc(block_size*sizeof(char));
                    strncpy(random_blocks[i], random_char_block(block_size), block_size);
                }
                start = time_stamp();
                for (int j = 0; j < n; j++){
                    s_insert_block(test_array, random_blocks[i], rand()%array_size);
                }
                end = time_stamp();

                printf("ADD %d BLOCKS: \n real time: %f\n user time: %f\n system time: %f\n", n, 
                        (float)(end.real_time - start.real_time) / 1000000000.0, 
                        (float) (end.user_time - start.user_time) / (float) sysconf(_SC_CLK_TCK), 
                        (float)(end.system_time - start.system_time) / (float) sysconf(_SC_CLK_TCK));
                fprintf(file, "ADD %d BLOCKS: \n real time: %f\n user time: %f\n system time: %f\n", n, 
                        (float)(end.real_time - start.real_time) / 1000000000.0, 
                        (float) (end.user_time - start.user_time) / (float) sysconf(_SC_CLK_TCK), 
                        (float)(end.system_time - start.system_time) / (float) sysconf(_SC_CLK_TCK));
            }
            else if (strcmp(argv[i], "remove") == 0){
                if (i+1 >= argc){
                    printf("REMOVE: You need to specify the number of blocks to be removed");
                    return 0;
                }
                char * pEnd;
                int n = (int) strtol (argv[i+1], &pEnd, 10);

                start = time_stamp();
                for (int j = 0; j < n; j++){
                    s_delete_block(test_array, rand()%array_size);
                }
                end = time_stamp();

                printf("REMOVE %i BLOCKS: \n real time: %f\n user time: %f\n system time: %f\n", n, 
                        (float)(end.real_time - start.real_time) / 1000000000.0, 
                        (float) (end.user_time - start.user_time) / (float) sysconf(_SC_CLK_TCK), 
                        (float)(end.system_time - start.system_time) / (float) sysconf(_SC_CLK_TCK));
                fprintf(file, "REMOVE %i BLOCKS: \n real time: %f\n user time: %f\n system time: %f\n", n, 
                        (float)(end.real_time - start.real_time) / 1000000000.0, 
                        (float) (end.user_time - start.user_time) / (float) sysconf(_SC_CLK_TCK), 
                        (float)(end.system_time - start.system_time) / (float) sysconf(_SC_CLK_TCK));
            }
            else if (strcmp(argv[i], "search") == 0){
                if (i+1 >= argc){
                    printf("SEARCH: You need to specify the value to be searched for");
                    return 0;
                }
                char * pEnd;
                int n = (int) strtol (argv[i+1], &pEnd, 10);

                start = time_stamp();
                s_ascii_search(test_array, n);
                end = time_stamp();

                printf("SEARCH: \n real time: %f\n user time: %f\n system time: %f\n", 
                        (float)(end.real_time - start.real_time) / 1000000000.0, 
                        (float) (end.user_time - start.user_time) / (float) sysconf(_SC_CLK_TCK), 
                        (float)(end.system_time - start.system_time) / (float) sysconf(_SC_CLK_TCK));
                fprintf(file, "SEARCH: \n real time: %f\n user time: %f\n system time: %f\n", 
                        (float)(end.real_time - start.real_time) / 1000000000.0, 
                        (float) (end.user_time - start.user_time) / (float) sysconf(_SC_CLK_TCK), 
                        (float)(end.system_time - start.system_time) / (float) sysconf(_SC_CLK_TCK));
            }
            else if (strcmp(argv[i], "remove_and_add") == 0){
                if (i+1 >= argc){
                    printf("REMOVE AND ADD: You need to specify the number of times you wish to add and remove this block");
                    return 0;
                }
                char * pEnd;
                int n = (int) strtol (argv[i+1], &pEnd, 10);
                int location = rand()%array_size;
                char* block = random_char_block(block_size);

                start = time_stamp();
                for (int j = 0; j < n; j++){
                    s_delete_block(test_array, location);
                    s_insert_block(test_array, block, location);
                }
                end = time_stamp();

                printf("REMOVE AND ADD %d TIMES: \n real time: %f\n user time: %f\n system time: %f\n", n, 
                        (float)(end.real_time - start.real_time) / 1000000000.0, 
                        (float) (end.user_time - start.user_time) / (float) sysconf(_SC_CLK_TCK), 
                        (float)(end.system_time - start.system_time) / (float) sysconf(_SC_CLK_TCK));
                fprintf(file, "REMOVE AND ADD %d TIMES: \n real time: %f\n user time: %f\n system time: %f\n", n, 
                        (float)(end.real_time - start.real_time) / 1000000000.0, 
                        (float) (end.user_time - start.user_time) / (float) sysconf(_SC_CLK_TCK), 
                        (float)(end.system_time - start.system_time) / (float) sysconf(_SC_CLK_TCK));
            }
        }
    }
    else {
        printf("%s", frame);
        printf("DYNAMIC ALLOCATION\n");
        printf("%s", frame);
        fprintf(file, "%s", frame);
        fprintf(file, "DYNAMIC ALLOCATION\n");
        fprintf(file, "%s", frame);

        full_time start = time_stamp();
        d_block_array* test_array = d_create_array(array_size, block_size);
        full_time end = time_stamp();

        printf("CREATE ARRAY: \n real time: %f\n user time: %f\n system time: %f\n", 
                (float)(end.real_time - start.real_time) / 1000000000.0, 
                (float) (end.user_time - start.user_time) / (float) sysconf(_SC_CLK_TCK), 
                (float)(end.system_time - start.system_time) / (float) sysconf(_SC_CLK_TCK));
        fprintf(file, "CREATE ARRAY: \n real time: %f\n user time: %f\n system time: %f\n", 
                (float)(end.real_time - start.real_time) / 1000000000.0, 
                (float) (end.user_time - start.user_time) / (float) sysconf(_SC_CLK_TCK), 
                (float)(end.system_time - start.system_time) / (float) sysconf(_SC_CLK_TCK));

        for (int i = 0; i < argc; i++) {
            if (strcmp(argv[i], "add") == 0) {
                if (i + 1 >= argc) {
                    printf("ADD: You need to specify the number of blocks to be added");
                    return 0;
                }
                char *pEnd;
                int n = (int) strtol(argv[i + 1], &pEnd, 10);
                char** random_blocks = malloc(n*sizeof(char*));
                for (int i = 0; i < array_size; i++){
                    random_blocks[i] = malloc(block_size*sizeof(char));
                    strncpy(random_blocks[i], random_char_block(block_size), block_size);
                }
                start = time_stamp();
                for (int j = 0; j < n; j++) {
                    d_insert_block(test_array, random_blocks[i], rand() % array_size);
                }
                end = time_stamp();

                printf("ADD %d BLOCKS: \n real time: %f\n user time: %f\n system time: %f\n", n,
                       (float) (end.real_time - start.real_time) / 1000000000.0,
                       (float) (end.user_time - start.user_time) / (float) sysconf(_SC_CLK_TCK),
                       (float) (end.system_time - start.system_time) / (float) sysconf(_SC_CLK_TCK));
                fprintf(file, "ADD %d BLOCKS: \n real time: %f\n user time: %f\n system time: %f\n", n,
                       (float) (end.real_time - start.real_time) / 1000000000.0,
                       (float) (end.user_time - start.user_time) / (float) sysconf(_SC_CLK_TCK),
                       (float) (end.system_time - start.system_time) / (float) sysconf(_SC_CLK_TCK));
            } 
            else if (strcmp(argv[i], "remove") == 0) {
                if (i + 1 >= argc) {
                    printf("REMOVE: You need to specify the number of blocks to be removed");
                    return 0;
                }
                char *pEnd;
                int n = (int) strtol(argv[i + 1], &pEnd, 10);

                start = time_stamp();
                for (int j = 0; j < n; j++) {
                    d_delete_block(test_array, rand() % array_size);
                }
                end = time_stamp();

                printf("REMOVE %i BLOCKS: \n real time: %f\n user time: %f\n system time: %f\n", n,
                       (float) (end.real_time - start.real_time) / 1000000000.0,
                       (float) (end.user_time - start.user_time) / (float) sysconf(_SC_CLK_TCK),
                       (float) (end.system_time - start.system_time) / (float) sysconf(_SC_CLK_TCK));
                fprintf(file, "REMOVE %i BLOCKS: \n real time: %f\n user time: %f\n system time: %f\n", n,
                       (float) (end.real_time - start.real_time) / 1000000000.0,
                       (float) (end.user_time - start.user_time) / (float) sysconf(_SC_CLK_TCK),
                       (float) (end.system_time - start.system_time) / (float) sysconf(_SC_CLK_TCK));
            } 
            else if (strcmp(argv[i], "search") == 0) {
                if (i + 1 >= argc) {
                    printf("SEARCH: You need to specify the value to be searched for");
                    return 0;
                }
                char *pEnd;
                int n = (int) strtol(argv[i + 1], &pEnd, 10);

                start = time_stamp();
                d_ascii_search(test_array, n);
                end = time_stamp();

                printf("SEARCH: \n real time: %f\n user time: %f\n system time: %f\n",
                       (float) (end.real_time - start.real_time) / 1000000000.0,
                       (float) (end.user_time - start.user_time) / (float) sysconf(_SC_CLK_TCK),
                       (float) (end.system_time - start.system_time) / (float) sysconf(_SC_CLK_TCK));
                fprintf(file, "SEARCH: \n real time: %f\n user time: %f\n system time: %f\n",
                       (float) (end.real_time - start.real_time) / 1000000000.0,
                       (float) (end.user_time - start.user_time) / (float) sysconf(_SC_CLK_TCK),
                       (float) (end.system_time - start.system_time) / (float) sysconf(_SC_CLK_TCK));
            } 
            else if (strcmp(argv[i], "remove_and_add") == 0) {
                if (i + 1 >= argc) {
                    printf("REMOVE AND ADD: You need to specify the number of times you wish to add and remove this block");
                    return 0;
                }
                char *pEnd;
                int n = (int) strtol(argv[i + 1], &pEnd, 10);
                int location = rand() % array_size;
                char *block = random_char_block(block_size);

                start = time_stamp();
                for (int j = 0; j < n; j++) {
                    d_delete_block(test_array, location);
                    d_insert_block(test_array, block, location);
                }
                end = time_stamp();

                printf("REMOVE AND ADD %d TIMES: \n real time: %f\n user time: %f\n system time: %f\n", n,
                       (float) (end.real_time - start.real_time) / 1000000000.0,
                       (float) (end.user_time - start.user_time) / (float) sysconf(_SC_CLK_TCK),
                       (float) (end.system_time - start.system_time) / (float) sysconf(_SC_CLK_TCK));
                fprintf(file, "REMOVE AND ADD %d TIMES: \n real time: %f\n user time: %f\n system time: %f\n", n,
                       (float) (end.real_time - start.real_time) / 1000000000.0,
                       (float) (end.user_time - start.user_time) / (float) sysconf(_SC_CLK_TCK),
                       (float) (end.system_time - start.system_time) / (float) sysconf(_SC_CLK_TCK));
            }
        }
    }
    fclose (file);
    return 0;
}