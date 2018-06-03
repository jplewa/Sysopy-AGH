#ifndef PRODUCER_CONSUMER_H
#define PRODUCER_CONSUMER_H 

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

size_t buffer_size;
char* prod_buffer;
char* cons_buffer;

pthread_t* consumers;
pthread_t* producers;

int QUIT_FLAG;
int LAST_PROD;
int LAST_CONS;

int exit_strategy();
void* consumer(void* args);
void* producer(void* args);
void atexit1();
void atexit2();
void atexit3();
int initialize();
int utf8_strlen(char* buffer);
void consumer_log(char* buffer, int index, int id);
void producer_log(char* buffer, int index, int id);
void set_quit_flag();
int quit();
int create_threads();
int parse_number(FILE* args, int* var);
int parse_configuration(int argc, char* argv[]);
void print_error(int error_code);

#endif

