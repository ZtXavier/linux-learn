#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>

#define   FIFO_NAME  "myfifo"
#define   BUF_SIZE    1024

int main(void){
    int fd;
    char buf[BUF_SIZE] = "hel proc,lllll";
    umask(0);

    if(mkfifo(FIFO_NAME, S_IFIFO | 0666) == -1){
        perror("mkfifo error!\n");
        exit(1);
    }
    if((fd = open(FIFO_NAME,O_WRONLY)) == -1)/* 以写的方式打开FIFO */
    {
        perror("mkfifo error!");
        exit(1);
    }
   write(fd,buf,strlen(buf)+1);/* 向FIFO写数据 */
   close(fd);
   //unlink(FIFO_NAME); 
   exit(0);
}