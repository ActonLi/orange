#ifndef _H_COMMON_
#define _H_COMMON_
#include "cJSON.h"
#include "db.h"
#include "define.h"
#include "osa_file.h"
typedef BOOL (*CHECK_FUN)(char* value);
extern CHECK_FUN check_funs[];

#define KEY_TYPE_SIZE 256

#define CHECK_RULES_FUN_NUM 6

#define CHECK_RULES_FUN_PWD 0
#define CHECK_RULES_FUN_USER 1
#define CHECK_RULES_FUN_EMAIL 2
#define CHECK_RULES_FUN_IP 3
#define CHECK_RULES_FUN_URL 4
#define CHECK_RULES_FUN_MOBILE 5

#define DB_COL_TYPE_INT 0
#define DB_COL_TYPE_BOOL 1
#define DB_COL_TYPE_STRING 2
#define DB_COL_TYPE_JSON 3
#define DB_COL_FLAG_INSERT_MANDATORY 1
#define DB_COL_FLAG_UPDATE_RESTRICT 2
#define DB_COL_FLAG_NOT_SAVETODB 4
#define DB_COL_FLAG_INFO_BRIEF 8
#define DB_COL_FLAG_INFO_DETAIL 16

#define DB_COL_FLAG_INSERT_RESTRICT 32
typedef struct {
	char* colname;
	int   type;
	U32   flags;
	char* rules;
} STRDBDefs;
typedef struct {
	char* colname;
	char* defvalue;
	int   coltype;
} STRDBDefValueType;
typedef struct {
	char* displayname;
	char* columnname;
	int   coltype;
	U32   flags;
} STRDBSelectInfo;
#define TM_TICK_NUM 5
#define TM_TICK_100ms 0
#define TM_TICK_200ms 1
#define TM_TICK_400ms 2
#define TM_TICK_800ms 3
#define TM_TICK_1s 4

typedef struct {
	BOOL tick[TM_TICK_NUM];
	U32  count[TM_TICK_NUM];
} STRTimeTickRegType;
extern BOOL msg_findpathbyid(char* pid, char* path);
#endif
