#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>

void handler(){
    ;
}

int my_sleep(int time){
    struct sigaction act,oact;  //对act里的参数进行修改来改变信号的处理动作，oact则是传出该信号的原来的动作
    act.sa_flags = 0;
    act.sa_handler = handler;
    sigemptyset(&act.sa_mask);    //使屏蔽信号(阻塞)为空
    sigaction(SIGALRM,&act,&oact);
    alarm(time);         //timeout秒后闹钟超时，内核发SIGALRM给这个进程
    pause();             //pause函数返回-1，然后调用alarm(0)取消闹钟，调用sigaction恢复信号处理前的动作                 
    int ret = alarm(time);
    sigaction(SIGALRM,&oact,NULL);//将信号的处理动作初始化
    return ret;
}
int main(){
    while(1){
        my_sleep(2);
        printf("success!\n");
    }
}

