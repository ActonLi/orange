
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/time.h>
#include <arpa/inet.h>


#define TEST 13

#include "stdio.h"
#include "string.h"
#include "stdlib.h"


/*
static void __qrcode_scanner_rotate_270(unsigned char* src, int width, int height)
{
    int copy_bytes = 4;
    int bytes_perline = width << 1;

    printf("<==========================%s:%d size: %d===================================>\n", __func__, __LINE__, bytes_perline * height);

    int step = height << 2;
    int offset = bytes_perline - copy_bytes;

    unsigned char* dst = (unsigned char*)malloc(bytes_perline * height);
    if (NULL == dst) {
        return;
    }

    unsigned char* dest = dst;
    unsigned char* source = src;

    unsigned char* psrc = NULL;
    unsigned char* pdst[2] = {NULL, NULL};
    int i, j, k;
    unsigned char temp;

    for (i = 0; i < bytes_perline; i += copy_bytes) {
        pdst[1] = dest;
        pdst[0] = dest + (height << 1);
        psrc = source + offset;

        for (j = 0; j < height; ++j) {
            k = j % 2;
            *((unsigned int *)pdst[k]) = *((unsigned int *)psrc);
            if (1 == k) {
                temp = *(pdst[1] + 1);
                *(pdst[1] + 1) = *(pdst[0] - 1);
                *(pdst[0] - 1) = temp;
            }
            pdst[k] += copy_bytes;
            psrc += bytes_perline;
        }

        dest += step;
        source -= copy_bytes;
    }

    memcpy(src, dst, width * height * 2);
    printf("<==========================%s:%d size: %d===================================>\n", __func__, __LINE__, width * height * 2);

    free(dst);

    return; 
}

static void __qrcode_scanner_rotate_90(unsigned char* src, int width, int height)
{
    int copy_bytes = 4;
    int bytes_perline = width << 1;

    int step = height << 2;
    int offset = (height - 1)* bytes_perline;

    unsigned char* dst = (unsigned char*)malloc(bytes_perline * height);
    if (NULL == dst) {
        return;
    }

    unsigned char* dest = dst;
    unsigned char* source = src;

    unsigned char* psrc = NULL;
    unsigned char* pdst[2] = {NULL, NULL};
    int i, j, k;
    unsigned char temp;

    for (i = 0; i < bytes_perline; i += copy_bytes) {
        pdst[0] = dest;
        pdst[1] = dest + (height << 1);
        psrc = source + offset;

        for (j = 0; j < height; ++j) {
            k = j % 2;
            *((unsigned int *)pdst[k]) = *((unsigned int *)psrc);
            if (1 == k) {
                temp = *(pdst[0] - 1);
                *(pdst[0] - 1) = *(pdst[1] + 1);
                *(pdst[1] + 1) = temp;
            }
            pdst[k] += copy_bytes;
            psrc -= bytes_perline;
        }

        dest += step;
        source += copy_bytes;
    }

    memcpy(src, dst, width * height * 2);
    free(dst);

    return; 
    }
*/

#define SENDSIGTIME 10

pthread_cond_t g_cond;
pthread_mutex_t g_mutex;

static void* thread1(void *arg)
{
    //int __attribute__((unused)) inArg = (int)arg;
    int ret = 0;
    struct timeval now;
    struct timespec outtime;

    pthread_mutex_lock(&g_mutex);

    gettimeofday(&now, NULL);
    outtime.tv_sec = now.tv_sec + 15;
    outtime.tv_nsec = now.tv_usec * 1000;

    printf("thread1 begin %s:%d .\n", __func__, __LINE__);

    ret = pthread_cond_timedwait(&g_cond, &g_mutex, &outtime);
    pthread_mutex_unlock(&g_mutex);

    printf("thread 1 running ret: %d\n", ret);

    return NULL;
}

