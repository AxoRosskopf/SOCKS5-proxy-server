#pragma once
#include <stddef.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>

#include <stdio.h>       
#include <sys/socket.h>  

int recv_exact(int fd, void* buffer, size_t buffer_size, int flags);
int recv_string(int fd, char *str);
