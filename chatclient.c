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
#define   MAXSIZE    1032
#define   IP     "127.0.0.1" //ip地址

#define   USER_SIGN     1      //用户注册
#define   USER_CHANGE   2      //用户更改密码
#define   USER_LOGIN    3      //用户登陆
#define   USER_LOGOUT   4      //用户登出
#define   USER_MASSAGE  5      //用户消息
#define   USER_AGREE    6      //用户同意
#define   USER_DISAG    7      //用户拒绝
#define   USER_PRIVATE  8      //用户私聊
#define   USER_GROUP    9      //用户群聊
#define   USER_FILE     10     //用户文件
#define   USER_NUM      11     //在线人数
#define   USER_KICK     12     //用户踢人
#define   USER_MASTER   13     //用户群主
#define   USER_UNMASTER 14     //不是群主
#define   USER_PHCHG    15     //电话修改
#define   USER_EMCHG    16     //邮箱修改
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

int l = 0;
int choice = 0;
int connfd;
int flag = 1;

void my_err(const char *err_string, int line)
{
	fprintf(stderr, "line:%d ", line);
	perror(err_string);
	exit(1);
}

void sighandler(){
    free(rd);
	free(sd);
	exit(0);
}

void my_chatroom(){
    printf("Welcome");
    getchar();
}


