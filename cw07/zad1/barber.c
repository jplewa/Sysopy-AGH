#include "barber_shop.h"

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
            printf("Couldn't attach shared memory segment\n");
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
        case -9:
            perror("Error");
            printf("Couldn't obtain time stamp\n");
            exit(0);
        case -10:
            perror("Error");
            printf("Couldn't manipulate semaphore set\n");
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

void SIGTERM_handler(int signum){
    exit(0);
}

int setup_signals(){
    struct sigaction act;
    act.sa_handler = &SIGTERM_handler;
    if (sigfillset(&act.sa_mask)) return -11;
    act.sa_flags = SA_RESTART;
    if (sigaction(SIGTERM, &act, NULL)) return -11;

    sigset_t set;
    if (sigfillset(&set)) return -11;
    if (sigdelset(&set, SIGTERM)) return -11;
    if (sigprocmask(SIG_SETMASK, &set, NULL)) return -11;

    return 0;
}

int setup_shm(){
    key_t shm_key;

    if ((shm_key = ftok(get_home_path(), SHM_CODE)) == -1) return -2;
    if ((SHM_ID = shmget(shm_key, sizeof(pid_t)*(CHAIRS+EXTRA_FIELDS), IPC_CREAT | 0777)) < 0) return -3;
    
    if (atexit(&atexit1)) return -12;
    
    if ((MEM = (pid_t*) shmat(SHM_ID, NULL, 0)) == (void*) (-1)) return -4;
    
    if (atexit(&atexit2)) return -12;
    
    for (int i = 0; i < (CHAIRS + EXTRA_FIELDS); i++) MEM[i] = 0;
    
    MEM[FIRST_CUSTOMER_MEM] = -1;
    MEM[LAST_CUSTOMER_MEM] = -1;
    MEM[SEATS_MEM] = CHAIRS;
    
    return 0;
}

int setup_sem(){
    key_t sem_key;

    if ((sem_key = ftok(get_home_path(), SEM_CODE)) == -1) return -6;
    if ((SEM_ID = semget(sem_key, (CHAIRS+EXTRA_FIELDS), IPC_CREAT | 0777)) < 0) return -7;
    
    if (atexit(&atexit3)) return -12;

    if (semctl(SEM_ID, WAITING_ROOM_SEM, SETVAL, CHAIRS)) return -13;
    if (semctl(SEM_ID, BARBER_CHAIR_SEM, SETVAL, 0)) return -13;
    if (semctl(SEM_ID, DOOR_SEM, SETVAL, 0)) return -13;
    if (semctl(SEM_ID, BARBER_STATE_SEM, SETVAL, 1)) return -13;
    if (semctl(SEM_ID, NAP_SEM, SETVAL, 0)) return -13;
    if (semctl(SEM_ID, SYNC_SEM, SETVAL, 0)) return -13;
    for (int i = EXTRA_FIELDS; i < (CHAIRS + EXTRA_FIELDS); i++){
        if (semctl(SEM_ID, i, SETVAL, 0)) return -13;
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

int serve_customer(int pid){
    if (release_lock(BARBER_CHAIR_SEM)) return -10;
    if (get_lock(SYNC_SEM)) return -10;
    if (print_log("Giving customer a haircut", pid)) return -9;
    if (get_lock(SYNC_SEM)) return -10;
    if (print_log("Finished haircut", pid)) return -9;
    if (get_lock(BARBER_STATE_SEM)) return -10;    
    if (release_lock(DOOR_SEM)) return -10;
    if (get_lock(SYNC_SEM)) return -10;
    return 0;
}

int invite_customer(int pid){
    if (print_log("Inviting customer", pid)) return -9;
    if (release_lock(MEM[FIRST_CUSTOMER_MEM])) return -10;
    MEM[FIRST_CUSTOMER_MEM] = EXTRA_FIELDS + (MEM[FIRST_CUSTOMER_MEM]+1-EXTRA_FIELDS)%CHAIRS;
    int result;
    if ((result = serve_customer(pid)) != 0) return result;
    if (release_lock(WAITING_ROOM_SEM)) return -9;
    return 0;
}

int take_a_nap(){
    MEM[BARBER_STATE_MEM] = ASLEEP;
    if (print_log("Barber asleep", -1)) return -9;
    if (release_lock(BARBER_STATE_SEM)) return -10;
    if (get_lock(NAP_SEM)) return -10;
    if (print_log("Barber awake", -1)) return -9;
    int result;
    if ((result = serve_customer(MEM[BARBER_CHAIR_MEM])) != 0) return result;
    return 0;
}

int barber_shop(){
    int result;
    while(1){
        if (release_lock(BARBER_STATE_SEM)) return -10;    
        if (get_lock(BARBER_STATE_SEM)) return -10;
        if (semctl(SEM_ID, WAITING_ROOM_SEM, GETVAL, 0) == CHAIRS){
            if ((result = take_a_nap()) != 0) return result;
        }
        else {
            if (release_lock(BARBER_STATE_SEM)) return -10;
            if ((result = invite_customer(MEM[MEM[FIRST_CUSTOMER_MEM]])) != 0) return result;;
        }
    }
    return 0;
}

int main (int argc, char* argv[]){
    int result;
    if ((result = parse_args(argc, argv)) != 0) print_error(result);
    if ((result = setup()) != 0) print_error(result);    
    if ((result = barber_shop()) != 0) print_error(result);    
    return 0;
}