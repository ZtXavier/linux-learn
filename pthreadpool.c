#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include<stdlib.h>
#include<string.h>


const int NUMBER = 3;    // 每次添加的最大线程数(定义为常量)


/* 任务结构体，任务回调函数，任务队列 */
typedef struct mission{
    void *(*func)(void* arg);     //任务回调函数:由于所存储的任务基于函数完成，所以来存储函数的地址
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
    pthread_cond_t   notFull;          /* 任务队列是否满(生产者) */
    pthread_cond_t   notEmpty;         /* 任务队列是否空(消费者) */  
    pthread_t        *threadID;        /* 工作线程ID */
    pthread_t         manageID;        /* 管理者线程ID */      
    int maxthread_num;                 /* 线程池最大线程数 */
    int minthread_num;                 /* 线程池最小线程数 */  
    int busynum;                       /* 工作线程个数 */
    int livenum;                       /* 存活线程个数 */
    int exitnum;                       /* 要销毁的线程个数 */
    int shutdown;                      /* 是否摧毁线程池0：不摧毁；1：摧毁 */
}threadpool;



//创建线程池并初始化
threadpool * threadPoolcreate(int max,int min,int queuesize);
//销毁线程池
int threaddestroy(threadpool *pool);
//添加任务函数
void threadpooladd(threadpool *pool,void *func(void*),void *arg);
//获取工作的线程个数
int threadbusynum(threadpool *pool);
//获取线程池中或者的线程个数
int threadalivenum(threadpool *pool);

void *worker(void* arg);
void *manager(void* arg);

void threadexit(threadpool *pool);



/* 创建线程池（初始化） */
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
    pool->livenum = min;      //存活的线程，以最小个数为初始化

    if(pthread_mutex_init(&pool->pool_mutex,NULL) != 0 ||
       pthread_mutex_init(&pool->busy_mutex,NULL) != 0 ||
       pthread_cond_init(&pool->notEmpty,NULL) != 0 || 
       pthread_cond_init(&pool->notFull,NULL) != 0){  
       //初始化成功返回0

       printf("pthread_init failed\n");

       break;
       }
       

    //任务队列
    pool->mission = (CThread_mission*)malloc(sizeof(CThread_mission)*queuesize);
    pool->queue_capacity = queuesize;
    pool->queue_head = 0;
    pool->queue_rear = 0;
    pool->queue_size_mission = 0;
    pool->shutdown = 0;   

    pthread_create(&pool->manageID,NULL,manager,pool);

    for(int i = 0; i < min ; i++){
       
         
        pthread_create(&(pool->threadID[i]),NULL,worker,pool);

       

    }
    
    return pool;

   } while (0);  //循环中可以使用break来结束并且释放内存
 

   //释放资源
   if(pool->threadID) free(pool->threadID);
   if(pool->mission)  free(pool->mission);
   if(pool)           free(pool);
   return NULL;
}


/* 消费者线程 */
void * worker(void* arg){
    //先将函数类型转换
    threadpool *pool = (threadpool*)arg;
    //由于该函数需要不断向任务队列中取任务，所以先来个循环
    while(1){      
       pthread_mutex_lock(&pool->pool_mutex);
       //当前任务队列是否为空，while是用来连续判断每个进来的线程所要的任务队列中的任务是否为空
       while((pool->queue_size_mission == 0) && !(pool->shutdown)){// 使用while而不用if是为了防止其虚假唤醒，也就是没有broadcast或signal时wait有返回值（下面的生产者阻塞也同理）
           //若为空且线程池未关闭,阻塞工作线程（线程池为空说明没有任务，所以阻塞消费者）
           pthread_cond_wait(&pool->notEmpty,&pool->pool_mutex);
           //pool->notEmpty用来唤醒线程，用来记录哪些任务被阻塞，若为零则阻塞，不为零时下次将该处阻塞的地方唤醒
            if(pool->exitnum > 0){
            pool->exitnum--;       //每次唤醒线程后，要销毁的线程数减一
            //在判断线程池中最小线程数小于存在线程后才可将存在的空闲的线程销毁
            if(pool->livenum > pool->minthread_num){

            pool->livenum--;       
            pthread_mutex_unlock(&pool->pool_mutex);
            threadexit(pool);

            }
          }
       }
       // 判断线程池是否关闭
       if(pool->shutdown){
           pthread_mutex_unlock(&pool->pool_mutex);
           threadexit(pool);
       }

       //从任务队列中取出一个任务
       CThread_mission mission;
       mission.func = pool->mission[pool->queue_head].func; //取出任务函数的地址
       mission.arg = pool->mission[pool->queue_head].arg;  //取出参数
       
       //移动头结点
       pool->queue_head = (pool->queue_head + 1) % pool->queue_capacity;
       pool->queue_size_mission--;

       //解锁
       pthread_cond_signal(&pool->notFull);  // 在上面队列中任务已被消费后若有阻塞的生产者可以将其唤醒
       pthread_mutex_unlock(&pool->pool_mutex);
       
       printf("thread %ld start working\n",pthread_self());

       pthread_mutex_lock(&pool->busy_mutex);
       pool->busynum++;     //工作线程加一
       pthread_mutex_unlock(&pool->busy_mutex);

       mission.func(mission.arg);//第二种取地址来调用(*mission.func)(mission.arg);
       free(mission.arg);
       mission.arg == NULL;

       printf("thread %ld end working\n",pthread_self());

       pthread_mutex_lock(&pool->busy_mutex);
       pool->busynum--;    //工作线程减一
       pthread_mutex_unlock(&pool->busy_mutex);
       
    }
     return NULL;      
}
    

    /* 管理者线程 */
