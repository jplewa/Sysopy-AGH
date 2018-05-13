#include "barber_shop.h"

pid_t* MEM;
int CUSTOMERS;
int HAIRCUTS;

int SHM_ID;
int SEM_ID;

void print_error(int error_code){
    switch(error_code){
        case -1:
            printf("Error: incorrect arguments\n");
            printf("Please provide: [number_of_customers] [number_of_chairs]\n");
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
            printf("Couldn't attach shared memory segment\n");
            exit(0);
        case -6:
            perror("Error");
            printf("Couldn't obtain semaphore memory key\n");
            exit(0);
        case -7:
            perror("Error");
            printf("Couldn't get semaphore set\n");
            exit(0);
    }
}

int parse_args(int argc, char* argv[]){
    char* pEnd;
    CUSTOMERS =  strtol (argv[1], &pEnd, 10);
    if (CUSTOMERS <= 0) return -1;
    HAIRCUTS =  strtol (argv[2], &pEnd, 10);
    if (HAIRCUTS <= 0) return -1;
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
    if ((shm_key = ftok(get_home_path(), SHM_CODE)) == -1){
        return -2;
    }
    if ((SHM_ID = shmget(shm_key, 0, 0)) < 0){
        return -3;
    }
    if ((MEM = (pid_t*) shmat(SHM_ID, NULL, 0)) == (void*) (-1)){
        return -4;
    }
    if ((sem_key = ftok(get_home_path(), SEM_CODE)) == -1){
        return -6;
    }
    if ((SEM_ID = semget(sem_key, 0, 0)) < 0){
        return -7;
    }
    return 0;
}

void get_lock(int semnum){
    struct sembuf buf;
    buf.sem_num = semnum;
    buf.sem_op = -1;
    buf.sem_flg = 0;
    semop(SEM_ID, &buf, 1);
}

void print_log(char* msg, int pid){
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    printf("<%d>\t\t%lld.%.9ld\t\t%s\n", pid, (long long)tp.tv_sec, tp.tv_nsec, msg);
}

int get_lock_nowait(int semnum){
    struct sembuf buf;
    buf.sem_num = semnum;
    buf.sem_op = -1;
    buf.sem_flg = IPC_NOWAIT;
    return semop(SEM_ID, &buf, 1);
}

void release_lock(int semnum){
    struct sembuf buf;
    buf.sem_num = semnum;
    buf.sem_op = 1;
    buf.sem_flg = 0;
    semop(SEM_ID, &buf, 1);
}

void get_haircuts(int pid){
    for (int j = 0; j < HAIRCUTS; j++){
        get_lock(MEMORY_SEM);
        if (MEM[BARBER_STATE_MEM] == ASLEEP){
            release_lock(BARBER_CHAIR_SEM);
            MEM[BARBER_CHAIR_MEM] = pid;
            release_lock(MEMORY_SEM);
            get_lock(BARBER_CHAIR_SEM);
            get_lock(DOOR_SEM);
        }
        else{
            if (get_lock_nowait(WAITING_ROOM_SEM) < 0){
                print_log("No seats in waiting room", pid);
            }
            else{
                print_log("Seating down in waiting room", pid);
                int my_seat = EXTRA_FIELDS + (MEM[FIRST_CUSTOMER_MEM] + CUSTOMERS - semctl(SEM_ID, WAITING_ROOM_SEM, GETVAL, 0))%CUSTOMERS;
                MEM[my_seat] = pid;
                release_lock(MEMORY_SEM);
                get_lock(my_seat);
                get_lock(BARBER_CHAIR_SEM);
                get_lock(DOOR_SEM);

            }
        }
    }
}

int spawn(){
    int pid;
    for (int i = 0; i < CUSTOMERS; i++){
        if ((pid = fork()) > 0){
            get_haircuts(pid);
        }
        else if (pid == 0){

        }
        else{

        }
    }
    return 0;
}


int main(int argc, char* argv[]){
    int result;
    if ((result = parse_args(argc, argv) != 0)) print_error(result);
    if ((result = setup()) != 0) print_error(result);    
    spawn();
    return 0;
}