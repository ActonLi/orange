#include "common.h"
#include "cJSON.h"
#include "message.h"
#include "string.h"
#include <dirent.h>
#include <sys/time.h>
#include <unistd.h>

#if 0
#define DEBUGP printf
#else
#define DEBUGP(format, args...)
#endif

const U32 TM_TICK_DIV[] = {100, 200, 400, 800, 1000};

typedef struct {
	char  pid[MSG_ID_SIZE];
	char* path;
} STRMsgAddrType;

const STRMsgAddrType msg_addr[] = {
	{SOCK_QXMPP_ID, SOCK_QXMPP_PATH}, {SOCK_WIFI_ID, SOCK_WIFI_PATH}, {SOCK_UPFW_ID, SOCK_UPFW_PATH}, {SOCK_AC_ID, SOCK_AC_PATH},
};

BOOL msg_findpathbyid(char* pid, char* path)
{
	if (NULL == pid || NULL == path) {
		return FALSE;
	}

	U32  i;
	BOOL res = FALSE;

	for (i = 0; i < sizeof(msg_addr) / sizeof(STRMsgAddrType); i++) {
		if (0 == memcmp(pid, msg_addr[i].pid, MSG_ID_SIZE)) {
			memset(path, 0, strlen(msg_addr[i].path) + 1);
			strcpy(path, msg_addr[i].path);
			res = TRUE;
			break;
		}
	}

	return (res);
}
U32 timer_gettick(STRTimeTickRegType* timer)
{
#if 0
    U32 i;
    struct timeval tv;
    U32 stamp;
    U32 q;

    gettimeofday(&tv, NULL);
    stamp = tv.sec * 1000;
    stamp += tv.usec / 1000;

    for (i = 0; i < TM_TICK_NUM; i++) {
        q = stamp / TM_TICK_DIV[i];
        if (q != timer->count[i]) {
            timer->count[i] = q;
            timer->tick[i] = TRUE;
        }
    }
#endif

	return 0;
}

void timer_cleartick(STRTimeTickRegType* timer)
{
	U32 i;
	for (i = 0; i < TM_TICK_NUM; i++) {
		timer->tick[i] = FALSE;
	}
}

