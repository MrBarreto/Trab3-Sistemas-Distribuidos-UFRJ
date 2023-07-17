#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>

int descriptor;

int main(int argc, char *argv[]){
    int iter = 1, pos = 0, slp, count_iter;
    char message[10], server_message[2], time_str[20];
    struct tm *timeinfo;
    time_t rawtime;
    slp = atoi(argv[2]);
    count_iter = atoi(argv[3]);
    message[0] = '1';
    message[1] = '|';
    for(int i = 2; i < 10; i++){
        if(iter){
            message[i] = argv[1][pos];
            pos += 1;
            if(argv[1][pos] == '\0'){
                iter = 0;
                message[i+1] = '|';
                i++;
            }
        }
        else{
            message[i] = '0';
            }
    }
    
    struct sockaddr_in addr_server;
    int server_struct_length = sizeof(addr_server);
    descriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    addr_server.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr_server.sin_port = htons(3008);
    addr_server.sin_family = AF_INET;
   
    for(int i = 0; i < count_iter;i++){
        message[0] = '1';
        sendto(descriptor, message, strlen(message), 0, (struct sockaddr*)&addr_server, server_struct_length);
        recvfrom(descriptor, server_message, sizeof(server_message), 0, (struct sockaddr*)&addr_server, &server_struct_length);
        if(server_message[0] == '2'){
            time(&rawtime);
            timeinfo = localtime(&rawtime);
            strftime(time_str, sizeof(time_str), "%H:%M:%S:%03m", timeinfo);
            FILE *file = fopen("processos.txt", "a");
            fprintf(file, "O processo %s escreve nesse arquivo às %s \n", argv[1], time_str);
            fclose(file);
            sleep(slp);
            message[0] = '3';
            sendto(descriptor, message, strlen(message), 0, (struct sockaddr*)&addr_server, server_struct_length);
        }
        else{
            break;
        }
    }
    close(descriptor);
    return 0;
}