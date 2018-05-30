#ifndef __ORANGE_FILELOG_H__
#define __ORANGE_FILELOG_H__

extern int orange_filelog_level_set(int level);
extern int orange_filelog_level_get(void);
extern int orange_filelog_init(const char* ident, const char* logfile);
extern void orange_filelog_fini(void);
extern int  orange_filelog_reinit(void);
extern void orange_filelog_print(int level, char* buffer, int len);

#endif /* __ORANGE_FILELOG_H__ */
