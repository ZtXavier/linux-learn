/* 示例pause挂起等待事件的发生 */

#include<stdio.h>
#include<signal.h>
#include<stdlib.h>
#include<unistd.h>


#define   UNHAPPEN   0//未发生
#define   HAPPENED   1//已发生

/* 定义全局变量以标识事件是否发生 */
int   flag_happen = UNHAPPEN;

void  handler_sigint(int signo){
    flag_happen = HAPPENED;
}

int main(){


    /* 安装信号处理函数 */
    if(signal(SIGINT,handler_sigint) == SIG_ERR){
        perror("signal");
        exit(1);
    }


    while(flag_happen == UNHAPPEN)
    //sleep(10);//若有信号发生在此处则会使pause一直挂起，处理不了此次事件的发生
    pause();


    printf("event happened\n");
            /* ...可继续添加任务 */
        
    
    return 0;
}