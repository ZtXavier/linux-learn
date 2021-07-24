#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<string.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/epoll.h>
#include<errno.h>
#include<stdlib.h>
#include<pthread.h>
#include<signal.h>
#include<mysql/mysql.h>
#include"head.h"
#include"mysql.h"


recv_datas  *send_data;
recv_datas  *recv_data;

void my_err(const char *err_string, int line)
{
	fprintf(stderr, "line:%d ", line);
	perror(err_string);
	exit(1);
}


void first_menu(void){
        printf("\t***************************************\n");
        printf("\t************欢迎来到登录界面***********\n");
        printf("\t***************************************\n");
        printf("\t*****        1.登录               *****\n");
        printf("\t*****                             *****\n");
        printf("\t*****        2.注册               *****\n");
        printf("\t*****                             *****\n");
        printf("\t*****        3.找回密码           *****\n");
        printf("\t*****                             *****\n");
        printf("\t*****        4.退出               *****\n");
        printf("\t***************************************\n");
        printf("\t***************************************\n");
}

void second_menu(void){
        printf("\t***************************************\n");
        printf("\t********welcome to chatroom************\n");
        printf("\t***************************************\n");
        printf("\t*****        1.修改密码           *****\n");
        printf("\t*****                             *****\n");
        printf("\t*****        2.注册               *****\n");
        printf("\t*****                             *****\n");
        printf("\t*****        3.找回密码           *****\n");
        printf("\t*****                             *****\n");
        printf("\t*****        4.找回密码           *****\n");
        printf("\t*****                             *****\n");
        printf("\t*****        9.退出               *****\n");
        printf("\t*****                             *****\n");
        printf("\t***************************************\n");

}

