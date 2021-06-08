#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>


typedef struct dns_message {
    unsigned short     id;
    unsigned short     qtype;
    unsigned short     qclass;
    unsigned short     msglen;

    unsigned int       malformed : 1;
    unsigned int       qr : 1;
    unsigned int       rd1 : 1; /* set if RECUSION DESIRED bit is set */
    unsigned int       rd2 : 1; /* set if RECUSION DESIRED bit is set */
    unsigned int       rd3 : 1; /* set if RECUSION DESIRED bit is set */
    unsigned int       rd4 : 1; /* set if RECUSION DESIRED bit is set */
    unsigned int       rd5 : 1; /* set if RECUSION DESIRED bit is set */
    unsigned int       rd6 : 1; /* set if RECUSION DESIRED bit is set */
    unsigned int       rd7 : 1; /* set if RECUSION DESIRED bit is set */
    unsigned int       rd8 : 1; /* set if RECUSION DESIRED bit is set */
    unsigned int       rd9 : 1; /* set if RECUSION DESIRED bit is set */
    unsigned int       rd10 : 1; /* set if RECUSION DESIRED bit is set */
    unsigned int       rd11 : 1; /* set if RECUSION DESIRED bit is set */
    unsigned int       rd12 : 1; /* set if RECUSION DESIRED bit is set */
} dns_message_t; 

typedef struct dns_message2 {
    unsigned short     id;
    unsigned short     qtype;
    unsigned short     qclass;
    unsigned short     msglen;

    unsigned int       malformed : 1;
    unsigned int       qr : 1;
    unsigned int       rd : 1; /* set if RECUSION DESIRED bit is set */
    unsigned int       aa : 1; /* set if AUTHORITATIVE ANSWER bit is set */
    unsigned int       tc : 1; /* set if TRUNCATED RESPONSE bit is set */
    unsigned int       ad : 1; /* set if AUTHENTIC DATA bit is set */
} dns_message2_t; 

typedef struct dns_message1 {
    unsigned short     id;
    unsigned short     qtype;
    unsigned short     qclass;
    unsigned short     msglen;

    unsigned int       malformed;
    unsigned int       qr;
    unsigned int       rd; /* set if RECUSION DESIRED bit is set */
    unsigned int       aa; /* set if AUTHORITATIVE ANSWER bit is set */
    unsigned int       tc; /* set if TRUNCATED RESPONSE bit is set */
    unsigned int       ad; /* set if AUTHENTIC DATA bit is set */
} dns_message1_t; 

typedef void (*printfunc_t)(int);

static void __print(int ret)
{
    printf("%s:%d ret: %d\n", __func__, __LINE__, ret);
    return;
}

static void* thread1(void *arg)
{
    printfunc_t printfunc = (printfunc_t)arg; 

    printfunc(1);

    return NULL;
}

static void* thread2(void *arg)
{
    printfunc_t printfunc = (printfunc_t)arg; 

    printfunc(2);

    return NULL;
}

static void* thread3(void *arg)
{
    printfunc_t printfunc = (printfunc_t)arg; 

    printfunc(3);

    return NULL;
}

#define LEN_MAX 128
#define CNT_MAX 8

typedef struct queue_info {
    int cnt;
    char (*data)[LEN_MAX];
} queue_info_t; 

#define SQL_DATA_LEN_MAX 8192
#define SQL_DATA_DATA_MAX 256

typedef struct dataEntry {
    int iModuleType;
    int usDataLen;
    char arrData[SQL_DATA_LEN_MAX];
} TdataEntry;

typedef struct dataCache {
    unsigned int uiCnt;
    unsigned int timestamp; 
    struct dataEntry* tData;
} TdataCache;

static struct dataCache g_dataCache; 

static int updateCompareUTCTimeStringWithCurrentTime(char* utcTimeString)
{
    time_t now = time(&now);
    struct tm* ptm = gmtime(&now);
    char currentTimeString[64] = {0};

    snprintf(currentTimeString, sizeof(currentTimeString), "%.4d-%.2d-%.2d %.2d:%.2d:%.2d", 
            ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, 
            ptm->tm_hour, ptm->tm_min, ptm->tm_sec);


    return strcmp(utcTimeString, currentTimeString);
}

