#include<stdio.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<stdlib.h>


/* 把一个已存在的路径名和一个整数标识符转换成IPC键值 
 参数pathname在系统中一定要存在且有进程访问，参数proj_id取值为1~255 */


int main(){
    int i;
    for(i = 1;i <= 5 ;i++)
    printf("key[%d] = %lu \n",i,ftok(".",i));
    exit(0);
}