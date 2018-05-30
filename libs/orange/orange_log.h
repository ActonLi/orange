#ifndef __ORANGE_LOG_H__
#define __ORANGE_LOG_H__

#include "orange.h"

#ifdef _KERNEL
#define ORANGE_LOG_EMERG LOG_EMERG
#define ORANGE_LOG_ALERT LOG_ALERT
#define ORANGE_LOG_CRIT LOG_CRIT
#define ORANGE_LOG_ERR LOG_ERR
#define ORANGE_LOG_WARNING LOG_WARNING
#define ORANGE_LOG_NOTICE LOG_NOTICE
#define ORANGE_LOG_INFO LOG_INFO
#define ORANGE_LOG_DEBUG LOG_DEBUG
#else
#define ORANGE_LOG_EMERG 0x00
#define ORANGE_LOG_ALERT 0x01
#define ORANGE_LOG_CRIT 0x02
#define ORANGE_LOG_ERR 0x03
#define ORANGE_LOG_WARNING 0x04
#define ORANGE_LOG_NOTICE 0x05
#define ORANGE_LOG_INFO 0x06
#define ORANGE_LOG_DEBUG 0x07
#endif

#define ORANGE_LOG_IDENT_MAX 32
#define ORANGE_LOG_LINE_MAX 2048
#define ORANGE_LOG_TIME_BUFF_SIZE 128

static inline char* orange_log_get_time(char* time_buff)
{
	time_t currtime;
	char*  ptime = NULL;

	time(&currtime);
	ptime = ctime(&currtime);
	strncpy(time_buff, ptime, 128);
	time_buff[strlen(time_buff) - 1] = '\0';
	return time_buff;
}

extern int orange_log_init(const char* ident, int syslog, int filelog, int consolelog);
extern int  orange_log_reinit(void);
extern void orange_log_fini(void);
extern int  orange_log_open(void);
extern void orange_log_close(void);

extern int orange_log_level_set(int level);
extern int orange_log_level_get(void);

extern int orange_log_string(int level, char* buff, int len);
extern int orange_log(int level, const char* fmt, ...) __attribute__((format(printf, 2, 3)));
extern int orange_log_debug(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
extern int orange_log_info(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
extern int orange_log_notice(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
extern int orange_log_warning(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
extern int orange_log_alert(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
extern int orange_log_error(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
extern int orange_log_crit(const char* fmt, ...) __attribute__((format(printf, 1, 2)));

#endif /* __ORANGE_LOG_H__ */
