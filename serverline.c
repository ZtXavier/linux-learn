#include"unp.h"
#include<pthread.h>

typedef struct sock_addr{
    struct sockaddr_in servaddr;
    int   connfd;
}sock;

    sock   infos[50];

void *working(void*arg);

int main(int argc,char *argv[]){
    int listenfd,connfd;
    struct  sockaddr_in servaddr;
    //time_t ticks;
    //创建tcp套接字
    listenfd = socket(AF_INET,SOCK_STREAM,0);
    bzero(&servaddr,sizeof(servaddr));
    
    //绑定本地的端口地址
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//这里必将主机字节序（小端）转换为网络字节序（大端）
    servaddr.sin_port = htons(9988);

    //把服务器的众所周知端口绑到套接字
    Bind(listenfd,(SA*)&servaddr,sizeof(servaddr));
    //将套接字转换为监听套接字，这样来自客户的外来链接可以在该套接字上由内核接受用户链接，发送应答
    listen(listenfd,LISTENQ);
    //接受客户链接，发送应答

    int max = sizeof(infos)/sizeof(infos[0]);

    for(int i = 0;i < max;i++){
        bzero(&infos[i],sizeof(infos[i]));
        infos[i].connfd = -1;
    }

int   infolen = sizeof(sock);

    while(1){
        sock   *info;
        for(int i = 0;i < max; i++){
            if(infos[i].connfd == -1){
                info = &infos[i];
                break;
            }
        }
        connfd = Accept(listenfd,(SA *)&info->servaddr,&infolen);//每一个指针所指的空间大小用来接收客户端的地址与端口
        info->connfd = connfd;
        //ticks = time(NULL);
        if(connfd == -1){
            perror("accept");
            break;
        }
        //创建子线程
        pthread_t tid;
        pthread_create(&tid,NULL,working,(void*)info);
        pthread_detach(tid);
    }
    close(listenfd);
}




void *working(void*arg){
    sock *info = (sock*)arg;
    char ip[32];                //在地址转换时需要地址内存空间
    int  loop = 0;
    //建立链接成功，打印客户端的IP地址
    printf("客户端的IP：%s,port: %d",
    inet_ntop(AF_INET,&info->servaddr.sin_addr.s_addr,ip,sizeof(ip)),
    ntohs(info->servaddr.sin_port));
    //通信
    while(1){
        char buff[MAXLINE];
        int len = recv(info->connfd,buff,sizeof(buff),0);
        if(len > 0){
            printf("serv say: %s",buff);
            sprintf(buff,"你说的很对\n",loop++);
            send(info->connfd,buff,strlen(buff)+1,0);
        }
        else if(len == 0){
            printf("地址为%s的客户端断开了链接...\n",info->servaddr.sin_addr.s_addr);
        }else{
            perror("revc");
            break;
        }


    }
    close(info->connfd);
    info->connfd = -1;
    return NULL;
}