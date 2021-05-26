#ifndef _PTHREADPOOL_H
#define _PTHREADPOOL_H 1


typedef struct mission CThread_mission;
typedef struct CThread_Pool threadpool;

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

#endif  //_THREADPOOL_H