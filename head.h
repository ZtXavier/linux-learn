#ifndef BCCE2023_905A_4135_BD3E_F68333B9076F
#define BCCE2023_905A_4135_BD3E_F68333B9076F
#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<fcntl.h>
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
#define   BACKSPACE   127

#define FRIENDS_MAXNUM  100      // 最大好友数量
#define USER_LOGIN      1        // 登录
#define USER_SIGN       2        // 注册
#define USER_FIND       3        // 找回密码
#define USER_CHANGE     4        // 修改密码
#define SEND_INFO       5        // 发消息
#define ADD_FRIEND      6        // 加好友
#define DEL_FRIEND      7        // 删好友
#define LOOK_FRI_LS     8        // 查看好友状态
#define BLACK_LIST      9        //黑名单
#define QUIT_BLACK     10        //取消黑名单
#define RECV_INFO      11        //收消息
#define LOOK_HISTORY   12        //查看消息记录
#define DELE_HISTORY   13        //删除消息记录
#define FRIEND_PLS     14        //好友申请
#define CREATE_GROUP   15        //创建群聊
#define SET_ADMIN      16        //设置管理员
#define KICK_MEM       17        //踢人
#define ADD_GROUP      18        //加群
#define EXIT_GROUP     19        //退群
#define SEND_GROUP_MSG 20        //群聊
#define QUIT_ADMIN     21        //取消群管理员
#define LOOK_GROUP_LS  22        //查看加入的群
#define LOOK_GROUP_MEM 23        //查看群成员
#define DISSOLVE_GROUP 24        //解散群
#define RECV_GROUP_MSG 25        //收群消息
#define SEND_FILE      26        //发文件
#define RECV_FILE      27        //收文件
#define FINSH          28        //服务器接收到全部文件的标志
#define READ_FILE      29        //读客户端文件
#define WRITE_FILE     30        //写到本地文件中

#define ID_ERROR       -2        //帐号错误
#define USER_OUT       -1        //用户登出

pthread_mutex_t mutex;
pthread_mutex_t cl_mu;
pthread_cond_t  cl_co;

typedef struct{
    int     type;
    int     sendfd;
    int     recvfd;
    int     send_id;
    int     recv_id;
    char    send_name[24];
    char    recv_name[24];
    char    read_buff[MAX_MESSAGE];
    char    write_buff[MAX_MESSAGE];
    int     cont;
}recv_datas;

typedef struct friends{
    char   friend_nickname[FRIENDS_MAXNUM][24];
    int    friend_id[FRIENDS_MAXNUM];
    int    friend_state[FRIENDS_MAXNUM];
    int    friend_num;
}FRIENDS;

typedef struct file{
    char       filepath[50];
    int        num;
    int        send_id;
    char       send_nickname[50];
}FILE_INFO;

typedef struct message{
    int     num;
    int     send_id[512];
    int     recv_id[512];
    char    message[512][MAX_MESSAGE];
}MSG;

typedef struct box_message{
    struct box_message *next;
    int     recv_id;                       //收消息的id
    int     send_id[512];                  //发消息的id
    int     recv_msgnum;                   //收到的消息数量
    char    send_msg[512][MAX_MESSAGE];    //发送的消息

    int     fri_pls_id[512];               //发请求消息的id
    int     fri_pls_num;
    char    send_pls[512][MAX_MESSAGE];    //发送的请求

    int     group_send_id[512];              //发消息的人
    int     group_msg_num;                   //发消息的数量
    char    group_mem_nikename[512][50];     //发消息的人名
    char    group_message[512][MAX_MESSAGE]; //群消息
    int     group_id[512];                   //群号

}BOX_MSG,*list_box;
BOX_MSG *head;
BOX_MSG *tail;

typedef struct group_message{
    int      group_send_id;              //发消息的id
    int      group_msg_num;              //消息的数量
    int      recv_id[512];               //收消息的id
    char     message[512][MAX_MESSAGE];  //发送的消息
}GRP_MSG;

typedef struct group_list{
    // int        group_id;                    //群id
    int        group_mem_num;               //群成员数量
    int        group_mem_id[512];           //群成员id
    int        group_mem_state[512];        //群成员状态
    char       group_mem_nickname[512][24]; //群成员昵称
}GRP_MEM_LIST;

typedef struct group_info{
    int       group_id[50];               //群号
    int       number;                     //群数量
    int       group_state[50];            //群状态
    char      group_name[50][50];         //群名
}GRP_INFO;
#endif /* BCCE2023_905A_4135_BD3E_F68333B9076F */
