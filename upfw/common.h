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
U32 timer_gettick(STRTimeTickRegType* timer);
void timer_cleartick(STRTimeTickRegType* timer);
extern BOOL read_jsonfile(char* filename, cJSON** json);
extern BOOL write_jsonfile(char* filename, cJSON* json, E_FILE_OPEN_MODE mode);

extern BOOL create_jsonfile(char* filename, cJSON* json);
extern BOOL update_jsonfile(char* filename, cJSON* attributes);
extern BOOL replace_jsonfile(char* filename, cJSON* attributes);

extern BOOL del_file(char* filename);

extern int set_jsonfile_attributes(char* filename, cJSON* attributes);
extern int set_jsonfile_node_attrarray(char* filename, char* nodename, cJSON* attributes, int uid, int op);
extern U32 htoi(char* hex);
extern void add_json_fbstrarr(cJSON* json, char* str);
extern void add_json_fbnumarr(cJSON* json, int num);
extern U32 get_max_id(char* filepath);
extern BOOL check_column_valid(cJSON* val, STRDBDefs def);
extern BOOL check_column_valid_fordb(cJSON* colval, const STRDBDefs colinfo[], const int colinfonum, int* colinfoid);
extern BOOL check_column_mandatory_fordb(const STRDBDefs colinfo[], const int colinfonum, int colids[], int colnum);
extern BOOL check_column_updaterestrict_fordb(const STRDBDefs colinfo[], const int colinfonum, int colids[], int colnum);
extern BOOL check_column_insertrestrict_fordb(const STRDBDefs colinfo[], const int colinfonum, int colids[], int colnum);
extern BOOL get_insert_sql_defval(const STRDBDefValueType defvals[], int defvalcount, char* columns, int colsize, char* values, int valsize);
extern void get_insert_sql_bycolumninfo(cJSON* val, const STRDBDefs colinfo, char* columns, int maxcollen, char* values, int maxvallen);
extern void get_update_sql_bycolumninfo(cJSON* val, const STRDBDefs colinfo, char* sql, int maxsqllen);
extern void get_select_sql_bycolumninfo(char* sql, char* table, char* where, char* order, int page, int pagesize);
extern BOOL get_db_filters(char* filter, U32 filtersize, cJSON* jsonfilter, char* filterstr, const STRDBDefs colinfo[], const int colinfonum);

extern BOOL get_barejidaccount(char* fulljid, char* jidaccount, int maxlen);
extern BOOL set_json_attributes_bypath(cJSON* json, char* path, void* value, int datatype);
#endif
