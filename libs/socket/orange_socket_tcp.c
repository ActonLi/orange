#include "../orange/orange.h"
#include "../orange/orange_log.h"

#include "orange_socket.h"
#include "orange_socket_tcp.h"

#if 0
#define DEBUGP orange_log_error
#else
#define DEBUGP(args, ...)
#endif

#define LOG_PREFIX "SOCKET_TCP"

static int __orange_socket_tcp_connect(int s, struct sockaddr* addr, socklen_t addrlen)
{
	return connect(s, addr, addrlen);
}

static int __orange_socket_tcp_create(int protocol, struct sockaddr* addr, socklen_t addrlen, int backlog, uint8_t is_server)
{
	int s = -1;
	int ret;
	int set = 1;

	s = socket(PF_INET, SOCK_STREAM, 0);
	if (s == -1) {
		orange_log(ORANGE_LOG_ERR, "%s: socket create error: %s\n", LOG_PREFIX, strerror(errno));
		goto exit;
	}

	int yes = 1;
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const void*) &yes, sizeof(int)) == -1) {
		goto exit;
	}

	orange_socket_set_nonblock(s);
	setsockopt(s, SOL_SOCKET, MSG_NOSIGNAL, (void*) &set, sizeof(int));
	setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (void*) &set, sizeof(int));

	set = 1;
	setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &set, sizeof(int));

	DEBUGP("%s: socket created: %d\n", __func__, s);

	if (is_server) {
		if (addr != NULL) {
			ret = bind(s, addr, addrlen);
			if (ret != 0) {
				orange_log(ORANGE_LOG_ERR, "%s: bind socket error: %s\n", LOG_PREFIX, strerror(errno));
				close(s);
				s = -1;
				goto exit;
			}

			DEBUGP("%s: socket bind: %d, port: %u\n", __func__, s, ntohs(((struct sockaddr_in*) addr)->sin_port));
		}
		DEBUGP("%s: socket bind: %d, port: %u\n", __func__, s, ntohs(((struct sockaddr_in*) addr)->sin_port));
	}

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

static int __orange_socket_tcp_accept(int s, struct sockaddr* addr, socklen_t* addr_len)
{
	int fd;
	int set = 1;

	fd = accept(s, addr, addr_len);
	if (fd >= 0) {
		setsockopt(fd, SOL_SOCKET, MSG_NOSIGNAL, (void*) &set, sizeof(int));
	}

	return fd;
}

static ssize_t __orange_socket_tcp_recv(int s, void* buff, size_t len, int flags, struct sockaddr* from, socklen_t* from_len)
{
	int ret;

	ret = recv(s, buff, len, /*MSG_WAITALL*/ 0);

	return ret;
}

static ssize_t __orange_socket_tcp_send(int s, const void* mesg, size_t len, int flags, struct sockaddr* to, socklen_t to_len)
{
	return send(s, mesg, len, flags);
}

static int __orange_socket_tcp_close(int s)
{
	return close(s);
}

static int __orange_socket_tcp_getpeername(int s, struct sockaddr* name, socklen_t* name_len)
{
	*name_len = sizeof(struct sockaddr_in);
	return getpeername(s, name, name_len);
}

static char* __orange_socket_tcp_addrstring(struct sockaddr* addr, char* buff, int size)
{
	struct sockaddr_in* in_addr = (struct sockaddr_in*) addr;

	snprintf(buff, size, IPADDR_FMT "/%u", NIPQUAD(in_addr->sin_addr.s_addr), ntohs(in_addr->sin_port));

	return buff;
}

static struct orange_socket_func socket_funcs = {.domain	  = PF_LOCAL,
												 .create	  = __orange_socket_tcp_create,
												 .connect	 = __orange_socket_tcp_connect,
												 .accept	  = __orange_socket_tcp_accept,
												 .recv		  = __orange_socket_tcp_recv,
												 .send		  = __orange_socket_tcp_send,
												 .close		  = __orange_socket_tcp_close,
												 .getpeername = __orange_socket_tcp_getpeername,
												 .addrstring  = __orange_socket_tcp_addrstring};

int orange_socket_tcp_init(void)
{
	return orange_socket_func_register(ORANGE_SOCKET_TYPE_TCP, &socket_funcs);
}

void orange_socket_tcp_fini(void)
{
	orange_socket_func_unregister(ORANGE_SOCKET_TYPE_TCP);
}
