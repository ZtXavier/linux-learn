#ifndef BCCE2023_905A_4135_BD3E_F68333B9076F
#define BCCE2023_905A_4135_BD3E_F68333B9076F
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


#define   PORT 9988          //端口号
#define   LINSTENNUM  10      //最大监听数
#define   IP     "127.0.0.1" //ip地址
#define   MYSQL_MAX   1024   //数据库数组最大长度
#define   MAX_MESSAGE 8192   //最大数
#define   MAXEVENTS   1024   //最大事件数


#define MAX_FRIEND 500
#define USER_LOGIN 1              // 登录
#define USER_SIGN 2         // 注册
#define USER_FIND 3      // 找回密码
#define USER_CHANGE 4    // 修改密码
// 添加好友
#define ADD_FRIEND 5
// 删除好友
#define DEL_FRIEND 6
// 查看好友列表
#define LOOK_LIST 7
// 发消息
#define SEND_FMES 8
// 创建群
#define CREATE_GROUP 9
// 加入群
#define ADD_GROUP 10
// 退出群
#define EXIT_GROUP 11
// 删除群成员
#define DEL_MEMBER 12
// 设置管理员
#define SET_ADMIN 13
// 删除管理员
#define DEL_ADMIN 14
// 发送群消息
#define SEND_GMES 15
// 查看加入的群
#define LOOK_GROUP 16
// 群主删除群
#define DIS_GROUP 17
// 发送文件
#define SEND_FILE 18
// 好友请求
#define FRIENDS_PLZ 19
// 黑名单
#define BLACK_FRIEND 20
// 取消黑名单
#define WHITE_FRIEND 21
// 特别关心
#define CARE_FRIEND 22
// 取消特关
#define DISCARE_FRIEND 23
#define MAXIN 1024
#define USER_OUT -1
#define ID_ERROR -2

#define CARE 1
#define OK 0
#define RECV_FMES 24
#define BLACK -1
#define READ_MESSAGE 25
#define READ_GMES 26
#define RECV_GMES 27
#define DEL_MESSAGE 28
#define LOOK_MEMBER 29
#define LOOK_GROUP_LIST 30
#define RECV_FILE 31
#define READ_FILE 32
#define OK_FILE 33
#define SEND_F 34

pthread_mutex_t mutex;
pthread_mutex_t cl_mu;
pthread_cond_t  cl_co;
pthread_cond_t cond;

typedef struct{
    int     type;
    int     sendfd;
    int     recvfd;
    int     send_id;
    int     recv_id;
    char    send_name[20];
    char    recv_name[20];
    char    read_buff[MAX_MESSAGE];
    char    write_buff[MAX_MESSAGE];
}recv_datas;







#endif /* BCCE2023_905A_4135_BD3E_F68333B9076F */
