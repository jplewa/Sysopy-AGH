#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[]){
    if (argc < 3){
        return 0;
    }
    if (argv[1][0] == 'p' || argv[1][0] == 'l'){
        char str[1000];
        scanf("%1000[0-9a-zA-Z ]", str);
        char buffer[1000];
        strcpy(buffer, argv[2]);
        for (int i = 3; i < argc; i++){
            strcat(buffer, " ");
            strcat(buffer, argv[i]);
        }
        printf("%s %s", str, buffer);
        if (argv[1][0] == 'l') printf("\n");
    }
    else if (argv[1][0] == 'f'){
        char buffer[1000];
        strcpy(buffer, argv[2]);
        for (int i = 3; i < argc; i++) strcat(buffer, argv[i]);
        printf("%s", buffer);
    }
    return 0;
}