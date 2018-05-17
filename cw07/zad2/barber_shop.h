#ifndef BARBER_SHOP_H
#define BARBER_SHOP_H

#include <sys/sem.h>
#include <sys/ipc.h>

#include <sys/mman.h>
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
#include <semaphore.h>
#include <fcntl.h>

#define SHM_NAME "/shm_name"

#define EXTRA_FIELDS 6

#define NAP_SEM 0
#define NAP_SEM_NAME "/nap_sem"
#define WAITING_ROOM_SEM 1
#define WAITING_ROOM_SEM_NAME "/waiting_room_sem"
#define BARBER_CHAIR_SEM 2
#define BARBER_CHAIR_SEM_NAME "/barber_chair_sem"
#define DOOR_SEM 3
#define DOOR_SEM_NAME "/door_sem"
#define BARBER_STATE_SEM 4
#define BARBER_STATE_SEM_NAME "/barber_state_sem"
#define SYNC_SEM 5
#define SYNC_SEM_NAME "/sync_sem"

#define SEATS_MEM 0
#define FIRST_CUSTOMER_MEM 1
#define LAST_CUSTOMER_MEM 2
#define BARBER_STATE_MEM 3
#define BARBER_CHAIR_MEM 4

#define AWAKE 0
#define ASLEEP 1

int print_log(char* msg, int pid);
int get_lock(int semnum);
int get_lock_nowait(int semnum);
int release_lock(int semnum);

#endif