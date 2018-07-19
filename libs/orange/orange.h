#ifndef __ORANGE_H__
#define __ORANGE_H__

#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <dlfcn.h>
#include <elf.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <linux/limits.h>
#include <linux/sysctl.h>
#include <malloc.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <poll.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
//#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/sysctl.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/vfs.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

typedef enum orange_endianess {
	ORANGE_BIG_ENDIAN	= 0,
	ORANGE_LITTLE_ENDIAN = 1,
} orange_endianess_t;

static inline int orange_is_little_endian(void)
{
	const union {
		uint32_t u;
		uint8_t  c[4];
	} one = {1}; /* don't use static : performance detrimental  */
	return one.c[0];
}

#define ORANGE_VERSION_VALUE(major, minor, build) (uint32_t)(((uint32_t)(major) << 16) + ((uint16_t)(minor) << 8) + ((uint16_t)(build)))

#define ORANGE_VERSION_MAJOR(version) (((unsigned char*) &(version))[2])
#define ORANGE_VERSION_MINOR(version) (((unsigned char*) &(version))[1])
#define ORANGE_VERSION_BUILD(version) (((unsigned char*) &(version))[0])

#define ORANGE_VERSION_STRING_LEN 16
#define ORANGE_VERSION_FORMAT "%u.%u.%02u"
#define ORANGE_VERSION_QUAD(version) ((unsigned char*) &(version))[2], ((unsigned char*) &(version))[1], ((unsigned char*) &(version))[0]

#define ORANGE_VERSION_TYPE_RELEASE 0x03
#define ORANGE_VERSION_TYPE_BETA 0x02
#define ORANGE_VERSION_TYPE_ALPHA 0x01
#define ORANGE_VERSION_TYPE_UNKNOWN 0x00

#define ORANGE_VERSION_TYPE_RELEASE_STRING "RELEASE"
#define ORANGE_VERSION_TYPE_BETA_STRING "BETA"
#define ORANGE_VERSION_TYPE_ALPHA_STRING "ALPHA"
#define ORANGE_VERSION_TYPE_UNKNOWN_STRING "UNKNOWN"

static __inline__ const char* orange_version_type(int type)
{
	switch (type) {
		case ORANGE_VERSION_TYPE_RELEASE:
			return ORANGE_VERSION_TYPE_RELEASE_STRING;

		case ORANGE_VERSION_TYPE_BETA:
			return ORANGE_VERSION_TYPE_BETA_STRING;

		case ORANGE_VERSION_TYPE_ALPHA:
			return ORANGE_VERSION_TYPE_ALPHA_STRING;

		case ORANGE_VERSION_TYPE_UNKNOWN:
		default:
			return ORANGE_VERSION_TYPE_UNKNOWN_STRING;
	}
	return ORANGE_VERSION_TYPE_UNKNOWN_STRING;
}

#define ORANGE_VERSION_TYPE(name)                                                                                                                              \
	extern uint8_t name##_version_major(void);                                                                                                                 \
	extern uint8_t name##_version_minor(void);                                                                                                                 \
	extern uint8_t name##_version_build(void);                                                                                                                 \
	extern char*   name##_version_description(void);                                                                                                           \
	extern int	 name##_version_build_num(void);                                                                                                             \
	extern char*   name##_version_build_date(void);

#define ORANGE_VERSION_GENERATE(name, major, minor, build, type)                                                                                               \
	static int name##_version	  = ORANGE_VERSION_VALUE((major), (minor), (build));                                                                          \
	static int name##_version_type = (type);                                                                                                                   \
	char	   name##_description[128];                                                                                                                        \
	uint8_t	name##_version_major(void)                                                                                                                      \
	{                                                                                                                                                          \
		return ORANGE_VERSION_MAJOR(name##_version);                                                                                                           \
	}                                                                                                                                                          \
                                                                                                                                                               \
	uint8_t name##_version_minor(void)                                                                                                                         \
	{                                                                                                                                                          \
		return ORANGE_VERSION_MINOR(name##_version);                                                                                                           \
	}                                                                                                                                                          \
                                                                                                                                                               \
	uint8_t name##_version_build(void)                                                                                                                         \
	{                                                                                                                                                          \
		return ORANGE_VERSION_BUILD(name##_version);                                                                                                           \
	}                                                                                                                                                          \
                                                                                                                                                               \
	char* name##_version_description(void)                                                                                                                     \
	{                                                                                                                                                          \
		return name##_description;                                                                                                                             \
	}                                                                                                                                                          \
                                                                                                                                                               \
	int name##_version_build_num(void)                                                                                                                         \
	{                                                                                                                                                          \
		return name##_build_num;                                                                                                                               \
	}                                                                                                                                                          \
	char* name##_version_build_date(void)                                                                                                                      \
	{                                                                                                                                                          \
		return name##_build_date;                                                                                                                              \
	}

#define ORANGE_TIMEOUT_NONE ((uint32_t) 0)
#define ORANGE_TIMEOUT_FOREVER ((uint32_t) -1)
#define ORANGE_TIMEOUT_TIMED ((uint32_t) -2)

#define orange_malloc(size) malloc((size))
#define orange_zalloc(size) calloc(1, (size))
#define orange_zalloc_wait(size) calloc(1, (size))
#define orange_realloc(ptr, size) reallocf(ptr, size)
#define orange_free(p) free((p))
extern uint8_t			orange_ncpus;
extern __thread uint8_t orange_cpu_id;
#define ORANGE_CPU_ID orange_cpu_id

#define ORANGE_ASSERT(x) assert(x)

#ifndef likely
#define likely(x) __builtin_expect((x), 1)
#endif

#ifndef unlikely
#define unlikely(x) __builtin_expect((x), 0)
#endif

#define ORANGE_KB (1024)
#define ORANGE_MB (1024 * ORANGE_KB)
#define ORANGE_GB (uint64_t)(1024 * ORANGE_MB)

#ifndef min
#define min(a, b) ((a) > (b) ? (b) : (a))
#endif

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

#define strlen_min(str, min_len) (min(strlen(str), min_len - 1))

#endif // __ORANGE_H__