void timer_inittick(STRTimeTickRegType* timer)
{
#if 0
    U32 i;
    struct timeval tv;
    U32 stamp;
    U32 q;
    gettimeofday(&tv, NULL);
    stamp = tv.sec * 1000;
    stamp += tv.usec / 1000;
    for (i = 0; i < TM_TICK_NUM; i++) {
        q = stamp / TM_TICK_DIV[i];
        timer->count[i] = q;
        timer->tick[i] = FALSE;
    }
#endif
}
BOOL read_jsonfile(char* filename, cJSON** json)
{
	if (NULL == filename || NULL == json) {
		return FALSE;
	}

	FILE* file;
	char* stream;
	int   len, lsize;
	BOOL  res = FALSE;
	file	  = fopen(filename, "r");
	if (NULL != file) {
		fseek(file, 0, SEEK_END);
		lsize = ftell(file);
		rewind(file);
		stream		  = (char*) malloc(sizeof(char) * (1 + lsize));
		stream[lsize] = '\0';
		len			  = fread(stream, 1, lsize, file);

		if (len == lsize) {
			stream[len] = '\0';
			*json		= cJSON_Parse(stream);
			if (NULL != *json) {
				res = TRUE;
			}
		}
		free(stream);
		stream = NULL;
		fclose(file);
	}
	DEBUGP("read json file 11=%s, res=%d\n", filename, res);
	return (res);
}
BOOL write_jsonfile(char* filename, cJSON* json, E_FILE_OPEN_MODE mode)
{
	if (NULL == filename || NULL == json)
		return FALSE;

	BOOL		res = FALSE;
	FILE_HANDLE fhandle;
	char*		stream;
	stream = cJSON_Print(json);
	/// DEBUGP("start to write file = %s\n", stream);
	if (NULL != stream) {
		fhandle = OSA_FileOpen(filename, mode, ACCESS_WRITE);
		if (INVALID_FILE_HANDLE != fhandle)
			;
		{
			cJSON_Minify(stream);
			ftruncate(fhandle, 0); // ?????????ļ?
			OSA_FileSeek(fhandle, 0, SEEK_FROM_START);
			OSA_FileWrite(fhandle, (BYTE*) stream, strlen(stream));
			OSA_FileClose(fhandle);
			res = TRUE;
		}
		free(stream);
		stream = NULL;
	}
	return (res);
}
BOOL create_jsonfile(char* filename, cJSON* json)
{
	if (NULL == json) {
		return FALSE;
	}
	return write_jsonfile(filename, json, MODE_TRY_CREATE_NEW);
}
BOOL update_jsonfile(char* filename, cJSON* attributes)
{
	if (NULL == filename || NULL == attributes)
		return FALSE;

	BOOL   res		= FALSE;
	cJSON* jsonfile = NULL;
	cJSON* attr		= NULL;
	cJSON* param	= NULL;
	if (TRUE == read_jsonfile(filename, &jsonfile)) {
		if (NULL != jsonfile) {
			attr = attributes->child;
			while (NULL != attr) {
				if (NULL != attr->string) {
					param = cJSON_GetObjectItem(jsonfile, attr->string);
					if (NULL != param) {
						cJSON_DeleteItemFromObject(jsonfile, attr->string);
					}
					cJSON_AddItemToObject(jsonfile, attr->string, cJSON_Duplicate(attr, 1));
				}
				attr = attr->next;
			}
			if (TRUE == write_jsonfile(filename, jsonfile, MODE_OPEN_EXISTING)) {
				res = TRUE;
			}
		}
	}
	if (NULL != jsonfile) {
		cJSON_Delete(jsonfile);
		jsonfile = NULL;
	}
	return (res);
}
BOOL replace_jsonfile(char* filename, cJSON* attributes)
{
	BOOL res = FALSE;
	if (NULL == filename || NULL == attributes) {
		return res;
	}
	if (TRUE == OSA_FileIsExist(filename)) {
		if (TRUE == write_jsonfile(filename, attributes, MODE_OPEN_EXISTING)) {
			res = TRUE;
		}
	} else {
		if (TRUE == write_jsonfile(filename, attributes, MODE_TRY_CREATE_NEW)) {
			res = TRUE;
		}
	}
	return (res);
}
BOOL del_file(char* filename)
{
	BOOL res = FALSE;
	if (TRUE == OSA_FileIsExist(filename)) {
		if (0 != OSA_FileDel(filename)) {
			res = TRUE;
		}
	}
	return (res);
}
U32 htoi(char* hex)
{
	int i;
	U32 res = 0;
	for (i = 0; i < strlen(hex); i++) {
		res = (res << 4);
		if ((hex[i] >= '0') && (hex[i] <= '9')) {
			res |= (hex[i] - '0');
		}
		if ((hex[i] >= 'a') && (hex[i] <= 'f')) {
			res |= (hex[i] - 'a' + 10);
		}
		if ((hex[i] >= 'A') && (hex[i] <= 'F')) {
			res |= (hex[i] - 'A' + 10);
		}
	}
	return (res);
}

void add_json_fbnumarr(cJSON* json, int num)
{
	if (json == NULL)
		return;

	BOOL   hasadd = FALSE;
	cJSON* item;
	item = json->child;
	while (NULL != item) {
		if (item->valueint == num) {
			hasadd = TRUE;
			break;
		}
		item = item->next;
	}
	if (FALSE == hasadd) {
		item = cJSON_CreateNumber(num);
		cJSON_AddItemToArray(json, item);
	}
}

