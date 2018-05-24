#define _GNU_SOURCE 1

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <signal.h>
#include <string.h>
#include <mqueue.h> 

#define SERVER_NAME "/server114"
#define MAX_CLIENTS 2

#define NEW "10000000"
#define MIRROR "01000000"
#define CALC "00100000"
#define TIME "00010000"
#define END "00001000"
#define STOP "00000100"

#define MTEXTSIZE 256

struct client_info{
    mqd_t qd;
    char* name;
    pid_t pid;
};