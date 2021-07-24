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
#include"pack.h"
#include"mysql.h"
#include"ser_deal.h"

typedef struct  Userinfo
{
    int state;          //用户状态
    char name[24];      //用户名
    char password[24];  //密码
    char email[24];     //邮箱
    char phonenum[24];  //电话
}user;



void my_err(const char *err_string, int line)
{
	fprintf(stderr, "line:%d ", line);
	perror(err_string);
	exit(1);
}

MYSQL init_mysql(void){
    MYSQL  mysql;
    if(NULL == mysql_init(&mysql)){
        my_err("mysql_init",__LINE__);
    }
    if(mysql_library_init(0,NULL,NULL) != 0 ){
        my_err("mysql_library_init",__LINE__);
    }
    if(!mysql_real_connect(&mysql,"127.0.0.1","root","123456","chat",0,NULL,0)){
        my_err("mysql_real_connect",__LINE__);
    }
    if(mysql_set_character_set(&mysql, "utf8") < 0){
		my_err("mysql_set_character_set", __LINE__);
	}
    printf("连接mysql数据库成功!\n");
	return mysql;
}


// void sighandler(){

// }

list_status_info status_list_create(MYSQL mysql,list_status_info in_status){
    MYSQL_RES  *res;
    MYSQL_ROW   row;
    int ret = mysql_query(&mysql,"select * from person;");
    if(!ret){
    res = mysql_store_result(&mysql);
    while(row = mysql_fetch_row(res)){
    list_status_info status;
    List_Init(status,node_status_info);
    status->status = atoi(row[5]);            //传入表中用户状态
    status->fdtext = atoi(row[6]);            //传入fd
    strcpy(status->account,row[1]);     //传入姓名
    List_AddTail(in_status,status);
    }
    }
    return in_status;
}





void user_login(recv_info *mysock,MYSQL* mysql){
    int len = 0;
    MYSQL_RES *res = NULL;
    MYSQL_ROW row;
    msg *rd,*sd;
    rd =(msg*)malloc(sizeof(msg));//记得free
    sd =(msg*)malloc(sizeof(msg));
    char  sql[MYSQL_MAX];
    bzero(sd,sizeof(msg));
    sd->type = USER_AGREE;
    sd->msglen = 0;

    sprintf(sql,"select * from person where name = \'%s\';",mysock->send_Account);
    int ret = mysql_query(mysql,sql);
}

