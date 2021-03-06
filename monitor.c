#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<wait.h>

int main(int argc,char *argv[],char ** environ){
    int   fd[2];
    pid_t   pid;
    int    stat_val;

    if(argc < 2){
        printf("wrong parameters \n");
        exit(0);
    }


    if(pipe(fd)){
        perror("pipe failed");
        exit(1);
    }
    pid  = fork();
    switch(pid){

        case -1:
            perror("fork failed!\n");
            exit(1);

        case 0:
            /* child */
             close(fd[1]);
             dup2(fd[0],STDIN_FILENO); //这里dup2的第二个参数不单单指的是宏（文件描述符），而是作为文件数组的下标，然后通过寻找inode指针进行对文件的操作
             execve("ctrlprocess",argv,environ);
             exit(0);

        default:
            /* father*/
             close(fd[0]);
             /* 将命令行第一个参数写进管道 */
             write(fd[1],argv[1],strlen(argv[1]));
             break;
    }
    wait (&stat_val);
    exit(0);
}