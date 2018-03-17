/*
 * Zadanie 1. Alokacja tablicy z wskaźnikami na bloki pamięci zawierające znaki (25%)
 * Zaprojektuj i przygotuj zestaw funkcji (bibliotekę) do zarządzania tablicą bloków zawierających znaki.
 * Biblioteka powinna umożliwiać: 
 * - tworzenie i usuwanie tablicy
 * - dodanie i usunięcie bloków na które wskazują wybrane indeksy elementów tablicy 
 * - wyszukiwanie bloku w tablicy, którego suma znaków (kodów ASCII) w bloku jest najbliższa elementowi o zadanym numerze,
 * Tablice i bloki powinny być alokowane przy pomocy funkcji calloc (alokacja dynamiczna)
 * jak również powinny wykorzystywać tablicę dwuwymiarową (statyczny przydział pamięci).
 * Przygotuj plik Makefile, zawierający polecenia kompilujące pliki źródłowe biblioteki
 * oraz tworzące biblioteki w dwóch wersjach: statyczną i dzieloną.
 */

#ifndef EX1_LIBRARY_H
#define EX1_LIBRARY_H

#include <stdbool.h>

#define STATIC_ARRAY_SIZE 1000
#define STATIC_ARRAY_BLOCK_SIZE 1000

typedef struct s_block_array {
    char (*array) [STATIC_ARRAY_SIZE];
    int block_size;
    int array_size;
} s_block_array;

typedef struct d_block_array {
    char** array;
    int block_size;
    int array_size;
} d_block_array;

s_block_array* s_create_array (int array_size, int block_size);
d_block_array* d_create_array (int array_size, int block_size);

void s_insert_block (s_block_array* block_array, char* block, int index);
void d_insert_block (d_block_array* block_array, char* block, int index);

void s_delete_block (s_block_array* block_array, int index);
void d_delete_block (d_block_array* block_array, int index);

void s_delete_array (s_block_array* block_array);
void d_delete_array (d_block_array* block_array);

int ascii_sum (char* block);

int s_ascii_search (s_block_array* block_array, int index);
int d_ascii_search (d_block_array* block_array, int index);


#endif