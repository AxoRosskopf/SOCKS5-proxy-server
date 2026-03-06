#include "utils.h"

int recv_exact(int fd, void* buffer, size_t buffer_size, int flags){
    size_t total_left = buffer_size;
    while(total_left > 0){
        ssize_t recv_len = recv(fd, buffer, total_left,flags);
        if(recv_len < 0){
            if(errno == EINTR || errno == EAGAIN){
                continue;
            } else {
                perror("Error recv()");
                return -1;
            }
        } else if( recv_len == 0){
            return -1;
        } else {
            total_left -= recv_len;
            buffer += recv_len;
        }
    }
    return 0;
}

int recv_string(int fd, char *str){
    uint8_t len;
    if(recv_exact(fd, &len, sizeof(uint8_t),0) != 0){
        return -1;
    }

    if(recv_exact(fd, str, len, 0)){
        return -1;
    }

    str[len] = '\0';
    return len;
}