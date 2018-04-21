#include "client_server.h"

int server_qid;
struct client_info clients[MAX_CLIENTS+1];

void close_msgq(){
    for (int i = 1; i < MAX_CLIENTS + 1; i++){
        if (clients[i].pid != -1) kill(clients[i].pid, SIGINT);
    }
    printf("\nCLOSED QUEUE: %d\n", server_qid);
    if ((msgctl(server_qid, IPC_RMID, NULL)) == -1){
        printf("Failed to close message queue\n");
    }
}

void sigint_handler(int signum){
    write(1, "\n\nCaptured SIGINT", 17);
    exit(1);
}

void print_error(int error_code){
    switch(error_code){
        case -1:
            perror("Error");
            printf("Couldn't generate message queue ID\n");
            break;
        case -2:
            perror("Error");
            printf("Couldn't create message queue\n");
            break;
        case -3:
            perror("Error");
            printf("Couldn't set exit function\n");
            break;
        case -4:
            perror("Error");
            printf("Couldn't set SIGINT handler\n");
            break;
    }
}
int initialize(){
    if ((atexit(close_msgq)) != 0) return -3;
    struct sigaction act;
    act.sa_handler = &sigint_handler;
    sigfillset(&act.sa_mask);
    act.sa_flags = 0;
    if ((sigaction(SIGINT, &act, NULL)) == -1) return -4;
    const char *home_dir;
    key_t server_qkey;
    if ((home_dir = getenv("HOME")) == NULL) home_dir = getpwuid(getuid()) -> pw_dir;
    if ((server_qkey = ftok(home_dir, REQUEST_ID)) == -1){
        return -1;
    }
    if ((server_qid = msgget(server_qkey, IPC_CREAT | 0777)) == -1){
        return -2;
    }  
    printf("<%d>\nCREATED QUEUE: %d\n\n", getpid(), server_qid);
    printf("ID\tPID\t\tQID\t\tACTION\n");
    for (int i = 0; i < MAX_CLIENTS + 1; i++) clients[i].pid = -1;
    return 0;
}

int find_client(pid_t pid){
    for (int i = 1; i < MAX_CLIENTS + 1; i++){
        if (clients[i].pid == pid) return i;
    }
    return -1;
}

void new_client(struct msg* request, struct msg* response){
    int i = 1;
    while (i < MAX_CLIENTS + 1 && clients[i].pid != -1) i++;
    int qid;
    sscanf(request -> mtext, "%d", &qid);
    if (i != MAX_CLIENTS + 1){
        clients[i].qid = qid;
        clients[i].pid = request -> mpid;
        printf("%d\t%d\t\t%d\t\tCONNECT\n", i, clients[i].pid, clients[i].qid);
    }
    else{
        printf("-\t%d\t\t%d\t\tCONNECT - FAILED\n", request -> mpid, qid);
    }
    response -> mtype = i;
    response -> mtext[0] = '\0';
    if (msgsnd(qid, (void*) response, (sizeof(response -> mtext) + sizeof(response -> mpid)), 0) == -1){
        perror("Error");
    }
}

void mirror(struct msg* request, struct msg* response){
    int count = 0;
    while (request -> mtext[count] != '\0' && request -> mtext[count] != '\n' && count < MTEXTSIZE) count++;
    for (int i = 0; i < count; i++){
        response -> mtext[i] = request -> mtext[count - i - 1];
    }
    response -> mtext[count] = '\0';
    response -> mpid = -1;
    response -> mtype = find_client(request -> mpid);
    printf("%ld\t%d\t\t%d\t\tMIRROR\n", response -> mtype, clients[response -> mtype].pid, clients[response -> mtype].qid);
}

void calc(struct msg* request, struct msg* response){
    int n = 0;
    int m = 0;
    char op = '\0';
    int i = 0;
    while (request -> mtext[i] != '\n' && request -> mtext[i] != '\0' && i < MTEXTSIZE){
        if (op == '\0' && request -> mtext[i] >= 48 && request -> mtext[i] <= 57){
            n *= 10;
            n += request -> mtext[i];
            n -= 48;
        }
        else if (op == '\0' && (request -> mtext[i] == '+' || request -> mtext[i] == '-' || request -> mtext[i] == '*' || request -> mtext[i] == '/')){
            op = request -> mtext[i];
        }
        else if(request -> mtext[i] >= 48 && request -> mtext[i] <= 57){
            m *= 10;
            m += request -> mtext[i];
            m -= 48;
        }
        else if (request -> mtext[i] != 32){
            op = '\0';
            i = MTEXTSIZE;
        }
        i++;
    }
    if (m == 0 && op == '/') sprintf(response -> mtext,"Don't divide by zero!");
    else if (op == '\0') sprintf(response -> mtext,"Incorrect expression!");
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
        sprintf(response -> mtext,"%d", result);
    }
    response -> mpid = -1;
    response -> mtype = find_client(request -> mpid);
    printf("%ld\t%d\t\t%d\t\tCALC\n", response -> mtype, clients[response -> mtype].pid, clients[response -> mtype].qid);
}
void time(struct msg* request, struct msg* response){
    FILE* date = popen("date", "r");
    char* line = NULL;
    size_t len = 0;
    int n = getline(&line, &len, date) ;
    pclose(date);
    line[n-1] = '\0';
    sprintf(response -> mtext,"%s", line);
    response -> mpid = -1;
    response -> mtype = find_client(request -> mpid);
    printf("%ld\t%d\t\t%d\t\tTIME\n", response -> mtype, clients[response -> mtype].pid, clients[response -> mtype].qid);

}

void receive_messages(){
    int end_received = 0;
    int client_id;
    while(1){
        fflush(stdout);
        struct msg* request = malloc(sizeof(struct msg));
        struct msg* response = malloc(sizeof(struct msg));
        if (msgrcv(server_qid, (void*) request, (sizeof(request -> mtext) + sizeof(request -> mpid)), 0, IPC_NOWAIT) > 0){
            switch(request -> mtype){
                case NEW:
                    new_client(request, response);
                    break;
                case MIRROR:
                    mirror(request, response);
                    break;
                case CALC:
                    calc(request, response);
                    break;
                case TIME:
                    time(request, response);
                    break;      
                case END:
                    end_received = 1;
                    printf("%d\t%d\t\t%d\t\tEND\n", client_id, clients[client_id].pid, clients[client_id].qid);
                    break;                                        
                case STOP:
                    if ((client_id = find_client(request -> mpid)) != -1){
                        printf("%d\t%d\t\t%d\t\tDISCONNECT\n", client_id, clients[client_id].pid, clients[client_id].qid);
                        clients[client_id].pid = -1;
                    }
                    break;
            }
            client_id = find_client(request -> mpid);
            if (request -> mtype > 1 && request -> mtype < 5 && client_id != -1){
                if (msgsnd(clients[client_id].qid, (void*) response, (sizeof(response -> mtext) + sizeof(response -> mpid)), 0) == -1){
                    perror("Error");
                }
            }
        }
        free(request);
        free(response);
        if (end_received){
            exit(0);
        }
    }
}

int main (int argc, char* argv[]){
    int result;
    if ((result = initialize()) != 0){
        print_error(result);
        return 0;
    }
    receive_messages();
    return 0;
}
