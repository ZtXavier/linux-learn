#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>

void assistthread(void *arg){
    printf("I anm helping to do some jobs\n");
    sleep(3);
    pthread_exit(0);
}
int main(void){
    pthread_t assistthid;
    int status;

    pthread_create(&assistthid,NULL,(void*)assistthread,NULL);
    pthread_join(assistthid,&status);
    printf("assistthread's exit is a cause %d\n",status);
    return 0;
}