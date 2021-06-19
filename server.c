#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<errno.h>

#define FIFO_READ     "readfifo"
#define FIFO_WRITE    "writefifo"
#define BUF_SIZE      1024


int main(void){
    int          wfd,rfd;
    char         buf[BUF_SIZE];
    int          len;

    umask(0);
    if(mkfifo(FIFO_WRITE,0666)){
        printf("can not create FIFO %s because %s",FIFO_WRITE,strerror(errno));
        exit(1);
    }

    wfd = open(FIFO_WRITE,O_WRONLY);
    if(wfd == -1){
        printf("open FIFO %s errno :%s",FIFO_WRITE,strerror(errno));
        exit(1);
    }
    while((rfd = open(FIFO_READ,O_RDONLY)) == -1){
        sleep(1);
    }
    while(1){
        printf("Sme:");
        fgets(buf,BUF_SIZE,stdin);
        buf[strlen(buf)-1] = '\0';
        if(strncmp(buf,"quit",4) == 0){
            close(wfd);
            unlink(FIFO_WRITE); //当处理该文件的进程都关闭时，才会删除其指定的文件
            close(rfd);
            exit(0);
        }
        write(wfd,buf,strlen(buf));
        len = read(rfd,buf,BUF_SIZE);
        if(len > 0){
            buf[len] = '\0';
            printf("Cyou: %s\n",buf);
        }
    }

}