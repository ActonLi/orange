#include "orange_socket.h"
#include "../orange/orange_log.h"
#include "orange_socket_version.h"

ORANGE_VERSION_GENERATE(orange_socket, 1, 1, 1, ORANGE_VERSION_TYPE_ALPHA);

struct orange_socket_func socket_funcs[ORANGE_SOCKET_TYPE_MAX + 1];

struct orange_socket* orange_socket_create(struct orange_socket_config* config)
{
	struct orange_socket* socket = NULL;

	if (NULL == config || config->socket_type == ORANGE_SOCKET_TYPE_NONE || config->socket_type > ORANGE_SOCKET_TYPE_MAX) {
		goto exit;
	}

	socket = malloc(sizeof(struct orange_socket));
	if (NULL == socket) {
		goto exit;
	}

	socket->socket_fd = socket_funcs[config->socket_type].create(config->protocol, (struct sockaddr*) &(config->local_address.address),
																 config->local_address.address_size, config->backlog, config->is_server);
	if (socket->socket_fd <= 0) {
		free(socket);
		socket = NULL;
		goto exit;
	}

	memcpy(&(socket->config), config, sizeof(struct orange_socket_config));
exit:
	return socket;
}

int orange_socket_connect(struct orange_socket* socket)
{
	int ret = EINVAL;

	if (socket->config.is_server == 0 && socket->config.peer_address.address_size > 0 && socket_funcs[socket->config.socket_type].connect != NULL) {
		ret = socket_funcs[socket->config.socket_type].connect(socket->socket_fd, (struct sockaddr*) &socket->config.peer_address.address,
															   socket->config.peer_address.address_size);
	}

	return ret;
}

void orange_socket_destroy(struct orange_socket* socket)
{
	if (socket) {
		orange_socket_close(socket);
		free(socket);
		socket = NULL;
	}

	return;
}

struct orange_socket* orange_socket_accept(struct orange_socket* socket)
{
	int					  s;
	struct orange_socket* accept_socket = NULL;

	if (socket == NULL || socket->config.is_server == 0 || socket->config.socket_type == ORANGE_SOCKET_TYPE_NONE ||
		socket->config.socket_type > ORANGE_SOCKET_TYPE_MAX) {
		goto exit;
	}

	accept_socket = malloc(sizeof(struct orange_socket));
	if (accept_socket == NULL) {
		goto exit;
	}

	s = socket_funcs[socket->config.socket_type].accept(socket->socket_fd, (struct sockaddr*) &accept_socket->config.local_address.address,
														(socklen_t*) &accept_socket->config.local_address.address);

	if (s <= 0) {
		free(accept_socket);
		accept_socket = NULL;
	} else {
		accept_socket->socket_fd = s;
		memcpy(&accept_socket->config, &socket->config, sizeof(struct orange_socket_config));
		accept_socket->config.is_server = 0;
	}

exit:
	return accept_socket;
}

ssize_t orange_socket_receive(struct orange_socket* socket, void* buf, size_t len, int flags, struct sockaddr* from, socklen_t* from_len)
{
	ssize_t size = -1;

	if (socket == NULL || socket->config.socket_type == ORANGE_SOCKET_TYPE_NONE || socket->config.socket_type > ORANGE_SOCKET_TYPE_MAX) {
		goto exit;
	}

	size = socket_funcs[socket->config.socket_type].recv(socket->socket_fd, buf, len, flags, from, from_len);

exit:
	return size;
}

ssize_t orange_socket_receive_from(struct orange_socket* socket, void* buf, size_t len, int flags, struct sockaddr* from, socklen_t* from_len)
{
	ssize_t size = -1;

	if (socket == NULL || socket->config.socket_type == ORANGE_SOCKET_TYPE_NONE || socket->config.socket_type > ORANGE_SOCKET_TYPE_MAX) {
		goto exit;
	}

	if (socket_funcs[socket->config.socket_type].recvfrom != NULL) {
		size = socket_funcs[socket->config.socket_type].recvfrom(socket->socket_fd, buf, len, flags, from, from_len);
	}

exit:
	return size;
}

