#include "barber_shop.h"

int SHM_D;
int SEM_D;

pid_t* MEM;
sem_t** SEM;

int CUSTOMERS;
int HAIRCUTS;

void atexit1(){
    for (int i = 0; i < EXTRA_FIELDS; i++) sem_close(SEM[i]);
}

void atexit2(){
    munmap(MEM, sizeof(pid_t)*(MEM[0]+EXTRA_FIELDS));
    close(SHM_D);
}

void print_error(int error_code){
    switch(error_code){
        case -1:
            printf("Error: incorrect arguments\n");
            printf("Please provide: [number_of_customers] [number_of_chairs]\n");
            exit(0);  
        case -2:
            perror("Error");
            printf("Couldn't open shared memory object\n");
            exit(0); 
        case -4:
            perror("Error");
            printf("Couldn't attach shared memory segment\n");
            exit(0);
        case -8:
            perror("Error");
            printf("Couldn't obtain time stamp\n");
            exit(0);
         case -9:
            perror("Error");
            printf("Couldn't manipulate semaphore set\n");
            exit(0);
         case -10:
            perror("Error");
            printf("Couldn't create new process\n");
            exit(0);
         case -11:
            perror("Error");
            printf("Couldn't set signal handlers\n");
            exit(0);
        case -12:
            perror("Error");
            printf("Couldn't set exit handlers\n");
            exit(0);
        case -13:
            perror("Error");
            printf("Couldn't set initial semaphore values\n");
            exit(0);
    }
}

int parse_args(int argc, char* argv[]){
    if (argc < 3) return -1;
    char* pEnd;
    CUSTOMERS =  strtol (argv[1], &pEnd, 10);
    if (CUSTOMERS <= 0) return -1;
    HAIRCUTS =  strtol (argv[2], &pEnd, 10);
    if (HAIRCUTS <= 0) return -1;
    return 0;
}

int setup_signals(){
    sigset_t set;
    if (sigfillset(&set)) return -11;
    if (sigdelset(&set, SIGTERM)) return -11;
    if (sigprocmask(SIG_SETMASK, &set, NULL)) return -11;
    return 0;
}

int setup_shm(){
    if ((SHM_D = shm_open(SHM_NAME, O_RDWR, 0777)) == -1) return -2;
    if ((MEM = (pid_t*) mmap(NULL, sizeof(pid_t), PROT_READ | PROT_WRITE, MAP_SHARED, SHM_D, 0)) == MAP_FAILED) return -4;
    if ((MEM = (pid_t*) mmap(NULL, sizeof(pid_t)*(MEM[0]+EXTRA_FIELDS), PROT_READ | PROT_WRITE, MAP_SHARED, SHM_D, 0)) == MAP_FAILED) return -4;
    if (atexit(&atexit2)) return -12;
    return 0;
}

int setup_sem(){
    SEM = malloc((EXTRA_FIELDS+MEM[SEATS_MEM])*sizeof(sem_t*));
    SEM[NAP_SEM] = sem_open(NAP_SEM_NAME, 0, 0777, 0);
    SEM[WAITING_ROOM_SEM] = sem_open(WAITING_ROOM_SEM_NAME, 0);
    SEM[BARBER_CHAIR_SEM] = sem_open(BARBER_CHAIR_SEM_NAME, 0);
    SEM[DOOR_SEM] = sem_open(DOOR_SEM_NAME, 0);
    SEM[BARBER_STATE_SEM] = sem_open(BARBER_STATE_SEM_NAME, 0);
    SEM[SYNC_SEM] = sem_open(SYNC_SEM_NAME, 0);
    char* sem_name = malloc(128);
    for (int i = EXTRA_FIELDS; i < (MEM[SEATS_MEM] + EXTRA_FIELDS); i++){
        sprintf(sem_name, "/chair%d", (i-EXTRA_FIELDS));
        SEM[i] = sem_open(sem_name, 0);
    }
    if (atexit(&atexit1)) return -12;
    for (int i = 0; i < (MEM[SEATS_MEM] + EXTRA_FIELDS); i++){
        if (SEM[i] == SEM_FAILED) return -13;
    }
    return 0;
}

int setup(){
    int result;
    if ((result = setup_signals()) != 0) return result;
    if ((result = setup_shm()) != 0) return result;
    if ((result = setup_sem()) != 0) return result;
    return 0;
}

int wake_up_barber(int pid){
    MEM[BARBER_STATE_MEM] = AWAKE;
    MEM[BARBER_CHAIR_MEM] = pid;
    if (print_log("Waking up barber", pid)) return -8;
    if (release_lock(NAP_SEM)) return -9;
    if (release_lock(BARBER_STATE_SEM)) return -9;
    if (get_lock(BARBER_CHAIR_SEM)) return -9;
    if (print_log("Sitting down in barber's chair", pid)) return -8;
    if (release_lock(SYNC_SEM)) return -10;
    if (release_lock(SYNC_SEM)) return -10;
    if (get_lock(DOOR_SEM)) return -9;
    if (print_log("Leaving barber shop", pid)) return -8;
    if (release_lock(SYNC_SEM)) return -10;
    return 0;
}

int go_to_waiting_room(int pid){
    if (get_lock_nowait(WAITING_ROOM_SEM) < 0){
        if (print_log("No seats in waiting room", pid)) return -8;
        if (release_lock(BARBER_STATE_SEM)) return -9;
    }
    else {
        if (print_log("Sitting down in waiting room", pid)) return -8;
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
        if (release_lock(BARBER_STATE_SEM)) return -9;
        if (get_lock(my_seat)) return -9;
        if (get_lock(BARBER_CHAIR_SEM)) return -9;
        if (print_log("Sitting down in barber's chair", pid)) return -8;
        if (release_lock(SYNC_SEM)) return -10;
        if (release_lock(SYNC_SEM)) return -10;
        if (get_lock(DOOR_SEM)) return -9;
        if (print_log("Leaving barber shop", pid)) return -8;
        if (release_lock(SYNC_SEM)) return -10;
    }
    return 0;
}

int get_haircuts(int pid){
    int result;
    for (int j = 0; j < HAIRCUTS; j++){
        if (get_lock(BARBER_STATE_SEM)) return -9;
        if (MEM[BARBER_STATE_MEM] == ASLEEP){
            if ((result = wake_up_barber(pid)) != 0) return result;
        }
        else if ((result = go_to_waiting_room(pid)) != 0) return result;
    }
    return 0;
}

int spawn(){
    int result;
    int pid;
    for (int i = 0; i < CUSTOMERS; i++){
        pid = fork();
        if (pid == 0){
            if ((result = get_haircuts(getpid())) != 0) return result;
            exit(0);
        }
        else if (pid < 0){
            return -10;
        }
    }
    while(wait(NULL) > 0);
    return 0;
}

int main(int argc, char* argv[]){
    int result;
    if ((result = parse_args(argc, argv) != 0)) print_error(result);
    if ((result = setup()) != 0) print_error(result);    
    if ((result = spawn()) != 0) print_error(result);    
    return 0;
}