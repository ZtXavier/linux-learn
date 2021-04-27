#include<stdio.h>
#include<sys/types.h>
#include<unistd.h>

int main(void){
    pid_t  pid;

    printf("Process Creation Study\n");
    pid = fork();
    switch(pid){
        case 0:
        printf("Child Process is running,curpid is %d,Parentpid is %d\n",pid,getppid());
        break;
        case -1:
        printf("Process is failed\n");
        break;
        default:
        printf("Parernt process is running,Childpid is %d,Parentpid is %d\n",pid,getppid());
        break;
    }
    exit(0);
}