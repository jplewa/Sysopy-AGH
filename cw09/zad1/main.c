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

pthread_mutex_t* last_mutex;
pthread_mutex_t* product_array_mutex;
pthread_mutex_t* quit_mutex;
pthread_mutex_t* file_mutex;

pthread_cond_t* not_empty;
pthread_cond_t* not_full;

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
    if(pthread_mutex_destroy(last_mutex)) printf("Error: couldn't destroy mutex\n");
    if(pthread_mutex_destroy(product_array_mutex)) printf("Error: couldn't destroy mutex\n");
    if(pthread_mutex_destroy(quit_mutex)) printf("Error: couldn't destroy mutex\n");
    if(pthread_mutex_destroy(file_mutex)) printf("Error: couldn't destroy mutex\n");
    if(pthread_cond_destroy(not_empty)) printf("Error: couldn't destroy conditional variable\n");
    if(pthread_cond_destroy(not_full)) printf("Error: couldn't destroy conditional variable\n");
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
    last_mutex = malloc(sizeof(pthread_mutex_t));
    product_array_mutex = malloc(sizeof(pthread_mutex_t));
    quit_mutex = malloc(sizeof(pthread_mutex_t));
    file_mutex = malloc(sizeof(pthread_mutex_t));

    not_empty = malloc(sizeof(pthread_cond_t));
    not_full = malloc(sizeof(pthread_cond_t));

    if(pthread_mutex_init(last_mutex, NULL)) return -9;
    if(pthread_mutex_init(product_array_mutex, NULL)) return -9;
    if(pthread_mutex_init(quit_mutex, NULL)) return -9;
    if(pthread_mutex_init(file_mutex, NULL)) return -9;
    
    if (pthread_cond_init(not_empty, NULL)) return -10;
    if (pthread_cond_init(not_full, NULL)) return -10;

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
    pthread_mutex_lock(quit_mutex);
    quit_flag = 1;
    pthread_mutex_unlock(quit_mutex);
    pthread_cond_broadcast(not_full);
    pthread_cond_broadcast(not_empty); 
}

int quit(){
    int tmp;
    pthread_mutex_lock(quit_mutex);
    tmp = quit_flag;
    pthread_mutex_unlock(quit_mutex);
    return tmp;
}

int exit_strategy(){
    if (nk == 0){
        sigset_t set;
        siginfo_t info;
        const struct timespec timeout = {0, 0};

        sigemptyset(&set);
        sigaddset(&set, SIGINT);    

        while ((sigtimedwait(&set, &info, &timeout) != SIGINT) && !quit()){}
        set_quit_flag();
    }
    else {
        sleep(nk);
        set_quit_flag();      
    }
    return 0;
}

void consumer_log(char* buffer, int index, int id){
    if (verbose){
        printf("%d\tCONSUMER #%d\t", index, id);        
        if (((s_mode == 0) && (strlen(buffer) == L)) || ((s_mode != 0) && (((((int)strlen(buffer)) - L) * (int)s_mode) >= 0))){
            printf("+\t\t%s\n", buffer);
        }
        else{
            printf("-\n");
        }
    }
    else{
        if (((s_mode == 0) && (strlen(buffer) == L)) || ((s_mode != 0) && (((((int)strlen(buffer)) - L) * (int)s_mode) >= 0))){
            //printf("(%d - %d) * %d\n", strlen(buffer), L, s_mode);
            printf("%d\tCONSUMER #%d\t%s\n", index, id, buffer);
        }
    }
    fflush(stdout);
}

void producer_log(char* buffer, int index, int id){
    if (verbose){
        printf("%d\tPRODUCER #%d\t\t\t%s\n", index, id, buffer);        
    }
    fflush(stdout);
}

void* consumer(void* args){
    pthread_mutex_lock(last_mutex);
    //printf("got mutex\n");
    while(!quit()){
        //printf("inside loop\n");
        // to konsument dogonił producenta - tablica jest pusta
        pthread_mutex_lock(product_array_mutex);
        if ((LAST_CONS == LAST_PROD) && (product_array[LAST_PROD] == NULL)) {
            pthread_mutex_unlock(product_array_mutex);
            //`printf("consumer waiting\n");
            pthread_cond_wait(not_empty, last_mutex);
        } 
        else {
            //printf("consumer reading\n");
            LAST_CONS = (LAST_CONS+1)%N;
            int index = LAST_CONS;
            pthread_mutex_unlock(last_mutex);
            //printf("before\n");
            //printf("%d\n", strlen(product_array[index]));
            //printf("after\n");
            strncpy(cons_buffer, product_array[index], 1+strlen(product_array[index]));
            consumer_log(cons_buffer, index, *(int*)(args));
            free(product_array[index]);
            product_array[index] = NULL;
            if (((index-1+N)%N == LAST_PROD)){ //&& (product_array[(index-1+N)%N] != NULL) ) {
                pthread_cond_broadcast(not_full);
            }
            pthread_mutex_unlock(product_array_mutex);
            pthread_mutex_lock(last_mutex);
        }
    }
    pthread_mutex_unlock(last_mutex);
    //printf("consumer exiting\n");
    pthread_exit("Exit");
}
void* producer(void* args){
    pthread_mutex_lock(last_mutex);
    while(!quit()){
        // to producent dogonił konsumenta - tablica jest pełna
        pthread_mutex_lock(product_array_mutex);
        if ((LAST_CONS == LAST_PROD) && (product_array[LAST_PROD] != NULL)) {
            pthread_mutex_unlock(product_array_mutex);
            //printf("producer waiting\n");
            pthread_cond_wait(not_full, last_mutex);
        }
        else {
            pthread_mutex_lock(file_mutex);
            int chars;
            if ((chars = getline(&prod_buffer, &text_n, text)) < 0){
                if (nk == 0) set_quit_flag();
                pthread_mutex_unlock(last_mutex);
            }
            else {
                LAST_PROD = (LAST_PROD+1)%N;
                int index = LAST_PROD;
                pthread_mutex_unlock(last_mutex);
                if (prod_buffer[chars-1] == '\n') prod_buffer[chars-1] = '\0';
                product_array[index] = malloc(chars+1);
                strncpy(product_array[index], prod_buffer, 1+chars);
                producer_log(prod_buffer, index, *(int*)(args));
                // to konsument dogonił producenta - tablica jest pusta
                if ((LAST_CONS == (LAST_PROD-1+N)%N)){ //&& (product_array[(LAST_PROD-1+N)%N] == NULL) ) {
                    //printf("Broadcasting\n");
                    pthread_cond_broadcast(not_empty);
                }
            }
            pthread_mutex_unlock(product_array_mutex);
            pthread_mutex_unlock(file_mutex);
            pthread_mutex_lock(last_mutex);
        }
    }
    pthread_mutex_unlock(last_mutex);
    //printf("producer exiting\n");
    pthread_exit("Exit");
}

int create_threads(){
    if (verbose) printf("index\twho\t\tmatch\t\tstring\n");
    else printf("index\twho\t\tmatched string\n");
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
//    printf("Joined all\n");
    return result;
}

int main(int argc, char* argv[]){
    printf("%d\n", (0%1));
    int result;
    if ((result = parse_arguments(argc, argv)) != 0) print_error(result);
    if ((result = initialize()) != 0) print_error(result);
    if ((result = create_threads()) != 0) print_error(result);
 //   printf("P: %d\nK: %d\nN: %d\nL: %d\nnk: %d\n%d\n%d\n", P,K,N,L,nk,(int)verbose,(int)s_mode);

    return 0;
}