static int test13(int argc, char** argv)
{
    pthread_t id1;
    int ret;
    int a = -49;
    int b = ntohl(a);
    int c = ntohl(b);
    short a1 = -1;
    unsigned short a2 = ntohl(a1);
    short a3 = a2;
    int a4 = a2;
    int a5 = a3;
    char *str = "-100";
    int num = atoi(str);


    printf("a: %d, b: %d, c: %d, num: %d\n", a, b, c, num);
    printf("a1: %d, a2: %d a3: %d, a4: %d, a5: %d\n", a1, a2, a3, a4, a5);

    pthread_cond_init(&g_cond, NULL);
    pthread_mutex_init(&g_mutex, NULL);

    ret = pthread_create(&id1, NULL, (void *)thread1, (void *)1);
    if (0 != ret)
    {
        printf("thread 1 create failed!\n");
        return 1;
    }

    printf("等待%ds发送信号!\n", SENDSIGTIME);
    int i = SENDSIGTIME;
    while (i > 0) {
        printf("%s:%d i: %d \n", __func__, __LINE__, i);
        sleep(1);
        i--;
    }
    printf("正在发送信号....\n");
    pthread_mutex_lock(&g_mutex);
    pthread_cond_signal(&g_cond);
    pthread_mutex_unlock(&g_mutex);


    pthread_join(id1, NULL);

    pthread_cond_destroy(&g_cond);
    pthread_mutex_destroy(&g_mutex);

    return 0;
}

static int test12(int argc, char** argv)
{
    unsigned int N, m;
    int ret = scanf("%u, %u", &N, &m);
    if (ret <= 0) {
        printf("%s:%d error.\n", __func__, __LINE__);
    }

    unsigned int (*data)[3];
    data = (unsigned int (*)[3])malloc(m*3 * sizeof(unsigned int));
    int i;
    for (i = 0; i < m; i++) {
        ret = scanf("%u %u %u", &data[i][0], &data[i][1], &data[i][2]);
    }

    for (i = 0; i < m; i++) {
        printf("%u %u %u", data[i][0], data[i][1], data[i][2]);
    }

    int max = 0, j;
    int __attribute__((unused))value = 0;

    for (i = 0; i < m; i++) {
        for(j = i + 1; j < m; j++) {
            printf("%u N: %u\n", data[i][0]+data[j][0] , N);
            if (data[i][0]+data[j][0] <= N) {
                if (data[i][0]*data[i][1] + data[j][0]*data[j][1] > max) {
                    max = data[i][0]+data[j][0];
                    value = data[i][0]*data[i][1] + data[j][0]*data[j][1];
                }
            }
        }
    }
    printf("%d\n", max);
    return 0;
}

static int test11(int argc, char** argv)
{
    int count;
    char  (*word)[64];
    int __attribute__((unused)) ret = 0;
    ret = scanf("%d", &count);

    word = (char(*)[64])malloc(64 * count);
    memset(word, 0, 64 * count);
    int i;
    for (i = 0; i < count; i++) {
        memset(word[i], 0, 64);
        ret = scanf("%s", word[i]);
        printf("%s\n", word[i]);
    }

    return 0;
}

static char* reverse(char* sentense)
{
    if (NULL == sentense) {
        return NULL;
    }

    int len = strlen(sentense);
    printf("sentense: %s, len:%d\n", sentense, len);
    int word_len = 0;
    char* p = (char*)malloc(len + 1);
    char* end = sentense + len;
    char* pos;
    char* q = p;
    int i;
    for (i = len - 1; i >= 0; i--) {
        printf("index: %d, char: %c\n", i, sentense[i]);
        if (sentense[i] == ' ' || i == 0) {
          if (i == 0) {
              pos = sentense;
          } else {
              pos = sentense + i + 1;
          }
          word_len = end - pos;
          end = sentense + i;
          memcpy(q, pos, word_len);
            if (i == 0) {
                *(q+word_len) = '\0';
            } else {
                 *(q+word_len) = ' ';
            }
            q += (word_len + 1);
        }
    }
    return p;
}

