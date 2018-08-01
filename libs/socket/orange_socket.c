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
