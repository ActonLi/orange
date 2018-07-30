#pragma once

#include "../orange/orange.h"
#include "../orange/orange_module.h"

ORANGE_VERSION_TYPE(orange_socket);
ORANGE_DEFINE_MODULE_EXTENSION(orange_socket);

#define ORANGE_SOCKET_TYPE_NONE 0x00
#define ORANGE_SOCKET_TYPE_UN   0x01
#define ORANGE_SOCKET_TYPE_UDP  0x02
#define ORANGE_SOCKET_TYPE_TCP  0x03
#define ORANGE_SOCKET_TYPE_MAX  0x03

typedef struct orange_socket_address {
    int addess_size;

    union {
        struct sockaddr_un un;
        struct sockaddr_in in;
    } address;
} orange_socket_address_t;

typedef struct orange_socket_config {
    uint8_t socket_type;
    uint8_t is_server;

    int     protocol;
    int     backlog;

    struct orange_socket_address local_address;
    struct orange_socket_address peer_address;

} orange_socket_config_t; 

typedef struct orange_socket {
    int socket_fd;
    struct orange_socket_config config;
} orange_socket_t;


