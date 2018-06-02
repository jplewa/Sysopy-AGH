#define _GNU_SOURCE 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>

int P;
int K;
int N;
FILE* text;
int L;
int s_mode;
char verbose;
int nk;
char** product_array;

int quit_flag;

pthread_mutex_t* mutex;
pthread_cond_t* wake_up;

int LAST_PROD;
int LAST_CONS;

pthread_t signal_check;
pthread_t* consumers;
pthread_t* producers;

char* prod_buffer;
char* cons_buffer;
size_t text_n;

void atexit1(){
    if(fclose(text)) printf("Error: couldn't close text file\n");
}
void atexit2(){
    free(product_array);
    free(producers);
    free(consumers);
    free(prod_buffer);
    free(cons_buffer);
}

void atexit3(){
    if(pthread_mutex_destroy(mutex)) printf("Error: couldn't destroy mutex\n");
    if(pthread_cond_destroy(wake_up)) printf("Error: couldn't destroy conditional variable\n");
}

void print_error(int error_code){
    switch(error_code){
        case -1:
            printf("Error: incorrect number of arguments\n");
            printf("Please provide: [configuration_file_path]\n");
            exit(0);
        case -2:
            perror("Error");
            printf("Please provide an existing configuration file\n");
            exit(0);
        case -3:
            perror("Error");
            printf("Couldn't read from provided configuration file\n");
            exit(0);
        case -4:
            printf("Error: incorrect configuration file\n");
            printf("Please provide a file containing: \
                \n\tP\n\tK\n\tN\n\ttext file path\n\tL\n\tsearch mode \n\toutput mode\n\tnk\n");
            exit(0);
        case -5:
            perror("Error");
            printf("Couldn't open provided text file\n");
            exit(0);
        case -6:
            printf("Error: incorrect configuration file\n");
            printf("Available search modes are: <= | == | >=\n");
            exit(0);
        case -7:
            printf("Error: incorrect configuration file\n");
            printf("Available output modes are: verbose | quiet\n");
            exit(0);
        case -8:
            perror("Error");
            printf("Couldn't register exit function\n");
            exit(0);
        case -9:
            perror("Error");
            printf("Couldn't initialize mutex\n");
            exit(0);
        case -10:
            perror("Error");
            printf("Couldn't initialize conditional variable\n");
            exit(0);
    }
    exit(0);
}

int parse_number(FILE* args, int* var){
    char* buffer = NULL;
    char* pEnd;
    size_t n = 0;
    if (getline(&buffer, &n, args) <= 0) return -3;
    if ((*var = strtol(buffer, &pEnd, 10)) == 0) return -4;
    else return 0;
}

