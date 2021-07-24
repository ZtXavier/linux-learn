#ifndef DC127AC2_7971_46FC_8525_C5B1F21D8981
#define DC127AC2_7971_46FC_8525_C5B1F21D8981
#include <stdlib.h>
#include <assert.h>


#define   PORT 9988          //端口号
#define   LINSTENNUM  10      //最大监听数
#define   IP     "127.0.0.1" //ip地址
#define   MYSQL_MAX   1024   //数据库数组最大长度

#define MESSADES_PAGE_SIZE 15     //好友消息分页最大数
#define FRIEND_PAGE_SIZE   5      //好友列表分页最大数 
#define EVENTS_MAX_SIZE    10240   //epoll接收事件最大数 
#define MAX_CONTECT_SIZE   10240   //服务器最大连接数
#define MAX_PATH_NAME      255    //文件名最长255 路径最长4096
#define MAX_USERNAME       64     //名称长度最大数
#define MAX_ACCOUNT        32     //账号长度最大值
#define MAX_PASSWORD       32     //密码最大长度
#define MAX_TELEPHONE      32     //找回密码使用的电话最长长度
#define MESSAGE_FETCH_SEND 1      //在登录时加载消息　消息类型为
#define MESSAGE_FETCH_RECV 2

#define MAX_RECV           8096    //发包的大小 与接包大小相同

#define ERROR_IN_LOGIN     '@'      //登录失败
#define INVAILD            'n'      //用户信息无效
#define VAILD              'y'      //用户信息有效
#define USERNAME            0
#define PASSWORD            1
#define BOX_NO_MESSAGES    "@@@"  //无文件标识符
#define BOX_HAVE_MESSAGS   "$$$"  //有文件标识符
#define OWNER               1       //群主
#define ADMIN               2       //管理员
#define COMMON              3       //群员
/*-----------------------------------------------*/
#define NULL_OF_GROUP      -2       //群数量为零的特殊结束符号　用于login
#define EOF_OF_BOX         -1       //发包结束符
#define USER_LOGIN          0       //登录请求
#define USER_SIGN           1       //注册请求
#define USER_FIND            2       //找回密码
#define ADD_FRIENDS         3       //添加好友
#define ADD_FRIENDS_QUERY   4       //收到添加好友请求后做出答复的数据类型
#define DEL_FRIENDS         5       //删除好友
#define LIST_FRIENDS        6       //显示好友列表
#define SEND_MESSAGES       7       //单聊　发送信息
#define REGISTER_GROUP      8       //注册群
#define ADD_GROUP           9       //加入群
#define QUIT                10      //退出群
#define DISSOLVE            11      //解散群
#define SET_ADMIN           12      //设置管理员
#define KICKING             13      //踢人
#define SEND_GROUP_MESSAGES 14      //群消息
#define SEND_FILE           15      //发送文件
#define RECV_FILE           16      //客户端收文件
#define USER_AGREE          17      //
#define USER_DISAG          18
#define USER_MESSAGE             20

typedef struct {
    int  type;   //事件类型
    int send_fd;
    char send_Account[MAX_ACCOUNT];  //账号　用账号来区分不同的人
    int recv_fd;
    char recv_Acount[MAX_ACCOUNT];    //
    char message[MAX_RECV];
    char message_tmp[MAX_RECV];//放两个缓冲区　以备不时之需
    int epfd;
    int conn_fd;
}recv_info; //客户端第一次recv中包的格式

typedef struct {
    int type;
	int msglen;
	char msg[MAX_RECV];
}msg;



typedef struct
{
	char usename[MAX_USERNAME];
    char account[MAX_ACCOUNT];
    char message[MAX_RECV];
    int type;    //事件类型
}Box_info;

typedef struct message_node
{
	int type; //好友请求　单聊记录
	char send_account[MAX_ACCOUNT]; //朋友账号
	char recv_account[MAX_ACCOUNT]; //自己账号
	char nickname[MAX_USERNAME];    //朋友昵称
	char messages[MAX_RECV];
	struct message_node *next;
	struct message_node *prev;
}node_messages_info,*list_messages_info;


