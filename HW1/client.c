#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h> 
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>

#define MAXLINE 1024

int main(int argc, char* argv[]){

    int socketFd, uSocketFd, uRecvSocketFd;
    struct sockaddr_in serverAddress, fromAddress, clientAddress;
    char msg[MAXLINE];
    char cpyMsg[MAXLINE];
    char delim[] = " \n";
    char* ptr;
    int rMsgLength;
    socklen_t fromAddrSize = sizeof(serverAddress);
    
    if(argc != 3){
        printf("Usage : %s [IP_address] [Port number]\n", argv[0]);
        exit(1);
    }

    if(atoi(argv[2]) == -1){
        printf("Invalid port number !\n");
        exit(1);
    }

    // Set client address
    /*bzero(&clientAddress, sizeof(clientAddress));
    clientAddress.sin_family = AF_INET;
    clientAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    clientAddress.sin_port = htons(atoi("8888"));*/

    // TCP socket creation
    socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if(socketFd == -1){
        printf("Failed to create TCP socket\n");
        exit(1);
    }else{
        //printf("TCP socket : %d\n", socketFd);
    }

    // UDP socket creation
    uSocketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if(socketFd == -1){
        printf("Failed to create UDP socket\n");
        exit(1);
    }else{
        //printf("UDP socket : %d\n", uSocketFd);
    }

    /*// UDP receiving socket creation
    uRecvSocketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if(socketFd == -1){
        printf("Failed to create UDP receiving socket\n");
        exit(1);
    }else{
        //printf("UDP socket : %d\n", uSocketFd);
    }

    // UDP socket binding
    if(bind(uRecvSocketFd, (struct sockaddr*)&clientAddress, sizeof(clientAddress)) != 0){
        printf("Failed to bind\n");
        exit(1);
    }else{
        printf("bound\n");
    }*/


    // TCP connect
    bzero(&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(argv[1]);
    serverAddress.sin_port = htons(atoi(argv[2]));

    bzero(msg, sizeof(msg));
    if(connect(socketFd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) != 0){
        printf("Failed to connect\n");
        exit(1);
    }else{
        //printf("Connected\n");
        read(socketFd, msg, sizeof(msg));
        fputs(msg, stdout);
    }

    while(1){
        
        // Read and Print prompt (TCP)
        //bzero(msg, sizeof(msg));
        //read(socketFd, msg, sizeof(msg));
        //bzero(msg, sizeof(msg));
        //strcpy(msg, "%\0");
        //fputs(msg, stdout);

        // Enter and send user command (TCP or UDP)
        fgets(msg, sizeof(msg), stdin);
        strcpy(cpyMsg, msg);
        ptr = strtok(cpyMsg, delim);

        if(!strcmp(msg, "\n")){
            continue;
        }

        if(!strcmp(ptr, "register") || !strcmp(ptr, "game-rule")){
            
            // Do UDP routine
            //printf("UDP rouine entered ...\n");
            sendto(uSocketFd, msg, strlen(msg)+1, 0, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
            rMsgLength = recvfrom(uSocketFd, msg, sizeof(msg), 0, (struct sockaddr*)&serverAddress, &fromAddrSize);
            //printf("%d\n", rMsgLength);
            if(rMsgLength != 0){
                //printf("rMsglength: %d\n", rMsgLength);
                msg[rMsgLength] = 0;
                fputs(msg, stdout);
            }else{
                printf("Nothing received!\n");
            }

        }else if(!strcmp(ptr, "login") || !strcmp(ptr, "logout") || !strcmp(ptr, "start-game") || !strcmp(ptr, "exit")){

            // Do TCP routine
            write(socketFd, msg, sizeof(msg));
            if(strcmp(msg, "exit\n") != 0){
                read(socketFd, msg, sizeof(msg));
                fputs(msg, stdout);
            }   

        }else{

            // Do TCP routine (Default)
            write(socketFd, msg, sizeof(msg));
            if(strcmp(msg, "exit\n") != 0){
                read(socketFd, msg, sizeof(msg));
                fputs(msg, stdout);
            }   
        }


        if(!strcmp(msg, "exit\n")){
            //printf("bye...\n");
            break;
        }
    }

    // Close sockets
    close(socketFd);
    //printf("TCP socket closed ...\n");
    close(uSocketFd);
    //printf("UDP socket closed ...\n");
}
