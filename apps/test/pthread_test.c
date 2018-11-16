#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

pthread_t thread_a;
pthread_t thread_b;
pthread_cond_t cond;
pthread_mutex_t mutex_a;
pthread_mutex_t mutex_b;

void* __thread_func_a(void *arg) 
{
    pthread_mutex_lock(&mutex_a);
    printf("%s:%d\n", __func__, __LINE__);
    while(1) {
        printf("%s:%d\n", __func__, __LINE__);
        pthread_cond_wait(&cond, &mutex_a);
        printf("%s:%d\n", __func__, __LINE__);
    }
    printf("%s:%d\n", __func__, __LINE__);
    pthread_mutex_unlock(&mutex_a);
}

void* __thread_func_b(void *arg) 
{
    static int a = 0;
    pthread_mutex_lock(&mutex_b);
    printf("%s:%d\n", __func__, __LINE__);
    while(1) {
        printf("%s:%d\n", __func__, __LINE__);
        sleep(2);
        a++;
        if (a % 5 == 0) {
            printf("%s:%d cond signal\n", __func__, __LINE__);
            pthread_cond_signal(&cond);
        }
        printf("%s:%d\n", __func__, __LINE__);
    }
    printf("%s:%d\n", __func__, __LINE__);
    pthread_mutex_unlock(&mutex_b);

}

int main(int argc, char **argv) 
{
    pthread_mutex_init(&mutex_a, NULL);
    pthread_mutex_init(&mutex_b, NULL);
    pthread_cond_init(&cond, NULL);

    pthread_create(&thread_a, NULL, __thread_func_a, NULL);
    pthread_create(&thread_b, NULL, __thread_func_b, NULL);

    pthread_join(thread_a, NULL);
    pthread_join(thread_b, NULL);


    return 0;
}
