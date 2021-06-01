#include<signal.h>
#include<stdio.h>
#include<stdlib.h>


/* 3参数的信号处理函数 */
void handler_sigint(int signo,siginfo_t *siginfo,void * pvoid){
    printf("recv SIGINT,the data value is: %d\n",siginfo->si_int);
}



int main(){
    struct sigaction act;

    /* 赋值act结构 */
    act.sa_sigaction = handler_sigint;
    act.sa_flags = SA_SIGINFO;         //制定使用三参数的信号处理函数

    /* 安装信号处理函数 */
    sigaction(SIGINT,&act,NULL);
    while(1)
        ;
    return 0;
}