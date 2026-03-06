#include "handlers.h"

void send_ip_reply(int fd, uint8_t reply_type, in_addr_t ip, in_port_t port){
    uint8_t buffer[sizeof(socks_reply_t) + sizeof(in_addr_t) + sizeof(in_port_t)];
    uint8_t *ptr = buffer;
    *(socks_reply_t *) ptr = (socks_reply_t){
        .ver = PROTOCOL_VERSION_SOCKS_5,
        .rep = reply_type,
        .rsv = 0,
        .atyp = ATYP_IP_V4_ADDR
    };
    ptr += sizeof(socks_reply_t);
    *(in_addr_t *) ptr = ip;
    ptr += sizeof(in_addr_t);
    *(in_port_t *)ptr = port;
    send(fd, buffer, sizeof(buffer), 0);
}

void send_domain_reply(int fd, uint8_t reply_type, const char *domain, uint8_t domain_len, in_port_t port){
    uint8_t buffer[sizeof(socks_reply_t) + 1 + 255 + sizeof(in_port_t)]; // hard coding len, addr_size
    uint8_t *ptr = buffer;
    *(socks_reply_t *) ptr = (socks_reply_t){
        .ver = PROTOCOL_VERSION_SOCKS_5,
        .rep = reply_type,
        .rsv = 0,
        .atyp = ATYP_DOMAINNAME
    };
    ptr += sizeof(socks_reply_t);
    *ptr++ = domain_len;
    memcpy(ptr, domain, domain_len);
    ptr += domain_len;
    *(in_port_t *) ptr = port;
    ptr += sizeof(in_port_t);

    send(fd, buffer, ptr - buffer, 0);
}

void send_method_selection(int fd, uint8_t method){
    method_selection_msg_t method_selection = {
        .ver = PROTOCOL_VERSION_SOCKS_5,
        .method = method
    };
    send(fd, &method_selection, sizeof(method_selection_msg_t),0);
}

int handler_greeting(int client_fd){
    identidier_selection_msg_t client_selection;
    if(recv_exact(client_fd, &client_selection,sizeof(identidier_selection_msg_t),0) != 0){
        return -1;
    }
    if(client_selection.ver != PROTOCOL_VERSION_SOCKS_5){
        fprintf(stderr,"Unsupported socks version, v%#02x\n",client_selection.ver);
    }

    uint8_t methods[UINT8_MAX];
    if(recv_exact(client_fd,methods,client_selection.nmethods,0) != 0){
        return -1;
    }

    int found = 0;
    for(int i = 0; i <(int)client_selection.nmethods;i++){
        if(methods[i] == METHOD_VALUE_NO_AUTH_REQUIRED){
            found = 1;
            break;
        }
    }

    if(!found){
        fprintf(stderr, "No acceptable method from client\n");
        send_method_selection(client_fd,METHOD_VALUE_NO_ACCEPTABLE);
        return -1;
    }

    send_method_selection(client_fd, METHOD_VALUE_NO_AUTH_REQUIRED);
    return 0;
}

