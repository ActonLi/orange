#pragma once

#include "../orange/orange.h"
#include "../orange/orange_module.h"

ORANGE_VERSION_TYPE(orange_socket);
ORANGE_DEFINE_MODULE_EXTENSION(orange_socket);

#define ORANGE_SOCKET_TYPE_NONE 0x00
#define ORANGE_SOCKET_TYPE_UN 0x01
#define ORANGE_SOCKET_TYPE_UDP 0x02
#define ORANGE_SOCKET_TYPE_TCP 0x03
#define ORANGE_SOCKET_TYPE_MAX 0x03

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

	int protocol;
	int backlog;

	struct orange_socket_address local_address;
	struct orange_socket_address peer_address;

} orange_socket_config_t;

typedef struct orange_socket {
	int							socket_fd;
	struct orange_socket_config config;
} orange_socket_t;

typedef struct orange_socket_func {
	int domain;

	int (*create)(int protocol, struct sockaddr* addr, socklen_t addrlen, int backlog, uint8_t is_server);
	int (*connect)(int socket, struct sockaddr* addr, socklen_t addrlen);
	int (*accept)(int socket, struct sockaddr* addr, socklen_t* addr_len);
	int (*close)(int socket);

	ssize_t (*recv)(int socket, void* buff, size_t len, int falgs, struct sockaddr* from, socklen_t* from_len);
	ssize_t (*recvfrom)(int socket, void* buff, size_t len, int falgs, struct sockaddr* from, socklen_t* from_len);
	ssize_t (*send)(int socket, const void* mesg, size_t len, int flags, struct sockaddr* to, socklen_t to_len);

	char* (*addrstring)(struct sockaddr* addr, char* buff, int size);
	int (*getpeername)(int socket, struct sockaddr* name, socklen_t* namelen);
} orange_socket_func_t;

struct orange_socket* orange_socket_create(struct orange_socket_config* config);
int orange_socket_func_register(uint8_t type, struct orange_socket_func* func);
void orange_socket_func_unregister(uint8_t type);
