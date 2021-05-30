#include<stdio.h>
#include<signal.h>
#include<sys/types.h>
#include<stdlib.h>
#include<string.h>


int main(int argc,char *argv[]){
    int   i,j;
    int   signum = SIGTERM; //默认为该信号，该信号用来程序的正常退出(可以被阻塞和处理)
    pid_t  pid;
    /* 首先检查参数个数 */
    if(argc != 2 && argc != 4){
        printf("Usage : ./my_kill <-s signum>[PID]\n");
        exit(0);
    }

    /* 从命令行解析出信号编号 */
    for(i = 0;i < argc; i++){
        if(!strcmp(argv[i],"-s")){
            signum = atoi(argv[i+1]);
            break;
        }
    }

    /* 解析出进程号 */
    if(argc == 2){
       pid = atoi(argv[1]);
    }
       else{
           for(j = 1;j < argc;j++){
               if(j != i && j != i+1){
                   pid = atoi(argv[j]);
                   break;
               }
           }
       }
    
    if(kill(pid,signum) < 0){
        perror("kill");
        exit(1);
    }
    return 0;

}