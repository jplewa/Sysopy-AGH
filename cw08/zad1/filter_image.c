#include <stdlib.h>    
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

#ifndef max
    #define max(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef min
    #define min(a,b) ((a) < (b) ? (a) : (b))
#endif

int THREADS;
FILE* IMAGE_IN;
FILE* FILTER;
FILE* IMAGE_OUT;
int H;
int W;
unsigned char **image_in;
unsigned char **image_out;
int C;
float** filter;

void atexit1(){
    if (fclose(IMAGE_IN)) printf("atexit error: couldn't close input file\n");
}

void atexit2(){
    if (fclose(FILTER)) printf("atexit error: couldn't close input file\n");
}

void atexit3(){
    if (fclose(IMAGE_OUT)) printf("atexit error: couldn't close input file\n");
}

void atexit4(){
    for (int i = 1; i <= H; i++) free(image_in[i]);
    free(image_in);
    for (int i = 1; i <= H; i++) free(image_out[i]);
    free(image_out);
    
}

void atexit5(){
    for (int i = 0; i <= C; i++) free(filter[i]);
    free(filter);
}

void print_error(int error_code){
    switch(error_code){
        case -1:
            printf("Error: incorrect program arguments\n");
            printf("Please provide: [#_of_threads] [input_file] [filter_file] [output_file]\n");
            exit(0);
        case -2:
            perror("Error");
            printf("Couldn't set exit handlers\n");
        case -3:
            perror("Error");
            printf("Couldn't read input file\n");
            exit(0);
        case -4:
            printf("Error: incorrect ASCII PGM input file\n");
            printf("Please use the following format:\n");
            printf("    P2\n    W H\n    M\n    ...\nfollowed by W*H values in the range [0, 255] separated by whitespace characters\n");
            printf("Expected M value is 255\n");
            exit(0);
        case -5:
            printf("Error: incorrect filter file\n");
            printf("Please use the following format:\n    C\n    ...\nfollowed by C*C floating point values whose overall sum is equal to 1\n");
            exit(0);
        case -6:
            perror("Error");            
            printf("Failed to create a new thread\n");
            exit(0);
        case -7:
            perror("Error");
            printf("Failed to join with a terminated thread\n");
            exit(0);
        case -8:
            perror("Error");
            printf("Failed to save output\n\n");
            exit(0);
    }
}

int parse_args(int argc, char* argv[]){
    if (argc < 5) return -1;
    char* pEnd;
    if ((THREADS =  strtol (argv[1], &pEnd, 10)) <= 0) return -1;
    if ((IMAGE_IN = fopen(argv[2], "r")) == NULL) return -1;
    if (atexit(&atexit1)) return -2;
    if ((FILTER = fopen(argv[3], "r")) == NULL) return -1;
    if (atexit(&atexit2)) return -2;
    if ((IMAGE_OUT = fopen(argv[4], "w")) == NULL) return -1;
    if (atexit(&atexit3)) return -2;
    return 0;
}

int skip_comments(char** lineptr, size_t* n){
    if ((getline(lineptr, n, IMAGE_IN)) <= 0) return -3; 
    while(strncmp((*lineptr), "#", 1) == 0){
        if ((getline(lineptr, n, IMAGE_IN)) <= 0) return -3; 
    }
    return 0;
}

int parse_image_header(){
    char* lineptr = NULL;
    size_t n = 0;
    if (skip_comments(&lineptr, &n)) return -3;
    if ((strncmp("P2", lineptr, 2)) != 0) return -4;
    if (skip_comments(&lineptr, &n)) return -3;
    char* pEnd;
    if ((W =  strtol (lineptr, &pEnd, 10)) <= 0) return -4;
    if ((H =  strtol (pEnd, &pEnd, 10)) <= 0) return -4;
    if (skip_comments(&lineptr, &n)) return -3;
    if ((strtol (lineptr, &pEnd, 10)) != 255) return -4;
    if (THREADS > W){
        printf("Requested number of threads is larger than image width.\n%d threads will be used instead.\n", W);
        THREADS = W;
    }
    return 0;
}

int parse_image(){
    char* pEnd;
    char* lineptr = NULL;
    size_t n;
    image_in = malloc((H+1) * sizeof(unsigned char *));
    for (int i = 1; i <= H; i++) image_in[i] = malloc((W+1) * sizeof(unsigned char));
    image_out = malloc((H+1) * sizeof(unsigned char *));
    for (int i = 1; i <= H; i++) image_out[i] = malloc((W+1) * sizeof(unsigned char));
    if (atexit(&atexit4)) return -2;
    char* c;
    char* buffer = malloc(4*sizeof(char));
    int i = 0;
    long int tmp = 0;
    while (((getline(&lineptr, &n, IMAGE_IN)) > 0) && (errno == 0)){
        if (strncmp(lineptr, "#", 1) != 0){
            for (buffer = strtok_r(lineptr, " \r\t\n\v", &c); buffer != NULL; buffer = strtok_r(NULL, " \r\t\n\v", &c)){
                if (i >= H*W) return -4;
                if (((tmp = strtol(buffer, &pEnd, 10)) > 255) || (tmp < 0)) return -4;
                image_in[(i/W)+1][(i%W)+1] = tmp;
                i++;             
            }
        }
    }
    free(buffer);
    if (i != H*W) return -4;
    return 0;
}

