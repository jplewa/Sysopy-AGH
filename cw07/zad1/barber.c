#include "barber_shop.h"
#include <errno.h>

int CHAIRS = 0;

int SHM_ID;
int SEM_ID;

pid_t* MEM;

void atexit1(){
    shmctl(SHM_ID, IPC_RMID, NULL);
}

void atexit2(){
    shmdt(MEM);
}

void atexit3(){
    for (int i = 0; i < EXTRA_FIELDS; i++) semctl(SEM_ID, i, IPC_RMID, NULL);
}

void print_error(int error_code){
    switch(error_code){
        case -1:
            printf("Error: incorrect arguments\n");
            printf("Please provide: [number_of_chairs]\n");
            exit(0);
        case -2:
            perror("Error");
            printf("Couldn't obtain shared memory key\n");
            exit(0);
        case -3:
            perror("Error");
            printf("Couldn't get shared memory segment\n");
            exit(0);
        case -4:
            perror("Error");
            printf("%d Couldn't attach shared memory segment\n", errno);
            exit(0);
        case -5:
            perror("Error");
            printf("Couldn't detach shared memory segment\n");
            exit(0);
        case -6:
            perror("Error");
            printf("Couldn't obtain semaphore memory key\n");
            exit(0);
        case -7:
            perror("Error");
            printf("Couldn't get semaphore set\n");
            exit(0);
        case -8:
            perror("Error");
            printf("Couldn't set initial semaphore states\n");
            exit(0);
    }
}

int parse_args(int argc, char* argv[]){
    if (argc < 2) return -1;
    char* pEnd;
    CHAIRS =  strtol (argv[1], &pEnd, 10);
    if (CHAIRS <= 0) return -1;
    return 0;
}

const char *get_home_path(){
    const char* home_dir;
    if ((home_dir = getenv("HOME")) == NULL) home_dir = getpwuid(getuid()) -> pw_dir;
    return home_dir;
}

int setup(){
    key_t shm_key;
    key_t sem_key;

    // shared memory
    if ((shm_key = ftok(get_home_path(), SHM_CODE)) == -1){
        return -2;
    }
    if ((SHM_ID = shmget(shm_key, sizeof(pid_t)*(CHAIRS+EXTRA_FIELDS), IPC_CREAT | 0777)) < 0){
        return -3;
    }
    atexit(&atexit1);
    if ((MEM = (pid_t*) shmat(SHM_ID, NULL, 0)) == (void*) (-1)){
        return -4;
    }
    atexit(&atexit2);
    for (int i = 0; i < (CHAIRS + EXTRA_FIELDS); i++) MEM[i] = 0;
    MEM[FIRST_CUSTOMER_MEM] = EXTRA_FIELDS-1;

    // semaphores
    if ((sem_key = ftok(get_home_path(), SEM_CODE)) == -1){
        return -6;
    }
    if ((SEM_ID = semget(sem_key, (CHAIRS+EXTRA_FIELDS), IPC_CREAT | 0777)) < 0){
        return -7;
    }
    atexit(&atexit3);
    semctl(SEM_ID, MEMORY_SEM, SETVAL, 1);
    semctl(SEM_ID, WAITING_ROOM_SEM, SETVAL, CHAIRS);
    semctl(SEM_ID, BARBER_CHAIR_SEM, SETVAL, 1);
    for (int i = 2; i < (CHAIRS + EXTRA_FIELDS); i++) semctl(SEM_ID, i, SETVAL, 0);

    return 0;
}

void get_lock(int semnum){
    struct sembuf buf;
    buf.sem_num = semnum;
    buf.sem_op = -1;
    buf.sem_flg = 0;
    if (semop(SEM_ID, &buf, 1));
}

void release_lock(int semnum){
    struct sembuf buf;
    buf.sem_num = semnum;
    buf.sem_op = 1;
    buf.sem_flg = 0;
    if (semop(SEM_ID, &buf, 1));
}

void print_log(char* msg, int pid){
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    printf("<%d>\t\t%lld.%.9ld\t\t%s\n", pid, (long long)tp.tv_sec, tp.tv_nsec, msg);
}

void go_to_sleep(){

}

void take_a_nap(){
    get_lock(MEMORY_SEM);
    MEM[BARBER_STATE_MEM] = ASLEEP;
    print_log("Barber asleep", getpid());
    release_lock(MEMORY_SEM);
    get_lock(BARBER_CHAIR_SEM);
    MEM[BARBER_STATE_MEM] = WORKING;
    print_log("Barber awake", getpid());
}

void serve_customer(int pid){
    print_log("Giving customer a haircut", pid);
    release_lock(BARBER_CHAIR_SEM);
    print_log("Finished haircut", pid);
    release_lock(DOOR_SEM);
    MEM[FIRST_CUSTOMER_MEM] = EXTRA_FIELDS + (MEM[FIRST_CUSTOMER_MEM]+1)%CHAIRS;
    release_lock(MEMORY_SEM);
}

void invite_customer(int pid){
    get_lock(MEMORY_SEM);
    get_lock(DOOR_SEM);
    print_log("Inviting customer", pid);
    release_lock(MEM[FIRST_CUSTOMER_MEM]);
    serve_customer(pid);
}

int barber_shop(){
    while(1){
        if (semctl(SEM_ID, WAITING_ROOM_SEM, GETVAL, 0) == CHAIRS){
            take_a_nap();
            serve_customer(MEM[BARBER_STATE_MEM]);
        }
        else{
            invite_customer(MEM[MEM[FIRST_CUSTOMER_MEM]]);
        }
    }
    return 0;
}


int main (int argc, char* argv[]){
    int result;
    if ((result = parse_args(argc, argv)) != 0) print_error(result);
    if ((result = setup()) != 0) print_error(result);    
    barber_shop();
    return 0;
}