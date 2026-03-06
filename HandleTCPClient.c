#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#define RCV_BUF_SIZE 32

void HandleTCPClient(int clntSock){
    char buffer[RCV_BUF_SIZE];
    ssize_t recvMsgSize;

    if((recvMsgSize = recv(clntSock, buffer, RCV_BUF_SIZE, 0)) < 0){
        perror("recv() failed");
    }

    while (recvMsgSize > 0)
    {
        
        if(send(clntSock, buffer, recvMsgSize, 0 ) != recvMsgSize){
            perror("send() failed");
        }

        if((recvMsgSize = recv(clntSock, buffer,RCV_BUF_SIZE, 0)) < 0){
            perror("recv() failed");
        }
    }
    
    printf("Cliente desconectado. Cerrando socket.\n");
    close(clntSock);
}