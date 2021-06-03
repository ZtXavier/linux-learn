/* 哲学家问题，一般方法会有五个哲学家同时拿走右边的筷子，从而导致死锁
解决方案一：重新定义一个锁，在取筷子时加锁，这样取筷子的人数只有一个人，可以防止多个线程同时取筷而导致的死锁*/

#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>


#define     N   5  //总的哲学家的数量



pthread_mutex_t  mutex;
pthread_mutex_t forks[5] = {PTHREAD_MUTEX_INITIALIZER};
typedef struct Cthread{
    pthread_t thinkerID;
    int i;
}thread;




void thinking(int id){
    printf("thinker %d is thinking!\n",id);
   
}

void eating(int id){
    printf("thinker %d is eating!\n",id);
    
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
    pthread_mutex_unlock(&forks[(id+1)%5]);
    return;
}

void  *thinker_work(void * arg){
    thread *id = (thread*)arg;
    
    printf("thinker init[%d]\n",id->i);
    while(1){
        thinking(id->i);
        //pthread_mutex_lock(&mutex);    //在这里添加互斥锁后，线程将一个一个进行工作，不会有多个线程同时取筷子而阻塞该进程，有效防止了死锁，但是其效率较低
        take_chopstick(id->i);
        //pthread_mutex_unlock(&mutex);
        eating(id->i);
        put_downcps(id->i);
    }  

}

int main(void){

    thread *th = (thread *)malloc(5*sizeof(thread));

    
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