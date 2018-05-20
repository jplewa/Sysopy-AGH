#include <stdlib.h>    
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>

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
unsigned int **image_out;
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
            printf("Error: incorrect input file\n");
            printf("Please use ASCII PGM format\n");
            exit(0);
        case -5:
            printf("Error: incorrect filter file\n");
            printf("Please use ASCII PGM format\n");
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
    if ((IMAGE_OUT = fopen(argv[4], "r+")) == NULL) return -1;
    if (atexit(&atexit3)) return -2;
    return 0;
}

int parse_image_header(){
    char* lineptr = NULL;
    size_t n = 0;
    if ((getline(&lineptr, &n, IMAGE_IN)) <= 0) return -3;
    if ((strncmp("P2", lineptr, 2)) != 0) return -4;
    if ((getline(&lineptr, &n, IMAGE_IN)) <= 0) return -3;
    char* pEnd;
    if ((W =  strtol (lineptr, &pEnd, 10)) <= 0) return -4;
    if ((H =  strtol (pEnd, &pEnd, 10)) <= 0) return -4;
    if ((getline(&lineptr, &n, IMAGE_IN)) <= 0) return -3;
    if ((strtol (lineptr, &pEnd, 10)) != 255) return -4;
    return 0;
}

int parse_image(){
    char* pEnd;
    char* lineptr = NULL;
    size_t n;
    image_in = malloc((H+1) * sizeof(unsigned char *));
    for (int i = 1; i <= H; i++) image_in[i] = malloc((W+1) * sizeof(unsigned char));
    image_out = malloc((H+1) * sizeof(unsigned int *));
    for (int i = 1; i <= H; i++) image_out[i] = malloc((W+1) * sizeof(unsigned int));
    if (atexit(&atexit4)) return -2;
    char* c;
    char* buffer = malloc(4*sizeof(char));
    int i = 1;
    int j = 1;
    long int tmp = 0;
    while (((getline(&lineptr, &n, IMAGE_IN)) > 0) && (errno == 0)){
        if (i > H) return -4;
        j = 1;
        buffer = strtok_r(lineptr, " \t\n", &c);  
        if (((tmp = strtol(buffer, &pEnd, 10)) > 255) || (tmp < 0)) return -4;
        image_in[i][j] = tmp;
        buffer = strtok_r(NULL, " \t\n", &c);   
        while (buffer != NULL){
            j++;
            if (j > W) return -4;
            tmp = strtol(buffer, &pEnd, 10);
            image_in[i][j] = tmp; 
            buffer = strtok_r(NULL, " \t\n", &c);   
        }
        i++;
    }
    free(buffer);
    return 0;
}

int parse_filter(){
    char* lineptr = NULL;
    size_t n = 0;
    if ((getline(&lineptr, &n, FILTER) <= 0)) return -3;
    char* pEnd;
    if ((C =  strtol (lineptr, &pEnd, 10)) <= 0) return -4;
    filter = (float **) malloc((C+1) * sizeof(float*));
    for (int i = 1; i <= C; i++) filter[i] = (float*) malloc((C+1) * (sizeof(float)));
    if (atexit(&atexit5)) return -2;
    char* c;
    char* buffer = malloc(33*sizeof(char));
    int i = 1;
    int j = 1;
    
    while (((getline(&lineptr, &n, FILTER)) > 0) && (errno == 0)){
        if (i > C) return -5;
        j = 1;
        buffer = strtok_r(lineptr, " \t\n", &c);  
        filter[i][j] = strtof(buffer, &pEnd);
        buffer = strtok_r(NULL, " \t\n", &c);   
        while (buffer != NULL){
            j++;
            if (j > C) return -5;
            filter[i][j] = strtof(buffer, &pEnd); 
            buffer = strtok_r(NULL, " \t\n", &c);   
        }
        i++;
    }
    free(buffer);
    return 0;
}

int get_filtered_value(int x, int y){
    double s = 0; 
    //printf("(%d, %d) ", x, y); 
    for (int i = 1; i <= C; i++){
        for (int j = 1; j <= C; j++){
            //printf("[%d][%d] ", (int) (min(H, max(1, x - ceil((double)C/2) + i))), (int) (min(W, max(1, y - ceil((double)C/2) + j))));
            s += image_in[(int) (min(H, max(1, x - ceil(C/2) + i)))][(int) (min(W, max(1, y - ceil(C/2) + j)))] * filter[i][j];
        }
    }
    //printf("\n");
    return round(s);
}

void* filter_function(void* args){
    for (int i = 1; i <= H; i++){
        for (int j = ((int*) args)[0]; j <= ((int*) args)[1]; j++){
            image_out[i][j] = get_filtered_value(i, j);
        }
    }
    return NULL;
}


int filter_image(){
    //pthread_t* threads = malloc(THREADS * sizeof(pthread_t));
    int range[] = {0,0}; 
    for (int i = 1; i <= THREADS; i++){
        range[0] = range[1] + 1;
        range[1] = (i <= (W%THREADS)) ? (i*ceil((double)W/THREADS)) : (i*W/THREADS);
        //printf("%d %d\n", range[0], range[1]);
        filter_function((void*) range);
        //pthread_create(&threads[i], NULL, filter_function, (void*)(range));
    }
    return 0;
}

int main(int argc, char* argv[]){
    int result;
    if ((result = parse_args(argc, argv)) != 0) print_error(result);
    if ((result = parse_image_header(image_in)) != 0) print_error(result);    
    if ((result = parse_image(image_in)) != 0) print_error(result);
    if ((result = parse_filter(image_in)) != 0) print_error(result);
    
    for (int i = 1; i <= H; i++){
        for (int j = 1; j <= W; j++) printf("%d ", image_in[i][j]);
        printf("\n");
    }
    printf("\n\n");
    for (int i = 1; i <= C; i++){
        for (int j = 1; j <= C; j++) printf("%f ", filter[i][j]);
        printf("\n");
    }
    filter_image();
    for (int i = 1; i <= H; i++){
        for (int j = 1; j <= W; j++) printf("%d ", image_out[i][j]);
        printf("\n");
    }
    return 0;
}