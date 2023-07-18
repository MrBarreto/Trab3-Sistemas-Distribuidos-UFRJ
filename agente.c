#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

int descriptor;
void write_log(char * string){
    char time_str[26];
    struct timeval tv;
    struct tm *timeinfo;
    gettimeofday(&tv, NULL);
    timeinfo = localtime(&tv.tv_sec);
    strftime(time_str, sizeof(time_str), "%H:%M:%S", timeinfo);
    char milliseconds[5];
    sprintf(milliseconds, ":%03ld", tv.tv_usec / 1000);
    strcat(time_str, milliseconds);
    FILE *file = fopen("processos.txt", "a");
    fprintf(file, "O processo %s escreve nesse arquivo às %s \n",string, time_str);
    fclose(file);
}

int main(int argc, char *argv[]){
    int iter = 1, pos = 0, slp, count_iter;
    char message[10], server_message[2];
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
            write_log(argv[1]);
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