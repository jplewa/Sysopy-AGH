#define _GNU_SOURCE 1

#include "../producer_consumer.h"

#include <semaphore.h>

sem_t* mutex;

sem_t** not_empty;      // semaphores for producers and consumers have been added to implement
sem_t** not_full;       // a solution equivalent to the mutex-based solution
int* c_waiting;         // these arrays allowed me to implement a broadcast function
int* p_waiting;

void atexit3(){
    if (sem_destroy(mutex)) printf("Error: couldn't destroy semaphore\n");
    for (int i = 0; i < P; i++){
        if (sem_destroy(not_full[i])) printf("Error: couldn't destroy semaphore\n");
        free(not_full[i]);        
    }
    for (int i = 0; i < K; i++){
        if (sem_destroy(not_empty[i])) printf("Error: couldn't destroy semaphore\n");
        free(not_empty[i]);
    }
    free(mutex);
    free(p_waiting);
    free(c_waiting);
    free(not_empty);
    free(not_full);
}

int sem_initialize(){
    mutex = malloc(sizeof(sem_t));

    not_empty = malloc(K*sizeof(sem_t*));
    not_full = malloc(P*sizeof(sem_t*));
    
    p_waiting = malloc(P*sizeof(int));
    c_waiting = malloc(K*sizeof(int));

    for (int i = 0; i < P; i++){
        not_full[i] = malloc(sizeof(sem_t));
        if (sem_init(not_full[i], 0, 0)) return -11;
        p_waiting[i] = 0;
    }
    for (int i = 0; i < K; i++){
        not_empty[i] = malloc(sizeof(sem_t));
        if (sem_init(not_empty[i], 0, 0)) return -11;
        c_waiting[i] = 0;
    }
    if (sem_init(mutex, 0, 1)) return -11;
    return 0;
}

void broadcast_not_full(){
    for (int i = 0; i < P; i++){
        if (p_waiting[i]){
            sem_post(not_full[i]);
            p_waiting[i] = 0;
        }
    }
}

void broadcast_not_empty(){
    for (int i = 0; i < K; i++){
        if (c_waiting[i]){
            sem_post(not_empty[i]);
            c_waiting[i] = 0;
        }
    }
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
    sem_wait(mutex);
    set_quit_flag();
    broadcast_not_empty();
    broadcast_not_full();
    sem_post(mutex);
    return 0;
}

void* consumer(void* args){
    sem_wait(mutex);
    while(!quit()){
        // to konsument dogonił producenta - tablica jest pusta
        if ((LAST_CONS == LAST_PROD) && (product_array[LAST_PROD] == NULL)) {
            consumer_log(NULL, -1, *(int*)(args));
            c_waiting[*(int*)(args)] = 1;
            sem_post(mutex);
            sem_wait(not_empty[*(int*)(args)]);
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
                consumer_log(NULL, -2, *(int*)(args));
                broadcast_not_full();
            }
            sem_post(mutex);
        }
        sem_wait(mutex);
    }
    sem_post(mutex);
    pthread_exit("Exit");
}

void* producer(void* args){
    sem_wait(mutex);
    while(!quit()){
        // to producent dogonił konsumenta - tablica jest pełna
        if ((LAST_CONS == LAST_PROD) && (product_array[LAST_PROD] != NULL)) {
            producer_log(NULL, -1, *(int*)(args));
            p_waiting[*(int*)(args)] = 1;
            sem_post(mutex);
            sem_wait(not_full[*(int*)(args)]);
        }
        else {
            int chars;
            do {
                if ((chars = getline(&prod_buffer, &buffer_size, text)) < 0) break;
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
                // to konsument dogonił producenta - tablica była pusta
                if ((LAST_CONS == (LAST_PROD-1+N)%N)){
                    producer_log(NULL, -2, *(int*)(args));
                    broadcast_not_empty();
                }
            }
            sem_post(mutex);
        }
        sem_wait(mutex);
    }
    sem_post(mutex);
    pthread_exit("Exit");
}

int main(int argc, char* argv[]){
    int result = 0;
    if ((result = parse_configuration(argc, argv)) != 0) print_error(result);
    if ((result = sem_initialize()) != 0) print_error(result);
    if ((result = initialize()) != 0) print_error(result);
    if ((result = create_threads()) != 0) print_error(result);
    return 0;
}