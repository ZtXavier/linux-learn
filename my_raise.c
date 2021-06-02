#include<stdio.h>
#include<signal.h>
#include<stdlib.h>
#include<unistd.h>

int my_raise(int signo){
    pid_t pid;
    union sigval    value;
    pid = getpid();
    sigqueue(pid,signo,value);
    return 0;
}
void handler_sig(){
    printf("recv sign\n");
}
int main(){
    signal(SIGINT,handler_sig);
    sleep(2);
    //my_raise(SIGINT);
    raise(SIGINT);
    while(1)
       ;
   return 0;
}