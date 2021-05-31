#include<stdio.h>
#include<signal.h>
#include<unistd.h>
#include<stdlib.h>

/* 信号处理函数 */
void handler_sigint(int signo){
    printf("recv SIGINT\n");
}

int main(void){
    /* 安装信号处理函数 */
    signal(SIGINT,handler_sigint);
    while(1){
       sleep(3);
       printf("正在工作\n");
    }
    return 0;
}