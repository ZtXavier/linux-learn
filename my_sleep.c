#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>

void handler(){
    ;                           //可添加任务
}

int my_sleep(int time){
    struct sigaction act,oact;  //对act里的参数进行修改来改变信号的处理动作，oact则是传出该信号的原来的动作
    act.sa_flags = 0;
    act.sa_handler = handler;
    //sigemptyset(&act.sa_mask);    //使屏蔽信号(阻塞)为空
    sigaction(SIGALRM,&act,&oact);
    alarm(time);         //timeout秒后闹钟超时，内核发SIGALRM给这个进程
    //while(1);
    pause();             //pause函数返回-1，然后调用alarm(0)取消闹钟，调用sigaction恢复信号处理前的动作                 
    int ret = alarm(0);
    sigaction(SIGALRM,&oact,NULL);//将信号的处理动作初始化
    return ret;
}
int main(){
    while(1){
        my_sleep(2);
        printf("success!\n");
    }
}

/* 如果在alarm 与 pause之间有其他函数，则会导致信号异步，pause函数接收不到信号一直挂起 */