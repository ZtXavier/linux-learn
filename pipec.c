#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>

#define Max_Data_len  256
#define DELAY_TIME 5

int main()
{
    int fd[2];
    pid_t pid;
    char buf[Max_Data_len];
    char recvbuf[Max_Data_len];
    const char data[] = "Pipe Test Program";
    int real_read = 0,real_write = 0;
    // memset((void *)buf,0,sizeof(buf));
    if(pipe(fd) < 0)
    {
        printf("pipe create error\n");
        exit(1);
    }
    if((pid = fork()) < 0)
    {
        printf("fork create error\n");
        exit(1);
    }
    else if(pid == 0)
    {
        
        close(fd[1]);
        sleep(DELAY_TIME);
        if((real_read =  read(fd[0],(void*)recvbuf,sizeof(recvbuf))) != -1)  // 接受数据的缓冲区需要接受所有的缓冲区的内容
        {
            printf("%d bytes read from the pipd is %s\n",real_read,recvbuf);  
        }
        close(fd[0]);
    }
    else
    {
        close(fd[0]);
        memset(buf,0,sizeof(buf));
        scanf("%s",buf);
        if((real_write = write(fd[1],(void *)buf,strlen(buf))) != -1)    //写入的是需要的字符串，而不是所有缓冲区的内容
        {
            printf("%d bytes have write in buf",real_write);
        }
        close(fd[1]);
        waitpid(pid,NULL,0);
        exit(0);
    }

    return 0;
}