int parse_filter(){
    char* lineptr = NULL;
    size_t n = 0;
    if ((getline(&lineptr, &n, FILTER) <= 0)) return -3;
    char* pEnd;
    if ((C =  strtol (lineptr, &pEnd, 10)) <= 0) return -4;
    filter = malloc((C+1) * sizeof(float*));
    for (int i = 1; i <= C; i++) filter[i] = malloc((C+1) * (sizeof(float)));
    if (atexit(&atexit5)) return -2;
    char* c;
    char* buffer = malloc(33*sizeof(char));
    int i = 0;
    while (((getline(&lineptr, &n, FILTER)) > 0) && (errno == 0)){
        if (strncmp(lineptr, "#", 1) != 0){
            for (buffer = strtok_r(lineptr, " \t\n", &c); buffer != NULL; buffer = strtok_r(NULL, " \t\n", &c)){
                if (i >= C*C) return -5;
                filter[(i/C)+1][(i%C)+1] = strtof(buffer, &pEnd);
                i++;
            }
        }
    }
    free(buffer);
    if (i != C*C) return -5;
    float sum = 0;
    i--;
    while (i >= 0){
        sum += filter[(i/C)+1][(i%C)+1];
        i--;
    }    
    if (abs(sum - 1.0) > __FLT_EPSILON__) return -5;
    return 0;
}

int get_filtered_value(int x, int y){
    double s = 0; 
    for (int i = 1; i <= C; i++){
        for (int j = 1; j <= C; j++){
            s += image_in[(int) (min(H, max(1, x - ceil(C/2) + i)))][(int) (min(W, max(1, y - ceil(C/2) + j)))] * filter[i][j];
        }
    }
    return round(s);
}

void* filter_function(void* args){
    int index = *((int*) args);
    int start;
    if (index == 1) start = 0;
    else start = ((index-1) <= (W%THREADS)) ? ((index-1)*ceil((double)W/THREADS)) : ((W%THREADS)*(ceil((double)W/THREADS)))+((index-1-W%THREADS)*(W/THREADS));
    start += 1;
    int end = (index <= (W%THREADS)) ? (index*ceil((double)W/THREADS)) : ((W%THREADS)*(ceil((double)W/THREADS))+(index-W%THREADS)*(W/THREADS));
    for (int i = 1; i <= H; i++){
        for (int j = start; j <= end; j++){
            image_out[i][j] = get_filtered_value(i, j);
        }
    }
    pthread_exit("Exit");
}

int filter_image(){
    int result = 0;
    pthread_t* threads = malloc((THREADS+1) * sizeof(pthread_t));
    for (int i = 1; i <= THREADS; i++){
        int* index = malloc(sizeof(int));
        index[0] = i;
        if (pthread_create(&(threads[i]), NULL, filter_function, (void*)(index))) result = -6;
    }
    void* ptr;
    for (int i = 1; i <= THREADS; i++){
        if (pthread_join(threads[i], &ptr)) result = -7;

    }
    free(threads);
    return result;  
}

int save_out_image(){
    char* header = calloc(256, 1);
    if (sprintf(header, "P2\n%d %d\n255\n", W, H) <= 0) return -8;
    if (fwrite(header, 1, strlen(header), IMAGE_OUT) != strlen(header)) return -8;
    free(header);
    char* buffer = malloc(5);
    for (int i = 1; i <= H; i++){
        for (int j = 1; j <= W; j++){
            if (sprintf(buffer, "%d ", image_out[i][j]) <= 0) return -8;
            if (fwrite(buffer, 1, strlen(buffer), IMAGE_OUT) != strlen(buffer)) return -8;
        }
    }
    free(buffer);
    return 0;
}

void print_log(struct timespec before, struct timespec after){
    printf("%dx%d\t\t%d\t\t%d\t\t%.9f s\n", W, H, C, THREADS, ((double)after.tv_sec + after.tv_nsec/1000000000.0) - ((double)before.tv_sec + before.tv_nsec/1000000000.0));    
}

int main(int argc, char* argv[]){
    int result;
    struct timespec before;
    struct timespec after;
    if ((result = parse_args(argc, argv)) != 0) print_error(result);
    if ((result = parse_image_header(image_in)) != 0) print_error(result);    
    if ((result = parse_image(image_in)) != 0) print_error(result);
    if ((result = parse_filter(image_in)) != 0) print_error(result);
    if (clock_gettime(CLOCK_REALTIME, &before)) print_error(-9);
    if ((result = filter_image()) != 0) print_error(result);
    if (clock_gettime(CLOCK_REALTIME, &after)) print_error(-9);
    print_log(before, after);
    if ((result = save_out_image()) != 0) print_error(result);
    return 0;
}
