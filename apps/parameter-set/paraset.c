#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void __usage(void)
{
	fprintf(stderr, "./paraset [filepath] [key1] [key2] [key3] ... [value]\n");
	return;
}


static int __Readfile2Block(const char* strFilepath, char** const p2strReadbuf)
{
    FILE* tFilehandle = NULL;
    long lFilelen = 0;
    const size_t tReadMembCnt = 1;
    int  ret = -1;
    if (0 == strFilepath || 0 == p2strReadbuf) return -1;
    
    *p2strReadbuf = NULL;
    
    if (NULL == (tFilehandle = fopen(strFilepath, "rb")))
    {
        return ret;
    }

    if(0 == fseek(tFilehandle,0L,SEEK_END))
    {
        lFilelen   = ftell(tFilehandle);
        *p2strReadbuf = (char *)malloc(lFilelen + 1);
        
        if(NULL == *p2strReadbuf)  
        {  
            fclose(tFilehandle);
            return ret;
        }

        fseek(tFilehandle,0L,SEEK_SET);  
        if(tReadMembCnt == fread(*p2strReadbuf,lFilelen, tReadMembCnt, tFilehandle))
        {
            (*p2strReadbuf)[lFilelen] = '\0';
            ret = lFilelen + 1;
        }
        else
        {        
        }
    }
    else
    {
    }

    fclose(tFilehandle);
    return ret;    
}

static int __WriteBlock2File(const char* strFilepath, const char* cszWriteBuf, unsigned int uiSize)
{
    FILE* tFilehandle = NULL;
    const size_t tWriteMembCnt = 1;
    int ret = 0;
    if (0 == strFilepath || 0 == cszWriteBuf) return -1;
    if (NULL == (tFilehandle = fopen(strFilepath, "w+")))
    {
        return ret;
    }
    if(0 == fseek(tFilehandle,0L,SEEK_SET))
    {
        if(tWriteMembCnt != fwrite(cszWriteBuf, uiSize, tWriteMembCnt, tFilehandle))
        {        
            ret = -1;
        }
    }
    else
    {
        ret = -1;
    }

    fclose(tFilehandle);
    return ret;
}

int main(int argc, char** argv) 
{
    if (argc < 3) {
        __usage();
        return -1;
    }

    cJSON* root = NULL;
    cJSON* item = NULL;
    cJSON* p_root = NULL;
    char* datastr = NULL;
    int i;


    if (0 < __Readfile2Block(argv[1], &datastr)) {
        root = cJSON_Parse(datastr);
        p_root = root;
        if (NULL != datastr) {
            free(datastr);
        }
    }

    if (NULL != root) {
        for (i = 2; i < argc - 2; i++) {
            item = cJSON_GetObjectItem(p_root, argv[i]);
            if (NULL != item) {
                p_root = item;
            }
        }

        if (NULL != item) {
            cJSON_ReplaceItemInObject(item, argv[argc - 2], cJSON_CreateString(argv[argc - 1]));
        }

        datastr = cJSON_Print(root);
        if (datastr) {
            __WriteBlock2File(argv[1], datastr, strlen(datastr));
            free(datastr);
        }
    }

    return 0;
}
