#include "barber_shop.h" 

sem_t** SEM;

int print_log(char* msg, int pid){
    struct timespec tp;
    if (clock_gettime(CLOCK_MONOTONIC, &tp) < 0) return -8;
    if (pid != -1) printf("<%d>\t\t%lld.%.9ld\t\t%s\n", pid, (long long)tp.tv_sec, tp.tv_nsec, msg);
    else printf("\t\t%lld.%.9ld\t\t%s\n", (long long)tp.tv_sec, tp.tv_nsec, msg);
    fflush(stdout);
    return 0;
}

int get_lock(int semnum){
    return sem_wait(SEM[semnum]);
}

int get_lock_nowait(int semnum){
    return sem_trywait(SEM[semnum]);
}

int release_lock(int semnum){
    return sem_post(SEM[semnum]);
}