void user_sign(recv_info *mysock,MYSQL* mysql){
    int len = 0;
    MYSQL_RES *res = NULL;
    MYSQL_ROW row;
    msg *rd,*sd;
    user usr;
    char sql[MYSQL_MAX];
    rd =(msg*)malloc(sizeof(msg));//记得free
    sd =(msg*)malloc(sizeof(msg));
    bzero(&usr,sizeof(user));
    memcpy(usr.name,mysock->send_Account,strlen(mysock->send_Account));
    sprintf(sql,"select * from person where name = \'%s\';",usr.name);
    int ret = mysql_query(mysql,sql);
    res = mysql_store_result(mysql);
    int num_rows = mysql_num_rows(res);
    if(num_rows == 0){
    //说明没有找到可以注册
    bzero(sd,sizeof(msg));
    sd->type = USER_AGREE;
    sd->msglen = 0;
    if(send(mysock->conn_fd,sd,sizeof(msg),0) < 0){
    my_err("send",__FILE__);
    }
}    else{
    bzero(sd,sizeof(msg));
    sd->type = USER_DISAG;
    sd->msglen = 0;
    if(send(mysock->conn_fd,sd,sizeof(msg),0) < 0){
    my_err("send",__FILE__);
    }
    printf("注册失败！！！\n");
    return;
}
    //当可以注册时，接收用户的密码信息
     bzero(rd,sizeof(msg));
    if((len = recv(mysock->conn_fd,rd,sizeof(msg),0)) < 0){
    perror("login_recv2");
    }else if(len == 0){
    printf("客户端断开连接...\n");
    }
    if(rd->type == USER_MESSAGE){
    memcpy(usr.password,rd->msg,rd->msglen);
    sd->type = USER_AGREE;
    sd->msglen = 0;
    if(send(mysock->conn_fd,sd,sizeof(msg),0) < 0){
    my_err("login",__LINE__);
    }
    }
    bzero(rd,sizeof(msg));
    if((len = recv(mysock->conn_fd,rd,sizeof(msg),0)) < 0){
    my_err("login_recv3",__LINE__);
    }else if(len == 0){
    printf("客户端断开连接...\n");
    }
    if(USER_MESSAGE == rd->type){
    memcpy(usr.email,rd->msg,rd->msglen);
    sd->type = USER_AGREE;
    sd->msglen = 0;
    if(send(mysock->conn_fd,sd,sizeof(msg),0) < 0){
    my_err("login_send5",__LINE__);
    }
    }
     //接收来自客户端的电话
            bzero(rd,sizeof(msg));
            if((len = recv(mysock->conn_fd,rd,sizeof(msg),0)) < 0){
                my_err("login_recv3",__LINE__);
            }else if(len == 0){
                printf("客户端断开连接...\n");
            }
            if(USER_MESSAGE == rd->type){
                memcpy(usr.phonenum,rd->msg,rd->msglen);
                sd->type = USER_AGREE;
                sd->msglen = 0;
                if(send(mysock->conn_fd,sd,sizeof(msg),0) < 0){
                    my_err("login_send6",__LINE__);
                }
}
            puts("name:");
            puts(usr.name);
            puts("passwd:");
            puts(usr.password);
            puts("email:");
            puts(usr.email);
            puts("phone:");
            puts(usr.phonenum);

            //将帐号等信息存入数据库
            bzero(sql,sizeof( sql ));
            sprintf(sql,"INSERT INTO person (name,passwd,email,phone,state,fd) VALUES (\'%s\',\'%s\',\'%s\',\'%s\',%d,%d)",usr.name,usr.password,usr.email,usr.phonenum,0,0);//注意字符串的格式！！！！
            int ret = mysql_query(mysql,sql);
            printf("%d\n",ret);
            puts("账户信息注册成功！\n");
}




void *ser_deal(void *arg){
    MYSQL mysql;
    mysql = init_mysql();
    recv_info *recv_buf = (recv_info*)arg;
    int choice = recv_buf->type;
    switch(choice){
        case USER_LOGIN:
            user_login(recv_buf,&mysql);//登录后看消息盒子，包

        break;
        case USER_SIGN:
            user_sign(recv_buf,&mysql);
        break;
    }

}





