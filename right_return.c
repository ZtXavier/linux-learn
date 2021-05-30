#include<stdio.h>
#include<setjmp.h>
#include<signal.h>
#include<unistd.h>

#define       ENV_UNSAVE    0
#define       ENV_SAVED     1


int           flag_saveenv = ENV_UNSAVE;
jmp_buf       env;    //保存待跳转位置的栈信息

/* 信号 SIGRTIMIN + 15的处理函数 */
void handler_sigrtmin15(int signo)
{
    if (flag_saveenv == ENV_UNSAVE){
        return;
    }
    printf("recv SIGRTMIM + 15\n");
    sleep(1);
    printf("in handler_sigrtmin15,after sleep\n");
    siglongjmp(env,1); //返回到env处，注意第二个参数的值，调用该函数会将信号屏蔽字去除
}

int main(){
    //sleep(20);              //若此处添加睡眠函数，当发送信号时，主函数还未到sigsetjmp会导致没有记录保存返回栈点的信息
    /* 设置返回点 */
    switch(sigsetjmp(env,ENV_SAVED)){    //sigsetjmp的第二个参数只要非0即可
    case 0:
    printf("return 0\n");
    flag_saveenv = ENV_SAVED;           //在sigsetjmp之后才将flag_saveenv设置为ENV_SAVED，若不这样处理，当信号发生在调用sigetjmp之前时，可能返回到未知地点或程序崩溃
    break;
    case 1:
    printf("return from SIGRTMIN+15\n");
    break;
    default:
    printf("return else\n");
    break;
             
    }
    /* 捕捉信号，安装信号处理函数 */
    
signal(SIGRTMIN+15,handler_sigrtmin15);
    while(1)
        ;

    return 0;
}