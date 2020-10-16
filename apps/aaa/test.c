#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/time.h>


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
    

    return 0;
}
