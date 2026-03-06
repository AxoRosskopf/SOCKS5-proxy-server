#pragma once
#include "socks.h"
#include <stdio.h>
#include "utils.h"
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <string.h>      
#include <unistd.h>      
#include <sys/select.h>  
#include <sys/socket.h>  

void send_ip_reply(int fd, uint8_t reply_type ,in_addr_t ip, in_port_t port);
void send_method_selection(int fd, uint8_t method);
int handler_greeting(int fd);
int handle_request(int fd);
void start_tunnel(int client_fd, int remote_fd);