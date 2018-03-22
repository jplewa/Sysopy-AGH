#define _XOPEN_SOURCE 500

#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ftw.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

typedef struct DIR_node DIR_node;
typedef struct DIR_queue DIR_queue;

struct DIR_node{
  char* full_path;
  DIR_node* next;
};
struct DIR_queue{
  DIR_node* head;
  DIR_node* tail;
};
DIR_queue* new_queue(){
  DIR_queue* queue = malloc(sizeof(DIR_queue));
  DIR_node* dummy = malloc(sizeof(DIR_node));
  dummy -> next = NULL;
  dummy -> full_path = NULL;
  queue -> head = dummy;
  queue -> tail = dummy;
  return queue;
}
void DIR_enqueue(DIR_queue* queue, char* full_path){
    DIR_node* new_node = malloc(sizeof(DIR_node));
    new_node -> full_path = full_path;
    new_node -> next = NULL;
    (queue -> tail) -> next = new_node;
    queue -> tail = new_node;
}
DIR_node* DIR_dequeue(DIR_queue* queue){
    DIR_node* tmp = queue -> head -> next;
    if (tmp != NULL){
        queue -> head -> next = tmp -> next;
        if (tmp -> next == NULL) queue -> tail = queue -> head;
        else tmp -> next = NULL;
    }
    return tmp;
}
bool DIR_queue_is_empty(DIR_queue* queue){
    return (queue -> head -> next == NULL);
}
char* permissions(const struct stat* path_stat){
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
    struct tm* file_d = malloc(sizeof(struct tm));
    memcpy (file_d, localtime(&file), sizeof(struct tm));
    struct tm* file_r = malloc(sizeof(struct tm));
    memcpy (file_r, localtime(&requested), sizeof(struct tm));
    if (cmp == 0 && (file_d -> tm_year) == (file_r -> tm_year) && (file_d -> tm_mon) == (file_r -> tm_mon) && (file_d -> tm_mday) == (file_r -> tm_mday)) return true;
    if (cmp != 0 && ((difftime(file, requested)*cmp) > 0) && !(time_equal(requested, file, 0))) return true;
    else return false; 
}
void search_dirs(char* current_path, DIR* directory, int cmp, time_t date){
    DIR_queue* queue = new_queue();
    DIR_enqueue(queue, current_path);
    DIR_node* node = malloc(sizeof(DIR_node));
    struct dirent* current = malloc(sizeof(struct dirent));
    struct stat* path_stat = malloc(sizeof(struct stat));
    char* new_name;
    DIR* new_directory;

    while(!DIR_queue_is_empty(queue)){
        node = DIR_dequeue(queue);
        new_directory = opendir(node -> full_path);
        if (new_directory != NULL){
            while((current = readdir(new_directory)) != NULL){
                new_name = malloc(strlen(node -> full_path) + strlen(current -> d_name) + 2);
                new_name[0] = '\0';
                strcat(new_name, node -> full_path);
                strcat(new_name, current -> d_name);
                if (lstat(new_name, path_stat) == 0){
                    if (S_ISREG(path_stat -> st_mode)){
                        if (time_equal(date, path_stat -> st_mtime, cmp)){
                            char buff[20]; 
                            struct tm * timeinfo;
                            timeinfo = localtime (&(path_stat -> st_mtime)); 
                            strftime(buff, 20, "%Y-%m-%d %H:%M:%S", timeinfo); 
                            printf("%s\t%8ld\t%s\t\t%s%s\n", permissions(path_stat), (long) path_stat -> st_size, buff, node -> full_path, current -> d_name);
                        }
                    }
                    else if (S_ISDIR(path_stat -> st_mode)){
                        if (strncmp(current -> d_name, ".",  strlen(current -> d_name)) != 0 && strncmp(current -> d_name, "..",  2) != 0){
                            if (strncmp(current -> d_name, "/",  strlen(current -> d_name)) != 0) strcat(new_name, "/");
                            DIR_enqueue(queue, new_name);
                        }
                    }
                }
            }
            if (closedir(new_directory) != 0){
                printf("Error: a problem occured while closing directory %s.\n", node -> full_path);
            }
        }
    }
    free(new_name);
    free(directory);
    free(queue -> head);
    free(queue);
    free(node);
    free(current);
    free(path_stat);
}
void search_dirs_nftw (char* directory, int cmp, time_t date){
    int flags = 0;
    flags |= FTW_PHYS;
    int fn (const char* file_path, const struct stat* path_stat, int flag, struct FTW* ftw) {
        if (S_ISREG(path_stat -> st_mode) && time_equal(date, path_stat -> st_mtime, cmp)){
            char buff[20]; 
            struct tm * timeinfo;
            timeinfo = localtime (&(path_stat -> st_mtime)); 
            strftime(buff, 20, "%Y-%m-%d %H:%M:%S", timeinfo);
            printf("%s\t%8ld\t%s\t\t%s\n",  permissions(path_stat), (long) path_stat -> st_size, buff, file_path); 
        }
        return 0;
    };
    if (nftw(directory, fn, 20, flags) != 0){
        printf("Error: a problem occured while accessing %s.\n", directory);
        return;
    }
}
void parse(int argc, char* argv[]){
    if (argc < 4){
        printf("Incorrect arguments!\n");
        printf("Please provide the following: [folder] [comparison type] [yyyy-mm-dd] [mode].\n");
        printf("Comparison options: '<' '=' '>'.\n");
        printf("Additional mode option availavle: nfts.\n");
        return;
    }
    DIR* directory = opendir(argv[1]);
    if (directory == NULL){
        printf("Error: a problem occured while trying to access directory %s.\n", argv[1]);
        return;
    }
    if (closedir(directory) != 0){
        printf("Error: a problem occured while trying to access directory %s.\n", argv[1]);
        return;
    }
    char* full_path = calloc(1024, 1);
    full_path[0] = '\0';
    chdir(argv[1]);
    char* cwd = getcwd(NULL, 0);
    strcat(full_path, cwd);
    if (strncmp(full_path, "/",  strlen(full_path)) != 0) strcat(full_path, "/");
    int cmp = ((int) argv[2][0]) - 61;
    if (abs(cmp) > 1){
        printf("Incorrect arguments!\n");
        printf("Available comparison types: '<' '=' '>'.\n");
        return;
    }
    struct tm* tm = malloc(sizeof(struct tm));
    if (strptime(argv[3], "%Y-%m-%d", tm) == NULL){
        printf("Incorrect arguments!\n");
        printf("Required date format: yyyy-mm-dd.\n");
        return;
    }
    time_t date = mktime(tm);
    free(tm);
    if(argc >= 5 && strncmp(argv[4], "nftw", 4) == 0) search_dirs_nftw(full_path, cmp, date);
    else search_dirs(full_path, directory, cmp, date);  
}
int main(int argc, char* argv[]){
    parse(argc, argv);
}