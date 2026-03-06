#pragma once
#include "socks.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <pthread.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "handlers.h"

void *client_worker(void *args);
_Noreturn void server_loop(int server_fd);

