#include"libcal.h"
int in = 0;         /*生产者放置产品的位置*/
int out = 0;             /*消费者取产品的位置*/

int buff[M] = {0};     /*缓冲初始化为0， 开始时没有产品*/

sem_t empty_sem;         /*同步信号量，当满了时阻止生产者放产品*/
sem_t full_sem;         /*同步信号量，当没产品时阻止消费者消费*/
pthread_mutex_t mutex; /*互斥信号量， 一次只有一个线程访问缓冲*/
sem_t pr_sem;
sem_t co_sem;


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