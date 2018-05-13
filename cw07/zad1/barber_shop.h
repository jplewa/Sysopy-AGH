#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <pwd.h>
#include <time.h> 

#define SHM_CODE 109
#define SEM_CODE 115

#define EXTRA_FIELDS 4

#define MEMORY_SEM 0
#define WAITING_ROOM_SEM 1
#define BARBER_CHAIR_SEM 2
#define DOOR_SEM 3

#define BARBER_STATE_MEM 0
#define FIRST_CUSTOMER_MEM 1
#define SEATS 2
#define BARBER_CHAIR_MEM 3

#define WORKING 0
#define ASLEEP 1
#define WOKEN_UP 2

union semnum {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};