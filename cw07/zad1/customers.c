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

void print_log(char* msg, int pid){
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    printf("<%d>\t\t%lld.%.9ld\t\t%s\n", pid, (long long)tp.tv_sec, tp.tv_nsec, msg);
    fflush(stdout);
}

void get_lock(int semnum){
    struct sembuf buf;
    buf.sem_num = semnum;
    buf.sem_op = -1;
    buf.sem_flg = 0;
    semop(SEM_ID, &buf, 1);
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

void wake_up_barber(int pid){
    //get_lock(BUSY_SEM);
    MEM[BARBER_STATE_MEM] = AWAKE;
    release_lock(BARBER_STATE_SEM);
    MEM[BARBER_CHAIR_MEM] = pid;
    release_lock(NAP_SEM);
    print_log("Waking up barber", pid);
    get_lock(BARBER_CHAIR_SEM);
    print_log("Sitting down in barber's chair", pid);
    get_lock(DOOR_SEM);
    print_log("Leaving barber shop", pid);
}

void go_to_waiting_room(int pid){
    release_lock(BARBER_STATE_SEM);
    if (get_lock_nowait(WAITING_ROOM_SEM) < 0) print_log("No seats in waiting room", pid);
    else {
        get_lock(MEMORY_SEM);
        print_log("Sitting down in waiting room", pid);
        int my_seat;
        if (MEM[FIRST_CUSTOMER_MEM] == -1){
            my_seat = EXTRA_FIELDS;
            MEM[FIRST_CUSTOMER_MEM] = EXTRA_FIELDS;
            MEM[LAST_CUSTOMER_MEM] = EXTRA_FIELDS;
        }
        else {
            MEM[LAST_CUSTOMER_MEM] = EXTRA_FIELDS + (MEM[LAST_CUSTOMER_MEM] + 1 - EXTRA_FIELDS)%MEM[SEATS_MEM];
            my_seat = MEM[LAST_CUSTOMER_MEM];
        }
        MEM[my_seat] = pid;
        release_lock(MEMORY_SEM);
        get_lock(my_seat);
        get_lock(BARBER_CHAIR_SEM);
        release_lock(WAITING_ROOM_SEM);
        print_log("Sitting down in barber's chair", pid);
        get_lock(DOOR_SEM);
        print_log("Leaving barber shop", pid);
    }
}

void get_haircuts(int pid){
    for (int j = 0; j < HAIRCUTS; j++){
        get_lock(BARBER_STATE_SEM);
        if (MEM[BARBER_STATE_MEM] == ASLEEP) wake_up_barber(pid);
        else go_to_waiting_room(pid);
    }
}

int spawn(){
    int status;
    int pid;
    for (int i = 0; i < CUSTOMERS; i++){
        if ((pid = fork()) > 0){

        }
        else if (pid == 0){
            get_haircuts(getpid());
            exit(0);
        }
        else{
            exit(0);
        }
    }
    while(wait(&status) > 0);
    return 0;
}


int main(int argc, char* argv[]){
    int result;
    if ((result = parse_args(argc, argv) != 0)) print_error(result);
    if ((result = setup()) != 0) print_error(result);    
    spawn();
    return 0;
}