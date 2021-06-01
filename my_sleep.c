#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>

void handler(){
    ;
}

int my_sleep(int time){
    struct sigaction act,oact;
    act.sa_flags = 0;
    act.sa_handler = handler;
    sigemptyset(&act.sa_mask);    //使屏蔽信号(阻塞)为空
    sigaction(SIGALRM,&act,&oact);
    alarm(time);
    pause();             
    int ret = alarm(time);
    sigaction(SIGALRM,&oact,NULL);
    return ret;
}
int main(){
    while(1){
        my_sleep(2);
        printf("success!\n");
    }
}