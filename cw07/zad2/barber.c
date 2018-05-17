#include "barber_shop.h"

int CHAIRS = 0;

int SHM_D;

sem_t** SEM;
pid_t* MEM;

void atexit1(){
    shm_unlink(SHM_NAME);
}

void atexit2(){
    munmap(MEM, sizeof(pid_t)*(CHAIRS+EXTRA_FIELDS));
}

void atexit3(){
    for (int i = 0; i < EXTRA_FIELDS; i++){
        sem_close(SEM[i]);
    }
    sem_unlink(NAP_SEM_NAME);
    sem_unlink(WAITING_ROOM_SEM_NAME);
    sem_unlink(BARBER_CHAIR_SEM_NAME);
    sem_unlink(DOOR_SEM_NAME);
    sem_unlink(BARBER_STATE_SEM_NAME);
    sem_unlink(SYNC_SEM_NAME);

    char* sem_name = malloc(128);
    for (int i = EXTRA_FIELDS; i < (CHAIRS + EXTRA_FIELDS); i++){
        sprintf(sem_name, "/chair%d", (i-EXTRA_FIELDS));
        sem_unlink(sem_name);
    }
}

void print_error(int error_code){
    switch(error_code){
        case -1:
            printf("Error: incorrect arguments\n");
            printf("Please provide: [number_of_chairs]\n");
            exit(0);
        case -2:
            perror("Error");
            printf("Couldn't open shared memory object\n");
            exit(0);
        case -3:
            perror("Error");
            printf("Couldn't stretch shared memory segment\n");
            exit(0);
        case -4:
            perror("Error");
            printf("Couldn't map shared memory segment\n");
            exit(0);
        case -5:
            perror("Error");
            printf("Couldn't obtain semaphore value\n");
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
    if ((SHM_D = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0777)) == -1) return -2;
    if (ftruncate(SHM_D, sizeof(pid_t)*(CHAIRS+EXTRA_FIELDS))) return -3;

    if (atexit(&atexit1)) return -12;
    
    if ((MEM = (pid_t*) mmap(NULL, sizeof(pid_t)*(CHAIRS+EXTRA_FIELDS), PROT_READ | PROT_WRITE, MAP_SHARED, SHM_D, 0)) == MAP_FAILED) return -4;
    
    if (atexit(&atexit2)) return -12;
    
    for (int i = 0; i < (CHAIRS + EXTRA_FIELDS); i++) MEM[i] = 0;
    
    MEM[FIRST_CUSTOMER_MEM] = -1;
    MEM[LAST_CUSTOMER_MEM] = -1;
    MEM[SEATS_MEM] = CHAIRS;
    
    return 0;
}

int setup_sem(){
    SEM = malloc((EXTRA_FIELDS+CHAIRS)*sizeof(sem_t*));
    SEM[NAP_SEM] = sem_open(NAP_SEM_NAME, O_CREAT, 0777, 0);
    SEM[WAITING_ROOM_SEM] = sem_open(WAITING_ROOM_SEM_NAME, O_CREAT, 0777, CHAIRS);
    SEM[BARBER_CHAIR_SEM] = sem_open(BARBER_CHAIR_SEM_NAME, O_CREAT, 0777, 0);
    SEM[DOOR_SEM] = sem_open(DOOR_SEM_NAME, O_CREAT, 0777, 0);
    SEM[BARBER_STATE_SEM] = sem_open(BARBER_STATE_SEM_NAME, O_CREAT, 0777, 1);
    SEM[SYNC_SEM] = sem_open(SYNC_SEM_NAME, O_CREAT, 0777, 0);
    char* sem_name = malloc(128);
    for (int i = EXTRA_FIELDS; i < (CHAIRS + EXTRA_FIELDS); i++){
        sprintf(sem_name, "/chair%d", (i-EXTRA_FIELDS));
        SEM[i] = sem_open(sem_name, O_CREAT, 0777, 0);
    }
    for (int i = 0; i < (CHAIRS + EXTRA_FIELDS); i++){
        if (SEM[i] == SEM_FAILED) return -13;
    }

    if (atexit(&atexit3)) return -12;
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
    int value;
    while(1){
        if (get_lock(BARBER_STATE_SEM)) return -10;
        if (sem_getvalue(SEM[WAITING_ROOM_SEM], &value)) return -5;
        if (value == CHAIRS){
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