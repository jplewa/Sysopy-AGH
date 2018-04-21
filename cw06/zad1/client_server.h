#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h> 
#include <sys/ipc.h> 
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <signal.h>
#include <string.h>

#define REQUEST_ID 114
#define MAX_CLIENTS 1

#define NEW 1
#define MIRROR 2
#define CALC 3
#define TIME 4
#define END 5
#define STOP 6

#define MTEXTSIZE 1024

struct msg{
    long mtype;
    pid_t mpid;
    char mtext[MTEXTSIZE];
};

struct client_info{
    int qid;
    pid_t pid;
};