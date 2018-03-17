#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

void generate (FILE* file, int size, int record_size){
    for (int i = 0; i < size; i++){
        for (int j = 0; j < record_size - 1; j++){
            char c = (char)(' ' + rand()%58);
            fwrite(&c, 1, 1, file);
        }
        char c = (char) 10;
        fwrite(&c, 1, 1, file);
    }
}
void sys_sort (){

}
void lib_sort(){

}
void sys_copy(){

}
void lib_copy(){

}

void parse(int argc, char* argv[]){
    if (argc < 5) return;
    if (strncmp(argv[1], "generate", 8) == 0){
        printf("%s", "ok\n");
        const char* filename = argv[2];
        const char* mode = "w";
        FILE* file =  fopen(filename, mode);
        char * pEnd;
        int size = (int) strtol(argv[3],&pEnd,10);
        int record_size = (int) strtol(argv[4],&pEnd,10);
        if (size == 0 || record_size == 0) return;
        else generate(file, size, record_size);
        fclose(file);
    }
}


int main(int argc, char* argv[]){
    parse(argc, argv);
}