int parse_arguments(int argc, char* argv[]){
    if (argc != 2) return -1;
    FILE* args = malloc(sizeof(FILE));
    if ((args = fopen(argv[1], "r")) == NULL) return -2;
    int result = 0;
    if ((result = parse_number(args, &P)) != 0){
        if(fclose(args)) printf("Error: couldn't close configuration file\n");
        return result;
    }
    if ((result = parse_number(args, &K)) != 0){
        if(fclose(args)) printf("Error: couldn't close configuration file\n");
        return result;   
    }
    if ((result = parse_number(args, &N)) != 0){
        if(fclose(args)) printf("Error: couldn't close configuration file\n");
        return result;
    }
    char* buffer = NULL;
    size_t n = 0;
    int chars;
    if ((chars = getline(&buffer, &n, args)) <= 0){
        if(fclose(args)) printf("Error: couldn't close configuration file\n");
        return -3;
    }
    buffer[chars-1] = '\0';
    if ((text = fopen(buffer, "r")) == NULL){
        if(fclose(args)) printf("Error: couldn't close configuration file\n");
        return -5;
    }
    if (atexit(&atexit1)) return -8;
    if ((result = parse_number(args, &L)) != 0){
        if(fclose(args)) printf("Error: couldn't close configuration file\n");
        return result;
    }
    if (getline(&buffer, &n, args) <= 0){
        if(fclose(args)) printf("Error: couldn't close configuration file\n");
        return -3;
    }
    if (strncmp(buffer, "<=", 2) == 0) s_mode = -1;    
    else if (strncmp(buffer, "==", 2) == 0) s_mode = 0;    
    else if (strncmp(buffer, ">=", 2) == 0) s_mode = 1;
    else{
        if(fclose(args)) printf("Error: couldn't close configuration file\n");
        return -6;
    }
    if (getline(&buffer, &n, args) <= 0){
        if(fclose(args)) printf("Error: couldn't close configuration file\n");
        return -3;        
    }
    if (strncmp(buffer, "verbose", 7) == 0) verbose = 1;    
    else if (strncmp(buffer, "quiet", 5) == 0) verbose = 0;
    else{
        if(fclose(args)) printf("Error: couldn't close configuration file\n");
        return -7; 
    }
    char* pEnd;
    if (getline(&buffer, &n, args) <= 0){
        if(fclose(args)) printf("Error: couldn't close configuration file\n");
        return -3;
    }
    nk = strtol(buffer, &pEnd, 10);
    if(fclose(args)) printf("Error: couldn't close configuration file\n");
    return 0;
}

int initialize(){
    mutex = malloc(sizeof(pthread_mutex_t));
    wake_up = malloc(sizeof(pthread_cond_t));

    if(pthread_mutex_init(mutex, NULL)) return -9;   
    if (pthread_cond_init(wake_up, NULL)) return -10;

    if(atexit(&atexit3)) return -8;
    
    product_array = malloc(N*sizeof(char*));
    producers = malloc(P*sizeof(pthread_t));
    consumers = malloc(K*sizeof(pthread_t));
    prod_buffer = malloc(1024);
    cons_buffer = malloc(1024);
    text_n = 1024;
    if (atexit(&atexit2)) return -8;

    sigset_t set;
    if (sigfillset(&set)) return -12;
    if (sigprocmask(SIG_BLOCK, &set, NULL)) return -12;
    LAST_CONS = N-1;
    LAST_PROD = N-1;
    for (int i = 0; i < N; i++) product_array[i] = NULL;
    return 0;
}

void set_quit_flag(){
    quit_flag = 1;
}

int quit(){
    return quit_flag;
}

int exit_strategy(){
    if (nk == 0){
        sigset_t set;
        siginfo_t info;
        const struct timespec timeout = {0, 0};

        sigemptyset(&set);
        sigaddset(&set, SIGINT);    

        while ((sigtimedwait(&set, &info, &timeout) != SIGINT) && (!quit())){}
    }
    else sleep(nk);
    pthread_mutex_lock(mutex);
    set_quit_flag();
    pthread_cond_broadcast(wake_up);
    pthread_mutex_unlock(mutex);
    return 0;
}

int utf8_strlen(char* buffer){
    int len = 0;
    while(*buffer){
        if ((*buffer & 0xC0) != 0x80) len++;
        buffer++;
    }
    return len;
}

void consumer_log(char* buffer, int index, int id){
    if (verbose){
        if (index == -1) printf("CONSUMER #%d\twaiting\n", id);        
        else {
            printf("CONSUMER #%d\treading\t\t%d\t", id, index);        
            if (((s_mode == 0) && (utf8_strlen(buffer) == L)) || ((s_mode != 0) && ((((utf8_strlen(buffer)) - L) * (int)s_mode) >= 0))){
                printf("🗸\t%s\n", buffer);
            }
            else printf("🞩\t%s\n", buffer);
        }
    }
    else if (index >= 0){
        if (((s_mode == 0) && (utf8_strlen(buffer) == L)) || ((s_mode != 0) && ((((utf8_strlen(buffer)) - L) * (int)s_mode) >= 0))){
            printf("CONSUMER #%d\t%d\t%s\n", id, index, buffer);
        }
    }
    fflush(stdout);
}

