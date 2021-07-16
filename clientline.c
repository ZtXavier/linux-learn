#include"unp.h"
//客户端
int main(){
    int     sockfd,n;
    int     loop =0;
    char    recvline[MAXLINE + 1];
    char    sendline[MAXLINE + 1];
    struct  sockaddr_in    servaddr;

    if((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0)  //创建tcp套接字，socket函数创建一个网际(AF_INET)字节流(sock_INET)套接字，该函数返回小整数描述符
    err_sys("socket error");

    servaddr.sin_family = AF_INET;       //调用htons转换二进制端口号，由主机字节顺序为网络字节顺序
    servaddr.sin_port = htons(9988);
    inet_pton(AF_INET,"127.0.0.1",&servaddr.sin_addr.s_addr);
    //与服务器建立联接
    if(connect(sockfd,(SA *)&servaddr,sizeof(servaddr)) < 0)
    err_sys("connect error");
    //读入与服务器的联接应答
    while(1){
    //发送数据
    sprintf(sendline,"你好，啦啦啦,%d...\n",loop++);
    send(sockfd,sendline,strlen(sendline)+1,0);
    //接收数据
    memset(recvline,0,sizeof(recvline));
    int len = recv(sockfd,recvline,sizeof(recvline),0);
    if(len > 0){
        printf("secv say: %s",recvline);
    }else if(len == 0){
        printf("服务器断开了链接...\n");
        break;
    }else{
        perror("secv");
        break;
    }
    sleep(2);
    }
 close(sockfd);
    return 0;
}
