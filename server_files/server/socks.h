#pragma once

#include <stdint.h>

#define PROTOCOL_VERSION_SOCKS_5            0x05

#define METHOD_VALUE_NO_AUTH_REQUIRED       0x00
#define METHOD_VALUE_GSSAPI                 0x01
#define METHOD_VALUE_USERNAME_PASSWORD      0x02
#define METHOD_VALUE_IANA_ASSIGNED_MIN      0x03
#define METHOD_VALUE_IANA_ASSIGNED_MAX      0x7F
#define METHOD_VALUE_RESERVE_PRIVATE_MIN    0x80
#define METHOD_VALUE_RESERVE_PRIVATE_MAX    0xFE
#define METHOD_VALUE_NO_ACCEPTABLE          0xFF

#define REQUEST_CMD_CONNECT                 0x01
#define REQUEST_CMD_BIND                    0x02
#define REQUEST_CMD_UDP_ASSOCIATE           0x03

#define ATYP_IP_V4_ADDR                     0x01
#define ATYP_DOMAINNAME                     0x03
#define AYTP_IP_V6_ADDR                     0x04

#define REPLY_VALUE_SUCCEEDED               0x00
#define REPLY_VALUE_FAILURE                 0x01
#define REPLY_VALUE_NO_ALLOWED              0x02
#define REPLY_VALUE_NET_UNREACHABLE         0x03
#define REPLY_VALUE_HOST_UNREACHABLE        0x04
#define REPLY_VALUE_CONNECTION_REFUSED      0x05
#define REPLY_VALUE_TTL_EXPIRED             0x06
#define REPLY_VALUE_CMD_NOT_SUPPORTED       0x07
#define REPLY_VALUE_ADDR_TYPE_NO_SUPPORTED  0x08
#define REPLY_VALUE_UNASSIGNED_MIN          0x09
#define REPLY_VALUE_UNASSIGNED_MAX          0xFF


typedef struct __attribute__((packed)){
    uint8_t ver;
    uint8_t nmethods;
} identidier_selection_msg_t;

typedef struct __attribute__((packed)){
    uint8_t ver;
    uint8_t method;
} method_selection_msg_t;

typedef struct __attribute__((packed)){
    uint8_t ver;
    uint8_t cmd;
    uint8_t rsv; // RESERVE SPACE (0x00)
    uint8_t atyp;
} socks_request_t;

typedef uint16_t socks_request_dst_port_t; 

typedef struct __attribute__((packed)){
    uint8_t ver;
    uint8_t rep;
    uint8_t rsv; // RESERVE SPACE (0x00)
    uint8_t atyp;
} socks_reply_t;

typedef uint16_t socks_reply_bnd_port_t;





