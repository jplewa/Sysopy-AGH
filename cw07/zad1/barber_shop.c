#include "barber_shop.h" 

int print_log(char* msg, int pid){
    struct timespec tp;
    if (clock_gettime(CLOCK_MONOTONIC, &tp) < 0) return -8;
    if (pid != -1) printf("<%d>\t\t%lld.%.9ld\t\t%s\n", pid, (long long)tp.tv_sec, tp.tv_nsec, msg);
    else printf("\t\t%lld.%.9ld\t\t%s\n", (long long)tp.tv_sec, tp.tv_nsec, msg);
    fflush(stdout);
    return 0;
}

int get_lock(int semnum){
    struct sembuf buf;
    buf.sem_num = semnum;
    buf.sem_op = -1;
    buf.sem_flg = 0;
    return semop(SEM_ID, &buf, 1);
}

int get_lock_nowait(int semnum){
    struct sembuf buf;
    buf.sem_num = semnum;
    buf.sem_op = -1;
    buf.sem_flg = IPC_NOWAIT;
    return semop(SEM_ID, &buf, 1);
}

int release_lock(int semnum){
    struct sembuf buf;
    buf.sem_num = semnum;
    buf.sem_op = 1;
    buf.sem_flg = 0;
    return semop(SEM_ID, &buf, 1);
}