void add_json_fbstrarr(cJSON* json, char* str)
{
	if (json == NULL)
		return;

	BOOL   hasadd = FALSE;
	cJSON* item;
	item = json->child;
	while (NULL != item) {
		if (NULL != item->valuestring) {
			if (0 == strcmp(item->valuestring, str)) {
				hasadd = TRUE;
				break;
			}
		}
		item = item->next;
	}
	if (FALSE == hasadd) {
		item = cJSON_CreateString(str);
		cJSON_AddItemToArray(json, item);
	}
}
#define FILE_NAME_LEN 8
U32 get_max_id(char* filepath)
{
	if (NULL == filepath)
		return 0;
	DIR*		   dir;
	char		   ids[1 + FILE_NAME_LEN];
	U32			   maxid = 0;
	U32			   curid;
	struct dirent* pFiles;
	char *		   fp, *ep;

	dir = opendir(filepath);
	if (NULL == dir) {
		return 0;
	}
	while ((pFiles = readdir(dir)) != NULL) {
		if (0 == strcmp(pFiles->d_name, ".") || 0 == strcmp(pFiles->d_name, ".."))
			continue;
		fp = pFiles->d_name;
		ep = strchr(fp, '.');

		if ((ep != NULL) && ((ep - fp) == FILE_NAME_LEN)) {
			memcpy(ids, fp, FILE_NAME_LEN);
			ids[FILE_NAME_LEN] = 0;
			curid			   = htoi(ids);
			if (maxid < curid) {
				maxid = curid;
			}
		}
	}
	closedir(dir);
	return (maxid + 1);
}
int set_jsonfile_attributes(char* filename, cJSON* attributes)
{
	if (NULL == filename || NULL == attributes)
		return -1;

	cJSON *attr, *param;
	cJSON* jsonfile  = NULL;
	int	attrcount = 0;
	int	res		 = 0;
	if ((TRUE == read_jsonfile(filename, &jsonfile)) && (NULL != attributes)) {
		attr = attributes->child;
		while (NULL != attr) {
			if (NULL != attr->string) {
				param = cJSON_GetObjectItem(jsonfile, attr->string);
				if (NULL != param) {
					cJSON_DeleteItemFromObject(jsonfile, attr->string);
				}
				cJSON_AddItemToObject(jsonfile, attr->string, cJSON_Duplicate(attr, 1));
				attrcount++;
			}
			attr = attr->next;
		}
		if (TRUE == write_jsonfile(filename, jsonfile, MODE_OPEN_EXISTING)) {
			res = attrcount;
		}
	}
	if (NULL != jsonfile) {
		cJSON_Delete(jsonfile);
		jsonfile = NULL;
	}
	return (res);
}

int set_jsonfile_node_attrarray(char* filename, char* nodename, cJSON* attributes, int uid, int op)
{
	cJSON *attr, *param;
	cJSON* jsonfile = NULL;
	int	res		= -1;
	BOOL   change   = FALSE;
	cJSON *jsonnode, *jsonitem, *jsonuid;
	cJSON* newattibutes;
	int	maxid   = 0;
	int	whichin = -1;
	int	pointer = 0;
	if ((TRUE == read_jsonfile(filename, &jsonfile)) && (NULL != jsonfile)) {
		jsonnode = cJSON_GetObjectItem(jsonfile, nodename);
		if (op == 0) {
			if (NULL == jsonnode) {
				change   = TRUE;
				jsonnode = cJSON_CreateArray();
				cJSON_AddItemToObject(jsonfile, nodename, jsonnode);
			}
		}
		if (NULL != jsonnode) {
			if (0 == op) {
				if (NULL != attributes) {
					jsonitem = jsonnode->child;
					while (NULL != jsonitem) {
						jsonuid = cJSON_GetObjectItem(jsonitem, "uid");
						if (NULL != jsonuid) {
							if (maxid < jsonuid->valueint) {
								maxid = jsonuid->valueint;
							}
						}
						jsonitem = jsonitem->next;
					}
					maxid		 = maxid + 1;
					newattibutes = cJSON_Duplicate(attributes, 1);
					cJSON_AddNumberToObject(newattibutes, "uid", maxid);
					cJSON_AddItemToArray(jsonnode, newattibutes);
					res	= maxid;
					change = TRUE;
				}
			} else {
				jsonitem = jsonnode->child;
				pointer  = 0;
				whichin  = -1;
				while (NULL != jsonitem) {
					jsonuid = cJSON_GetObjectItem(jsonitem, "uid");
					if (NULL != jsonuid) {
						if (uid == jsonuid->valueint) {
							whichin = pointer;
							break;
						}
					}
					pointer++;
					jsonitem = jsonitem->next;
				}
				if (whichin >= 0) {
					if ((op == 1) && (NULL != attributes)) {
						change = TRUE;
						attr   = attributes->child;
						while (NULL != attr) {
							if (NULL != attr->string) {
								param = cJSON_GetObjectItem(jsonitem, attr->string);
								if (NULL != param) {
									cJSON_DeleteItemFromObject(jsonitem, attr->string);
								}
								cJSON_AddItemToObject(jsonitem, attr->string, cJSON_Duplicate(attr, 1));
							}
							attr = attr->next;
						}
						change = TRUE;
					}

					if (op == 2) {
						cJSON_DeleteItemFromArray(jsonnode, whichin);
						// DEBUGP("delete node from jsonnode at which=%d,
						// op=%d\n, %s", whichin, op,
						// cJSON_Print(jsonfile));
						change = TRUE;
					}
					res = uid;
				}
			}
		}
	}
	if (TRUE == change) {
		if (TRUE != write_jsonfile(filename, jsonfile, MODE_OPEN_EXISTING)) {
			res = -1;
		}
	}
	if (NULL != jsonfile) {
		cJSON_Delete(jsonfile);
		jsonfile = NULL;
	}
	return (res);
}