typedef struct friend_node//好友链表类型
{
    int status;//是否在线
    char nickname[MAX_USERNAME];//昵称
    char recv_account[MAX_ACCOUNT];//朋友账号　接收者
    char send_account[MAX_ACCOUNT];//本人账号　发送者
    struct friend_node *next;
    struct friend_node *prev;
    //不需要加套接字　在服务器会进行查询
}node_friend_info,*list_friend_info;


typedef struct group_node //群链表
{
	char nickname[MAX_USERNAME];//群名称
	char account[MAX_ACCOUNT];//群账号
	struct group_node *next;
	struct group_node *prev;
}node_group_info,*list_group_info;

typedef struct member_node//群成员链表
{
	char nickname[MAX_USERNAME]; //昵称
	char account[MAX_ACCOUNT];   //账号
	int type;                    //标记位
	struct member_node *next;
	struct member_node *prev;
}node_member_info,*list_member_info;

typedef struct group_messages_node//群消息链表
{
	char nickname[MAX_USERNAME]; //昵称
	char account[MAX_ACCOUNT];   //账号
	char messages[MAX_RECV];     //消息内容
	int type;                    //标记位
	struct group_messages_node *next;
	struct group_messages_node *prev;
}node_group_messages_info,*list_group_messages_info;


typedef struct status_node //在服务器使用
{
	int status;
	int id;
	char account[MAX_ACCOUNT];
	int fdtext;                  //用来判断是否在线
	struct status_node *next;
	struct status_node *prev;
}node_status_info,*list_status_info;

typedef struct  //把一个结构体当做参数传入服务器传输线程
{
	char count[MAX_ACCOUNT];
	char path[256];
}recv_file_info;




#define List_Init(list, list_node_t) {					\
		list=(list_node_t*)malloc(sizeof(list_node_t)); \
		(list)->next=(list)->prev=list;					\
	}

#define List_Free(list, list_node_t) {			\
		assert(NULL!=list);						\
		list_node_t *tmpPtr;					\
		(list)->prev->next=NULL; 				\
		while(NULL!=(tmpPtr=(list)->next)){ 	\
			(list)->next=tmpPtr->next;			\
			free(tmpPtr);						\
		}										\
		(list)->next=(list)->prev=list;			\
	}

#define List_Destroy(list, list_node_t) {		\
		assert(NULL!=list);						\
		List_Free(list, list_node_t)			\
		free(list);								\
		(list)=NULL;							\
	}

#define List_AddHead(list, newNode) {			\
		(newNode)->next=(list)->next;		 	\
		(list)->next->prev=newNode;			 	\
		(newNode)->prev=(list);				 	\
		(list)->next=newNode;				 	\
	}

#define List_AddTail(list, newNode) {			\
		(newNode)->prev=(list)->prev; 		 	\
		(list)->prev->next=newNode;			 	\
		(newNode)->next=list;				 	\
		(list)->prev=newNode;				 	\
	}

#define List_InsertBefore(node, newNode) {		\
		(newNode)->prev=(node)->prev; 		 	\
		(newNode)->next=node;			 		\
		(newNode)->prev->next=newNode;			\
		(newNode)->next->prev=newNode;			\
	}

#define List_InsertAfter(node, newNode) {		\
		(newNode)->next=node->next;			 	\
		(newNode)->prev=node; 				 	\
		(newNode)->next->prev=newNode;			\
		(newNode)->prev->next=newNode;			\
	}

#define List_IsEmpty(list)  ((list != NULL)	\
	&& ((list)->next == list)				\
	&& (list == (list)->prev))

#define List_DelNode(node) {\
			assert(NULL!=node && node!=(node)->next && node!=(node)->prev);				\
			(node)->prev->next=(node)->next; 	\
			(node)->next->prev=(node)->prev;	\
	}

#define List_FreeNode(node) {	\
		List_DelNode(node);		\
		free(node);				\
	}


#define List_ForEach(list, curPos) 		\
	for (   curPos = (list)->next;  	\
		    curPos != list;       \
		    curPos=curPos->next	\
	    )









#endif /* DC127AC2_7971_46FC_8525_C5B1F21D8981 */
