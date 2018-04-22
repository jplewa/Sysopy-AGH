#include "client_server.h"

int client_qid;
int server_qid;
long id;

void close_msgq(){
    printf("CLOSED QUEUE: %d\n", client_qid);
    if ((msgctl(client_qid, IPC_RMID, NULL)) == -1){
        printf("Error: Failed to close message queue\n");
    }
}

void inform_server(){
    struct msg* message = malloc(sizeof(struct msg));
    message -> mtype = STOP;
    message -> mpid = getpid();
    if (msgsnd(server_qid, (void*) message, (sizeof(message -> mtext) + sizeof(message -> mpid)), 0) == -1){
        printf("Error: failed to notify server\n");
    }
}

void sigint_handler(int signum){
    write(1, "\n\nCaptured SIGINT\n", 18);
    inform_server();
    close_msgq();
    _exit(0);
}

void sigterm_handler(int signum){
    write(1, "\nTerminated by server\n", 22);
    close_msgq();
    _exit(0);
}

void print_error(int error_code){
    if (error_code > 0){
        printf("Error: Incorrect request in line %d\n", error_code);
        exit(0);
    }
    switch(error_code){
        case -1:
            perror("Error");
            printf("Couldn't generate message queue ID\n");
            close_msgq();
            _exit(0);
        case -2:
            perror("Error");
            printf("Couldn't create client's message queue\n");
            _exit(0);
        case -3:
            perror("Error");
            printf("Couldn't set exit function\n");
            _exit(0);
        case -4:
            perror("Error");
            printf("Couldn't set signal handlers\n");
            exit(0);
        case -5:
            perror("Error");
            printf("Couldn't send message to server\n");
            exit(0);
        case -6:
            perror("Error");
            printf("Couldn't receive message from server\n");
            exit(0);
        case -7:
            printf("Error: Incorrect arguments\n");
            printf("Available input options: -c (command line) or -f [filename] (file)\n");
            exit(0);
        case -8:
            printf("Error: Authentication failed\n");
            printf("Client ID doesn't match\n");
            exit(0);
        case -9:
            printf("Error: Server is at its full capacity\n");
            printf("Please try to connect later\n");
            close_msgq();
            _exit(0);
        case -10:
            perror("Error");
            printf("Couldn't open server's message queue\n");
            close_msgq();
            _exit(0);
    }
}

int initialize(){
    printf("<%d>\n", getpid());
    struct sigaction act;
    act.sa_handler = &sigint_handler;
    sigfillset(&act.sa_mask);
    act.sa_flags = 0;
    if ((sigaction(SIGINT, &act, NULL)) == -1) return -4;
    act.sa_handler = &sigterm_handler;
    if ((sigaction(SIGTERM, &act, NULL)) == -1) return -4;
   
    if ((atexit(close_msgq)) != 0) return -3;
    if ((atexit(inform_server)) != 0) return -3;
   
    if ((client_qid = msgget(IPC_PRIVATE, IPC_CREAT | 0777)) == -1){
        return -2;
    }
    printf("CREATED QUEUE: %d\n", client_qid);
    key_t server_qkey;
    const char *home_dir;
    if ((home_dir = getenv("HOME")) == NULL) home_dir = getpwuid(getuid()) -> pw_dir;
    if ((server_qkey = ftok(home_dir, REQUEST_ID)) == -1){
        return -1;
    }
    if ((server_qid = msgget(server_qkey, 0)) == -1){
        return -10;
    }
    struct msg* message = malloc(sizeof(struct msg));
    message -> mtype = NEW;
    message -> mpid = getpid();
    sprintf(message -> mtext,"%d", client_qid);;
    if (msgsnd(server_qid, (void*) message, (sizeof(message -> mtext) + sizeof(message -> mpid)), 0) == -1){
        free(message);
        return -5;
    }
    if (msgrcv(client_qid, (void*) message, (sizeof(message -> mtext) + sizeof(message -> mpid)), 0, 0) == -1){
        free(message);
        return -6;
    }
    id = message -> mtype;
    if (id == MAX_CLIENTS + 1){
        free(message);
        return -9;
    }
    printf("RECEIVED ID: %ld\n\n", id);
    free(message);
    return 0;
}

FILE* parse_arguments(int argc, char* argv[]){
    if (argc == 1) return stdin;
    if (sizeof(argv[1]) >= 2){
        if (argv[1][0] == '-' && argv[1][1] == 'f'){
            FILE* file;
            if ((file = fopen(argv[2], "r")) == NULL) return NULL;
            else return file;
        }
        else if (argv[1][0] == '-' && argv[1][1] == 'c') return stdin;        
    }
    return NULL;
}

int get_mtype(char* buffer){
    if (strncmp(buffer, "MIRROR", 6) == 0) return MIRROR;
    if (strncmp(buffer, "CALC", 4) == 0) return CALC;
    if (strncmp(buffer, "TIME", 4) == 0) return TIME;
    if (strncmp(buffer, "END", 3) == 0) return END;
    return 0;
}

char* get_mtext(char* buffer){
    char* c = buffer;
    while (*c != ' ' && *c != '\n') c++;
    c++;
    return c;
}

int send_requests(FILE* source){
    size_t n = 1024;
    char* buffer = malloc(n);
    int line = 1;
    struct msg* message;
    while (getline(&buffer, &n, source) != -1){        
        message = malloc(sizeof(struct msg));
        int mtype = get_mtype(buffer);
        if (mtype == 0){
            free(buffer);
            free(message);
            fclose(source);
            return line;
        }
        message -> mtype = mtype;
        message -> mpid = getpid();
        strncpy(message -> mtext, get_mtext(buffer), 1024);
        if (msgsnd(server_qid, (void*) message, (sizeof(message -> mtext) + sizeof(message -> mpid)), 0) == -1){
            free(buffer);
            free(message);
            fclose(source);
            return -5;   
        }
        if (mtype == 5){
            free(message);
            free(buffer);
            fclose(source);
            close_msgq();
            _exit(0);
        }
        else{
            if (msgrcv(client_qid, (void*) message, (sizeof(message -> mtext) + sizeof(message -> mpid)), 0, 0) == -1){
                free(message);
                free(buffer);
                fclose(source);
                return -6;
            }
            if (message -> mtype != id){
                free(message);
                free(buffer);
                fclose(source);
                return -8;
            }
            printf("%s\n", message -> mtext);
        }
        free(message);
        line++;
    }
    free(buffer);
    fclose(source);
    return 0;
}

int main (int argc, char* argv[]){
    FILE* source = parse_arguments(argc, argv);
    if (source == NULL) print_error(-7); 
    int result;
    if ((result = initialize()) != 0) print_error(result);
    if ((result = send_requests(source)) != 0) print_error(result);
    return 0;
}
