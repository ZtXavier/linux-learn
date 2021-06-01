#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<signal.h>


void  send(int signo){

    printf("recv signal %d\n",signo);
}
void handler_sigalarm(int signo){
    send(signo);
    alarm(2);
}

void io(){

    while(1)

    ;
}

int main(){
    // if(signal(SIGALRM,handler_sigalrm) < 0){
    //     printf("failed to recv...\n");
    // }else{
    //     printf("success!\n");
    // }
    signal(SIGALRM,handler_sigalarm);

    raise(SIGALRM);
    //io();
    while(1)

    ;
    return 0;
}