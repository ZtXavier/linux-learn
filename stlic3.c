#include"libcal.h"



void *consumer()
{
    for(;;)
    {
        sleep(2);
        P(full_sem);
        sem_wait(&co_sem);
        pthread_mutex_lock(&mutex);

        out = out % M;
        printf("%d(-)consume a product. buffer:",gettid());

        buff[out] = 0;
        print();
        ++out;

        pthread_mutex_unlock(&mutex);
        V(empty_sem);
        sem_post(&co_sem);
    }
    
}