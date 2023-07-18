#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdatomic.h>
#include <sys/time.h>
#include <time.h>

int sockets_ports[128], views[200];
int descriptor, running = 1, count = 0, sniffing = 1;

atomic_flag lock = ATOMIC_FLAG_INIT;
atomic_flag lock2 = ATOMIC_FLAG_INIT;
atomic_flag lock3 = ATOMIC_FLAG_INIT;

typedef struct node {
        int val;
        struct node * next;
} node;

node * create(int id){
    node * head = NULL;
    head = calloc(1, sizeof(node));
    head->val = id;
    head->next = NULL;
    return head;
}

void insert(node ** head, int id){
    if (*head == NULL) {
        node * init = NULL;
        init = calloc(1, sizeof(node));
        init->val = id;
        init->next = NULL;
        *head = init;
    }
    else{
        node * now = *head;
        while (now->next != NULL)
        {
            now = now->next;
        }
        now->next = calloc(1, sizeof(node));
        now->next->val = id;
    }
}

int rem(node ** head){
    node * substitute = NULL;
    int ret;
    if (*head == NULL) {
        return -1;
    }
    substitute = (*head)->next;
    ret = (*head)->val;
    free(*head);
    *head = substitute;
    return ret;
}

void write_log(int message, int id){
    char time_str[26]; 
    struct timeval tv;
    struct tm *timeinfo;
    gettimeofday(&tv, NULL);
    timeinfo = localtime(&tv.tv_sec);
    strftime(time_str, sizeof(time_str), "%H:%M:%S", timeinfo);
    char milliseconds[5];
    sprintf(milliseconds, ":%03ld", tv.tv_usec / 1000);
    strcat(time_str, milliseconds);
    FILE *file = fopen("log.txt", "a");
    if(message == 1){
        fprintf(file, "%s - %i - [R] Request \n", time_str, id);
    }
     else if (message == 2){
        fprintf(file, "%s - %i - [S] Grant \n", time_str, id);
    }
    else if (message == 3){
        fprintf(file, "%s - %i - [R] Release \n", time_str, id);
    }
    fclose(file);
}

void *socket_listening(node ** queue){
    int idn;
    struct sockaddr_in client_addr;
    int client_struct_length = sizeof(client_addr);
    char ids[10], process_message[10];
    memset(process_message, '\0', sizeof(process_message));
    memset(ids, '\0', sizeof(ids));
    while (running)
    {   
    struct timeval timeout;
    timeout.tv_sec = 5; 
    timeout.tv_usec = 0;
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(descriptor, &readfds);
    int ready = select(descriptor + 1, &readfds, NULL, NULL, &timeout);

    if (ready > 0)
    {
        recvfrom(descriptor, process_message, sizeof(process_message), 0, (struct sockaddr*)&client_addr, &client_struct_length);
        int k = 0;
        for (int i = 2; process_message[i] != '|'; i++){
            ids[k] = process_message[i];
            k+=1;
        }
        idn = atoi(ids);
        memset(ids, '\0', sizeof(ids));
        if (process_message[0] == '1'){
            while (atomic_flag_test_and_set(&lock3));
            write_log(1, idn);
            atomic_flag_clear(&lock3);
            if(views[idn] == 0){
                sockets_ports[idn] = ntohs(client_addr.sin_port);
                if(idn > count){
                    count = idn;
                }
            }
            views[idn] += 1;
            while (atomic_flag_test_and_set(&lock2));
            insert(queue, idn);
            atomic_flag_clear(&lock2);

        }
        else if (process_message[0] == '3') {
            atomic_flag_clear(&lock);
            while (atomic_flag_test_and_set(&lock3));
            write_log(3, idn);
            atomic_flag_clear(&lock3);

        }
    }
    else
    {
        if (!running)
            break;
    }
    }
    atomic_flag_clear(&lock);
}

void *terminal(node ** queue){
    int op;
    node * current;
    while(running){
        printf("Selecione a opção desejada:\n1)Printa a quantidade de chamadas de cada processo\n2) Imprimir a fila de pedidos atual\n3) Parar o coordenador\n");
        scanf("%i", &op);
        if (op == 1)
        {
            for(int i = 0;i <= count; i++){
                printf("O processo %i foi atendido %i\n", i, views[i]);
            }
        }
        else if(op == 2)
        {
            current = *queue;
             if (current == NULL) {
                printf("Fila vazia\n");
            }
            else{
                while (1)
                {
                    printf("O processo de id %i está na fila\n", current->val);
                    if(current->next == NULL){
                        break;
                    }
                    current = current->next;
                }
            }
            
        }
        else if(op == 3){
            running = 0;
            break;
        }
    }
    printf("Terminal encerrado\n");
}

void *grant_send(node ** queue){
    int id, loop = 1;
    char coordinator_message[2];
    struct sockaddr_in client_addr;
    int client_struct_length = sizeof(client_addr);
    while(running || loop){
            coordinator_message[0] = '2';
            while (atomic_flag_test_and_set(&lock2));
            id = rem(queue);
            atomic_flag_clear(&lock2);
            if (id == -1){
                loop = 0;
            }
            else{
                client_addr.sin_family = AF_INET;
                client_addr.sin_port = htons(sockets_ports[id]);
                client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
                while (atomic_flag_test_and_set(&lock));
                while (atomic_flag_test_and_set(&lock3));
                write_log(2, id);
                atomic_flag_clear(&lock3);
                sendto(descriptor, coordinator_message, strlen(coordinator_message), 0, (struct sockaddr*)&client_addr, client_struct_length);
            }
        }
    printf("Grant encerrado\n");
}

void thread_creation(node ** queue, pthread_t *threads)
{
    pthread_create(&threads[0], NULL, socket_listening, (void *)queue);
    pthread_create(&threads[1], NULL, terminal, (void *)queue);
    pthread_create(&threads[2], NULL, grant_send, (void *)queue);
    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);
    pthread_join(threads[2], NULL);

}

int main(){
    node* queue = NULL;
    struct sockaddr_in server_addr, client_addr;
    descriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);    
    pthread_t threads[2];
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(3008);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if(bind(descriptor, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("Não foi possível realizar o bind na porta.\n");
    }
    thread_creation(&queue, threads);
    printf("Encerrado\n");
}