//来用写发数据
void *read_mission(void*arg){
    int   connfd = *(int*)arg;
    int   choice;
    int   i;
    int   ret;
    char  ch;
    char  password1[24],password2[24];
    send_data = (recv_datas*)malloc(sizeof(recv_datas));

    while(1){
        /*  1，登录 2注册 3*/
        first_menu();
        printf("请选择: ");
        scanf("%d",&choice);
        getchar();
        switch(choice){
            case 1:
            send_data->type = USER_LOGIN;
            printf("please input id: ");
            scanf("%d",&send_data->send_id);
            getchar();
            printf("please input password: ");
            i = 0;
            while(1){
            scanf("%c",&ch);
            //printf("\b");
            if(ch == '\n'){
            send_data->read_buff[i] = '\0';
            break;
            }
            //printf("*");
            send_data->read_buff[i++] = ch;
            }
            printf("\n");
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
            my_err("send",__LINE__);
            }
            pthread_mutex_lock(&cl_mu);
            pthread_cond_wait(&cl_co,&cl_mu);
            pthread_mutex_unlock(&cl_mu);
            printf("\t--%s--\n",send_data->write_buff);
            break;


            case 2:
                i = 0;
                send_data->type = USER_SIGN;
                printf("please input you name: ");
                scanf("%s",send_data->send_name);
                getchar();
                printf("please input passwd: ");
                while(1){
                scanf("%c", &ch);
                if(ch == '\n'){
                password1[i] = '\0';
                break;
                }
                password1[i++] = ch;
                //printf("*");
                }
                printf("\n");
                printf("please input passwd again: ");
                i = 0;
                while(1){
                scanf("%c", &ch);
                if(ch == '\n'){
                password2[i] = '\0';
                break;
                }
                password2[i++] = ch;
                    //printf("*");
                }
                printf("\n");
                if(strcmp(password1,password2) == 0){
                    printf("输入一致!\n");
                    bzero(send_data->read_buff,sizeof(send_data->read_buff));
                    strcpy(send_data->read_buff,password1);
                    if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
                        my_err("send",__LINE__);
                    }
                    pthread_mutex_lock(&cl_mu);
                    pthread_cond_wait(&cl_co,&cl_mu);
                    pthread_mutex_unlock(&cl_mu);
                }else{
                    printf("两次输入不一致\n");
                    printf("按回车继续\n");
                    choice = 10;
                    getchar();
                }
        break;

        case 3:
        send_data->type = USER_FIND;
        printf("please input your id:");
        scanf("%d",&send_data->send_id);
        getchar();
        printf("please input your nickname:");
        scanf("%s",send_data->send_name);
        getchar();
        if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
            my_err("send",__LINE__);
        }
        pthread_mutex_lock(&cl_mu);
        pthread_cond_wait(&cl_co,&cl_mu);
        pthread_mutex_unlock(&cl_mu);
        printf("%s\n",send_data->write_buff);
        printf("按任意键返回...");
        getchar();
        break;
        
        case 4:
        send_data->type = USER_OUT;
        if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
        my_err("send",__LINE__);
        }
        pthread_exit(0);
        break;

        default:
        printf("非法输入！\n");
        printf("按任意键继续\n");
        getchar();
        break;

        }if((choice > 4) || (choice < 1))    continue;
        else if (choice == 1) {
            if (strcmp(send_data->write_buff, "id error") == 0) {
                printf("ID or PASSWD error!!!\n");
                printf("按任意键继续.....\n");
                getchar();
                continue;
            } else {
                printf("登陆成功!!\n");
                printf("按下回车继续......\n");
                getchar();
                break;
            }
        }
        else if (choice == 2){
            printf("%s\n",send_data->write_buff);
            printf("您的账号为:%d\n", send_data->send_id);
            printf("按下回车继续.......");
            getchar();
            continue;
        }else if(choice == 3)   continue;
    }
    while(1){
    second_menu();
    printf("请选择: ");
    scanf("%d",&choice);
    getchar();
    switch(choice){
    case 1:         //修改密码
    send_data->type = USER_CHANGE;
    printf("please input your original passwd: ");
    i = 0;
    while(1){
    scanf("%c",&ch);
    if(ch == '\n'){
    send_data->read_buff[i] = '\0';
    break;
    }
    send_data->read_buff[i++] = ch;

    }
    printf("\n");
    printf("please input your new passwd: ");
    i = 0;
    while(1){
    scanf("%c",&ch);
    if(ch == '\n'){
    send_data->write_buff[i] = '\0';
    break;
    }
    send_data->write_buff[i++] = ch;

    }
    if((ret = send(connfd,send_data,sizeof(recv_datas),0)) < 0){
        my_err("send",__LINE__);
    }
    pthread_mutex_lock(&cl_mu);
    pthread_cond_wait(&cl_co,&cl_mu);
    pthread_mutex_unlock(&cl_mu);
    printf("%s\n",send_data->write_buff);
    if(strcmp(send_data->write_buff,"success") == 0){
        printf("change passwd success!!!\n");
        printf("按任意键继续...\n");
        getchar();
    }else if(strcmp(send_data->write_buff,"error") == 0){
        printf("original passwd error");
        printf("按任意键继续...\n");
        getchar();
    }
    bzero(send_data->write_buff,sizeof(send_data->write_buff));
    bzero(send_data->read_buff,sizeof(send_data->read_buff));
    break;






    case 9:         //二级界面退出
    send_data->type = USER_OUT;
        if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
        my_err("send",__LINE__);
        }
        pthread_exit(0);
        break;
    }
    }
}
void *write_mission(void*arg){
    int   connfd = *(int*)arg;
    int   ret;
    recv_data  = (recv_datas*)malloc(sizeof(recv_datas));
    while(1){
        bzero(recv_data,sizeof(recv_datas));
        if((ret = recv(connfd,recv_data,sizeof(recv_datas),0)) < 0){
            my_err("recv",__LINE__);
        }
        switch(recv_data->type){
            case USER_OUT:
                printf("客户端退出...");
                pthread_exit(0);
                break;

            case USER_LOGIN:
                strcpy(send_data->send_name,recv_data->send_name);
                bzero(send_data->write_buff,sizeof(send_data->write_buff));
                strcpy(send_data->write_buff,recv_data->write_buff);
                send_data->sendfd = recv_data->recvfd;


                pthread_mutex_lock(&cl_mu);
                pthread_cond_signal(&cl_co);
                pthread_mutex_unlock(&cl_mu);
                break;

            case USER_SIGN:
            send_data->send_id = recv_data->send_id;
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_mutex_lock(&cl_mu);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case USER_FIND:
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_mutex_lock(&cl_mu);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case ID_ERROR:
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_mutex_lock(&cl_mu);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case USER_CHANGE:
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_mutex_lock(&cl_mu);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;
        }


}
}



int main(){
    int connfd;
    pthread_t tid1, tid2;
    struct sockaddr_in servaddr;
    pthread_mutex_init(&cl_mu, NULL);
    pthread_cond_init(&cl_co, NULL);
    if((connfd = socket(AF_INET, SOCK_STREAM,0))<0){
        my_err("socket",__LINE__);
    }
    bzero(&servaddr,sizeof(struct sockaddr_in));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    inet_pton(AF_INET,"127.0.0.1",(void*)&servaddr.sin_addr.s_addr);
    printf("正在与服务器建立连接...\n");
    if((connect(connfd,(struct sockaddr*)&servaddr,sizeof(servaddr))) < 0){
        perror("connect");
    }else
    printf("与服务器链接成功！！！\n");
    pthread_create(&tid1,NULL,read_mission,(void*)&connfd);
    pthread_create(&tid2,NULL,write_mission,(void*)&connfd);
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    return 0;
}