ssize_t orange_socket_send(struct orange_socket* socket, const void* msg, size_t len, int flags, struct sockaddr* to, socklen_t to_len)
{
	ssize_t size = -1;

	if (socket == NULL || socket->config.socket_type == ORANGE_SOCKET_TYPE_NONE || socket->config.socket_type > ORANGE_SOCKET_TYPE_MAX) {
		goto exit;
	}

	size = socket_funcs[socket->config.socket_type].send(socket->socket_fd, msg, len, flags, to, to_len);
exit:
	return size;
}

int orange_socket_close(struct orange_socket* socket)
{
	int ret = EINVAL;

	if (socket == NULL || socket->socket_fd <= 0 || socket->config.socket_type == ORANGE_SOCKET_TYPE_NONE ||
		socket->config.socket_type > ORANGE_SOCKET_TYPE_MAX) {
		goto exit;
	}

	ret				  = socket_funcs[socket->config.socket_type].close(socket->socket_fd);
	socket->socket_fd = -1;
exit:
	return ret;
}

char* orange_socket_addrstring(struct orange_socket* socket, struct sockaddr* addr, char* buff, int size)
{
	char* addrstring = NULL;

	if (socket == NULL || addr == NULL || socket->config.socket_type == ORANGE_SOCKET_TYPE_NONE || socket->config.socket_type > ORANGE_SOCKET_TYPE_MAX) {
		goto exit;
	}

	addrstring = socket_funcs[socket->config.socket_type].addrstring(addr, buff, size);
exit:
	return addrstring;
}

int orange_socket_getpeername(struct orange_socket* socket, struct sockaddr* name, socklen_t* name_len)
{
	int ret = EINVAL;

	if (socket == NULL || name == NULL || name_len == NULL || socket->config.socket_type == ORANGE_SOCKET_TYPE_NONE ||
		socket->config.socket_type > ORANGE_SOCKET_TYPE_MAX) {
		goto exit;
	}

	ret = socket_funcs[socket->config.socket_type].getpeername(socket->socket_fd, name, name_len);
exit:
	return ret;
}

int orange_socket_set_nonblock(int fd)
{
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);

	return 0;
}

int orange_socket_func_register(uint8_t type, struct orange_socket_func* func)
{
	if (type == ORANGE_SOCKET_TYPE_NONE || type > ORANGE_SOCKET_TYPE_MAX || NULL == func) {
		return -1;
	}

	memcpy(&(socket_funcs[type]), func, sizeof(struct orange_socket_func));
	return 0;
}

void orange_socket_func_unregister(uint8_t type)
{
	memset(&(socket_funcs[type]), 0, sizeof(struct orange_socket_func));
	return;
}

static int __orange_socket_module_init(void)
{

	snprintf(orange_socket_description, 127, "Orange Socket Module " ORANGE_VERSION_FORMAT "-%s #%u: %s", ORANGE_VERSION_QUAD(orange_socket_version),
			 orange_version_type(orange_socket_version_type), orange_socket_build_num, orange_socket_build_date);

	orange_log(ORANGE_LOG_INFO, "%s\n", orange_socket_description);

	return 0;
}

static void __orange_socket_module_fini(void)
{
	return;
}

static int orange_socket_modevent(orange_module_t mod, int type, void* data)
{
	int ret = 0;

	switch (type) {
		case ORANGE_MOD_LOAD:
			ret = __orange_socket_module_init();
			break;
		case ORANGE_MOD_UNLOAD:
			__orange_socket_module_fini();
			break;
		default:
			return (EOPNOTSUPP);
	}
	return ret;
}

static orange_moduledata_t orange_socket_mod = {"orange_socket", orange_socket_modevent, 0};

ORANGE_DECLARE_MODULE(orange_socket, orange_socket_mod, ORANGE_SI_SUB_PSEUDO, ORANGE_SI_ORDER_ANY);

ORANGE_MODULE_VERSION(orange_socket, 1);
ORANGE_DECLARE_MODULE_EXTENSION(orange_socket);
