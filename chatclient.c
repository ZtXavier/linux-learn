#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<time.h>
#include<mysql/mysql.h>

#define   PORT 9988          //端口号
#define   LINSTENNUM  20      //最大监听数
#define   MSGSIZE    1024    //最大消息长度
#define   IP     "127.0.0.1" //ip地址

#define   USER_SIGN    1      //用户注册
#define   USER_CHANGE  2      //用户更改密码
#define   USER_LOGIN   4      //用户登陆
#define   USER_LOGOUT  8      //用户登出
#define   USER_MASSAGE 16     //用户消息
#define   USER_AGREE   32     //用户同意
#define   USER_DISAG   64     //用户拒绝
#define   USER_PRIVATE 128    //用户私聊
#define   USER_GROUP   256    //用户群聊
#define   USER_FILE    512    //用户文件
#define   USER_NUM     1024   //在线人数
#define   USER_KICK    2048   //用户踢人
#define   USER_MASTER  100    //用户群主
#define   USER_UNMASTER 50    //不是群主
#define   USER_
#define   USER_
#define   USER_
#define   USER_


char pass[24],pass_t[24];



typedef struct  Userinfo
{
    int state;          //用户状态
    char name[24];      //用户名
    char password[24];  //密码
    char email[24];     //邮箱
    char phonenum[24];  //电话
}user;


typedef struct message{
int    type;            //消息类型
int    msglen;          //消息长度
char   data[MSGSIZE];   //消息数据
}msg;

typedef struct mesg{
int    type;            //消息类型
int    msglen;          //消息长度
user   use;             //消息数据
}mg;
msg *rd,*sd;
mg  *urd,*usd;

int main(){
    int l = 0;
    int choice = 0;
    int connfd;
    struct sockaddr_in servaddr;
    rd = (msg*)malloc(MSGSIZE);
    sd = (msg*)malloc(MSGSIZE);
    if((connfd = socket(AF_INET,SOCK_STREAM,0)) < 0){
        perror("socket");
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    inet_pton(AF_INET,"127.0.0.1",(void*)&servaddr.sin_addr);

    //与服务器建立连接
    printf("正在与服务器建立连接...\n");
    if((connect(connfd,(struct sockaddr*)&servaddr,sizeof(servaddr))) < 0){
        perror("connect");
    }
    printf("与服务器链接成功！！！\n");
    while(1){
        while(1){
        printf("\t***************************************\n");
        printf("\t*****     欢迎来到登录界面        *****\n");
        printf("\t*****                             *****\n");
        printf("\t*****        1.注册               *****\n");
        printf("\t*****                             *****\n");
        printf("\t*****        2.登陆               *****\n");
        printf("\t*****                             *****\n");
        printf("\t*****        3.找回密码           *****\n");
        printf("\t*****                             *****\n");
        printf("\t*****        0.退出               *****\n");
        printf("\t*****                             *****\n");
        printf("\t***************************************\n");
        printf("请选择:");
        scanf("%d",&choice);
        getchar();
        switch(choice){
        case 1:
            bzero(sd,sizeof(msg));
            sd->type = USER_MASSAGE;
            printf("请输入用户名：");
            scanf("%s",sd->data);
            getchar();
            sd->msglen = strlen(sd->data);
            if((send(connfd,(void*)&sd,sizeof(msg),0)) < 0){
                perror("send_client_sign1");
            }
            bzero(rd,sizeof(msg));
            if((l = recv(connfd,(void *)&rd,sizeof(msg),0)) < 0){
                perror("recv_client_login1");
                getchar();
                exit(1);
            }else if(l > 0){
                if(rd->type = USER_DISAG){
                printf("该用户名已被注册...\n");
                printf("按任意键继续...\n");
                getchar();
                printf("\033c");
                continue;
                }else{
                    while(1){
                    //当可以注册时，输入密码信息
                    bzero(sd,sizeof(msg));
                    sd->type = USER_MASSAGE;
                    printf("请输入密码："); //记得加密
                    scanf("%s",pass);
                    getchar();
                    printf("\n请再次输入密码：");
                    scanf("%s",pass_t);
                    getchar();
                    if(strcmp(pass,pass_t) == 0 ){
                    strcpy(sd->data,pass);
                    sd->msglen = strlen(sd->data);
                    if(send(connfd,(void *)&sd,sizeof(msg),0) < 0){
                        perror("send_client_sign2");
                    }
                    bzero(rd,sizeof(rd));
                    if((l = recv(connfd,(void *)&rd,sizeof(msg),0)) < 0){
                        perror("recv_sign2");
                    }
                    if(rd->type == USER_AGREE){
                        printf("密码保存成功！\n");
                        printf("按任意键继续...");
                        getchar();
                        break;
                    }
                    }else{
                        printf("两次密码输入不一致，请重新输入...\n");
                        printf("按任意键继续...");
                        getchar();
                        printf("\033c");
                        continue;
                    }
                }
                //输入账户email
                bzero(sd,sizeof(sd));
                sd->type = USER_MASSAGE;
                printf("请输入邮箱：");
                scanf("%s",sd->data);
                getchar();
                sd->msglen = strlen(sd->data);
                if(send(connfd,(void *)&sd,sizeof(msg),0) < 0){
                    perror("send_client_sign3");
                }
                bzero(rd,sizeof(rd));
                if((l = recv(connfd,(void*)&rd,sizeof(msg),0)) < 0){
                    perror("recv_sigin3");
                }
                if(rd->type == USER_AGREE){
                    printf("电话保存成功！！！\n");
                    printf("按任意键继续...\n");
                    getchar();
                    printf("\033c");
                    //输入用户电话
                    bzero(sd,sizeof(sd));
                sd->type = USER_MASSAGE;
                printf("请输入电话：");
                scanf("%s",sd->data);
                getchar();
                sd->msglen = strlen(sd->data);
                if(send(connfd,(void *)&sd,sizeof(msg),0) < 0){
                    perror("send_client_sign4");
                }
                bzero(rd,sizeof(rd));
                if((l = recv(connfd,(void*)&rd,sizeof(msg),0)) < 0){
                    perror("recv_sigin4");
                }
                if(rd->type == USER_AGREE){
                printf("用户注册成功！！！\n");
                getchar();
                printf("\033c");
                break;
                }
               }
              }
             }
        break;








        case 0:
        return 0;
        break;
        }
    }
}
}