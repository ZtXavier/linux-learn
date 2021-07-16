#include"unp.h"
#include<time.h>
//服务器端

// int Socket(int family,int type,int protocol){
//     int n;
//     if((n = socket(family,type,protocol)) < 0)
//     printf("socket err\n");
//     return (n);
// }

int main(int argc,char *argv[]){
    int listenfd,connfd;
    struct  sockaddr_in servaddr;
    char buff[MAXLINE];
    char bufrecv[MAXLINE];
    time_t ticks;
    //创建tcp套接字
    listenfd = socket(AF_INET,SOCK_STREAM,0);
    bzero(&servaddr,sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//这里必将主机字节序（小端）转换为网络字节序（大端）
    servaddr.sin_port = htons(13);

    //把服务器的众所周知端口绑到套接字
    Bind(listenfd,(SA*)&servaddr,sizeof(servaddr));
    //将套接字转换为监听套接字，这样来自客户的外来链接可以在该套接字上由内核接受用户链接，发送应答
    listen(listenfd,LISTENQ);
    //接受客户链接，发送应答
    for(;;){
        connfd = Accept(listenfd,(SA *)NULL,NULL);
        ticks = time(NULL);
        //发送链接
        snprintf(buff,sizeof(buff),"%.24s\r\n",ctime(&ticks));
        Write(connfd,buff,strlen(buff));
        //接收信息
        int ret = recv(connfd,bufrecv,sizeof(bufrecv),0);
        if(ret > 0){
            printf("ser:  %s\n",bufrecv);
        }
        else if(ret == 0){
            printf("客户端已断开连接...\n");
        }else{
            perror("recv");
        }
        //printf("%s",buff);
        //终止链接
        Close(connfd);
    }
    return 0;

}