static char* ConvertUTCToLocalTime(const char* src_time, char* dst_time)
{
    struct tm t;
    char* dst = NULL;
    struct tm* tTime = NULL;

    if (NULL == src_time || NULL == dst_time) {
        goto exit;
    }

    printf("%s:%d src time: %s\n", __func__, __LINE__, src_time);

    memset(&t, 0, sizeof(t));
    t.tm_year = atoi(src_time) - 1900;
    t.tm_mon = atoi(src_time + 5) - 1;
    t.tm_mday = atoi(src_time + 8);
    t.tm_hour = atoi(src_time + 11);
    t.tm_min = atoi(src_time + 14);
    t.tm_sec = atoi(src_time + 17);

    time_t tt = mktime(&t);
    if (-1 == tt) {
        goto exit;
    }

    time_t now = time(&now);

    struct tm* ptm = gmtime(&now);

    struct timeval tv;
    struct timezone tz;
    gettimeofday (&tv, &tz);
    struct tm* xx = gmtime(&tv.tv_sec);

    printf("UTC time: %s", asctime(ptm));
    printf("UTC time1: %s", asctime(xx));

    sprintf(dst_time, "%.4d-%.2d-%.2d %.2d:%.2d:%.2d", 
            ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, 
            ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
    printf("%s:%d utc time: %s\n", __func__, __LINE__, dst_time);

    tTime = localtime(&tt); 
    sprintf(dst_time, "%.4d-%.2d-%.2d %.2d:%.2d:%.2d", 
            tTime->tm_year + 1900, tTime->tm_mon + 1, tTime->tm_mday, 
            tTime->tm_hour, tTime->tm_min, tTime->tm_sec);

    printf("%s:%d dst time time: %s\n", __func__, __LINE__, dst_time);
    dst = dst_time;
exit:
    return dst;
}

static void remove_space_and_tab(char *buf)
{
    char ch;
    char *src_ptr;
    char *dst_ptr;
    int  head = 1;
    int bracket = 0;

    src_ptr = dst_ptr = buf;   
    ch = *src_ptr;
    while(ch != '\0'){
        if(head)
        {            
            if(ch == '\t' || ch == ' ')
            {
                src_ptr++;
                ch = *src_ptr;
                continue;
            }
        }
        if(!bracket){
            if(ch == '\t' || ch == ' '){
                src_ptr++;
                ch = *src_ptr;
                continue;
            }
        }
        if(ch == '[')
        {
            bracket++;

        }

        if(ch == ']'){
            bracket--;
        }

        if(ch != 0x0a && ch != 0x0d)
        {
            *dst_ptr = ch;
            dst_ptr++;
            head = 0;
        }
        src_ptr++;
        ch = *src_ptr;
    }
    *dst_ptr = 0;
}

static int DeleteDir(const char* path) 
{

    DIR* dp = NULL;
    DIR* dpIn = NULL;

    printf("%s:%d dir: %s\n", __func__, __LINE__, path);

    char pathName[512];
    struct dirent* pDir;

    if (NULL == path) {
        return -1;
    }

    dp = opendir(path);
    if (NULL == dp) {
        return -1;
    }

    while(NULL != (pDir = readdir(dp))) {
        if (0 == strcmp(pDir->d_name, "..") || 0 == strcmp(pDir->d_name, ".")) {
            continue;
        }

        snprintf(pathName, sizeof(pathName), "%s/%s", path, pDir->d_name);
        dpIn = opendir(pathName);
        if (NULL != dpIn) {
            DeleteDir(pathName);
        } else {
            unlink(pathName);
        }
        closedir(dpIn);
        dpIn = NULL;
    }

    rmdir(path);
    closedir(dp);

    return 0;
}

int main(int argc, char** argv)
{
    char strs[3][64] = {"apple", "apple", "banana"};
    pthread_t id1, id2, id3;

    char localtime[64] = "";
    char line[128] = "xxxx";

    printf("line: %s\n", line);

    memset(&line, 0, 128);

    printf("line: %s\n", line);

    unlink(argv[1]);
    exit(0);

    DeleteDir(argv[1]);

    exit(0);

    if (updateCompareUTCTimeStringWithCurrentTime(argv[1]) <= 0) {
        printf("time is up\n");
    }
    else {
        printf("time is not up\n");
    }
    printf("localtime: %s\n", ConvertUTCToLocalTime(argv[1], localtime));

    exit(0);

    pthread_create(&id1, NULL, (void *)thread1, (void *)__print);
    pthread_create(&id2, NULL, (void *)thread2, (void *)__print);
    pthread_create(&id3, NULL, (void *)thread3, (void *)__print);

    pthread_join(id1, NULL);
    pthread_join(id2, NULL);
    pthread_join(id3, NULL);
    int i, j;
    int count;

    g_dataCache.tData = (struct dataEntry *)malloc(SQL_DATA_DATA_MAX * sizeof(struct dataEntry));
    if (g_dataCache.tData) {
        for (i = 0; i < SQL_DATA_DATA_MAX; i++) {
            g_dataCache.tData[i].iModuleType = 1;
            g_dataCache.tData[i].usDataLen = snprintf(g_dataCache.tData[i].arrData, SQL_DATA_LEN_MAX, "dsaffffffffsadf_%d", i+1);
            g_dataCache.uiCnt += 1;
        }

        for (i = 0; i < g_dataCache.uiCnt; i++) {
            printf("%s:%d length: %d, type: %d, data: %s\n", __func__, __LINE__, 
                    g_dataCache.tData[i].usDataLen, g_dataCache.tData[i].iModuleType, g_dataCache.tData[i].arrData);
        }
    }

    struct queue_info qi;
    qi.data = (char(*)[LEN_MAX])malloc(LEN_MAX * CNT_MAX);
    printf("%s:%d sizeof(data): %lu\n", __func__, __LINE__, sizeof(qi.data));

    for (i = 0; i < CNT_MAX; i++) {
        snprintf(qi.data[i], LEN_MAX, "orange-%d", i + 1);
    }
     
    for (i = 0; i < CNT_MAX; i++) {
        printf("%s\n", qi.data[i]);
    }
     
    for (i = 0; i < 3; i++) {
        count = 0;
        for (j = 0; j < 3; j++) {
            if (i == j) {
                continue;
            }
            if (strcmp(strs[i], strs[j]) == 0) {
                count++;
            }
        }
        if (count == 0) {
            printf("%s\n", strs[i]);
        }
    }

    struct dns_message2  xxx;

    xxx.malformed  = 0;
    xxx.qr  = 1;
    xxx.rd  = 0;

    printf("malformed: %d, qr: %d, rd: %d\n", xxx.malformed, xxx.qr, xxx.rd);

    printf("sizeof(struct dns_message): %lu\n", sizeof(struct dns_message));
    printf("sizeof(struct dns_message1): %lu\n", sizeof(struct dns_message1));
    printf("sizeof(struct dns_message2): %lu\n", sizeof(struct dns_message2));

    return 0;
}