BOOL check_rules_password(char* value)
{
	BOOL res = TRUE;
	DEBUGP("check password\n");
	return (res);
}
BOOL check_rules_user(char* value)
{
	BOOL res = TRUE;
	DEBUGP("check user\n");
	return (res);
}
BOOL check_rules_email(char* value)
{
	BOOL res = TRUE;
	DEBUGP("check email\n");
	return (res);
}
BOOL check_rules_ip(char* value)
{
	BOOL res = TRUE;
	DEBUGP("check ip\n");
	return (res);
}
BOOL check_rules_url(char* value)
{
	BOOL res = TRUE;
	DEBUGP("check url\n");
	return (res);
}
BOOL check_rules_mobile(char* value)
{
	BOOL res = TRUE;
	DEBUGP("check url\n");
	return (res);
}
BOOL check_rules_userrule(char* value)
{
	BOOL res = TRUE;
	DEBUGP("check url\n");
	return (res);
}
CHECK_FUN check_funs[] = {check_rules_password, check_rules_user,   check_rules_email,   check_rules_ip,
						  check_rules_url,		check_rules_mobile, check_rules_userrule

};
BOOL check_column_valid(cJSON* val, STRDBDefs def)
{
	if (NULL == val)
		return FALSE;

	char   numval[16];
	cJSON* ruleobj	= NULL;
	cJSON* param	  = NULL;
	int	checkfunid = -1;
	BOOL   needcheckfun, needmax, needmin;
	BOOL   isvalid;
	int	max   = 1024;
	int	min   = 0;
	needcheckfun = FALSE;
	needmax		 = FALSE;
	needmin		 = FALSE;
	isvalid		 = TRUE;
	if (strlen(def.rules) > 0) {
		ruleobj = cJSON_Parse(def.rules);
		if (NULL != ruleobj) {
			param = cJSON_GetObjectItem(ruleobj, "checkfun");
			if ((NULL != param) && (param->valueint < CHECK_RULES_FUN_NUM)) {
				checkfunid   = param->valueint;
				needcheckfun = TRUE;
			}

			param = NULL;
			param = cJSON_GetObjectItem(ruleobj, "max");
			if (NULL != param) {
				max		= param->valueint;
				needmax = TRUE;
			}

			param = NULL;
			param = cJSON_GetObjectItem(ruleobj, "min");
			if (NULL != param) {
				min		= param->valueint;
				needmin = TRUE;
			}
		}
		if (DB_COL_TYPE_STRING == def.type) {
			if (TRUE == needcheckfun) {
				isvalid = check_funs[checkfunid](val->valuestring);
			}
			if ((TRUE == isvalid) && (TRUE == needmax) && (strlen(val->valuestring) > max)) {
				isvalid = FALSE;
			}
			if ((TRUE == isvalid) && (TRUE == needmin) && (strlen(val->valuestring) < min))

			{
				isvalid = FALSE;
			}
		} else if ((DB_COL_TYPE_INT == def.type) || (DB_COL_TYPE_BOOL == def.type)) {
			snprintf(numval, sizeof(numval), "%d", val->valueint);
			if (TRUE == needcheckfun) {
				isvalid = check_funs[checkfunid](numval);
			}
			if ((TRUE == isvalid) && (TRUE == needmax) && (val->valueint > max)) {
				isvalid = FALSE;
			}
			if ((TRUE == isvalid) && (TRUE == needmin) && (val->valueint < min)) {
				isvalid = FALSE;
			}
		}
	}

	return (isvalid);
}
BOOL check_column_valid_fordb(cJSON* colval, const STRDBDefs colinfo[], const int colinfonum, int* colinfoid)
{
	if (NULL == colinfo || NULL == colinfoid)
		return FALSE;
	int  i;
	BOOL valid = TRUE;
	*colinfoid = -1;
	for (i = 0; i < colinfonum; i++) {
		if (0 == strcmp(colinfo[i].colname, colval->string))

		{
			*colinfoid = i;

			//?ж???Ч??
			if ((DB_COL_TYPE_STRING == colinfo[i].type) && (NULL == colval->valuestring)) {
				valid = FALSE;
			} else {
				if (TRUE == check_column_valid(colval, colinfo[i])) {
				} else {
					valid = FALSE;
				}
			}
			break;
		}
	}
	if (*colinfoid < 0) {
		valid = FALSE;
	}
	DEBUGP("get column=%s, valid=%d\n", colval->string, valid);
	return (valid);
}

