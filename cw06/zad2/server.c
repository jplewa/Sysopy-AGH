#include "client_server.h"

mqd_t server_qd;
struct client_info clients[MAX_CLIENTS+1];

void close_msgq(){
    for (int i = 1; i < MAX_CLIENTS + 1; i++){
        if (clients[i].pid != -1){ 
            kill(clients[i].pid, SIGTERM);
            mq_close(clients[i].qd);
            printf("%d\t%d\t\t%s\t\tDISCONNECT\n", i, clients[i].pid, clients[i].name);                
            clients[i].pid = -1;
        }
    }
    printf("\nCLOSED QUEUE: %s\n", SERVER_NAME);
    if (mq_unlink(SERVER_NAME) == -1){
        perror("Error");
        printf("Server's queue couldn't be deleted\n");
    }
    if (mq_close(server_qd) == -1){
        perror("Error");
        printf("Server's queue couldn't be closed\n");
    }
}

void sigint_handler(int signum){
    write(1, "\n\nCaptured SIGINT", 17);
    exit(1);
}

void print_error(int error_code){
    switch(error_code){
        case -2:
            perror("Error");
            printf("Couldn't create message queue\n");
            _exit(0);
        case -3:
            perror("Error");
            printf("Couldn't set exit function\n");
            _exit(0);
        case -4:
            perror("Error");
            printf("Couldn't set SIGINT handler\n");
            exit(0);
        case -5:
            printf("Error: client authentication failed\n");
            printf("Server couldn't identify client\n");
            exit(0);
        case -6:
            perror("Error");
            printf("Couldn't open client's queue\n");
            exit(0);
        case -7:
            perror("Error");
            printf("Couldn't send message to client\n");
            exit(0);
        case -8:
            perror("Error");
            printf("Couldn't close client's queue\n");
            exit(0);
    }
}
int initialize(){
    struct sigaction act;
    act.sa_handler = &sigint_handler;
    sigfillset(&act.sa_mask);
    act.sa_flags = 0;
    if ((sigaction(SIGINT, &act, NULL)) == -1) return -4;
    
    if ((atexit(close_msgq)) != 0) return -3;
    
    struct mq_attr *attr = malloc(sizeof(struct mq_attr));
    attr -> mq_flags = O_NONBLOCK;
    attr -> mq_msgsize = MTEXTSIZE;
    attr -> mq_maxmsg = 10;
    attr -> mq_curmsgs = 0;  
    if ((server_qd = mq_open(SERVER_NAME, O_RDONLY | O_CREAT | O_NONBLOCK, 0777, attr)) == -1){
        return -2;
    }
    free(attr);
    printf("<%d>\nCREATED QUEUE: %s\n\n", getpid(), SERVER_NAME);
    printf("ID\tPID\t\tQUEUE\t\t\tACTION\n");
    for (int i = 0; i < MAX_CLIENTS + 1; i++) clients[i].pid = -1;
    return 0;
}

int find_client(pid_t pid){
    for (int i = 1; i < MAX_CLIENTS + 1; i++){
        if (clients[i].pid == pid) return i;
    }
    return -1;
}

void parse_message(char* buffer, char* mtype, int* pid){
    strncpy(mtype, buffer, 8);
    for (int i = 0; i < 8; i++) buffer++;
    char* tmp = malloc(9);
    strncpy(tmp, buffer, 8);
    sscanf(tmp, "%d", pid);
}

int new_client(char* request, int pid){
    char* response = calloc(MTEXTSIZE-1,1);
    int i = 1;
    while (i < MAX_CLIENTS + 1 && clients[i].pid != -1) i++;

    char* client_name = calloc(20,1);
    sscanf(request, "%s", client_name);

    if (i != MAX_CLIENTS + 1){
        clients[i].pid = pid;
        clients[i].name = client_name;        
        printf("%d\t%d\t\t%s\t\tCONNECT\n", i, clients[i].pid, client_name);
    }
    else printf("-\t%d\t\t%s\t\tCONNECT - FAILED\n", pid, client_name);
    sprintf(response, "%08d", i);
    mqd_t client_qd;
    if ((client_qd = mq_open(client_name, O_WRONLY)) == -1){
        free(client_name);
        free(response);
        return -6;
    }
    if (mq_send(client_qd, response, strlen(response)+1, 1) == -1){
        free(client_name);
        free(response);
        return -7;
    }
    if (i != MAX_CLIENTS + 1){
        clients[i].qd = client_qd;
        clients[i].name = client_name;
    }
    else if (mq_close(client_qd) == -1){
        free(client_name);
        free(response);
        return -8;
    }
    free(response);
    return 0;
}

