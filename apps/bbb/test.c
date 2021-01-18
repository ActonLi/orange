#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>

#define return_if_fail(p) if((p) == 0){printf ("[%s]:func error!/n", __func__);return NULL;}

typedef struct priv_info {
    sem_t s1;
    sem_t s2;
    time_t end_time;
} priv_info_t;

static void* info_init(struct priv_info* thiz) 
{
    return_if_fail(thiz);
    thiz->end_time = time(NULL) + 10;
    sem_init(&thiz->s1, 0, 1);
    sem_init(&thiz->s2, 0, 0);
    return NULL;
}

static void* info_destroy(struct priv_info* thiz)
{
    return_if_fail(thiz);

    sem_destroy(&thiz->s1);
    sem_destroy(&thiz->s2);
    free(thiz);
    thiz = NULL;

    return NULL;
}

static void* pthread_func_1(struct priv_info* thiz)
{
    return_if_fail(thiz);

    while(time(NULL) < thiz->end_time) {
        sem_wait(&thiz->s2);
        printf("%s:%d lock\n", __func__, __LINE__);
        sem_post(&thiz->s1);
        printf("%s:%d unlock\n", __func__, __LINE__);
        sleep(1);
    }

    return NULL;
}

static void* pthread_func_2(struct priv_info* thiz)
{
    return_if_fail(thiz);

    while(time(NULL) < thiz->end_time) {
        sem_wait(&thiz->s1);
        printf("%s:%d lock\n", __func__, __LINE__);
        sem_post(&thiz->s2);
        printf("%s:%d unlock\n", __func__, __LINE__);
        sleep(1);
    }

    return NULL;
}

int main(int argc, char** argv)
{

    pthread_t pt_1 = 0;
    pthread_t pt_2 = 0;

    int ret = 0;
    struct priv_info* thiz = NULL;

    thiz = (struct priv_info*)malloc(sizeof(struct priv_info));
    if (NULL == thiz) {
        return -1;
    } 

    info_init(thiz);

    ret = pthread_create(&pt_1, NULL, (void*)pthread_func_1, thiz);
    if (0 != ret) {
        perror("thread 1 create error");
    }

    ret = pthread_create(&pt_2, NULL, (void*)pthread_func_2, thiz);
    if (0 != ret) {
        perror("thread 2 create error");
    }

    pthread_join(pt_1, NULL);
    pthread_join(pt_2, NULL);
    info_destroy(thiz);

    return 0;

    unsigned int arr[8] = {73, 74, 75, 71, 69, 72, 76, 80};
    int i, j;
    int count;
    int find;

    int pe = 0, pt = 0;

    while (pe < 8) {
        for (pt = pe; pt < 8; pt++) {
            count++;
            if (arr[pt] > arr[pe]) {
                break;
            }
        }
        pe++;
    }


    for (i = 0; i < 8; i++) {
        count = 0;
        find = 0;
        for (j = i + 1; j < 8; j++) {
            count++;
            if (arr[j] > arr[i]) {
                find = 1;
                break;
            }
        }

        if (find == 0) {
            count = 0;
        }

        printf("%d ", count);
    }

    printf ("\n");

    return 0;
}


