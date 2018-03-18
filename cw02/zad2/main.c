#define _XOPEN_SOURCE 500

#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

char* permissions(struct stat* path_stat){
    char* result = malloc(11);
    result = strncpy(result, "----------\0", 11);
    if (S_IRUSR & path_stat -> st_mode) result[1] = 'r';  
    if (S_IWUSR & path_stat -> st_mode) result[2] = 'w';  
    if (S_IXUSR & path_stat -> st_mode) result[3] = 'x';  
    if (S_IRGRP & path_stat -> st_mode) result[4] = 'r';  
    if (S_IWGRP & path_stat -> st_mode) result[5] = 'w';  
    if (S_IXGRP & path_stat -> st_mode) result[6] = 'x'; 
    if (S_IROTH & path_stat -> st_mode) result[7] = 'r';  
    if (S_IWOTH & path_stat -> st_mode) result[8] = 'w';  
    if (S_IXOTH & path_stat -> st_mode) result[9] = 'x'; 
    return result;
}

bool time_equal(time_t requested, time_t file, int cmp){
    if (cmp == 0 && (int)difftime(file, requested)/86400 == 0) return true;
    if (cmp != 0 && ((difftime(file, requested)*cmp) > 0) && (time_equal(requested, file, 0) == false)) return true;
    else return false; 
}

void search_dirs(char* current_path, DIR* directory, int cmp, time_t date){
    struct dirent* current = readdir(directory);
    if (current == NULL) return;
    struct stat* path_stat = malloc(sizeof(struct stat));
    char* new_str = malloc(strlen(current_path) + strlen(current -> d_name) + 2);
    new_str[0] = '\0';
    strcat(new_str, current_path);
    strcat(new_str,current -> d_name);
    if (lstat(new_str, path_stat) == 0){
        if (S_ISREG(path_stat -> st_mode)){
            if (time_equal(date, path_stat -> st_mtime, cmp)){
                printf("%s%s\n%10ld\t%s", current_path, current -> d_name, (long) path_stat -> st_size, permissions(path_stat));
                char buff[20]; 
                struct tm * timeinfo;
                timeinfo = localtime (&(path_stat -> st_mtime)); 
                strftime(buff, 20, "%Y-%m-%d %H:%M:%S", timeinfo); 
                printf("\t%s\n",buff);
            }
        }
        else if (S_ISDIR(path_stat -> st_mode)){
            if (strncmp(current -> d_name, ".",  1) != 0 && strncmp(current -> d_name, "..",  2) != 0){
                strcat(new_str, "/");
                DIR* next_directory = opendir(new_str);
                if (next_directory != NULL){
                    search_dirs(new_str, next_directory, cmp, date);
                }
            }
        }
    }
    else printf("Oh dear something went wrong with read()! %s\n", strerror(errno));
    search_dirs(current_path, directory, cmp, date);
}

void parse(int argc, char* argv[]){
    if (argc < 4) return;
    DIR* directory = opendir(argv[1]);
    if (directory == NULL) return;
    int cmp = ((int) argv[2][0]) - 61;
    if (abs(cmp) > 1){
        printf("ups\n");
        return;
    }
    struct tm* tm = malloc(sizeof(struct tm));
    if (strptime(argv[3], "%Y-%m-%d", tm) == NULL) return;
    time_t date = mktime(tm);
    search_dirs(argv[1], directory, cmp, date);
}

int main(int argc, char* argv[]){
    parse(argc, argv);
}