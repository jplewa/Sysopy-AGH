#ifndef BARBER_SHOP_H
#define BARBER_SHOP_H

#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <pwd.h>
#include <time.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define SHM_CODE 109
#define SEM_CODE 115

#define EXTRA_FIELDS 5

#define NAP_SEM 0
#define WAITING_ROOM_SEM 1
#define BARBER_CHAIR_SEM 2
#define DOOR_SEM 3
#define BARBER_STATE_SEM 4

#define BARBER_STATE_MEM 0
#define FIRST_CUSTOMER_MEM 1
#define LAST_CUSTOMER_MEM 2
#define SEATS_MEM 3
#define BARBER_CHAIR_MEM 4

#define AWAKE 0
#define ASLEEP 1

int SEM_ID;

union semnum {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int print_log(char* msg, int pid);
int get_lock(int semnum);
int get_lock_nowait(int semnum);
int release_lock(int semnum);

#endif