#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void handler_sigint(int signo)
{
    ;
}

int my_sleeppuls(int time)
{
    sigset_t newmask, oldmask, mask; //定义信号集
    signal(SIGALRM, handler_sigint);

    sigemptyset(&newmask);
    sigemptyset(&oldmask);
    sigaddset(&newmask, SIGALRM);
    

    if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0) //将本进程的信号屏蔽字改为newmask,将原来的信号屏蔽字保存到oldmask中并返回
    {
        printf("sigproc failed...\n");
    }
    
     alarm(time);
    // mask = oldmask;
    // sigdelset(&mask, SIGALRM);
    // 设置为oldmask，之后醒来再恢复原有的newmask
    sigsuspend(&oldmask);
    
    //恢复oldmask
    if (sigprocmask(SIG_UNBLOCK, &newmask, NULL) < 0)
    {
        printf("sigproc failed\n");
    }
    
    return 0;
}

int main()
{
    while (1)
    {
        my_sleeppuls(2);
        printf("success!\n");
    }
    return 0;
}