int handle_request(int fd){
    socks_request_t request;
    if(recv_exact(fd,&request,sizeof(socks_request_t),0)< 0 ){
        return -1;
    }

    if(request.ver != PROTOCOL_VERSION_SOCKS_5){
        fprintf(stderr,"Unsupported socks version %#02x\n",request.ver);
        return -1;
    }

    if(request.cmd != REQUEST_CMD_CONNECT){
        fprintf(stderr,"Unsupported socks command %#02x\n",request.ver);
        return -1;
    }

    int remote_fd = -1;
    if(request.atyp == ATYP_IP_V4_ADDR){
        in_addr_t ip;
        if(recv_exact(fd,&ip,sizeof(in_addr_t),0) < 0){
            return -1;
        }

        in_port_t port;
        if(recv_exact(fd, &port, sizeof(port), 0) < 0){
            return -1;
        }

        struct sockaddr_in remote_addr;
        remote_addr.sin_family = AF_INET;
        remote_addr.sin_addr.s_addr = ip;
        remote_addr.sin_port = port;
        if((remote_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
            perror("Error socket() on remote_fd, handle_request");
            send_ip_reply(fd, REPLY_VALUE_FAILURE, ip, port);
            return -1;
        }

        if((connect(remote_fd,(struct sockaddr *)&remote_addr, sizeof(remote_addr))) < 0){
            perror("Error connect() on remote_fd, handle_request");
            close(remote_fd);
            send_ip_reply(fd, REPLY_VALUE_FAILURE, ip, port);
            return -1;
        }

        printf("Connected to remote address [%s:%d] with fd %d\n", 
            inet_ntoa(remote_addr.sin_addr),
            ntohs(port),
            remote_fd
        );

        send_ip_reply(fd, REPLY_VALUE_SUCCEEDED, ip, port);

    } else if (request.atyp == ATYP_DOMAINNAME){
        char domain[UINT8_MAX + 1];
        int domain_len = recv_string(fd, domain);
        if(domain_len <= 0){
            return -1;
        }

        in_port_t port;
        if(recv_exact(fd, &port, sizeof(in_port_t), 0) != 0){
            return -1;
        }

        char port_s[8]; // hostname
        sprintf(port_s,"%d",ntohs(port));
        struct addrinfo *addr_info;
        if(getaddrinfo(domain, port_s, NULL, &addr_info) != 0){
            perror("Error on handle request, getaddrinfo()");
            send_domain_reply(fd, REPLY_VALUE_FAILURE, domain, domain_len, port);
            return -1;
        }

        for (struct addrinfo *ai = addr_info; ai != NULL; ai = ai->ai_next) {
            if(ai->ai_socktype == SOCK_STREAM){ // only tcp 
                int try_fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
                if (try_fd == -1) { continue; }
                if (connect(try_fd, ai->ai_addr, ai->ai_addrlen) == 0) {
                    remote_fd = try_fd;
                    break;
                } else {
                    close(try_fd);
                }
            }
        }
        freeaddrinfo(addr_info);
        if (remote_fd == -1) {
            fprintf(stderr, "Cannot connect to remote address %s:%d\n", domain, ntohs(port));
            send_domain_reply(fd, REPLY_VALUE_FAILURE, domain, domain_len, port);
            return -1;
        }
        printf("Connected to remote address %s:%d with FD %d\n", domain, ntohs(port), remote_fd);

        send_domain_reply(fd, REPLY_VALUE_SUCCEEDED, domain, domain_len, port);
    }else{
        fprintf(stderr, "Unsupported address type %#02x\n", request.atyp);
        return -1;
    }
    return remote_fd;
}

void start_tunnel(int client_fd, int remote_fd){
    printf("Running tunnel between fd %d and %d \n", client_fd, remote_fd);
    int max_fd = (client_fd > remote_fd) ? client_fd : remote_fd;
    uint8_t buffer[BUFSIZ];
    while(1){
        fd_set rd_set;
        FD_ZERO(&rd_set);
        FD_SET(client_fd, &rd_set);
        FD_SET(remote_fd, &rd_set);

        if(select(max_fd+1, &rd_set, NULL, NULL, NULL) < 0){
            perror("Error select() on start_tunnel\n");
            continue;
        }

        if(FD_ISSET(client_fd, &rd_set)){
            ssize_t lenght = recv(client_fd, buffer, BUFSIZ,0);
            if(lenght <= 0){
                break;
            }
            send(remote_fd, buffer, lenght, 0);
        }

        if(FD_ISSET(remote_fd, &rd_set)){
            ssize_t lenght = recv(remote_fd, buffer, BUFSIZ, 0);
            if(lenght <= 0){
                break;
            }
            send(client_fd, buffer, lenght, 0);
        }

    }
}