void *manager(void* arg){
    threadpool *pool = (threadpool *)arg;
    //当线程池没有关闭时管理者函数检测线程状态
    while(!pool->shutdown){
        //每隔三秒检测一次
        sleep(3);

       /* 取出线程池中任务的数量和当前线程的数量 */
       pthread_mutex_lock(&pool->pool_mutex);
       int queuesize = pool->queue_size_mission;
       int livenum   = pool->livenum;
       pthread_mutex_unlock(&pool->pool_mutex);

       /* 取出忙的线程的数量 */
       pthread_mutex_lock(&pool->busy_mutex);
       int busy_num = pool->busynum;
       pthread_mutex_unlock(&pool->busy_mutex);

       /* 添加线程 (对线程池中的线程操作) */
       /* 任务的个数大于存活的线程个数 && 存活的线程数小于最大线程数 */
       if((queuesize > livenum) && (livenum < pool->maxthread_num)){
           pthread_mutex_lock(&pool->pool_mutex);
           int counter = 0;
           for(int i=0;i < pool->maxthread_num && counter < NUMBER && pool->livenum < pool->maxthread_num;i++){
               if(pool->threadID[i] == 0){                   // 添加线程时需考虑每个线程id是否为零，若为零则没有创建线程
                   pthread_create(&pool->threadID[i],NULL,worker,pool);
                   counter++;          //添加的线程数+1
                   pool->livenum++;    //存活的线程+1
               }
           }
           pthread_mutex_unlock(&pool->pool_mutex);
       }
       
       /* 销毁线程 */
       /* 正在工作的线程*3 小于 存活的线程 && 存活的线程 大于 最小线程数 */
       if((busy_num*3 < livenum) && (livenum > pool->minthread_num)){
        pthread_mutex_lock(&pool->pool_mutex);
        pool->exitnum = NUMBER;
        pthread_mutex_unlock(&pool->pool_mutex);
        //让工作的线程自杀
        for(int i = 0;i < NUMBER;i++){
           pthread_cond_signal(&pool->notEmpty);  // 这里是将线程唤醒

        }
      }
    }
    return NULL;
}



/* 定义该函数是因为退出后其线程id没有初始化为零，下次调用会出现问题 */
void threadexit(threadpool *pool){
    pthread_t tid = pthread_self();
    for(int i = 0;i < pool->maxthread_num;i++){
        if(pool->threadID[i] == tid){
        printf("threadexit() called,%ld exiting...\n",tid);
        pool->threadID[i] = 0;   
        break;
        }
    }
      pthread_exit(NULL);
}



/* 生产者线程 */
void threadpooladd(threadpool *pool,void *func(void*),void *arg){
    pthread_mutex_lock(&pool->pool_mutex);
    while((pool->queue_size_mission == pool->queue_capacity) && !(pool->shutdown)){   //满足该条件时说明线程池可以使用但是队列已经满了

        //当队列已满时，需要阻塞生产者线程
        pthread_cond_wait(&pool->notFull,&pool->pool_mutex);

    }
    if(pool->shutdown){                                                               // 判断线程池是否被关闭，若关闭进入函数退出，否则将向下执行..
        pthread_mutex_unlock(&pool->pool_mutex);
        return ;
    }

    //添加任务(从队尾添加，从队头取任务)
    pool->mission[pool->queue_rear].func = func;
    pool->mission[pool->queue_rear].arg = arg;
    pool->queue_rear = (pool->queue_rear+1) % pool->queue_capacity;
    pool->queue_size_mission++;

    //唤醒阻塞的生产者线程
    pthread_cond_signal(&pool->notFull);
    pthread_mutex_unlock(&pool->pool_mutex);
}



/* 工作线程计数 */
int threadbusynum(threadpool *pool){
    pthread_mutex_lock(&pool->busy_mutex);
    int busynum = pool->busynum; 
    pthread_mutex_unlock(&pool->busy_mutex);
    return busynum;
}



/* 存活线程计数 */
int threadalivenum(threadpool *pool){
    pthread_mutex_lock(&pool->pool_mutex);
    int alivenum = pool->livenum; 
    pthread_mutex_unlock(&pool->pool_mutex);
    return alivenum;
}



/* 销毁线程池 */
int threaddestroy(threadpool *pool){

    if(pool == NULL){
       return -1;
    }

    //关闭线程池

    pool->shutdown = 1;                 //当线程池关闭后，唤醒之后的子线程就会自动退出（内部有条件）
    return 0;

    //阻塞回收管理者线程
    pthread_join(pool->manageID,NULL);

    //唤醒消费者线程
    for(int i = 0;i < pool->livenum; i++){

        pthread_cond_signal(&pool->notEmpty);
    }
    //释放内存
    if(pool->mission){
        free(pool->mission);
        pool->mission = NULL;
    }
    if(pool->threadID){
        free(pool->threadID);
        pool->threadID = NULL;
    }
    free(pool);
    pool = NULL;

    //释放条件变量和互斥锁资源
    pthread_cond_destroy(&pool->notEmpty);
    pthread_cond_destroy(&pool->notFull);
    pthread_mutex_destroy(&pool->busy_mutex);
    pthread_mutex_destroy(&pool->pool_mutex);

    return 0;
}



void *taskfunc(void *arg){

    int num = *(int*)arg;

    printf("thread %ld is working,tid = %d",pthread_self(),num);

    usleep(1000);
}


int main(void){

    
    threadpool *pool = threadPoolcreate(10,3,50);
    
    for(int i = 0;i < 50;i++){
        int *num = (int*)malloc(sizeof(int));
        *num = i + 100;
        threadpooladd(pool,taskfunc,num);
    }
    sleep(30);                        //等待子线程处理完任务

    threaddestroy(pool);
    return 0;
}