BOOL check_column_mandatory_fordb(const STRDBDefs colinfo[], const int colinfonum, int colids[], int colnum)
{
	if (NULL == colinfo || NULL == colids)
		return FALSE;

	BOOL hascol = FALSE;
	BOOL valid  = TRUE;
	int  i, j;
	for (i = 0; i < colinfonum; i++) {
		if (DB_COL_FLAG_INSERT_MANDATORY == (colinfo[i].flags & DB_COL_FLAG_INSERT_MANDATORY)) {
			hascol = FALSE;
			for (j = 0; j < colnum; j++) {
				if (i == colids[j]) {
					hascol = TRUE;
					break;
				}
			}
			if (FALSE == hascol) {
				valid = FALSE;
				break;
			}
		}
	}
	return (valid);
}

BOOL check_column_updaterestrict_fordb(const STRDBDefs colinfo[], const int colinfonum, int colids[], int colnum)
{
	if (NULL == colinfo || NULL == colids)
		return FALSE;

	BOOL hascol = FALSE;
	BOOL valid  = TRUE;
	int  i, j;
	for (i = 0; i < colinfonum; i++) {
		if (DB_COL_FLAG_UPDATE_RESTRICT == (colinfo[i].flags & DB_COL_FLAG_UPDATE_RESTRICT)) {
			hascol = FALSE;
			for (j = 0; j < colnum; j++) {
				if (i == colids[j]) {
					hascol = TRUE;
					break;
				}
			}
			if (TRUE == hascol) {
				valid = FALSE;
				break;
			}
		}
	}
	return (valid);
}

