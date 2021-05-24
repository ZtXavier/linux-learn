#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
 
/* 任务,任务回调函数,任务队列 */
typedef struct worker {  
    void* (*callback) (void* arg); /*任务回调函数*/
    void* arg;                     /*回调函数的参数*/  
    struct worker* next;           /*任务队列链表*/  
} CThread_worker;
 
/*回调函数*/
static void* callback(void* arg) {
    printf("threadid:0x%x, working on task %d\n", pthread_self(), *(int*)arg);  
    sleep(1);
    return(NULL);
}
 
/*线程池结构*/  
typedef struct {  
    pthread_mutex_t mutex;      /*互斥锁 */
    pthread_cond_t cond;        /*条件变量 */
  
    CThread_worker* queue_head; /*线程池的任务队列*/
  
    int shutdown;               /*是否摧毁线程池 0:不摧毁 1:摧毁 */
    pthread_t* threadid;        /*线程ID数组*/
    int max_thread_num;         /*线程池最大线程数*/  
    int cur_queue_size;         /*任务队列在任务数目*/
  
} CThread_pool;
 
/*线程函数*/
void* thread_routine(void* arg);
 
/*线程池实例*/
static CThread_pool* pool = NULL; 
 
/*线程池初始化*/
void pool_init(int max_thread_num) {
	/*一些列初始化*/
	pool = (CThread_pool*) malloc(sizeof(CThread_pool));
 
	pthread_mutex_init(&(pool->mutex), NULL);
    pthread_cond_init(&(pool->cond), NULL);
 
    pool->queue_head = NULL;
 
    pool->max_thread_num = max_thread_num;
 
    pool->shutdown = 0; /*0打开1关闭*/
 
    pool->cur_queue_size = 0;
 
	pool->threadid = (pthread_t*) malloc(max_thread_num * sizeof (pthread_t));
 
	/*创建工作线程*/
    int i = 0;  
    for (i=0; i<max_thread_num; ++i) {
        pthread_create(&(pool->threadid[i]), NULL, thread_routine, NULL);
    }  
 
}
 
/*将任务加入队列*/
int pool_add_worker(void* (*callback) (void* arg), void* arg) {
    /*构造一个新任务*/
    printf("pool add worker arg:%d\n", *(int*)arg);
    CThread_worker* newworker = (CThread_worker*) malloc(sizeof(CThread_worker));
    newworker->callback = callback;  
    newworker->arg = arg;  
    newworker->next = NULL; /*SET NULL*/
 
    pthread_mutex_lock(&(pool->mutex));
 
    /*将任务加入到任务队列中,也就是链表末端*/
    CThread_worker* worker = pool->queue_head;
    if (worker != NULL) {  
        while (worker->next != NULL)
            worker = worker->next;
        worker->next = newworker;
    }  
    else { 
        pool->queue_head = newworker;
    }
  	
  	/*是否需要唤醒线程*/
    int dosignal;
    if (pool->cur_queue_size == 0)
     	dosignal = 1;
 
    pool->cur_queue_size += 1; /*计数+1*/
 
    pthread_mutex_unlock(&(pool->mutex));
 
    /*需要叫醒工作线程*/
    if (dosignal)
    	pthread_cond_signal(&(pool->cond));
 
    return 0;
}
 
/*销毁线程池*/
int pool_destroy() {
	printf("pool destroy now\n");
 
	/*启用关闭开关*/
    if (pool->shutdown)  
        return -1; /*防止两次调用*/
    pool->shutdown = 1;  
  
    /*唤醒所有等待线程*/  
    pthread_cond_broadcast(&(pool->cond));
  
    /*阻塞等待线程退出回收资源，还有另一种办法就是线程分离*/
    int i;  
    for (i=0; i<pool->max_thread_num; ++i)  
        pthread_join(pool->threadid[i], NULL);
    free(pool->threadid);
    pool->threadid = NULL;
  
    /*销毁任务队列*/  
    CThread_worker* head = NULL;  
    while (pool->queue_head != NULL) {  
        head = pool->queue_head;
        pool->queue_head = pool->queue_head->next;
        free(head);
        head = NULL;
    }
 
    /*销毁互斥锁与条件变量*/
    pthread_mutex_destroy(&(pool->mutex));
    pthread_cond_destroy(&(pool->cond));
      
    free(pool); 
    pool = NULL;
    printf("pool destroy end\n");
    return 0;  
}  
 
/*工作线程函数*/
void* thread_routine(void* arg) {
    printf("starting threadid:0x%x\n", pthread_self());  
    
    for (; ;) {
 
        pthread_mutex_lock(&(pool->mutex));  
        /*任务队列为空时wait唤醒,当销毁线程池时跳出循环*/ 
        while (pool->cur_queue_size == 0 && !pool->shutdown) {
            printf("threadid:0x%x is waiting\n", pthread_self());  
            pthread_cond_wait(&(pool->cond), &(pool->mutex));  
        }  
  
        /*线程池要销毁了*/  
        if (pool->shutdown) {
            pthread_mutex_unlock(&(pool->mutex));  
            printf("threadid:0x%x will exit\n", pthread_self());  
            pthread_exit(NULL);  
        }
  		
  		/*开始执行任务*/
        printf("threadid:0x%x is starting to work\n", pthread_self());
          
        /*等待队列长度减去1，并取出链表中的头元素*/  
        pool->cur_queue_size -= 1;  
        CThread_worker* worker = pool->queue_head;  
        pool->queue_head = worker->next;  
        pthread_mutex_unlock(&(pool->mutex));  
  
        /*调用回调函数，执行任务*/  
        (*(worker->callback)) (worker->arg);  
        free(worker);  
        worker = NULL;  
    }
    return(NULL);
}
 
 
/*测试*/
int main(int argc, char const *argv[])
{	
    pool_init(2); /*创建n个线程*/  
      
    /*添加n个任务*/
    int* workingnum = (int*) malloc(sizeof(int) * 10); /* 一定要动态创建 */
    int i;  
    for (i=0; i<5; ++i) {
    	workingnum[i] = i;
        pool_add_worker(callback, &workingnum[i]);
    }  
    
    sleep(5); /*等待所有任务完成*/ 
     
    pool_destroy(); /*销毁线程池*/ 
    free(workingnum);
    workingnum = NULL;
    return 0; 
}