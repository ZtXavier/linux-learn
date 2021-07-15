#include"unp.h"
//客户端
int main(int argc,char *argv[]){
    int     sockfd,n;
    char    recvline[MAXLINE + 1];
    struct  sockaddr_in    servaddr;

    if(argc != 2)
    err_quit("usage: a.out <IPaddress");

    if((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0)  //创建tcp套接字，socket函数创建一个网际(AF_INET)字节流(sock_INET)套接字，该函数返回小整数描述符
    err_sys("socket error");

    bzero(&servaddr,sizeof(servaddr));   //将套接字结构清零

    servaddr.sin_family = AF_INET;       //调用htons转换二进制端口号，由主机字节顺序为网络字节顺序
    servaddr.sin_port = htons(13);
    //把命令转化为合适的格式
    if(inet_pton(AF_INET,argv[1],&servaddr.sin_addr) <= 0){
        err_quit("inet_pton error for %s",argv[1]);
    }
    //与服务器建立联接
    if(connect(sockfd,(SA *)&servaddr,sizeof(servaddr)) < 0)
    err_sys("connect error");
    //读入与服务器的联接应答
    while((n = read(sockfd,recvline,MAXLINE)) > 0){
    recvline[n] = 0;
    if((fputs(recvline,stdout)) == EOF)
    err_sys("fputs error");
    }
    if(n < 0){
        err_sys("read error");
    }
    //终止程序
    exit(0);
}

