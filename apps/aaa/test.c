#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/time.h>


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

int main(int argc, char** argv)
{
    char strs[3][64] = {"apple", "apple", "banana"};
    pthread_t id1, id2, id3;

    pthread_create(&id1, NULL, (void *)thread1, (void *)__print);
    pthread_create(&id2, NULL, (void *)thread2, (void *)__print);
    pthread_create(&id3, NULL, (void *)thread3, (void *)__print);

    pthread_join(id1, NULL);
    pthread_join(id2, NULL);
    pthread_join(id3, NULL);
    int i, j;
    int count;

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
