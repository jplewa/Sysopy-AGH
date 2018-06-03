#include "producer_consumer.h"

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

int initialize(){
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

void set_quit_flag(){
    quit_flag = 1;
}

int quit(){
    return quit_flag;
}

int create_threads(){
    if (verbose) printf("thread\t\taction\t\tindex\tmatch\tstring\n");
    else printf("thread\t\tindex\tmatched string\n");
    int** index = malloc((P+K)*sizeof(int*));
    for (int i = 0; i < P; i++){
        index[i] = malloc(sizeof(int));
        *(index[i]) = i;
        if (pthread_create(&(producers[i]), NULL, producer, (void*)index[i])) return -11;
    }
    for (int i = 0; i < K; i++){
        index[P+i] = malloc(sizeof(int));
        *(index[P+i]) = i;
        if (pthread_create(&(consumers[i]), NULL, consumer, (void*)index[P+i])) return -11;
    }
    int result;
    result = exit_strategy();
    void* ptr = NULL;
    for (int i = 0; i < P; i++){
        if (pthread_join(producers[i], &ptr)) result = -13;
    }
    for (int i = 0; i < K; i++){
        if (pthread_join(consumers[i], &ptr)) result = -13;
    }
    for (int i = 0; i < N; i++){
        if (product_array[i] != NULL) free(product_array[i]);
    }
    for(int i = 0; i < P+K; i++) free(index[i]);
    free(index);
    return result;
}

int parse_number(FILE* args, int* var){
    char* buffer = NULL;
    char* pEnd;
    size_t n = 0;
    if (getline(&buffer, &n, args) <= 0){
        free(buffer);
        return -3;
    }
    if ((*var = strtol(buffer, &pEnd, 10)) == 0){
        free(buffer);
        return -4;
    }
    free(buffer);
    return 0;
}

int parse_configuration(int argc, char* argv[]){
    if (argc != 2) return -1;
    FILE* args;
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
    int chars = 0;
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
    free(buffer);
    return 0;
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
        case -11:
            perror("Error");
            printf("Couldn't initialize semaphore\n");
            exit(0);
    }
    exit(0);
}