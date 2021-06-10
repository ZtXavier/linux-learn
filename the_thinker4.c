/* 哲学家问题，一般方法会有五个哲学家同时拿走右边的筷子，从而导致死锁
解决方案四：利用信号量来进行对线程数的控制 */
#include<semaphore.h>
#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>


#define     N   5  //总的哲学家的数量;
sem_t      sem ;
pthread_mutex_t  mutex;
pthread_mutex_t forks[5] = {PTHREAD_MUTEX_INITIALIZER};
typedef struct Cthread{
    pthread_t thinkerID;
    int i;
}thread;




void thinking(int id){
    printf("thinker %d is thinking!\n",id);
    //sleep(2);
   
}

void eating(int id){
    printf("thinker %d is eating!\n",id);
    //sleep(3);
    
}

void take_chopstick(int id){
    pthread_mutex_lock(&forks[(id+N-1)%5]);    //哲学家右手筷子
    pthread_mutex_lock(&forks[(id+1)%5]);      //哲学家左手筷子
    printf("the thinker %d take_chopsticks\n",id);
    return;
}


void put_downcps(int id){
    printf("the thinker %d put_downcps\n",id);
    pthread_mutex_unlock(&forks[(id+N-1)%5]);
    sem_post(&sem);
    pthread_mutex_unlock(&forks[(id+1)%5]);

    return;
}

void  *thinker_work(void * arg){
    thread *id = (thread*)arg;
    
    printf("thinker init[%d]\n",id->i);
    while(1){
       
        thinking(id->i);
        sem_wait(&sem);                  //利用信号量来控制线程的个数
        //pthread_mutex_lock(&mutex);    //在这里添加互斥锁后，线程将一个一个进行工作，不会有多个线程同时取筷子而阻塞该进程，有效防止了死锁，但是其效率较低
        take_chopstick(id->i);
        //pthread_mutex_unlock(&mutex);
        eating(id->i);
        put_downcps(id->i);
        
    }  

}

int main(void){

    thread *th = (thread *)malloc(5*sizeof(thread));
    sem_init(&sem,0,4);                           //初始化信号量
    
    for(int i = 0;i < 5; i++){
    th[i].thinkerID = 0;
    th[i].i = i;
    }
    for(int i = 0;i < 5;i++){
    if(pthread_create(&(th[i].thinkerID),NULL,thinker_work,&th[i]) < 0)
      printf("created pthread failed\n");
    }
    for(int i = 0;i< 5;i++){
    pthread_join((th[i].thinkerID),NULL);
    }
    
    
    return 0;
}