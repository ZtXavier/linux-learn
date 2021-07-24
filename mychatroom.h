#ifndef CD3CD505_C109_4334_96CC_BD6684BE2382
#define CD3CD505_C109_4334_96CC_BD6684BE2382
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
#include<linux/posix_types.h>


#define   PORT 9988          //端口号
#define   LINSTENNUM  10      //最大监听数
#define   MSGSIZE    1024    //消息长度
#define   MAXSIZE    1032    //最大包的长度
#define   IP     "127.0.0.1" //ip地址
#define   MYSQL_MAX   1024   //数据库数组最大长度

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


#endif /* CD3CD505_C109_4334_96CC_BD6684BE2382 */