int mirror(char* request, int pid){
    char* response = calloc(MTEXTSIZE-1,1);
    int count = 0;
    int index = find_client(pid); 
    while (request[count] != '\0' && request[count] != '\n' && count < MTEXTSIZE-1) count++;
    sprintf(response, "%08d",  index);
    for (int i = 0; i < count; i++) response[8+i] = request[count - i - 1];
    response[8+count] = '\0';
    if (index == -1){
        free(response);
        return -5;
    }
    printf("%d\t%d\t\t%s\t\tMIRROR\n", index, pid, clients[index].name);
    if (mq_send(clients[index].qd, response, strlen(response)+1, 1) != 0){
        free(response);
        return -7;
    }
    free(response);
    return 0;
}

int calc(char* request, int pid){
    char* response = calloc(MTEXTSIZE-1,1);
    int n = 0;
    int m = 0;
    char op = '\0';
    int i = 0;
    while (request[i] != '\n' && request[i] != '\0' && i < MTEXTSIZE-1){
        if (op == '\0' && request[i] >= 48 && request[i] <= 57){
            n *= 10;
            n += request[i];
            n -= 48;
        }
        else if (op == '\0' && (request[i] == '+' || request[i] == '-' || request[i] == '*' || request[i] == '/')){
            op = request[i];
        }
        else if(request[i] >= 48 && request[i] <= 57){
            m *= 10;
            m += request[i];
            m -= 48;
        }
        else if (request[i] != 32){
            op = '\0';
            i = MTEXTSIZE;
        }
        i++;
    }
    int index = find_client(pid);
    if (m == 0 && op == '/') sprintf(response, "%08d%s",  index, "Don't divide by zero!");
    else if (op == '\0') sprintf(response, "%08d%s",  index, "Incorrect expression!");
    else{
        int result;
        switch (op){
            case '-':
                result = n - abs(m);
                break;
            case '+':
                result = n + m;
                break;
            case '/':
                result = n / m;
                break;
            case '*':
                result = n * m;
                break;
        }
        sprintf(response, "%08d%d", index, result);
    }
    if (index == -1){
        free(response);
        return -5;
    }
    printf("%d\t%d\t\t%s\t\tCALC\n", index, pid, clients[index].name);
    if (mq_send(clients[index].qd, response, strlen(response)+1, 1) == -1){
        free(response);
        return -7;
    }
    free(response);
    return 0;
}
int time(char* request, int pid){
    char* response = calloc(MTEXTSIZE-1,1);
    FILE* date = popen("date", "r");
    char* line = NULL;
    size_t len = 0;
    int n = getline(&line, &len, date) ;
    pclose(date);
    line[n-1] = '\0';
    int index = find_client(pid);
    if (index == -1){
        free(response);
        return -5;
    }
    sprintf(response, "%08d%s", index,line);
    
    printf("%d\t%d\t\t%s\t\tTIME\n", index, pid, clients[index].name);
    if (mq_send(clients[index].qd, response, strlen(response)+1, 1) == -1){
        free(response);
        return -7;
    }
    free(response);
    return 0;
}

int receive_messages(){
    int end_received = 0;
    int index;
    int result = 0;
    while(1){
        fflush(stdout);
        char* request = calloc(MTEXTSIZE+1,1);
        if ((mq_receive(server_qd, request, MTEXTSIZE+1, NULL)) > 0){
            char* mtype = malloc(9);
            int pid;
            parse_message(request, mtype, &pid);
            if (strncmp(mtype, "10000000", 8) == 0){
                result = new_client(request+16, pid);
            }
            else if (strncmp(mtype, "01000000", 8) == 0){
                result = mirror(request+16, pid);
            }
            else if (strncmp(mtype, "00100000", 8) == 0){
                result = calc(request+16, pid);
            }
            else if (strncmp(mtype, "00010000", 8) == 0){
                result = time(request+16, pid);
            }
            else if (strncmp(mtype, "00001000", 8) == 0){
                end_received = 1;
                index = find_client(pid);
                if (index == -1) return -5;
                printf("%d\t%d\t\t%s\t\tEND\n", index, pid, clients[index].name);
                if (mq_close(clients[index].qd) == -1) return -8;
                printf("%d\t%d\t\t%s\t\tDISCONNECT\n", index, pid, clients[index].name);                
                clients[index].pid = -1;
            }
            else if (strncmp(mtype, "00000100", 8) == 0){
                index = find_client(pid);
                if (index == -1) return -5;
                if (mq_close(clients[index].qd) == -1) return -8;
                printf("%d\t%d\t\t%s\t\tDISCONNECT\n", index, pid, clients[index].name);                
                clients[index].pid = -1;
            }
        }
        free(request);
        if (end_received) exit(0);
        if (result != 0) return result;
        sleep(1);
    }
    return 0;
}

int main (int argc, char* argv[]){
    int result;
    if ((result = initialize()) != 0) print_error(result);
    if ((result = receive_messages()) != 0) print_error(result);
    return 0;
}