void client_menu(){

    while(1){
        while(1){
        
        printf("\t***************************************\n");
        printf("\t************欢迎来到登录界面***********\n");
        printf("\t***************************************\n");
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
        case 1://客户端注册
            bzero(sd,sizeof(msg));
            sd->type = choice;
            if((send(connfd,sd,MAXSIZE,0)) < 0){
                perror("send_client_sign0");
            }
            bzero(rd,sizeof(msg));
            if((l = recv(connfd,rd,MAXSIZE,0)) < 0){
                perror("recv_client_login1");
                getchar();
                exit(1);
            }else if(l > 0){
            if(rd->type == USER_SIGN){   //获得登陆认可
            bzero(sd,sizeof(msg));
            sd->type = USER_MASSAGE;
            printf("请输入用户名：");
            scanf("%s",sd->data);
            getchar();
            sd->msglen = strlen(sd->data);
            sd->data[sd->msglen] = '\0';
            if((send(connfd,sd,MAXSIZE,0)) < 0){
                perror("send_client_sign1");
            }
            bzero(rd,sizeof(msg));
            if((l = recv(connfd,rd,MAXSIZE,0)) < 0){
                perror("recv_client_login1");
                getchar();
                exit(1);
            }else if(l > 0){
                if(rd->type == USER_DISAG){
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
                    if(send(connfd,sd,MAXSIZE,0) < 0){
                        perror("send_client_sign2");
                    }
                    bzero(rd,sizeof(msg));
                    if((l = recv(connfd,rd,MAXSIZE,0)) < 0){
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
                bzero(sd,sizeof(msg));
                sd->type = USER_MASSAGE;
                printf("请输入邮箱：");
                scanf("%s",sd->data);
                getchar();
                sd->msglen = strlen(sd->data);
                if(send(connfd,sd,MAXSIZE,0) < 0){
                    perror("send_client_sign3");
                }
                bzero(rd,sizeof(msg));
                if((l = recv(connfd,rd,MAXSIZE,0)) < 0){
                    perror("recv_sigin3");
                }
                if(rd->type == USER_AGREE){
                    printf("邮箱保存成功！！！\n");
                    printf("按任意键继续...\n");
                    getchar();
                    printf("\033c");
                    //输入用户电话
                    bzero(sd,sizeof(msg));
                sd->type = USER_MASSAGE;
                printf("请输入电话：");
                scanf("%s",sd->data);
                getchar();
                sd->msglen = strlen(sd->data);
                if(send(connfd,sd,MAXSIZE,0) < 0){
                    perror("send_client_sign4");
                }
                bzero(rd,sizeof(msg));
                if((l = recv(connfd,rd,MAXSIZE,0)) < 0){
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
            }
           }
        break;



        case 2://客户端登陆
            bzero(sd,sizeof(msg));
            sd->type = choice;
            sd->msglen = 0;
            if((send(connfd,sd,MAXSIZE,0)) < 0){
                my_err("send_log",__LINE__);
            }
            bzero(rd,sizeof(msg));
            if((l = recv(connfd,rd,MAXSIZE,0)) < 0){
                my_err("recv_client_login",__LINE__);
            }else if(0 == l){
                printf("服务器已断开...");
                getchar();
            }
            if(rd->type == USER_AGREE){
                bzero(sd,sizeof(msg));
                sd->type = USER_MASSAGE;
                printf("请输入您的用户名：");
                scanf("%s",sd->data);
                getchar();
                sd->msglen = strlen(sd->data);
                sd->data[sd->msglen] = '\0';
                if(send(connfd,sd,MAXSIZE,0) < 0){
                    my_err("send_client_login",__LINE__);
                }
                bzero(rd,sizeof(msg));
                if((l = recv(connfd,rd,MAXSIZE,0)) < 0){
                    my_err("recv_client_login",__LINE__);
                }else if(0 == l){
                    printf("服务器已断开...");
                    getchar();
                }
                if(rd->type == USER_AGREE){

                bzero(sd,sizeof(msg));

                sd->type = USER_MASSAGE;
                printf("请输入您的密码：");
                scanf("%s",sd->data);
                getchar();
                sd->msglen = strlen(sd->data);
                sd->data[sd->msglen] = '\0';
                if(send(connfd,sd,MAXSIZE,0) < 0){
                    my_err("send_client_login",__LINE__);
                }

                bzero(rd,sizeof(msg));

                if((l = recv(connfd,rd,MAXSIZE,0))< 0 ){
                    my_err("recv_client_login",__LINE__);
                }


                if(rd->type == USER_AGREE){
                    printf("密码正确！！！\n");
                    printf("正在请求登陆...\n");

                    bzero(sd,sizeof(msg));
                    sd->type = USER_LOGIN;
                    sd->msglen = 0;
                    if(send(connfd,sd,MAXSIZE,0) < 0 ){
                        my_err("send_client_login",__LINE__);
                    }
                    bzero(rd,sizeof(msg));
                    if((l = recv(connfd,rd,MAXSIZE,0)) < 0){
                        my_err("recv_client_login",__LINE__);
                    }else if(l == 0){
                        printf("服务器已断开...");
                    }
                    if(rd->type == USER_AGREE){
                        printf("登陆成功！！！\n");
                        my_chatroom();
                    }else{
                        printf("登陆失败！！！");
                        continue;
                    }
                }else if(rd->type == USER_DISAG){
                    printf("密码错误！！！");
                    getchar();
                }
            }else{
                printf("没有找到用户信息\n");
                getchar();
            }
        }
        break;
        
        case 3://用户修改密码
            bzero(sd,sizeof(msg));
            sd->type = choice;
            sd->msglen = 0;
            if((send(connfd,sd,MAXSIZE,0)) < 0){
                my_err("send_log",__LINE__);
            }
            bzero(rd,sizeof(msg));
            if((l = recv(connfd,rd,MAXSIZE,0)) < 0){
                my_err("recv",__LINE__);
            }else if(0 == l){
                printf("服务器断开连接...");
                getchar();
            }
            if(rd->type == USER_AGREE){
                bzero(sd,sizeof(msg));
                sd->type = USER_MASSAGE;
                printf("请输入您的用户名：");
                scanf("%s",sd->data);
                getchar();
                sd->msglen = strlen(sd->data);
                sd->data[sd->msglen] = '\0';
                if(send(connfd,sd,MAXSIZE,0) < 0){
                    my_err("send",__LINE__);
                }
                bzero(rd,sizeof(msg));
                if((l = recv(connfd,rd,MAXSIZE,0)) < 0){
                    my_err("recv",__LINE__);
                }else if(0 == l){
                printf("服务器断开连接...");
                getchar();
            }
            if(rd->type == USER_AGREE){
                printf("\t******************\n");
                printf("\t*****修改密码*****\n");
                printf("\t******************\n");
                printf("\t****1.邮箱验证****\n");
                printf("\t****          ****\n");
                printf("\t****2.电话验证****\n");
                printf("\t****          ****\n");
                printf("\t****  0.退出  ****\n");
                printf("请选择：   (1 or 2 or 0)\n");
                scanf("%d",&choice);
                getchar();
                if((choice != 1)&&(choice != 2)&&(choice != 0)){
                    printf("输入非法数字，请重新输入！\n");
                    printf("按任意键返回...\n");
                    getchar();
                    continue;
                }
                switch(choice){
                    case 1:
                        bzero(sd,sizeof(msg));
                        sd->type = USER_EMCHG;
                        printf("请输入您的邮箱：");
                        scanf("%s",sd->data);
                        getchar();
                        sd->msglen = strlen(sd->data);
                        sd->data[sd->msglen] = '\0';
                        if(send(connfd,sd,MAXSIZE,0) < 0){
                            my_err("send",__LINE__);
                        }
                        bzero(rd,sizeof(msg));
                        if((l = recv(connfd,rd,MAXSIZE,0))< 0){
                            my_err("recv",__LINE__);
                        }else if(0 == l){
                            printf("与服务器断开连接...");
                        }if(rd->type == USER_AGREE){
                            bzero(sd,sizeof(msg));
                            sd->type = USER_MASSAGE;
                            do{
                                printf("请输入新的密码：(不超过20位)");
                                scanf("%s",sd->data);
                                getchar();
                            if(strlen(sd->data) > 20){
                                printf("输入非法\n");
                                getchar();
                                printf("\033c");
                                continue;
                            }
                            else
                                break;
                            }while(1);
                            sd->msglen = strlen(sd->data);
                            sd->data[sd->msglen] = '\0';
                            if(send(connfd,sd,MAXSIZE,0) < 0){
                                my_err("send",__LINE__);
                            }
                            bzero(rd,sizeof(msg));
                            if((l = recv(connfd,rd,MAXSIZE,0)) < 0){
                                my_err("recv",__LINE__);
                            }
                            if(rd->type == USER_AGREE){
                                printf("%s",sd->data);
                            }else if(rd->type == USER_DISAG){
                                printf("%s",sd->data);
                            }
                        }else{
                            printf("邮箱错误，修改失败\n");
                            getchar();
                        }
                    break;

                    case 2:
                        bzero(sd,sizeof(msg));
                        sd->type = USER_PHCHG;
                        printf("请输入您的电话：");
                        scanf("%s",sd->data);
                        getchar();
                        sd->msglen = strlen(sd->data);
                        sd->data[sd->msglen] = '\0';
                        if(send(connfd,sd,MAXSIZE,0) < 0){
                            my_err("send",__LINE__);
                        }
                        bzero(rd,sizeof(msg));
                        if((l = recv(connfd,rd,MAXSIZE,0))< 0){
                            my_err("recv",__LINE__);
                        }else if(0 == l){
                            printf("与服务器断开连接...");
                        }if(rd->type == USER_AGREE){
                            bzero(sd,sizeof(msg));
                            sd->type = USER_MASSAGE;
                            do{
                                printf("请输入新的密码：(不超过20位)");
                                scanf("%s",sd->data);
                                getchar();
                            if(strlen(sd->data) > 20){
                                printf("输入非法\n");
                                getchar();
                                printf("\033c");
                                continue;
                            }
                            else
                                break;
                            }while(1);
                            sd->msglen = strlen(sd->data);
                            sd->data[sd->msglen] = '\0';
                            if(send(connfd,sd,MAXSIZE,0) < 0){
                                my_err("send",__LINE__);
                            }
                            bzero(rd,sizeof(msg));
                            if((l = recv(connfd,rd,MAXSIZE,0)) < 0){
                                my_err("recv",__LINE__);
                            }
                            if(rd->type == USER_AGREE){
                                printf("%s",sd->data);
                                getchar();
                            }else if(rd->type == USER_DISAG){
                                printf("%s",sd->data);
                                getchar();
                            }
                        }else{
                            printf("电话错误，修改失败\n");
                            getchar();
                        }
                    break;
                    case 0:
                    return;
                
            }
        }else{
            printf("没有找到该用户！！！\n");
            printf("按任意键退出\n");
            getchar();
            return;
        }
    }
        break;





        case 4:
        


        break;

        case 0:
        return;
        break;
        }
    }
  }
}




















int main(){
    struct sockaddr_in servaddr;
    rd = (msg*)malloc(MAXSIZE);
    sd = (msg*)malloc(MAXSIZE);

    if((connfd = socket(AF_INET,SOCK_STREAM,0)) < 0){
        perror("socket");
    }

    bzero(&servaddr,sizeof(struct sockaddr_in));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    inet_pton(AF_INET,"127.0.0.1",(void*)&servaddr.sin_addr.s_addr);


    //与服务器建立连接
    printf("正在与服务器建立连接...\n");
    if((connect(connfd,(struct sockaddr*)&servaddr,sizeof(servaddr))) < 0){
        perror("connect");
    }
    printf("与服务器链接成功！！！\n");
    client_menu();
    return 0;










//     while(1){
//         while(1){
//         printf("\t***************************************\n");
//         printf("\t*****     欢迎来到登录界面        *****\n");
//         printf("\t*****                             *****\n");
//         printf("\t*****        1.注册               *****\n");
//         printf("\t*****                             *****\n");
//         printf("\t*****        2.登陆               *****\n");
//         printf("\t*****                             *****\n");
//         printf("\t*****        3.找回密码           *****\n");
//         printf("\t*****                             *****\n");
//         printf("\t*****        0.退出               *****\n");
//         printf("\t*****                             *****\n");
//         printf("\t***************************************\n");
//         printf("请选择:");
//         scanf("%d",&choice);
//         getchar();
//         switch(choice){
//         case 1:
            
//             bzero(sd,sizeof(msg));
//             sd->type = choice;
//             if((send(connfd,sd,MAXSIZE,0)) < 0){
//                 perror("send_client_sign0");
//             }
//             bzero(rd,sizeof(msg));
//             if((l = recv(connfd,rd,MAXSIZE,0)) < 0){
//                 perror("recv_client_login1");
//                 getchar();
//                 exit(1);
//             }else if(l > 0){
//             if(rd->type == USER_SIGN){   //获得登陆认可
//             bzero(sd,sizeof(msg));
//             sd->type = USER_MASSAGE;
//             printf("请输入用户名：");
//             scanf("%s",sd->data);
//             getchar();
//             sd->msglen = strlen(sd->data) + 1;
//             sd->data[sd->msglen] = '\0';
//             if((send(connfd,sd,MAXSIZE,0)) < 0){
//                 perror("send_client_sign1");
//             }
//             bzero(rd,sizeof(msg));
//             if((l = recv(connfd,rd,MAXSIZE,0)) < 0){
//                 perror("recv_client_login1");
//                 getchar();
//                 exit(1);
//             }else if(l > 0){
//                 if(rd->type == USER_DISAG){
//                 printf("该用户名已被注册...\n");
//                 printf("按任意键继续...\n");
//                 getchar();
//                 printf("\033c");
//                 continue;
//                 }else{
//                     while(1){
//                     //当可以注册时，输入密码信息
//                     bzero(sd,sizeof(msg));
//                     sd->type = USER_MASSAGE;
//                     printf("请输入密码："); //记得加密
//                     scanf("%s",pass);
//                     getchar();
//                     printf("\n请再次输入密码：");
//                     scanf("%s",pass_t);
//                     getchar();
//                     if(strcmp(pass,pass_t) == 0 ){
//                     strcpy(sd->data,pass);
//                     sd->msglen = strlen(sd->data);
//                     if(send(connfd,sd,MAXSIZE,0) < 0){
//                         perror("send_client_sign2");
//                     }
//                     bzero(rd,sizeof(msg));
//                     if((l = recv(connfd,rd,MAXSIZE,0)) < 0){
//                         perror("recv_sign2");
//                     }
//                     if(rd->type == USER_AGREE){
//                         printf("密码保存成功！\n");
//                         printf("按任意键继续...");
//                         getchar();
//                         break;
//                     }
//                     }else{
//                         printf("两次密码输入不一致，请重新输入...\n");
//                         printf("按任意键继续...");
//                         getchar();
//                         printf("\033c");
//                         continue;
//                     }
//                 }
//                 //输入账户email
//                 bzero(sd,sizeof(msg));
//                 sd->type = USER_MASSAGE;
//                 printf("请输入邮箱：");
//                 scanf("%s",sd->data);
//                 getchar();
//                 sd->msglen = strlen(sd->data);
//                 if(send(connfd,sd,MAXSIZE,0) < 0){
//                     perror("send_client_sign3");
//                 }
//                 bzero(rd,sizeof(msg));
//                 if((l = recv(connfd,rd,MAXSIZE,0)) < 0){
//                     perror("recv_sigin3");
//                 }
//                 if(rd->type == USER_AGREE){
//                     printf("邮箱保存成功！！！\n");
//                     printf("按任意键继续...\n");
//                     getchar();
//                     printf("\033c");
//                     //输入用户电话
//                     bzero(sd,sizeof(msg));
//                 sd->type = USER_MASSAGE;
//                 printf("请输入电话：");
//                 scanf("%s",sd->data);
//                 getchar();
//                 sd->msglen = strlen(sd->data);
//                 if(send(connfd,sd,MAXSIZE,0) < 0){
//                     perror("send_client_sign4");
//                 }
//                 bzero(rd,sizeof(msg));
//                 if((l = recv(connfd,rd,MAXSIZE,0)) < 0){
//                     perror("recv_sigin4");
//                 }
//                 if(rd->type == USER_AGREE){
//                 printf("用户注册成功！！！\n");
//                 getchar();
//                 printf("\033c");
//                 break;
//                 }
//                }
//               }
//              }
//             }
//            }
//         break;








//         case 0:
//         return 0;
//         break;
//         }
//     }
// }
}