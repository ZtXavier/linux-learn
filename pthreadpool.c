#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include<stdlib.h>

/* 任务结构体，任务回调函数，任务队列 */
typedef struct mission{
    void (*func) (void* arg);     //任务回调函数:由于所存储的任务基于函数完成，所以来存储函数的地址
    void *arg;                    //回调函数的参数  
}CThread_mission;


/* 回调函数 */
// static void *callback(void* arg){
//     printf("threadid:0x%x,working on task %d\n",pthread_self(),*(int*)arg);
//     sleep(1);
//     return (NULL);
// }
/* 线程池结构体 */
typedef struct CThread_Pool{
    
    CThread_mission  *mission;         /* 任务队列 */
    int queue_capacity;                /* 容量 */
    int queue_head;                    /* 队列头部->取数据 */
    int queue_rear;                    /* 队列尾部->存数据 */ 
    int queue_size_mission;            /* 任务队列在任务数目 */

    pthread_mutex_t  pool_mutex;       /* 互斥锁整个线程 */
    pthread_mutex_t  busy_mutex;       /* (找线程池中修改频率最多的来加锁) */
    pthread_cond_t   notFull;          /* 任务队列是否满 */
    pthread_cond_t   notEmpty;         /* 任务队列是否空 */  
    pthread_t        *threadID;        /* 工作线程ID */
    pthread_t        *manageID;        /* 管理者线程ID */      
    int maxthread_num;                 /* 线程池最大线程数 */
    int minthread_num;                 /* 线程池最小线程数 */  
    int busynum;                       /* 工作线程个数 */
    int livenum;                       /* 存活线程个数 */
    int exitnum;                       /* 要销毁的线程个数 */
    int shutdown;                      /* 是否摧毁线程池0：不摧毁；1：摧毁 */
}threadpool;




threadpool * threadPoolcreate(int max,int min,int queuesize){
    threadpool *pool = (threadpool*)malloc(sizeof(threadpool));
   do{
     if(pool == NULL){
        printf("malloc threadpool failed\n");
        break;
    }
    pool->threadID = (pthread_t*)malloc(sizeof(pthread_t)*max);
    if(pool->threadID == NULL){
        printf("malloc threadID fail...\n");
        break;
    }
    memset(pool->threadID,0,sizeof(pthread_t)*max);
    pool->minthread_num = min;
    pool->maxthread_num = max;
    pool->busynum = 0;        //开始没有工作的线程
    pool->exitnum = 0;        //退出的线程
    pool->livenum = min;      //存活的线程，先以最小个数为初始化
    
    if(pthread_mutex_init(&pool->pool_mutex,NULL) != 0 ||
       pthread_mutex_init(&pool->busy_mutex,NULL) != 0 ||
       pthread_mutex_cond(&pool->notEmpty,NULL) != 0 || 
       pthread_mutex_cond(&pool->notFull,NULL) != 0){ //初始化成功返回0
       printf("pthread_init failed\n");
       break;
       }

    //任务队列
    pool->mission =(threadpool*)malloc(sizeof(threadpool)*queuesize);
    pool->queue_capacity = queuesize;
    pool->queue_head = 0;
    pool->queue_rear = 0;
    pool->queue_size_mission = 0;
    pool->shutdown = 0;   

    pthread_create(&pool->manageID,NULL,manager,NULL);
    for(int i = 0; i < min; i++){
        pthread_create(&pool->threadID[i],NULL,worker,NULL);
    }
    return pool;
   }while (0);  //循环中可以使用break来结束并且释放内存
   //释放资源
   if(pool->threadID) free(pool->threadID);
   if(pool->mission)  free(pool->mission);
   if(pool)           free(pool);
   return NULL;
}

/* 任务回调函数 */
void *worker(void* arg){
    //先将函数类型转换
    threadpool *pool = (threadpool*)arg;
    //由于该函数需要不断向任务队列中取任务，所以先来个循环
    while(1){
       pthread_mutex_lock(&pool->pool_mutex);
       //当前任务队列是否为空，while是用来连续判断每个进来的线程所要的任务队列中的任务是否为空
       while(pool->queue_size_mission == 0 && !pool->shutdown){
           //若为空且线程池未关闭,阻塞工作线程
           pthread_cond_wait(&pool->notEmpty,&pool->pool_mutex);
           //pool->notEmpty用来唤醒时所设置的，用来记录哪些任务被阻塞，若为零则阻塞，不为零时下次将该处阻塞的地方唤醒
       }
       // 判断线程池是否关闭
       if(pool->shutdown){
           pthread_mutex_unlock(&pool->pool_mutex);
           pthread_exit(NULL);
       }

       //从任务队列中取出一个任务
       CThread_mission mission;
       mission.func = pool->mission[pool->queue_head].func; //取出任务函数的地址
       mission.arg = pool->mission[pool->queue_head].arg;  //取出参数
       
       //移动头结点
       pool->queue_head = (pool->queue_head + 1) % pool->queue_capacity;
       pool->queue_size_mission--;
       //解锁
       pthread_mutex_unlock(&pool->pool_mutex);
       
       printf("thread %ld start working\n");
       pthread_mutex_lock(&pool->busy_mutex);
       pool->busynum++;     //工作线程加一
       pthread_mutex_unlock(&pool->busy_mutex);
       mission.func(mission.arg);
       //第二种取地址来调用(*mission.func)(mission.arg);
       free(mission.arg);
       mission.arg == NULL;
       printf("thread %ld end working\n");
       pthread_mutex_lock(&pool->busy_mutex);
       pool->busynum--;    //工作线程减一
       pthread_mutex_unlock(&pool->busy_mutex);
    }


    return NULL;
}