#include"pthreadpool.h"
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>



void *taskfunc(void *arg){
    int num = *(int*)arg;
    printf("thread %ld is working,tid = %d",pthread_self(),num);
    usleep(1000);
}


int main(void){
    threadpool *pool = threadPoolcreate(10,3,100);

    for(int i = 0;i < 100;i++){
        
        int *num = (int*)malloc(sizeof(int));
        *num = i + 10;

        threadpooladd(pool,taskfunc,num);
    }
    sleep(30);                        //等待子线程处理完任务

    threaddestroy(pool);
    return 0;
}