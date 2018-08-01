#include "../orange/orange.h"
#include "../orange/orange_log.h"

#include "orange_socket.h"
#include "orange_socket_un.h"

#if 0
#define DEBUGP orange_log_info
#else
#define DEBUGP(args, ...)
#endif

#define LOG_PREFIX "SOCKET_UN"

static int __orange_socket_un_connect(int s, struct sockaddr* addr, socklen_t addrlen)
{
	return connect(s, addr, addrlen);
}

static int __orange_socket_un_create(int protocol, struct sockaddr* addr, socklen_t addrlen, int backlog, uint8_t is_server)
{
	int s = -1;
	int ret;

	s = socket(PF_LOCAL, protocol, 0);

	if (s == -1) {
		orange_log(ORANGE_LOG_ERR, "%s: socket create error: %s\n", LOG_PREFIX, strerror(errno));
		goto exit;
	}

	DEBUGP("%s: socket created: %d\n", __func__, s);

	if (addr != NULL) {
		/*  Delete any other file that gets in our way  */
		unlink(((struct sockaddr_un*) addr)->sun_path);

		ret = bind(s, addr, addrlen);
		if (ret != 0) {
			orange_log(ORANGE_LOG_ERR, "%s: bind socket error: %s\n", LOG_PREFIX, strerror(errno));
			close(s);
			s = -1;
			goto exit;
		}
		DEBUGP("%s: socket bind: %d, path: %s\n", __func__, s, ((struct sockaddr_un*) addr)->sun_path);
	}

	orange_socket_set_nonblock(s);

	if (backlog > 0) {
		ret = listen(s, backlog);
		if (ret != 0) {
			orange_log(ORANGE_LOG_ERR, "%s: listen socket error: %s\n", LOG_PREFIX, strerror(errno));
			close(s);
			s = -1;
		}
		DEBUGP("%s: socket listen: %d\n", __func__, s);
	}

exit:
	return s;
}

static int __orange_socket_un_accept(int s, struct sockaddr* addr, socklen_t* addr_len)
{
	int fd;

	fd = accept(s, addr, addr_len);

	return fd;
}

static ssize_t __orange_socket_un_recv(int s, void* buff, size_t len, int flags, struct sockaddr* from, socklen_t* from_len)
{
	return recv(s, buff, len, MSG_DONTWAIT);
}

static ssize_t __orange_socket_un_recvfrom(int s, void* buff, size_t len, int flags, struct sockaddr* from, socklen_t* from_len)
{
	return recvfrom(s, buff, len, flags, from, from_len);
}

static ssize_t __orange_socket_un_send(int s, const void* mesg, size_t len, int flags, struct sockaddr* to, socklen_t to_len)
{
	return send(s, mesg, len, flags);
}

static int __orange_socket_un_close(int s)
{
	return close(s);
}

static int __orange_socket_un_getpeername(int s, struct sockaddr* name, socklen_t* name_len)
{
	*name_len = sizeof(struct sockaddr_un);
	return getpeername(s, name, name_len);
}

static char* __orange_socket_un_addrstring(struct sockaddr* addr, char* buff, int size)
{
	snprintf(buff, size, "%s", (((struct sockaddr_un*) addr)->sun_path));
	return buff;
}

static struct orange_socket_func socket_funcs = {.domain	  = PF_LOCAL,
												 .create	  = __orange_socket_un_create,
												 .connect	 = __orange_socket_un_connect,
												 .accept	  = __orange_socket_un_accept,
												 .recv		  = __orange_socket_un_recv,
												 .recvfrom	= __orange_socket_un_recvfrom,
												 .send		  = __orange_socket_un_send,
												 .close		  = __orange_socket_un_close,
												 .getpeername = __orange_socket_un_getpeername,
												 .addrstring  = __orange_socket_un_addrstring};

int orange_socket_un_init(void)
{
	return orange_socket_func_register(ORANGE_SOCKET_TYPE_UN, &socket_funcs);
}

void orange_socket_un_fini(void)
{
	orange_socket_func_unregister(ORANGE_SOCKET_TYPE_UN);
}
