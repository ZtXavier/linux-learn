#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<signal.h>
#include<unistd.h>
#include<stdlib.h>
#include<assert.h>
#include<stdio.h>
#include<string.h>
#include<stdbool.h>

static bool stop = false;

/* SIGTERM 信号的处理函数，触发时结束主程序中的循环 */

static void handle_term(int sig)
{
    stop = true;
}

int main(int argc,char *argv[])
{
    signal(SIGTERM,handle_term);
    if (argc <= 3)
    {
        printf("usage: %s ip_address port_number backlog\n",basename(argv[0]));
        return 1;
    }
    const char*ip = argv[1];
    int port = atoi(argv[2]);
    int backlog = atoi(argv[3]);
    int sock = socket(PF_INET,SOCK_STREAM,0);

    /* 创建一个IPV4的socket的地址 */
    assert(sock >= 0);

    struct sockaddr_in address;              //直接使用该结构体
    bzero(&address,sizeof(address));       //初始化字段
    address.sin_family = AF_INET;                   
    inet_pton(AF_INET,ip,&address.sin_addr);        // 将字节序转换为网络字节序
    address.sin_port = htons(port);                       
     
    int ret = bind(sock,(struct sockaddr*)&address,sizeof(address));   //将sock进行连接
    assert(ret != -1);
    ret = listen(sock,backlog);
    assert(ret != -1);

    /* 循环等待链接，直到有信号将它中断 */
    while( !stop )
    {
        sleep(1);
    }
    /* 关闭socket */
    close(sock);
    return 0;
}

//backlog 参数是指所有处于半链接状态和链接状态的socket上限