BOOL check_column_insertrestrict_fordb(const STRDBDefs colinfo[], const int colinfonum, int colids[], int colnum)
{
	if (NULL == colinfo || NULL == colids)
		return FALSE;

	BOOL hascol = FALSE;
	BOOL valid  = TRUE;
	int  i, j;
	for (i = 0; i < colinfonum; i++) {
		if (DB_COL_FLAG_INSERT_RESTRICT == (colinfo[i].flags & DB_COL_FLAG_INSERT_RESTRICT)) {
			hascol = FALSE;
			for (j = 0; j < colnum; j++) {
				if (i == colids[j]) {
					hascol = TRUE;
					break;
				}
			}
			if (TRUE == hascol) {
				valid = FALSE;
				break;
			}
		}
	}
	return (valid);
}
BOOL get_insert_sql_defval(const STRDBDefValueType defvals[], int defvalcount, char* columns, int colsize, char* values, int valsize)
{
	if (NULL == defvals || NULL == columns || NULL == values)
		return FALSE;

	int  i;
	char value[256];
	for (i = 0; i < defvalcount; i++) {
		if (0 == strcmp(defvals[i].defvalue, "sys_uuid")) {
			snprintf(value, sizeof(value), "%s", "123456789");
		} else if (0 == strcmp(defvals[i].defvalue, "sys_time")) {
			snprintf(value, sizeof(value), "%lu", time(NULL));
		} else {
			snprintf(value, sizeof(value), "%s", defvals[i].defvalue);
		}
		snprintf(columns + strlen(columns), colsize - strlen(columns), ",%s", defvals[i].colname);
		if ((defvals[i].coltype == DB_COL_TYPE_STRING) || (defvals[i].coltype == DB_COL_TYPE_JSON)) {
			snprintf(values + strlen(values), valsize - strlen(values), ",\"%s\"", value);
		} else {
			snprintf(values + strlen(values), valsize - strlen(values), ",%s", value);
		}
	}

	return TRUE;
}
void get_update_sql_bycolumninfo(cJSON* val, const STRDBDefs colinfo, char* sql, int maxsqllen)
{
	if (NULL == val || NULL == sql)
		return;

	char* stream;
	if (0 == (colinfo.flags & DB_COL_FLAG_NOT_SAVETODB)) {
		if (DB_COL_TYPE_STRING == colinfo.type) {
			if (0 == sql[0])
				snprintf(sql, maxsqllen, "%s=\'%s\'", colinfo.colname, val->valuestring);
			else
				snprintf(sql + strlen(sql), maxsqllen - strlen(sql), ",%s=\'%s\'", colinfo.colname, val->valuestring);

		} else if (DB_COL_TYPE_JSON == colinfo.type) {
			stream = NULL;
			stream = cJSON_Print(val);
			cJSON_Minify(stream);
			if (0 == sql[0])
				snprintf(sql, maxsqllen, "%s=\'%s\'", colinfo.colname, stream);
			else
				snprintf(sql + strlen(sql), maxsqllen - strlen(sql), ",%s=\'%s\'", colinfo.colname, stream);
			free(stream);
			stream = NULL;

		} else {
			if (0 == sql[0])
				snprintf(sql, maxsqllen, "%s=%d", colinfo.colname, val->valueint);
			else
				snprintf(sql + strlen(sql), maxsqllen - strlen(sql), ",%s=%d", colinfo.colname, val->valueint);
		}
	}
}
void get_insert_sql_bycolumninfo(cJSON* val, const STRDBDefs colinfo, char* columns, int maxcollen, char* values, int maxvallen)
{
	if (NULL == columns || NULL == values)
		return;

	char* stream;
	if (0 == (colinfo.flags & DB_COL_FLAG_NOT_SAVETODB)) {
		if (columns[0] == 0) {
			snprintf(columns, maxcollen, "(%s", colinfo.colname);
		} else {
			snprintf(columns + strlen(columns), maxcollen - strlen(columns), ",%s", colinfo.colname);
		}
		if (values[0] == 0) {
			if (DB_COL_TYPE_STRING == colinfo.type) {
				snprintf(values, maxvallen, "(\'%s\'", val->valuestring);
			} else if (DB_COL_TYPE_JSON == colinfo.type) {
				stream = NULL;
				stream = cJSON_Print(val);
				cJSON_Minify(stream);
				snprintf(values, maxvallen, "(\'%s\'", stream);
				free(stream);
				stream = NULL;
			} else {
				snprintf(values, maxvallen, "(%d", val->valueint);
			}
		} else {
			if (DB_COL_TYPE_STRING == colinfo.type) {
				snprintf(values + strlen(values), maxvallen - strlen(values), ",\'%s\'", val->valuestring);
			} else if (DB_COL_TYPE_JSON == colinfo.type) {
				stream = NULL;
				stream = cJSON_Print(val);
				cJSON_Minify(stream);
				snprintf(values + strlen(values), maxvallen - strlen(values), ",\'%s\'", stream);
				free(stream);
				stream = NULL;
			} else {
				snprintf(values + strlen(values), maxvallen - strlen(values), ",%d", val->valueint);
			}
		}
	}
}
typedef struct {
	char* comparestr;
	int   coltype;
} STRDBCompareType;

