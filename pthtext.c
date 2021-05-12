#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

// void *func(void* args){
//     for(int i=0;i<=50;i++)
//     printf("%d %u\n",i,pthread_self());
//     return NULL;
// }

//这里传参可以考虑使用结构体类型来传参实现参数的来回传值

void *func(void* args){
    char* name = (char*)args;
    for(int i=0;i<=50;i++)
    printf("%s: %d\n",name,i);
    return NULL;
}

int main(void){
    pthread_t th1;
    pthread_t th2;

    // pthread_create(&th1,NULL,(void *)func,NULL);
    // pthread_create(&th2,NULL,(void *)func,NULL);

    pthread_create(&th1,NULL,(void *)func,"th1");
    pthread_create(&th2,NULL,(void *)func,"th2");

    pthread_join(th1,NULL);
    pthread_join(th2,NULL);
    return 0;
}
// void *func(void* args) 这是线程开创后来运行的函数，除了args可以更改其他格式都不可以改变
//pthread_create(定义的线程变量，NULL，在此开创的线程中需要运行的函数，此函数中args内的量)