int main(){
    MYSQL mysql;
    MYSQL_RES *res;
    MYSQL_ROW  row;
    int   connfd,sockfd;
    int   optval;
    int   flag_recv = 0;
    int   names;
    int   connects = 0;
    char  sql[MYSQL_MAX];
    char   ip[32];
    pthread_t tid;
    recv_info recv_buf;
    size_t ret;
    socklen_t clen;
    list_status_info in_status;
    struct sockaddr_in cliaddr,servaddr;

    mysql = init_mysql();
    signal(SIGPIPE,SIG_IGN);

    int epfd,npfd;
    struct epoll_event ev;
    struct epoll_event events[EVENTS_MAX_SIZE];

    List_Init(in_status,node_status_info);
    in_status = status_list_create(mysql,in_status);

    if((sockfd = socket(AF_INET,SOCK_STREAM,0))<0){
        my_err("socket",__LINE__);
    }
    optval = 1;
    if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(void*)&optval,sizeof(int)) < 0){
        my_err("setsockopt",__LINE__);
        exit(1);
    }
    setsockopt(sockfd,SOL_SOCKET,SO_KEEPALIVE,(void*)&optval,sizeof(int));
    bzero(&servaddr,sizeof(struct sockaddr_in));

    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(PORT);              //服务器端口
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY); //IP地址
    if(bind(sockfd,(struct sockaddr*)&servaddr,sizeof(struct sockaddr_in)) < 0){
        my_err("bind",__LINE__);
        exit(1);
    }
    if(listen(sockfd,LINSTENNUM) < 0){
        my_err("listen",__LINE__);
        exit(1);
    }

    epfd = epoll_create(1);
    ev.data.fd = sockfd;
    ev.events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLRDHUP;

    epoll_ctl(epfd,EPOLL_CTL_ADD,sockfd,&ev);
    connects++;

    while(1){

        npfd = epoll_wait(epfd,events,EVENTS_MAX_SIZE,-1);
        for(int i = 0;i < npfd;i++){
            connects++;
            if(events[i].data.fd == sockfd){
                if(connects > MAX_CONTECT_SIZE){
                    my_err("达到最大连接数...",__LINE__);
                    continue;
                }
                connfd = accept(events[i].data.fd,(struct sockaddr*)&cliaddr,&clen);
                 printf("客户端[IP：%s],[port: %d]已链接...\n",
                inet_ntop(AF_INET,&cliaddr.sin_addr.s_addr,ip,sizeof(ip)),
                ntohs(cliaddr.sin_port));
                if(connfd <= 0){
                    my_err("accpet",__LINE__);
                    continue;
                }
                ev.data.fd = connfd;
                ev.events = EPOLLIN  | EPOLLRDHUP;
                epoll_ctl(epfd,EPOLL_CTL_ADD,connfd,&ev); //新增套接字
            }
            /* 用户非正常挂掉 */
            else if((events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) != 0){
                if((epoll_ctl(epfd,EPOLL_CTL_DEL,events[i].data.fd,0)) != 0){
                    my_err("epoll",__LINE__);
                }
                printf("客户端ip[%d]非正常断开连接...\n",events[i].data.fd);
                        list_status_info p;
                        List_ForEach(in_status,p){
                        if(p->fdtext == events[i].data.fd){
                        //sprintf(sql,"update person set state = \'0\' where name = \'%s\';",p->account); //需要创建account
                        //int ret =mysql_query(&mysql,sql);
                        p->status = 0;
                        // List_DelNode(p);
                        break;
                        }
                    }
                close(events[i].data.fd);
            }
            /* 用户正常发来消息请求 */
            else if(events[i].events & EPOLLIN){
                if((ret = recv(events[i].data.fd,&recv_buf,sizeof(recv_buf),MSG_WAITALL)) < 0){
                    my_err("recv",__LINE__);
                    continue;
                }
                if(!ret){
                    list_status_info  cps ;
                    List_ForEach(in_status,cps){
                    if(cps->fdtext == events[i].data.fd){
                        sprintf(sql,"update person set state = \'0\' where name = \'%s\';",cps->account); //需要创建account
                        int ret =mysql_query(&mysql,sql);
                        List_DelNode(cps);
                        break;
                    }
                }

                printf("客户端ip[%d]已断开连接...\n",events[i].data.fd);
                bzero(sql,sizeof(sql));
                sprintf(sql,"select * from person where fd = \'%d\';",events[i].data.fd);
                mysql_query(&mysql,sql);
                res = mysql_store_result(&mysql);
                row = mysql_fetch_row(res);
                sprintf(sql,"update person set state = \'0\' where fd = \'%d\';",events[i].data.fd);
                mysql_query(&mysql,sql);
                mysql_free_result(res);
                continue;
            }
                if(recv_buf.type == USER_LOGIN){
                    list_status_info top = (list_status_info)malloc(sizeof(node_friend_info));
                    top->fdtext = events[i].data.fd;
                    strcpy(top->account,recv_buf.send_Account);
                    List_AddTail(in_status,top);                //加入在线链表
                }
                //recv_buf.send_fd = events[i].data.fd;
                recv_info *tp = (recv_info*)malloc(sizeof(recv_info));
                tp = &recv_buf;
                tp->epfd = epfd;
                tp->conn_fd = events[i].data.fd;
                pthread_create(&tid,NULL,(void*)ser_deal,(void*)tp);
                pthread_detach(tid);
            }
        }
    }
    close(sockfd);
}