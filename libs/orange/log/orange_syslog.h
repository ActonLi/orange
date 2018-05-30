#ifndef __ORANGE_SYSLOG_H__
#define __ORANGE_SYSLOG_H__

extern int orange_systemlog_level_set(int level);
extern int orange_systemlog_level_get(void);
extern int orange_systemlog_init(const char* ident, int option, int facility);

extern void orange_systemlog_fini(void);
extern int  orange_systemlog_reinit(void);
extern void orange_systemlog_print(int level, char* buffer, int len);

#endif /* __ORANGE_SYSLOG_H__ */
