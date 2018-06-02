#define _GNU_SOURCE 1

#include "../producer_consumer.h"

pthread_mutex_t* mutex;
pthread_cond_t* wake_up;

void atexit3(){
    if(pthread_mutex_destroy(mutex)) printf("Error: couldn't destroy mutex\n");
    if(pthread_cond_destroy(wake_up)) printf("Error: couldn't destroy conditional variable\n");
    free(mutex);
    free(wake_up);
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

int main(int argc, char* argv[]){
    int result = 0;
    if ((result = parse_arguments(argc, argv)) != 0) print_error(result);
    if ((result = initialize()) != 0) print_error(result);
    if ((result = create_threads()) != 0) print_error(result);
    return 0;
}