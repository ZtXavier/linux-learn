#ifndef _LIBCAL_H_
#define _LIBCAL_H_
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#define M 16             /*缓冲数目*/

#define P(x)     sem_wait(&x)
#define V(x)    sem_post(&x)

int in = 0;         /*生产者放置产品的位置*/
int out = 0;             /*消费者取产品的位置*/

int buff[M] = {0};     /*缓冲初始化为0， 开始时没有产品*/

sem_t empty_sem;         /*同步信号量，当满了时阻止生产者放产品*/
sem_t full_sem;         /*同步信号量，当没产品时阻止消费者消费*/
pthread_mutex_t mutex; /*互斥信号量， 一次只有一个线程访问缓冲*/
sem_t pr_sem;
sem_t co_sem;

void print();
void *producer();
void *consumer();
void sem_mutex_init();



#endif