void producer_log(char* buffer, int index, int id){
    if (verbose){
        if (index >= 0) printf("PRODUCER #%d\twriting\t\t%d\t\t%s\n", id, index, buffer);        
        else printf("PRODUCER #%d\twaiting\n", id);  
    }
    fflush(stdout);
}

void* consumer(void* args){
    pthread_mutex_lock(mutex);
    while(!quit()){
        // to konsument dogonił producenta - tablica jest pusta
        if ((LAST_CONS == LAST_PROD) && (product_array[LAST_PROD] == NULL)) {
            consumer_log(NULL, -1, *(int*)(args));
            pthread_cond_wait(wake_up, mutex);
        } 
        else {
            LAST_CONS = (LAST_CONS+1)%N;
            int index = LAST_CONS;
            strncpy(cons_buffer, product_array[index], 1+strlen(product_array[index]));
            consumer_log(cons_buffer, index, *(int*)(args));
            free(product_array[index]);
            product_array[index] = NULL;
            // to producent dogonił konsumenta - tablica jest pełna
            if (((index-1+N)%N == LAST_PROD)){
                pthread_cond_broadcast(wake_up);
            }
        }
        pthread_mutex_unlock(mutex);
        pthread_mutex_lock(mutex);

    }
    pthread_mutex_unlock(mutex);
    pthread_exit("Exit");
}
void* producer(void* args){
    pthread_mutex_lock(mutex);
    while(!quit()){
        // to producent dogonił konsumenta - tablica jest pełna
        if ((LAST_CONS == LAST_PROD) && (product_array[LAST_PROD] != NULL)) {
            producer_log(NULL, -1, *(int*)(args));
            pthread_cond_wait(wake_up, mutex);
        }
        else {
            int chars;
            do {
                if ((chars = getline(&prod_buffer, &text_n, text)) < 0) break;
                if (prod_buffer[chars-1] == '\n') prod_buffer[chars-1] = '\0';
            } while (strlen(prod_buffer) == 0);
            if (chars < 0) {
                if (nk == 0) set_quit_flag();
            }
            else {
                LAST_PROD = (LAST_PROD+1)%N;
                int index = LAST_PROD;
                product_array[index] = malloc(chars+1);
                strncpy(product_array[index], prod_buffer, 1+chars);
                producer_log(prod_buffer, index, *(int*)(args));
                // to konsument dogonił producenta - tablica jest pusta
                if ((LAST_CONS == (LAST_PROD-1+N)%N)){
                    pthread_cond_broadcast(wake_up);
                }
            }
            pthread_mutex_unlock(mutex);
            pthread_mutex_lock(mutex);
        }
    }
    pthread_mutex_unlock(mutex);
    pthread_exit("Exit");
}

int create_threads(){
    if (verbose) printf("thread\t\taction\t\tindex\tmatch\tstring\n");
    else printf("thread\t\tindex\tmatched string\n");
    for (int i = 0; i < P; i++){
        int* index = malloc(sizeof(int));
        *index = i;
        if (pthread_create(&(producers[i]), NULL, producer, (void*)index)) return -11;
    }
    for (int i = 0; i < K; i++){
        int* index = malloc(sizeof(int));
        *index = i;
        if (pthread_create(&(consumers[i]), NULL, consumer, (void*)index)) return -11;
    }
    int result;
    result = exit_strategy();
    void* ptr;
    for (int i = 0; i < P; i++){
        if (pthread_join(producers[i], &ptr)) result = -13;
    }
    for (int i = 0; i < K; i++){
        if (pthread_join(consumers[i], &ptr)) result = -13;
    }
    return result;
}

int main(int argc, char* argv[]){
    int result;
    if ((result = parse_arguments(argc, argv)) != 0) print_error(result);
    if ((result = initialize()) != 0) print_error(result);
    if ((result = create_threads()) != 0) print_error(result);
    return 0;
}