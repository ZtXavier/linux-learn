#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <mysql/mysql.h>

#define PORT 9988 //端口号
#define BACKLOG 50 //最大监听数量
#define MAXSIZE 1032 //消息msg的最大长度
#define IPNET "127.0.0.1" //ip地址
#define MAX_BUF_SIZE 1024 //数据库数组最大长度

#define MSG_ADD 1       //用户注册
#define MSG_CHANGE 2    //更改密码
#define MSG_LOGIN 3     //用户登录
#define MSG_LOGOUT 4    //用户退出
#define MSG_DATA 5  	//正常消息
#define MSG_ACK 6		//回复同意（正确）
#define MSG_UNACK 7 	//回复不同意（不正确）
#define MSG_PRIVATE 8	//私聊
#define MSG_GROUP 9		//群聊
#define MSG_FILE 10		//文件
#define MSG_NUM 11 		//在线人数
#define MSG_KICK 12     //踢人
#define MSG_MASTER 13   //群主
#define MSG_UNMASTER 14 //不是群主
#define MSG_PEOPLE 15
#define MSG_NONPEOPLE 16
#define MSG_FORBIDDEN 17
#define MSG_LIFT 18
//发送信息的结构体
struct msg
{
    int type; // 消息类型
    int data_len; // 数据长度
    char data[1024]; // 数据
};
//用户结构体
struct Login
{
	int state;
	char name[24];
	char passwd[20];
	char email[24];
};


//数据库变量
MYSQL *g_conn; // mysql 链接
MYSQL_RES *g_res; // mysql 记录集
MYSQL_ROW g_row; // 字符串数组，mysql 记录行
/*=============================================================*/
/**/const char *g_host_name = "localhost";
/**/const char *g_user_name = "root";
/**/const char *g_passwd = "";
/**/const char *g_db_name = "chat";
/*=============================================================*/
char sql[MAX_BUF_SIZE];
char Time[MAX_BUF_SIZE];
struct Login login;
int iNum_rows = 0; // mysql 语句执行结果返回行数赋初值