STRDBCompareType db_filter_compare[] = {
	{"=", DB_COL_TYPE_INT},		{">=", DB_COL_TYPE_INT},   {"<=", DB_COL_TYPE_INT},		 {">", DB_COL_TYPE_INT},
	{"<", DB_COL_TYPE_INT},		{"<>", DB_COL_TYPE_INT},   {"=", DB_COL_TYPE_BOOL},		 {"=", DB_COL_TYPE_JSON},
	{"like", DB_COL_TYPE_JSON}, {"=", DB_COL_TYPE_STRING}, {"like", DB_COL_TYPE_STRING},
};
BOOL get_db_filters(char* filter, U32 filtersize, cJSON* jsonfilter, char* filterstr, const STRDBDefs colinfo[], const int colinfonum)
{
	if (NULL == filter)
		return FALSE;
	int	i;
	int	colinfoid;
	BOOL   valid = TRUE;
	BOOL   comparevalid;
	cJSON *jsonitem, *jsoncolname, *jsoncompare, *jsonvalue;

	filter[0] = 0;
	if (NULL != jsonfilter) {
		jsonitem = NULL;
		jsonitem = jsonfilter->child;
		while (NULL != jsonitem) {
			colinfoid   = -1;
			jsoncolname = cJSON_GetObjectItem(jsonitem, "column");
			jsoncompare = cJSON_GetObjectItem(jsonitem, "compare");
			jsonvalue   = cJSON_GetObjectItem(jsonitem, "value");
			if ((NULL != jsoncolname) && (NULL != jsoncolname->valuestring) && (NULL != jsoncompare) && (NULL != jsoncompare->valuestring) &&
				(NULL != jsonvalue) && (NULL != jsonvalue->valuestring)) {
				/// DEBUGP("begin to get message 111 column=%s, compare=%s,
				/// value=%s\n", jsoncolname->valuestring,
				/// jsoncompare->valuestring, jsonvalue->valuestring);//,
				/// filterstr);
				for (i = 0; i < colinfonum; i++) {
					if (0 == strcmp(colinfo[i].colname, jsoncolname->valuestring)) {
						//?ж???Ч??
						colinfoid = i;
						break;
					}
				}
			}

			if (FALSE == valid) {
				break;
			} else {
				if (colinfoid >= 0) {
					comparevalid = FALSE;
					for (i = 0; i < sizeof(db_filter_compare) / sizeof(STRDBCompareType); i++) {
						// DEBUGP("begin to get filter compare %s, %s, %d,
						// %d,
						// %d\n", jsoncompare->valuestring,
						// db_filter_compare[i].comparestr,
						// colinfo[colinfoid].type,
						// db_filter_compare[i].coltype, i);

						if ((0 == strcmp(jsoncompare->valuestring, db_filter_compare[i].comparestr)) &&
							(colinfo[colinfoid].type == db_filter_compare[i].coltype)) {
							comparevalid = TRUE;
						}
					}
					if (FALSE == comparevalid) {
						valid = FALSE;
					}
				} else {
					valid = FALSE;
				}
			}
			if (TRUE == valid) {
				if ((DB_COL_TYPE_JSON == colinfo[colinfoid].type) || (DB_COL_TYPE_STRING == colinfo[colinfoid].type)) {
					if (0 == strcmp(jsoncompare->valuestring, "like")) {
						if (filter[0] == 0) {
							snprintf(filter, filtersize, "where %s like \"%s\"", jsoncolname->valuestring, jsonvalue->valuestring);
						} else {
							snprintf(filter + strlen(filter), filtersize - strlen(filter), " and %s like \"%s\"", jsoncolname->valuestring,
									 jsonvalue->valuestring);
						}
					} else {
						if (filter[0] == 0) {
							snprintf(filter, filtersize, "where %s%s\"%s\"", jsoncolname->valuestring, jsoncompare->valuestring, jsonvalue->valuestring);
						} else {
							snprintf(filter + strlen(filter), filtersize - strlen(filter), " and %s%s\"%s\"", jsoncolname->valuestring,
									 jsoncompare->valuestring, jsonvalue->valuestring);
						}
					}
				} else {
					if (filter[0] == 0) {
						snprintf(filter, filtersize, "where %s%s%s", jsoncolname->valuestring, jsoncompare->valuestring, jsonvalue->valuestring);
					} else {
						snprintf(filter + strlen(filter), filtersize - strlen(filter), " and %s%s%s", jsoncolname->valuestring, jsoncompare->valuestring,
								 jsonvalue->valuestring);
					}
				}
			}
			jsonitem = jsonitem->next;
		}
	}
	if (TRUE == valid) {
		if (NULL != filterstr) {
			if (filter[0] == 0) {
				snprintf(filter, filtersize, "where %s", filterstr);
			} else {
				snprintf(filter + strlen(filter), filtersize - strlen(filter), " and %s", filterstr);
			}
		}
	}
	return (valid);
}

