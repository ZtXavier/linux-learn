#include<stdio.h>
#include<unistd.h>
#include<signal.h>


int temp = 0;

/* 信号处理函数 */
void handler_sigint(int signo){
    printf("recv SIGINT\n");
    sleep(5);
    temp += 1;
    printf("the value of temp is : %d\n",temp);
    printf("in handler_sigint,after sleep\n");
}


int main(void){
    struct sigaction act;

    /* 赋值 act 结构 */
    act.sa_handler = handler_sigint;
    act.sa_flags = SA_NOMASK;         //当按下crtl + c 从函数返回时进程的信号屏蔽码恢复原来的值

    /* 安装信号处理函数 */
    sigaction(SIGINT,&act,NULL);

    while(1)
       ;
    return 0;
}