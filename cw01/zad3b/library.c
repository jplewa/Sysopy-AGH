#include "library.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#define STATIC_ARRAY_SIZE 1000
#define STATIC_ARRAY_BLOCK_SIZE 1000

char static_array [STATIC_ARRAY_SIZE][STATIC_ARRAY_BLOCK_SIZE];

s_block_array* s_create_array (int array_size, int block_size){
    if (array_size <= STATIC_ARRAY_SIZE && block_size <= STATIC_ARRAY_BLOCK_SIZE) {
        s_block_array *new_array = malloc(sizeof(s_block_array));
        new_array->array = static_array;
        new_array->block_size = block_size;
        new_array->array_size = array_size;
        return  new_array;
    }
    else return NULL;
}
d_block_array* d_create_array(int array_size, int block_size){
    d_block_array* new_array = malloc(sizeof(d_block_array));
    new_array -> block_size = block_size;
    new_array -> array_size = array_size;
    new_array -> array = (char**) calloc ((size_t) array_size, sizeof(char*));
    return new_array;
}
void s_insert_block (s_block_array* block_array, char* block, int index){
    if (index >= 0 && index < block_array -> array_size){
        int i = 0;
        while (i < strlen(block) && i < (block_array -> block_size - 1)){
            block_array -> array [index][i] = block[i];
            i++;
        }
        do{
            block_array -> array [index][i] = '\0';
            i++;
        } while (i < block_array -> block_size);
    }
}
void d_insert_block (d_block_array* block_array, char* block, int index){
    if (index >= 0 && index < block_array -> array_size){
        free(block_array -> array[index]);
        block_array -> array[index] = calloc(block_array -> block_size, sizeof(char));

        int i = 0;
        while (i < strlen(block) && i < (block_array -> block_size - 1)){
            block_array -> array [index][i] = block[i];
            i++;
        }
        do{
            block_array -> array [index][i] = '\0';
            i++;
        } while (i < block_array -> block_size);    }
}

void s_delete_block (s_block_array* block_array, int index){
    if (index >= 0 && index < block_array -> array_size){
        for (int i = 0; i < block_array -> block_size; i++) block_array -> array[index][i] = '\0';
    }
}
void d_delete_block (d_block_array* block_array, int index) {
    if (index >= 0 && index < block_array->array_size) {
        free(block_array -> array[index]);
        block_array -> array[index] = NULL;
    }
}
void s_delete_array (s_block_array* block_array){
    for (int i = 0; i < block_array -> array_size; i++){
        for (int j = 0; j < block_array -> block_size; j++) block_array -> array [i][j] = '\0';
    }
    free(block_array);
}
void d_delete_array (d_block_array* block_array){
    for (int i = 0; i < block_array -> array_size; i++) free(block_array -> array[i]);
    free(block_array -> array);
    free(block_array);
}
int ascii_sum (char* block) {
    if (block == NULL) return 0;
    int sum = 0;
    for (int i = 0; i < strlen(block); i++) sum += (int) block[i];
    return sum;
}
int s_ascii_search (s_block_array* block_array, int value){
    int smallest_difference = value;
    int index = -1;

    for (int i = 0; i < block_array -> array_size; i++){
        if (block_array -> array[i][0] != '\0') {
            int sum = ascii_sum(block_array -> array[i]);
            int difference = abs(sum - value);
            if (smallest_difference == value){
                smallest_difference = difference;
                index = i;
            }
            if (difference < smallest_difference) {
                smallest_difference = difference;
                index = i;
            }
        }
    } return index;
}
int d_ascii_search (d_block_array* block_array, int value){
    int smallest_difference = value;
    int index = -1;

    for (int i = 0; i < block_array -> array_size; i++){
        if (block_array -> array[i] != NULL) {
            int sum = ascii_sum(block_array -> array[i]);
            int difference = abs(sum - value);
            if (smallest_difference == value){
                smallest_difference = difference;
                index = i;
            }
            if (difference < smallest_difference) {
                smallest_difference = difference;
                index = i;
            }
        }
    } return index;
}
