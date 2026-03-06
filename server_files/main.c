#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "./server/server.h"
#include "./server/socks.h"

int main(int argc, char *argv[]){
    uint16_t bind_port = 1080;
    int server_fd;

    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Error socket()");
        exit(EXIT_FAILURE);
    }

    int optval = 1;
    if(setsockopt(server_fd,SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))<0){
        perror("Error setsockopt()");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in bind_addr;
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind_addr.sin_port = htons(bind_port);
    if((bind(server_fd, (struct sockaddr*)&bind_addr,sizeof(bind_addr))) < 0){
        perror("Error bind()");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if((listen(server_fd, SOMAXCONN))< 0){
        perror("Error listen()");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on [%s:%d]\n",inet_ntoa(bind_addr.sin_addr),bind_port);

    server_loop(server_fd);

    return EXIT_SUCCESS;
}