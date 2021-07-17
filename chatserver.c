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
#include<time.h>
#include<mysql/mysql.h>

#define   PORT 9988          //端口号
#define   LINSTENNUM  20      //最大监听数
#define   MSGSIZE    1024    //最大消息长度
#define   IP     "127.0.0.1" //ip地址
#define   MYSQL_MAX   1024   //数据库数组最大长度

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


/* 数据库变量 */
MYSQL *mysql;                            //用于初始化地址，如果用null的话可能在close时造成系统崩溃
MYSQL *conn ;                            //MYSQL链接
MYSQL_RES *res = NULL ;                  //MYSQL记录集
MYSQL_ROW  row;                          //字符串数组记录行
const char *host_name = "127.0.0.1";
const char *user_name = "root";
const char *passwd = "123456";
const char *mdb_name = "chat";
char  sql[MYSQL_MAX];                    //数据库命令
char Times[MYSQL_MAX];                   //时间显示
long long unsigned int  num_rows;        //返回mysql_num_rows的结果

/* 服务器变量 */
int  listenfd;
int  len;
int  start = 1;                          //创建线程开始的标志
int  startnum = 0;
msg *recvmsg,*sendmsg;                   //定义接收，发送消息的结构体指针
user usr;                 
struct sockaddr_in servaddr,cliaddr;
pthread_t  tid[10]={0};
socklen_t  cliaddr_len;




//执行SQL语句，成功返回0,失败返回-1
int executesql(const char *sql){
    if(mysql_real_query(conn,sql,strlen(sql)))
    return -1;                                  //如果可用则返回-1
    return 0;
}

int init_mysql(){
    

	//初始化mysql
      conn = mysql_init(NULL);   //当为null时会分配一个新的对象
    //mysql_init(conn);
    //链接数据库
    if(!mysql_real_connect(conn,host_name,user_name,passwd,mdb_name,0,NULL,0)){
       fprintf(stderr, "Failed to connect to database: Error: %s\n",
        mysql_error(conn));
    }
    printf("数据库链接成功...\n");
    //对数据库进行检查是否可用
    if(executesql("set names utf8"))  //可用时if内的值为-1,语句用于对编码设置来初始化
    return -1;
    return 0;      //返回成功
}

void create_databases(){
    sprintf(sql,"use chat;");
    if(executesql(sql) == -1){
        printf("创建数据库...\n");
        executesql("create database chat;");
        printf("选择数据库...\n");
        executesql("use chat;");
        printf("数据库创建成功！！！\n");
    }
}

//查看数据表
void find_table(){
    //
    sprintf(sql,"show tables;");
    executesql(sql);
    res = mysql_store_result(conn);
    num_rows = mysql_num_rows(res);
    if(num_rows == 0){
        printf("成功创建users表\n");
        executesql("create table users(name varchar(20) not null ,passwd varchar(24) not null ,email varchar(24) not null ,phone varchar(24) not null,state int(1) not null , fd int(1) not null);");
    }
    mysql_free_result(res); //释放结果集
}





//初始化服务器
void init_serv(){
    recvmsg = (msg*)malloc(MSGSIZE);
    sendmsg = (msg*)malloc(MSGSIZE);

    //初始化socket
    listenfd = socket(AF_INET,SOCK_STREAM,0);
    if(listenfd < 0){
        perror("socket");
    }
    bzero(&servaddr,sizeof(servaddr));
    //初始化servaddr内部变量
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);  //调用 htonl 来将地址转为小端地址
    servaddr.sin_port = htons(PORT);       //调用 htons 来将端口换为网络端口
    //绑定套接字
    if(bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) < 0)
    perror("bind");

    //设置监听
    if(listen(listenfd,LINSTENNUM) < 0){
        perror("listen");
    }
    //初始化mysql
    init_mysql();
    //选择创建数据库
    create_databases();
    //初始化表
    find_table();
}


//用户注册函数
void user_login(int connfd){
    //先定义客户端状态
    sendmsg->type = USER_SIGN;
    sendmsg->msglen = 0;
    int len = 0;
    if(send(connfd,sendmsg,sizeof(msg),0) < 0){
        perror("send");
    }
    while(1){
        //接收信息
        bzero(recvmsg,sizeof(msg));
        bzero(usr,sizeof(user));
        len = recv(connfd,recvmsg,MSGSIZE,0);
        if(len < 0){
            perror("recv");
        }else if(len == 0){
            printf("客户端与主机断开连接...\n");
        }
        if(recvmsg->type == USER_MASSAGE){
            memcpy(usr.name,recvmsg->data,recvmsg->msglen);
            //在数据库中查找客户端输入的用户名
            
        }


    }



}


void *server_meun(void *connfd){
    int cfd = *(int*)connfd;
    int flag = 0;
    int len = 0;
    while(1){
        bzero(recvmsg,sizeof(recvmsg));
        len = recv(*(int*)connfd,(void *)recvmsg,MSGSIZE,0);
        if(len < 0){
            perror("recv");
            exit(1);
        }else if(len == 0){
            printf("客户端断开连接...\n");
        }
    switch (recvmsg->type)
    {
    case USER_SIGN:   //注册
           user_login(cfd);
        break;
    case USER_CHANGE:
        break;
    case USER_LOGOUT:
        break;
    case USER_LOGIN:
        break;
    default:
        break;
    }
   }
}










//服务器线程创建
void serv_ptcreate(){
    int l;
    int ret;
    int i = 0;
    int connfd;
    //多个线程并发
    while(start && startnum < 10){
        cliaddr_len = sizeof(cliaddr);
        //接收来自客户端的请求,若没有客户端请求，则在此处阻塞
        if((connfd = accept(listenfd,(struct sockaddr*)&cliaddr,&cliaddr_len)) == -1){
            perror("accept");
        }
        //创建子线程
        if((ret = pthread_create(&tid[i++],NULL,serv_ptcreate,(void *)&connfd)) != 0){
            perror("pthread_create");
        }
        startnum++;
    }
}








void sighandler(){

}



int main(){
    //signal(SIGINT,sighandler);
    init_serv();
    return 0;
}