//服务端变量
int fd;
int sockfd[50];
pthread_t tid[50]={0};
int len;
int startlen = 0;
int start = 1; //启动开关
int master_flag = 0;
int master_fd;
struct msg *rm,*sm;
struct sockaddr_in servaddr,cliaddr;
socklen_t cliaddr_len;
pid_t pid;
char Num[5];
//执行sql语句，成功返回0，失败返回-1
int executesql( const char * sql)
{

	if(mysql_real_query( g_conn , sql , strlen(sql) ) )
			return -1;
	return 0;
}
//初始化链接
int init_mysql()
{
	//init the database connection
	g_conn = mysql_init(NULL);
	//connection the database
	if( !mysql_real_connect( g_conn , g_host_name , g_user_name , g_passwd , g_db_name , 0 , NULL , 0 ) )
		return -1; //连接失败
	//检查是否可以使用
	if( executesql("set names utf8") )
		return -1;
	return 0;//返回成功
}
//选择数据库，没有的时候创建
void create_database()
{
	sprintf(sql,"use chat;");
	if( executesql(sql) == -1 )
	{
		puts("create database");
		executesql("create database chat;");
		puts("choice database");
		executesql("use chat;");
		printf("success!");
	}
}
//查看表格完整性
void create_table()
{
	//user表的检查与创建
	sprintf(sql,"show tables;");
	executesql(sql);
	g_res = mysql_store_result(g_conn);
	iNum_rows = mysql_num_rows(g_res);
	if(iNum_rows == 0)
	{
		puts("create users table");
		executesql("create table users(name char(20) not null ,passwd char(24) not null ,email char(24) not null ,stat int(1) not null , fd int(1) not null);");
	}
	mysql_free_result(g_res); //释放结果集
}
//强制终止
void sigfun()
{
	int i;
	free(rm);
	free(sm);
	close(fd);
	exit(0);
}
//初始化服务器
void init_serv()
{
	rm=(struct msg*)malloc(MAXSIZE);
	sm=(struct msg*)malloc(MAXSIZE);
	// socket
	fd = socket( AF_INET , SOCK_STREAM , 0 );
	if( fd < 0 )
		perror("socket");
	bzero( &servaddr , sizeof(servaddr) );
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons( PORT );
	servaddr.sin_addr.s_addr = htonl( INADDR_ANY );
	// bind
	if( bind( fd , (struct sockaddr *)&servaddr , sizeof(servaddr) ) < 0 )
		perror("bind");
	//listen
	if( listen( fd , BACKLOG ) < 0 )
		perror("listen");
	//对mysql进行初始化
	if(init_mysql())
		perror("init_mysql");
	//对数据库进行选择（没有则创建）
	create_database();
	//对表进行创建
	create_table();
}
//用户注册函数
void add_msg( int connfd )
{
	//告诉客户机可以进行注册（如果不允许注册，将MSG_ACK改成MSG_UNACK）
	sm->type = MSG_ACK;
	sm->data_len = 0;
	if( send( connfd , (void *)sm , sizeof(struct msg) , 0 ) < 0 ) 
		perror("send_add");
	while( 1 )
	{
		//接收来自客户端的帐号信息
		bzero( rm , sizeof( struct msg ));
		bzero( &login , sizeof( login ));
		len = recv( connfd , (void *)rm , MAXSIZE , 0 );
		if( len < 0 )
			perror("add_recv_len2");
		if( rm->type == MSG_DATA )
		{
			memcpy( login.name , rm->data , rm->data_len );
			//从数据库中找客户端输入的用户名
			sprintf( sql , "select * from users where name ='%s';",login.name);
			executesql(sql);
			//下面这两个函数一起使用，用于统计出现的个数
			g_res = mysql_store_result(g_conn);
			iNum_rows = mysql_num_rows(g_res);
			//如果没有出现过，告诉客户端可以使用
			if( iNum_rows == 0)
			{
				sm->type = MSG_ACK;
				sm->data_len = 0;
				if( send( connfd , (void *)sm , sizeof(struct msg) , 0 ) < 0 ) 
					perror("send_add");
			}
			//否则告诉客户端不可以使用
			else
			{
				sm->type = MSG_UNACK;
				sm->data_len = 0;
				if( send( connfd , (void *)sm , sizeof(struct msg) , 0 ) < 0 ) 
					perror("send_add");
				continue;
			}
		}
		//接收来自客户端的密码信息
		bzero( rm , sizeof( rm ));
		len = recv( connfd , (void *)rm , MAXSIZE , 0 );
		if( len < 0 )
			perror("add_recv_len2");
		if( rm->type == MSG_DATA )
		{
			memcpy( login.passwd , rm->data , rm->data_len ); 
			sm->type = MSG_ACK;
			sm->data_len = 0;
			if( send( connfd , (void *)sm , sizeof(struct msg) , 0 ) < 0 ) 
					perror("send_add");
		}
		//接收来自客户端的邮箱信息
		bzero( rm , sizeof( rm ));
		len = recv( connfd , (void *)rm , MAXSIZE , 0 );
		if( len < 0 )
			perror("add_recv_len2");
		if( rm->type == MSG_DATA )
		{
			memcpy( login.email , rm->data , rm->data_len ); 
			sm->type = MSG_ACK;
			sm->data_len = 0;
			if( send( connfd , (void *)sm , sizeof(struct msg) , 0 ) < 0 ) 
					perror("send_add");
			puts("name:");
			puts(login.name);
			puts("passwd:");
			puts(login.passwd);
			puts("email:");
			puts(login.email);
			//将帐号密码以及邮箱放入数据库，并将在线状态初始化为0（离线）
			sprintf(sql,"insert into users values('%s','%s','%s',%d,%d);",login.name,login.passwd,login.email,0,0);
			executesql(sql);
			puts("用户注册成功！");
			break;
		}
	}
}
//更改密码函数
void chg_msg( int connfd )
{
	//告诉客户机可以进行修改（如果不允许修改，将MSG_ACK改成MSG_UNACK）
	sm->type = MSG_ACK;
	sm->data_len = 0;
	if( send( connfd , (void *)sm , sizeof(struct msg) , 0 ) < 0 ) 
		perror("send_add");
	//接收来自客户端的帐号信息
	bzero( rm , sizeof( rm ));
	bzero( &login , sizeof( login ));
	len = recv( connfd , (void *)rm , MAXSIZE , 0 );
	if( len < 0 )
		perror("add_recv_len2");
	if( rm->type == MSG_DATA )
	{
		memcpy( login.name , rm->data , rm->data_len );
		//从数据库中找客户端输入的用户名
		sprintf( sql , "select * from users where name ='%s';",login.name);
		executesql(sql);
		//下面这两个函数一起使用，用于统计出现的个数
		g_res = mysql_store_result(g_conn);
		iNum_rows = mysql_num_rows(g_res);
		//如果出现过，告诉客户端帐号存在
		if( iNum_rows != 0)
		{
			sm->type = MSG_ACK;
			sm->data_len = 0;
			if( send( connfd , (void *)sm , sizeof(struct msg) , 0 ) < 0 ) 
				perror("send_add");
		}
		//否则告诉客户端不存在
		else
		{
			sm->type = MSG_UNACK;
			sm->data_len = 0;
			if( send( connfd , (void *)sm , sizeof(struct msg) , 0 ) < 0 ) 
				perror("send_add");
			return;
		}
	}
	//接受来自客户端的邮箱信息
	bzero( rm , sizeof( rm ));
	len = recv( connfd , (void *)rm , MAXSIZE , 0 );
	if( len < 0 )
		perror("add_recv_len2");
	if( rm->type == MSG_DATA )
	{
		memcpy( login.email , rm->data , rm->data_len );
		//从数据库中找客户端输入的用户名、邮箱
		sprintf( sql , "select * from users where name ='%s' and email = '%s';",login.name,login.email);
		executesql(sql);
		//下面这两个函数一起使用，用于统计出现的个数
		g_res = mysql_store_result(g_conn);
		iNum_rows = mysql_num_rows(g_res);
		//如果出现过，告诉客户端帐号邮箱匹配正确
		if( iNum_rows != 0 )
		{
			sm->type = MSG_ACK;
			sm->data_len = 0;
			if( send( connfd , (void *)sm , sizeof(struct msg) , 0 ) < 0 ) 
				perror("send_add");
		}
		//否则告诉客户端不存在
		else
		{
			sm->type = MSG_UNACK;
			sm->data_len = 0;
			if( send( connfd , (void *)sm , sizeof(struct msg) , 0 ) < 0 ) 
				perror("send_add");
			return;
		}
	}
	//接收客户端传来的新密码
	bzero( rm , sizeof( rm ));
	len = recv( connfd , (void *)rm , MAXSIZE , 0 );
	if( len < 0 )
	perror("add_recv_len2");
	if( rm->type == MSG_DATA )
	{
		memcpy( login.passwd , rm->data , rm->data_len );
		puts("passwd:");
		//匹配帐号
		sprintf(sql,"update users set passwd = '%s' where name = '%s'",login.passwd,login.name);
        executesql(sql);
		puts("更改成功！");
	}
}
//在线人数查询
void num_people( int connfd )
{
	//从数据库中找在线人数
	sprintf( sql , "select * from users where stat = %d;",1);
	executesql(sql);
	//下面这两个函数一起使用，用于统计出现的个数
	g_res = mysql_store_result(g_conn);
	iNum_rows = mysql_num_rows(g_res);
	sm->type = MSG_NUM;
	//将数字转换成字符串
	sprintf(Num,"%d",iNum_rows);
	sm->data_len = sizeof(Num);
	memcpy( sm->data , Num , sm->data_len );
	if( send( connfd , (void *)sm , sizeof(struct msg) , 0 ) < 0 ) 
		perror("send_add");
}
//私聊
void chat_private( int connfd ,char myname[24] )
{
	int i;
	int profd;
	char proName[24];
	char content[130]={0};
	//告诉客户机可以进行私聊（如果不允许修改，将MSG_ACK改成MSG_UNACK）
	sm->type = MSG_ACK;
	sm->data_len = 0;
	if( send( connfd , (void *)sm , sizeof(struct msg) , 0 ) < 0 ) 
		perror("send_add1");
	//接收来自客户端的用户信息
	bzero( rm , sizeof( rm ));
	len = recv( connfd , (void *)rm , MAXSIZE , 0 );
	if( len < 0 )
		perror("add_recv_len2");
	memcpy( proName , rm->data , rm->data_len );
	printf("你输入的用户名：%s",proName);
	//从数据库中找客户端输入的用户名
	sprintf( sql , "select * from users where name ='%s' and stat = %d;",proName,1);
	executesql(sql);
	//下面这两个函数一起使用，用于统计出现的个数
	g_res = mysql_store_result(g_conn);
	iNum_rows = mysql_num_rows(g_res);
	//如果用户存在并且在线，告诉客户端可以进行私聊
	if( iNum_rows != 0)
	{
		sm->type = MSG_PEOPLE;
		sm->data_len = 0;
		if( send( connfd , (void *)sm , sizeof(struct msg) , 0 ) < 0 ) 
			perror("send_add2");
	}
	//否则告诉客户端用户不存在（或已离线）
	else
	{
		sm->type = MSG_NONPEOPLE;
		sm->data_len = 0;
		if( send( connfd , (void *)sm , sizeof(struct msg) , 0 ) < 0 ) 
			perror("send_add3");
		return;
	}
	printf("开始查找！\n");
	sprintf( sql , "select * from users where stat = %d;",1);
	executesql(sql);
	g_res = mysql_store_result(g_conn);
	while( (g_row = mysql_fetch_row(g_res) ) )
	{
		printf("%s\n",g_row[0]);
		if( strcmp( g_row[0] , proName ) == 0 )
		{
			printf("找到的用户名为：%s\n",g_row[0]);
			profd = atoi(g_row[4]);
			break;
		}
	}
	//接收用户发送过来的私聊信息
	bzero( rm , sizeof( rm ));
	len = recv( connfd , (void *)rm , MAXSIZE , 0 );
	if( len < 0 )
		perror("add_recv_len2");
	if( rm->type == MSG_PRIVATE )
	{
		sprintf( content , "【私聊】%s say :",myname);
		strncat( content , rm->data , rm->data_len );
		//发送给对方
		bzero( sm , sizeof( sm ));
		sm->type = MSG_PRIVATE;
		sm->data_len = sizeof(content);
		memcpy( sm->data , content , sm->data_len );
		if( send( profd , (void *)sm , sizeof(struct msg) , 0 ) < 0 ) 
			perror("send_add4");
	}
}
//群聊
void chat_group( int connfd ,char myname[24] )
{
	int i;
	char content[130]={0};
	//告诉客户机可以进行群聊（如果不允许群聊，将MSG_ACK改成MSG_UNACK）
	sm->type = MSG_ACK;
	sm->data_len = 0;
	if( send( connfd , (void *)sm , sizeof(struct msg) , 0 ) < 0 ) 
		perror("send_add1");
	//接收用户发送过来的群聊信息
	bzero( rm , sizeof( rm ));
	len = recv( connfd , (void *)rm , MAXSIZE , 0 );
	if( len < 0 )
		perror("add_recv_len2");
	if( rm->type == MSG_GROUP)
	{
		sprintf( content , "【群聊】%s say :",myname);
		strncat( content , rm->data , rm->data_len );
		//发送给所有人
		bzero( sm , sizeof( sm ));
		sm->type = MSG_GROUP;
		sm->data_len = sizeof(content);
		memcpy( sm->data , content , sm->data_len );
		//从数据库中查找
		sprintf( sql , "select * from users where stat = %d;",1);
		executesql(sql);
		g_res = mysql_store_result(g_conn);
		while( (g_row = mysql_fetch_row(g_res) ) )
		{
			if( send( atoi(g_row[4]) , (void *)sm , sizeof(struct msg) , 0 ) < 0 ) 
				perror("send_add2");
		}	
	}
}
//文件传输
void file_transfer( int connfd ,char myname[24] )
{

		//len = recv( connfd , (void *)rm , MAXSIZE , 0 );
			//if( len < 0 )
				//perror("recv");
		FILE *fp;
		fp = fopen("temp","a+");
		//fwrite(rm->data,sizeof(char),strlen(rm->data),fp);
		fprintf(fp,"%s",rm->data);
		fclose(fp);
		bzero( rm , sizeof(struct msg));
		//printf("%s 正在传文件\n",myname);

}
//踢人
void kick_people( int connfd )
{
	char kick_name[25]={0};
	int i,j,kickfd;
	if( connfd == master_fd ) //判断是否为群主
	{
		sm->type = MSG_MASTER;
	}
	else
		sm->type = MSG_UNMASTER;
	sm->data_len = 0;
	if( send( connfd , (void *)sm , sizeof(struct msg) , 0 ) < 0 ) 
		perror("send_add1");
	if( connfd == master_fd )
	{
		bzero( rm , sizeof( rm ));
		len = recv( connfd , (void *)rm , MAXSIZE , 0 );
		if( len < 0 )
			perror("add_recv_len2");
		if( rm->type == MSG_DATA )
		{
			memcpy( kick_name , rm->data , rm->data_len );
			sprintf( sql , "select * from users where name = '%s';",kick_name);
			executesql(sql);
			g_res = mysql_store_result(g_conn);
			while( (g_row = mysql_fetch_row(g_res) ) )
			{
				kickfd = atoi(g_row[4]);
			}
			bzero( sm , sizeof( sm ));
			sm->type = MSG_KICK;
			sm->data_len =0;
			if( send( kickfd , (void *)sm , sizeof(struct msg) , 0 ) < 0 ) 
				perror("send_add2");
			//将数据库中用户的在线状态设置为离线
			sprintf(sql,"update users set stat = %d where name = '%s'",0,kick_name);
    		executesql(sql);
			printf("用户%s已被踢出本群!\n",kick_name);
			
		}
	}
}
//禁言
void forbidden_people( int connfd )
{
    char forbidden_name[25]={0};
	int i,j,forbiddenfd;
	if( connfd == master_fd ) //判断是否为群主
	{
		sm->type = MSG_MASTER;
	}
	else
		sm->type = MSG_UNMASTER;
	sm->data_len = 0;
	if( send( connfd , (void *)sm , sizeof(struct msg) , 0 ) < 0 ) 
		perror("send_add1");
	if( connfd == master_fd )
	{
		bzero( rm , sizeof( rm ));
		len = recv( connfd , (void *)rm , MAXSIZE , 0 );
		if( len < 0 )
			perror("add_recv_len2");
		if( rm->type == MSG_DATA )
		{
			memcpy( forbidden_name , rm->data , rm->data_len );
			sprintf( sql , "select * from users where name = '%s';",forbidden_name);
			executesql(sql);
			g_res = mysql_store_result(g_conn);
			while( (g_row = mysql_fetch_row(g_res) ) )
			{
				forbiddenfd = atoi(g_row[4]);
			}
			bzero( sm , sizeof( sm ));
			sm->type = MSG_FORBIDDEN;
			sm->data_len =0;
			if( send( forbiddenfd , (void *)sm , sizeof(struct msg) , 0 ) < 0 ) 
				perror("send_add2");
			printf("用户%s已被禁言!\n",forbidden_name);
		}
	}
}
//解禁
void lift_a_ban( int connfd )
{
    char lift_name[25]={0};
	int i,j,liftfd;
	if( connfd == master_fd ) //判断是否为群主
	{
		sm->type = MSG_MASTER;
	}
	else
		sm->type = MSG_UNMASTER;
	sm->data_len = 0;
	if( send( connfd , (void *)sm , sizeof(struct msg) , 0 ) < 0 ) 
		perror("send_add1");
	if( connfd == master_fd )
	{
		bzero( rm , sizeof( rm ));
		len = recv( connfd , (void *)rm , MAXSIZE , 0 );
		if( len < 0 )
			perror("add_recv_len2");
		if( rm->type == MSG_DATA )
		{
			memcpy( lift_name , rm->data , rm->data_len );
			sprintf( sql , "select * from users where name = '%s';",lift_name);
			executesql(sql);
			g_res = mysql_store_result(g_conn);
			while( (g_row = mysql_fetch_row(g_res) ) )
			{
				liftfd = atoi(g_row[4]);
			}
			bzero( sm , sizeof( sm ));
			sm->type = MSG_LIFT;
			sm->data_len =0;
			if( send( liftfd , (void *)sm , sizeof(struct msg) , 0 ) < 0 ) 
				perror("send_add2");
			printf("用户%s已被解除禁言!\n",lift_name);
		}
	}
}
//用户注销
void cancellation_user( int connfd ,char myname[24] )
{
	int i,j;
	//告诉客户机可以进行注销
	sm->type = MSG_ACK;
	sm->data_len = 0;
	if( send( connfd , (void *)sm , sizeof(struct msg) , 0 ) < 0 ) 
		perror("send_add");
	//将数据库中用户的在线状态设置为离线
	sprintf(sql,"update users set stat = %d where name = '%s'",0,myname);
    executesql(sql);
	printf("用户%s已下线！\n",myname);
}
//聊天界面
void chat_menu( int connfd ,char myname[24] )
{
	int flag = 0;
	while( 1 )
	{
		bzero( rm , sizeof(struct msg));
		//接受来自客户端的请求
		len = recv( connfd , (void *)rm , MAXSIZE , 0 );
		if( len < 0 )
			perror("recv");
		switch( rm->type )
		{
			case 1:
						num_people(connfd); //在线人数查询
						break;
			case 2:
						chat_private(connfd,myname); //私聊
						break;
			case 3:
						chat_group(connfd,myname);//群聊
						break;
			case 4:
						file_transfer(connfd,myname);//文件传输
						break;
			case 5:
						kick_people(connfd);//踢人
						break;
			case 6:
						forbidden_people(connfd);//禁言
						break;
			case 7:
						lift_a_ban(connfd);//解禁
						break;
			case 0:
						cancellation_user(connfd,myname);//用户注销
						flag = 1;
						break;
		}
		if( flag == 1 )
		{
			printf("用户%s已退出！\n",myname);
			break;
		}
	}
}
//用户登录函数
void login_msg( int connfd )
{
	struct Login log;
	//告诉客户机可以进行登录（如果不允许登录，将MSG_ACK改成MSG_UNACK）
	sm->type = MSG_ACK;
	sm->data_len = 0;
	if( send( connfd , (void *)sm , sizeof(struct msg) , 0 ) < 0 ) 
		perror("send_add");
	//接收来自客户端的帐号信息
	bzero( &log , sizeof( log ));
	bzero( rm , sizeof( rm ));
	len = recv( connfd , (void *)rm , MAXSIZE , 0 );
	if( len < 0 )
		perror("add_recv_len2");
	if( rm->type == MSG_DATA )
	{
		memcpy( log.name , rm->data , rm->data_len );
		//从数据库中找客户端输入的用户名
		sprintf( sql , "select * from users where name ='%s';",log.name);
		executesql(sql);
		//下面这两个函数一起使用，用于统计出现的个数
		g_res = mysql_store_result(g_conn);
		iNum_rows = mysql_num_rows(g_res);
		//如果出现过，告诉客户端帐号存在
		if( iNum_rows != 0)
		{
			sm->type = MSG_ACK;
			sm->data_len = 0;
			if( send( connfd , (void *)sm , sizeof(struct msg) , 0 ) < 0 ) 
				perror("send_add");
		}
		//否则告诉客户端不存在
		else
		{
			sm->type = MSG_UNACK;
			sm->data_len = 0;
			if( send( connfd , (void *)sm , sizeof(struct msg) , 0 ) < 0 ) 
				perror("send_add");
			return;
		}
	}
	//接收来自客户端的密码信息
	bzero( rm , sizeof( rm ));
	len = recv( connfd , (void *)rm , MAXSIZE , 0 );
	if( len < 0 )
		perror("add_recv_len2");
	if( rm->type == MSG_DATA )
	{
		memcpy( log.passwd , rm->data , rm->data_len );
		//从数据库中找客户端输入的用户名、密码
		sprintf( sql , "select * from users where name ='%s' and passwd = '%s';",log.name,log.passwd);
		executesql(sql);
		//下面这两个函数一起使用，用于统计出现的个数
		g_res = mysql_store_result(g_conn);
		iNum_rows = mysql_num_rows(g_res);
		//如果出现过，告诉客户端密码正确
		if( iNum_rows != 0)
		{
			sm->type = MSG_ACK;
			sm->data_len = 0;
			if( send( connfd , (void *)sm , sizeof(struct msg) , 0 ) < 0 ) 
				perror("send_add");
		}
		//否则告诉客户端密码不正确
		else
		{
			sm->type = MSG_UNACK;
			sm->data_len = 0;
			if( send( connfd , (void *)sm , sizeof(struct msg) , 0 ) < 0 ) 
				perror("send_add");
			return;
		}
	}
	//接收用户发送过来的登录信号
	bzero( rm , sizeof( rm ));
	len = recv( connfd , (void *)rm , MAXSIZE , 0 );
	if( len < 0 )
		perror("add_recv_len2");
	if( rm->type == MSG_LOGIN )
	{
		//从数据库中找客户端输入的用户名、密码
		sprintf( sql , "select * from users where name ='%s' and passwd = '%s' and stat = %d;",log.name,log.passwd,0);
		executesql(sql);
		//下面这两个函数一起使用，用于统计出现的个数
		g_res = mysql_store_result(g_conn);
		iNum_rows = mysql_num_rows(g_res);
		//如果出现过，告诉客户端可以进行登录
		if( iNum_rows == 1)
		{
			sm->type = MSG_ACK;
			sm->data_len = 0;
			if( send( connfd , (void *)sm , sizeof(struct msg) , 0 ) < 0 ) 
				perror("send_add");
		}
		//否则告诉客户端用户登录已达上限
		else
		{
			sm->type = MSG_UNACK;
			sm->data_len = 0;
			if( send( connfd , (void *)sm , sizeof(struct msg) , 0 ) < 0 ) 
				perror("send_add");
			return;
		}
	}
	printf("用户%s已经成功登录!\n",log.name);
	//将数据库用户在线状态设置为1（在线）
	sprintf(sql,"update users set stat = %d  where name = '%s'",1,log.name);
    executesql(sql);
	sprintf(sql,"update users set fd = %d  where name = '%s'",connfd,log.name);
    executesql(sql);
	if( master_flag == 0 )
	{
		master_flag = 1;
		master_fd = connfd;
	}
	//进入聊天界面
	chat_menu( connfd , log.name );
}
//菜单
void *server_receive( void *connfd )
{
	int flag = 0;
	while( 1 )
	{
		//接受来自客户端的请求
		bzero( rm , sizeof( rm ));
		len = recv( *(int *)connfd , (void *)rm , MAXSIZE , 0 );
		if( len < 0 )
			perror("recv");
		switch( rm->type )
		{
			case MSG_ADD:
						add_msg( *(int *)connfd ); //用户注册
						break;
			case MSG_CHANGE:
						chg_msg( *(int *)connfd ); //更改密码
						break;
			case MSG_LOGIN:
						login_msg( *(int *)connfd );//用户登录
						break;
			case MSG_LOGOUT:
						flag = 1;
						break;
		}
		if( flag == 1 )
		{
			//close( *(int *)arg );
			puts("客户端已退出！");
			break;
		}
	}
}
//线程创建
void serv_menu()
{
	int len;
	int ret;
	int i=0;
	int connfd;
	while( start && startlen < 50 )
	{
		cliaddr_len = sizeof(cliaddr);
		//accept
		connfd = accept( fd , (struct sockaddr *)&cliaddr , &cliaddr_len);
		if( -1 == connfd )
			perror("accept");
		//创建子进程
		ret = pthread_create(&tid[i++], NULL, server_receive, (void *)&connfd);
        if(0 != ret)
            perror("pthread_create");
        startlen++;
	}
}
int main(void)
{
	signal(SIGINT,sigfun);
	init_serv();
	serv_menu();
	return 0;
}

