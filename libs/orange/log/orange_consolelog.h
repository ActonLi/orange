#ifndef __ORANGE_CONSOLELOG_H__
#define __ORANGE_CONSOLELOG_H__

extern int orange_consolelog_level_set(int level);
extern int orange_consolelog_level_get(void);
extern int orange_consolelog_init(const char* ident);
extern void orange_consolelog_fini(void);
extern int  orange_consolelog_reinit(void);
extern void orange_consolelog_print(int level, char* buffer, int len);

#endif /* __ORANGE_CONSOLELOG_H__ */