static int test10(int argc, char** argv)
{
    char a[2048] = "";

    char* __attribute__((unused)) ret = NULL;

    ret = fgets(a, 2048, stdin);

    char* b = reverse(a);
    if (b) {
        printf("%s\n", b);
        free(b);
        b = NULL;
    }

    return 0;
}

static int test9(int argc, char** argv)
{
    char input[1024] = {0};
    int len, i, j, k;
    int tmp;

    while (scanf("%s", input) != EOF) {
        len = strlen(input);

        for (i = 0; i < len / 2; i++) {
            tmp = input[i];
            input[i] = input[len - i - 1];
            input[len-i-1] = tmp;
        }

        printf("len: %d\n", len);
        for (i = 0; i < len; i++) {
            for (j = i + 1; j < len; j++) {
                if (input[i] == input[j]) {
                    for (k = j; k < len - 1; k++) {
                        input[k] = input[k+1];
                    }
                    j--;
                    len--;
                }
            }
        }

        for (i = 0; i < len; i++) {
            printf("%c", input[i]);
        }

        memset(input, 0, 1024);
    }
    
    return 0;
}

typedef struct key_value {
    int key;
    int value;
} key_value_t;

static int test8(int argc, char** argv)
{
    int i, j, k;
    int count = 0;
    struct key_value* data;
    struct key_value tmp;
    int __attribute__((unused)) ret = 0;

    while (scanf("%d", &count) != EOF) {
        data = malloc(count * sizeof(struct key_value));
        for (i = 0; i < count; i++) {
            ret = scanf("%d %d", &((data + i)->key), &((data + i)->value));
        }
        
        for (i = 0; i < count; i++) {
            for (j = i + 1; j < count; j++) {
                if ((data+i)->key > (data + j)->key) {
                    tmp.key = (data + i)->key;
                    tmp.value = (data + i)->value;
                    (data + i)->key = (data + j)->key;
                    (data + i)->value = (data + j)->value;
                    (data + j)->key = tmp.key;
                    (data + j)->value = tmp.value;
                }
            }
        }

        for (i = 0; i < count; i++) {
            for (j = i + 1; j < count; j++) {
                if ((data+i)->key == (data+j)->key) {
                    (data+i)->value += (data+j)->value;
                    for (k = j; k < count - 1; k++) {
                        (data+k)->key = (data+k+1)->key;
                        (data+k)->value = (data+k+1)->value;
                    }
                    j--;
                    count--;
                }
            }
        }

        for (i = 0; i < count; i++) {
            printf("%d %d\n", (data+i)->key, (data+i)->value);
        }
    }

    return 0;
}

static int test7(int argc, char** argv)
{
    float x = 0.0;

    while (scanf("%f", &x) != EOF) {
        if ((int)(x + 0.5) < (int)(x + 1)) {
            printf("%d", (int)x);
        }
        else {
            printf("%d", (int)x + 1);
        }
    }

    return 0;
}

static int test6(int argc, char** argv)
{
    unsigned long int in = 0;
    unsigned long int s = 0;
    unsigned long int fac[1024];
    unsigned long int i;
    

    while (scanf("%lu", &in) != EOF) {
        memset(fac, 0, sizeof(unsigned long int) * 1024);
        s = in;
        for (i = 2; i * i <= s; i++) {
            while (in % i == 0) {
                fac[++(fac[0])] = i;
                in /= i;
            }
        }
        if (in > 1) {
            fac[++(fac[0])] = in;
        }

        for (i = 1; i <= fac[0]; i++) {
            printf("%lu ", fac[i]);
        }
    } 

    return 0;
}

