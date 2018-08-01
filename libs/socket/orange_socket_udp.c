#include "../orange/orange.h"
#include "../orange/orange_log.h"

#include "orange_socket.h"
#include "orange_socket_udp.h"

#if 0
#define DEBUGP orange_log_error
#else
#define DEBUGP(args, ...)
#endif

#define LOG_PREFIX "SOCKET_UDP"

static int __orange_socket_udp_create(int protocol, struct sockaddr* addr, socklen_t addrlen, int backlog, uint8_t is_server)
{
	int s = -1;
	int ret;

	s = socket(PF_INET, SOCK_DGRAM, 0);
	if (s == -1) {
		orange_log(ORANGE_LOG_ERR, "%s: socket create error: %s\n", LOG_PREFIX, strerror(errno));
		goto exit;
	}

	DEBUGP("%s: socket created: %d\n", __func__, s);

	int yes = 1;
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const void*) &yes, sizeof(int)) == -1) {
		goto exit;
	}

	orange_socket_set_nonblock(s);

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
	}

exit:
	return s;
}

static int __orange_socket_udp_accept(int s, struct sockaddr* addr, socklen_t* addr_len)
{
	return EINVAL;
}

static ssize_t __orange_socket_udp_recv(int s, void* buff, size_t len, int flags, struct sockaddr* from, socklen_t* from_len)
{
	int size;
	int retry_num = 0;

	*from_len = sizeof(struct sockaddr_in);

retry:
	size = recvfrom(s, buff, len, flags, from, from_len);
	if (size == -1 && (errno == EWOULDBLOCK || errno == EAGAIN)) {
		if (retry_num > 3) {
			goto exit;
		}
		retry_num++;
		goto retry;
	}

exit:
	DEBUGP("%s: size: %d, sock: %d\n", __func__, size, s);
	return size;
}

static ssize_t __orange_socket_udp_send(int s, const void* mesg, size_t len, int flags, struct sockaddr* to, socklen_t to_len)
{
	int size;

	DEBUGP("%s: size: %d, sock: %d\n", __func__, size, s);
	size = sendto(s, mesg, len, flags, to, to_len);

	return size;
}

static int __orange_socket_udp_close(int s)
{
	return close(s);
}

static int __orange_socket_udp_getpeername(int s, struct sockaddr* name, socklen_t* name_len)
{
	*name_len = sizeof(struct sockaddr_in);
	return getpeername(s, name, name_len);
}

static char* __orange_socket_udp_addrstring(struct sockaddr* addr, char* buff, int size)
{
	struct sockaddr_in* in_addr = (struct sockaddr_in*) addr;

	snprintf(buff, size, IPADDR_FMT "/%u", NIPQUAD(in_addr->sin_addr.s_addr), ntohs(in_addr->sin_port));

	return buff;
}

static struct orange_socket_func socket_funcs = {.domain	  = PF_INET,
												 .create	  = __orange_socket_udp_create,
												 .accept	  = __orange_socket_udp_accept,
												 .recv		  = __orange_socket_udp_recv,
												 .send		  = __orange_socket_udp_send,
												 .close		  = __orange_socket_udp_close,
												 .getpeername = __orange_socket_udp_getpeername,
												 .addrstring  = __orange_socket_udp_addrstring};

int orange_socket_udp_init(void)
{
	return orange_socket_func_register(ORANGE_SOCKET_TYPE_UDP, &socket_funcs);
}

void orange_socket_udp_fini(void)
{
	orange_socket_func_unregister(ORANGE_SOCKET_TYPE_UDP);
}
