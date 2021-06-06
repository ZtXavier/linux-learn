/* 规定一个先例，偶数哲学家先左后右，奇数哲学家先右后左,这样总能保证一个哲学家先拿到筷子吃完，不会造成阻塞死锁 */
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>

#define   N    5

pthread_mutex_t chopsticks[N] = {PTHREAD_MUTEX_INITIALIZER};

void eat(int i){
    printf("thinker %d eat eat eat\n",i);
    //sleep(2);
}




void *work(void *argc){
    int i = *(int*)argc;
    int left = i;
    int right = (i+1)%N;
    while (1) {
        printf("哲学家%d正在思考问题\n", i);
        //sleep(2);

        printf("哲学家%d饿了\n", i);
        if (i % 2 == 0) {//偶数哲学家，先右后左
            pthread_mutex_lock(&chopsticks[right]);
            pthread_mutex_lock(&chopsticks[left]);
            eat(i);
            pthread_mutex_unlock(&chopsticks[left]);
            pthread_mutex_unlock(&chopsticks[right]);
        } else {//奇数哲学家，先左后又
            pthread_mutex_lock(&chopsticks[left]);
            pthread_mutex_lock(&chopsticks[right]);
            eat(i);
            pthread_mutex_unlock(&chopsticks[right]);
            pthread_mutex_unlock(&chopsticks[left]);
        }
    }
}




int main(){
    pthread_t threadID[N];
    for(int i = 0;i < 5;i++)
    pthread_create(&threadID[i],NULL,work,(void *)(&i));
    for(int i = 0;i<5;i++)
    pthread_join(threadID[i],NULL);
}