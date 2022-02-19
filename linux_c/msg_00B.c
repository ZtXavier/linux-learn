#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<assert.h>
#include<arpa/inet.h>

int main(int argc,char *argv[])
{
    if(argc <= 2)
    {
        printf("usage: %s ip_address port_number\n",basename(argv[0]));
        return 1;
    }
    
}