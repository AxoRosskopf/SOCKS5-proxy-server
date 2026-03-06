#include "server.h"

void *client_worker(void *args){
    int client_fd = (int)(intptr_t)args;
    if(handler_greeting(client_fd) != 0){
        close(client_fd);
        return NULL;
    }

    int remote_fd = handle_request(client_fd);
    if(remote_fd == -1){
        close(client_fd);
        return NULL;
    }

    start_tunnel(client_fd, remote_fd);

    printf("Closing session for FDs %d and %d\n", client_fd, remote_fd);
    close(remote_fd);
    close(client_fd);

    return NULL;
}

_Noreturn void server_loop(int server_fd){
    int client_fd;
    while(1){
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        if((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len))<0){
            perror("Error accept()\n");
            continue;
        }

        int optval = 1;
        if((setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY,&optval, sizeof(optval))) < 0){
            perror("Error disabling Nagle's algorithm\n");
            continue;
        }

        printf("Connection accepted: from [%s:%d] with fd %d\n", 
            inet_ntoa(client_addr.sin_addr), 
            ntohs(client_addr.sin_port), 
            client_fd
        );

        pthread_t client_tid;
        if(pthread_create(&client_tid, NULL, &client_worker,(void*) (intptr_t)client_fd) == 0){
            pthread_detach(client_tid);
        } else {
            perror("Error pthread_create()");
            close(client_fd);
        }


    }
}