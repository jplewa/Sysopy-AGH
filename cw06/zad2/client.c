#include "client_server.h"

mqd_t client_qd;
char* client_name;
mqd_t server_qd;
long id;
int end = 0;

void close_msgq(){
    write(1, "CLOSED QUEUE: ", 14);
    write(1, client_name, strlen(client_name));
    write(1, "\n", 1);
    mq_unlink(client_name);
    mq_close(client_qd);
    mq_close(server_qd);
}

void inform_server(){
    char* message = calloc(MTEXTSIZE-1,1);
    sprintf(message, "%s%08d", STOP, getpid());
    mq_send(server_qd, message, strlen(message)+1, 1);
    free(message);
}

void sigint_handler(int signum){
    write(1, "\n\nCaptured SIGINT\n", 18);
    inform_server();
    close_msgq();
    _exit(0);
}

void sigterm_handler(int signum){
    write(1, "\nTerminated by server\n", 23);
    close_msgq();
    _exit(0);
}

void print_error(int error_code){
    if (error_code > 0){
        printf("Error: Incorrect request in line %d\n", error_code);
        return;
    }
    switch(error_code){
        case -1:
            perror("Error");
            printf("Couldn't create message queue\n");
            _exit(0);
        case -2:
            perror("Error");
            printf("Couldn't open server's message queue\n");
            close_msgq();
            _exit(0);
        case -3:
            perror("Error");
            printf("Couldn't set exit functions\n");
            break;
        case -4:
            perror("Error");
            printf("Couldn't set signal handlers\n");
            break;
        case -5:
            perror("Error");
            printf("Couldn't send message to server\n");
            break;
        case -6:
            perror("Error");
            printf("Couldn't receive message from server\n");
            break;
        case -7:
            printf("Error: Incorrect arguments\n");
            printf("Available input options: -c (command line) or -f [filename] (file)\n");
            break;
        case -8:
            printf("Error: Authentication failed\n");
            printf("Client ID doesn't match\n");
            break;
        case -9:
            printf("Error: Server is at its full capacity\n");
            printf("Please try to connect later\n");
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

    client_name = calloc(12, 1);
    sprintf(client_name, "/client%d", getpid());
    struct mq_attr *attr = malloc(sizeof(struct mq_attr));
    attr -> mq_flags = 0;
    attr -> mq_msgsize = MTEXTSIZE;
    attr -> mq_maxmsg = 10;
    attr -> mq_curmsgs = 0;  
    if ((client_qd = mq_open(client_name, O_RDONLY | O_CREAT, 0777, attr)) == (mqd_t) -1){
        return -1;
    }
    free(attr);
    if ((server_qd = mq_open(SERVER_NAME, O_WRONLY)) == (mqd_t) -1){
        return -2;
    }
    printf("CREATED QUEUE: %s\n", client_name);

    char* message = calloc(MTEXTSIZE-1,1);
    char* response = calloc(MTEXTSIZE+1,1);
    sprintf(message, "%s%08d%s", NEW, getpid(), client_name);
    if ((mq_send(server_qd, message, strlen(message)+1, 1)) != 0){
        return -5;
    }
    if (mq_receive(client_qd, response, MTEXTSIZE+1, NULL) == -1){
        return -6;
    }
    char* id_string = malloc(9);
    char* pEnd;
    strncpy(id_string, response, 8);
    id = strtol(id_string,&pEnd,10);
    free(message);
    free(response);
    if (id == MAX_CLIENTS + 1) return -9;
    printf("RECEIVED ID: %ld\n\n", id);
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

char* get_mtype(char* buffer){
    if (strncmp(buffer, "MIRROR", 6) == 0) return MIRROR;
    if (strncmp(buffer, "CALC", 4) == 0) return CALC;
    if (strncmp(buffer, "TIME", 4) == 0) return TIME;
    if (strncmp(buffer, "END", 3) == 0) return END;
    return NULL;
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
    char* message;
    char* response;
    while (getline(&buffer, &n, source) != -1){        
        message = malloc(MTEXTSIZE-1);
        response = malloc(MTEXTSIZE+1);
        char* mtype = get_mtype(buffer);
        if (mtype == NULL){
            free(buffer);
            free(message);
            free(response);
            fclose(source);
            return line;
        }
        sprintf(message, "%s%08d%s", mtype, getpid(), get_mtext(buffer));
        if ((mq_send(server_qd, message, strlen(message)+1, 1)) != 0){
            free(buffer);
            free(message);
            free(response);
            fclose(source);
            return -5;
        }
        if (strncmp(mtype, END, 8) == 0){
            free(message);
            free(response);
            free(buffer);
            fclose(source);
            close_msgq();
            _exit(0);
        }
        if (mq_receive(client_qd, response, MTEXTSIZE+1, NULL) == -1){
            free(buffer);
            free(message);
            free(response);
            fclose(source);
            return -6;
        }
        char* id_string = malloc(9);
        char* pEnd;
        strncpy(id_string, response, 8);
        if (strtol(id_string,&pEnd,10) != id) return -8;
        printf("%s\n", response+8);
        free(message);
        free(response);
        free(id_string);
        line++;
    }
    free(buffer);
    fclose(source);
    return 0;
}

int main (int argc, char* argv[]){
    FILE* source = parse_arguments(argc, argv);
    if (source == NULL){
        print_error(-7);
        return 0;
    } 
    int result;
    if ((result = initialize()) != 0){
        print_error(result);
        return 0;
    }
    result = send_requests(source);
    if (result != 0) print_error(result);
    return 0;
}
