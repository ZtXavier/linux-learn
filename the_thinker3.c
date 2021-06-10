/* 简化方案，不对筷子进行标记，而是对哲学家进行信号标记 */
#include <pthread.h>
#include <stdio.h>
#include<stdlib.h>
#include<unistd.h>


#define N  5 
#define LEFT (signo+N-1)%N
#define RIGHT (signo+1)%N
#define THINK_TIME 2
#define EAT_TIME 1

enum {EATING,HUNNGRY,THINKING} state[N];//用枚举对哲学家的状态进行初始化

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER,thinker[N];

/* 这里对每个哲学家的左右都进行检测，如果左右两边哲学家都处于EATING状态，则自身放弃eating（解锁） */
void test(int signo){
    printf("the thinker %d is trying to take the chopsticks\n",signo);
    if(state[signo] == HUNNGRY  &&
       state[LEFT]  != EATING   &&
       state[RIGHT] != EATING     ){
           state[signo] = EATING;
           pthread_mutex_unlock(&thinker[signo]);
       }
}


void take_chopsticks(int signo){
    pthread_mutex_lock(&mutex);
    state[signo]= HUNNGRY;
    test(signo);
    pthread_mutex_unlock(&mutex);
    pthread_mutex_lock(&thinker[signo]);
}


void put_chopsticks(int signo){
    pthread_mutex_lock(&mutex);
    state[signo]= THINKING;
    test(LEFT);
    test(RIGHT);
    pthread_mutex_unlock(&mutex);
}

void think(int signo){
    printf("the think %d is thinking\n",signo);
    sleep(THINK_TIME);
}

void eating(int signo){
    printf("the think %d is eating\n",signo);
    sleep(EAT_TIME);
}

void *work(void * argc){
    int i = *(int *)argc;
    while(1){
        think(i);
        take_chopsticks(i);
        eating(i);
        put_chopsticks(i);
    }
}



int main(void){
    pthread_t th[N] = {0};
    for(int i = 0;i < 5; i++)
        pthread_create(&th[i],NULL,work,(void*)(&i));
    for(int i = 0;i < 5; i++)
        pthread_join(th[i],NULL);
    return 0;
}
