#include"libcal.h"

void print()
{
    int i;
    for(i = 0; i < M; i++)
        printf("%d ", buff[i]);
    printf("\n");
}

void sem_mutex_init()
{
    /*
     *semaphore initialize
     */
    int init1 = sem_init(&empty_sem, 0, M);
    int init2 = sem_init(&full_sem, 0, 0);
    int num1 = sem_init(&pr_sem,0,1);
    int num2 = sem_init(&co_sem,0,1);
    if((num1 !=0) && (num2 != 0))
    {
        printf("error in ctl pthread\n");
        exit(1);
    }
    if( (init1 != 0) && (init2 != 0))
    {
        printf("sem init failed \n");
        exit(1);
    }
    /*
     *mutex initialize
     */
    int init3 = pthread_mutex_init(&mutex, NULL);
    if(init3 != 0)
    {
        printf("mutex init failed \n");
        exit(1);
    }
    
}

void *producer()
{
    for(;;)
    {
        sleep(1);
        P(empty_sem);
        sem_wait(&pr_sem);
        pthread_mutex_lock(&mutex);

        in = in % M;
        printf("%d(+)produce a product. buffer:",gettid());

        buff[in] = 1;
        print();
        ++in;

        pthread_mutex_unlock(&mutex);
        V(full_sem);
        sem_post(&pr_sem);
    }
   
}

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