static int test5(int argc, char** argv) 
{
    char hexs[8] = {0};
    char *p;
    int len;
    int total;
    int i;

    while (scanf("%s", hexs) != EOF) {
        len = strlen(hexs);

        if (len > 2 && *hexs == '0' && (*(hexs+1) == 'x' || *(hexs+1) == 'X')) {
            p = hexs + 2;
            len -= 2;
        }
        else {
            p = hexs;
        }

        total = 0;

        for (i = len - 1; i >= 0; i--) {
            if(*(p+i) >= '0' && *(p+i) <= '9') {
                total += (*(p+i) - 48)*pow(16, len - i - 1);
            } else if(*(p+i) >= 'A' && *(p+i) <= 'F') {
                total += (*(p+i) - 55)*pow(16, len - i - 1);
            } else if (*(p+i) >= 'a' && *(p+i) <= 'f') {
                total += (*(p+i) - 87)*pow(16, len - i - 1);
            }
        } 

        printf("%d\n", total);

        memset(hexs, 0, 8);
    }

    return 0;
}

static int test4(int argc, char** argv) 
{
    char str[128] = {0};
    int len = 0;
    int i, j;
    int pos;

    while (scanf("%s", str) != EOF) {
        len = strlen(str);
        pos = 0;

        while (len > 0) {
            for (i = 0; i < 8 && i < len; i++) {
                printf("%c", str[pos++]);
            }

            if (i < 8) {
                for (j = 0; j < 8 - i; j++) {
                    printf("0");
                }
            }
            len -= i;
            printf("\n");
        }
        
        memset(str, 0, 128);
    }

    return 0;
}

static int test3(int argc, char** argv)
{
    int count;
    int i,j;
    int tmp;
    int __attribute__((unused))ret = 0;

    while (scanf("%d", &count) != EOF) {
        int* p = malloc(count * sizeof(int));
        if (p) {
            memset(p, 0, count * sizeof(int));
        }
        for (i = 0; i < count; i++) {
            ret = scanf("%d", p+i);
        }

        for (i = 0; i < count; i++) {
            for (j = i+1; j < count; j++) {
                if (*(p+i) > *(p+j)) {
                    tmp = *(p+i);
                    *(p+i) = *(p+j);
                    *(p+j) = tmp;
                }
            }
        }


        for (i = 0; i < count; i++) {
            if (i == 0) {
                printf("%d\n", *(p+i));
            } else {
                if(*(p+i-1) != *(p+i) ) {
                    printf("%d\n", *(p+i));
                }
            }
        }

        free(p);
    }

    return 0;
}

static int test2(int argc, char** argv)
{
    char buf[1024] = {0};
    char chr = 0;
    char* p = buf;
    int count = 0;

    int ret = scanf("%s %c", buf, &chr);

    printf("buf: %s, chr: %c, ret: %d\n", buf, chr, ret);

    while (*p != 0) {
        if (*p >= 'a' && *p <= 'z') {
            if (*p == chr || (*p - 32) == chr) {
                count++;
            }
        }
        else if (*p >= 'A' && *p <= 'Z') {
            if (*p == chr || (*p + 32) == chr) {
                count++;
            }
        }
        p++;
    }

    printf("count: %d\n", count);


    return 0;
}

static int test1(int argc, char** argv)
{
    int len = 0;
    char c;
    
    while((c = getchar()) != '\n') {
        if(c == ' ') {
            len = 0;
        }
        else {
            len++;
        }
    }
    
    printf( "%d\n", len );
    
    return 0;
}

int main(int argc, char** argv)
{
    int test = TEST;

    switch (test) {
        case 1:
            test1(argc, argv);
            break;
        case 2:
            test2(argc, argv);
            break;
        case 3:
            test3(argc, argv);
            break;
        case 4:
            test4(argc, argv);
            break;
        case 5:
            test5(argc, argv);
            break;
        case 6:
            test6(argc, argv);
            break;
        case 7:
            test7(argc, argv);
            break;
        case 8:
            test8(argc, argv);
            break;
        case 9:
            test9(argc, argv);
            break;
        case 10:
            test10(argc, argv);
            break;
        case 11:
            test11(argc, argv);
            break;
        case 12:
            test12(argc, argv);
        case 13:
            test13(argc, argv);
            break;
    }

	return 0;
}
