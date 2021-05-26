#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>


typedef struct CThread_mission{
    void *(*function)(void *arg);
    void *arg;
}mission;

typedef struct CThread_pool{

    mission *queue;
    int queuesize;
    int queuecapacity;
    int queue_head;
    int queue_rear;

    pthread_mutex_t pool_mutex;
    pthread_cond_t  not_empty;
    pthread_cond_t  not_full;
    int maxnum;
    int minnum;
    int busynum;
    int alivenum;
    int exitnum;
    int shutdown;

}pool;


void *worker(void *arg){
    
}