int get_record_array_bysql(DBHANDLE handle, char* sql, STRDBSelectInfo selectinfo[], int columnsize, int columnids[], cJSON* jsonarray)
{
	if (0 > handle || NULL == sql || NULL == jsonarray)
		return -1;

	int	res = 0;
	cJSON *jsonitem, *jsontemp;
	int	i	 = 0;
	int	j	 = 0;
	int	ncol  = 0;
	int	nrow  = 0, index;
	char** array = NULL;
	int	idp   = 0;
	db_sql_query(handle, sql, &array, &ncol, &nrow);
	DEBUGP("select sql=%s, %d\n", sql, nrow);
	for (i = 0; i < nrow; i++) {
		index	= (i + 1) * ncol;
		jsonitem = cJSON_CreateObject();
		for (j = 0; j < columnsize; j++) {
			if (NULL == columnids)
				idp = j;
			else
				idp = columnids[j];

			if (NULL != array[index + j]) {
				if (DB_COL_TYPE_INT == selectinfo[idp].coltype) {
					cJSON_AddNumberToObject(jsonitem, selectinfo[idp].displayname, atol(array[index + j]));
				} else if (DB_COL_TYPE_STRING == selectinfo[idp].coltype) {
					cJSON_AddStringToObject(jsonitem, selectinfo[idp].displayname, array[index + j]);
				} else if (DB_COL_TYPE_JSON == selectinfo[idp].coltype) {
					jsontemp = cJSON_Parse(array[index + j]);
					if (NULL != jsontemp) {
						cJSON_AddItemToObject(jsonitem, selectinfo[idp].displayname, jsontemp);
					}
				}
			}
		}
		cJSON_AddItemToArray(jsonarray, jsonitem);
		res++;
	}
	return (res);
}
BOOL get_barejidaccount(char* fulljid, char* jidaccount, int maxlen)
{
	if (NULL == fulljid || NULL == jidaccount)
		return FALSE;

	char* pTmp = strchr(fulljid, '@');
	int   len;
	jidaccount[0] = 0;
	if (pTmp != NULL) {
		len = pTmp - fulljid;
		if (len < (maxlen - 1)) {
			memcpy(jidaccount, fulljid, len);
			jidaccount[len] = 0;
		}
	}
	return (TRUE);
}
BOOL set_json_attributes_bypath(cJSON* json, char* path, void* value, int datatype)
{
	BOOL  res = FALSE;
	char  pathnode[256];
	char *pos, *lpos;

	int len;
	if (NULL == json) {
		return res;
	}
	cJSON* nodes = json;
	cJSON* item;
	lpos = path;
	while (lpos != NULL) {
		pathnode[0] = 0;
		pos			= strchr(path, '/');
		if (pos == NULL) {
			snprintf(pathnode, sizeof(pathnode), "%s", lpos);
		} else {
			len = pos - lpos;
			if (len < (sizeof(pathnode) - 1)) {
				memcpy(pathnode, lpos, len);
				pathnode[len] = 0;
			}
		}
		if (pathnode[0] == 0) {
			break;
		} else {
			if (pos == NULL) {
				item = cJSON_GetObjectItem(nodes, pathnode);
				if (NULL != item) {
					cJSON_DeleteItemFromObject(nodes, pathnode);
				}
				if (datatype == DB_COL_TYPE_STRING) {
					cJSON_AddStringToObject(nodes, pathnode, (char*) value);
				} else if (datatype == DB_COL_TYPE_INT) {
					cJSON_AddNumberToObject(nodes, pathnode, (int) value);
				}
				break;
			} else {
			}
			lpos = pos;
		}
	}

	return res;
}
