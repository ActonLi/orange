#ifndef __ORANGE_H__
#define __ORANGE_H__

#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <dlfcn.h>
#include <elf.h>
#include <endian.h>
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
