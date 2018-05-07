#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

int CHAIRS = 0;

int print_error(int error_code){

}

int parse_args(int argc, char* argv[]){
    char* pEnd;
    CHAIRS =  strtol (argv[1], &pEnd, 10);
    if (CHAIRS <= 0) return -1;
    return 0;
}

int main (int arc, char* argv[]){
    
    return 0;
}