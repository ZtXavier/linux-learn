#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>



int main(int argc,char*argv[])
{
    assert(argc == 2);
    char *host = argv[1];
    /* 获取目标地址信息 */
    struct hostent *hostinfo = gethostbyname(host);
    assert(hostinfo);
    /* 获取daytime服务信息 */
    struct servent *serinfo = getservbyname("daytime","tcp");
    assert(serinfo);
    printf("daytime port is %d\n",ntohs(serinfo->s_port));
    struct sockaddr_in address;
    address.sin_family = AF_INET; /* ipv4协议族 */
    address.sin_port = serinfo->s_port;
    address.sin_addr = *(struct in_addr*)*hostinfo->h_addr_list;
    int sockfd = socket(AF_INET, SOCK_STREAM,0);
    int result = connect(sockfd,(struct sockaddr*)&address,sizeof(address));

    assert(result != -1);
    char buffer[128];
    result = read(sockfd,buffer,sizeof(buffer));
    assert(result > 0);
    printf("the day time is %s",buffer);
    close(sockfd);
    return 0;
}