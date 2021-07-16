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
#define   LINSENNUM  20      //最大监听数
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
MYSQL *connect;  //MYSQL链接
MYSQL_RES *res;  //MYSQL记录集
MYSQL_ROW  row;  //字符串数组记录行
const char *host_name = "127.0.0.1";
const char *user_name = "root";
const char *passwd = "";
const char *mdb_name = "chat";

/* 服务器变量 */
int  linsenfd;
int  len;
int  start = 1;
struct sockaddr_in servaddr,cliaddr;


//执行SQL语句，成功返回0,失败返回-1
int executesql(char * sql){
    if(mysql_real_query(connect,sql,strlen(sql)))
    return -1;
    return 0;
}

//