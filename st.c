#include"mychatroom.h"


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

typedef struct sock_addr{
    struct sockaddr_in servaddr;
    int   connfd;
}sock;

sock infos[50];



/* 数据库变量 */
//MYSQL *mysql;                            //用于初始化地址，如果用null的话可能在close时造成系统崩溃
MYSQL *conn ;                            //MYSQL链接
MYSQL_RES *res = NULL ;                  //MYSQL记录集
MYSQL_ROW  row;
unsigned int num_fields;                        //字符串数组记录行
const char *host_name = "127.0.0.1";
const char *user_name = "root";
const char *passwd = "123456";
const char *mdb_name = "chat";
char  sql[MYSQL_MAX];                    //数据库命令
char Times[MYSQL_MAX];                   //时间显示
int  num_rows;        //返回mysql_num_rows的结果
int listenfd,connfd;
/* 服务器变量 */
//int  listenfd;
//int  len;
//int  start = 1;                          //创建线程开始的标志
//int  startnum = 0;
//int  cliaddr_len;
msg *rvms,*sdms;                   //定义接收，发送消息的结构体指针
user usr;                                //定义用户的结构体
struct sockaddr_in servaddr,cliaddr;
int person_fg = 0;                   //用户登陆状态
int person_fd;


void my_err(const char *err_string, int line)
{
	fprintf(stderr, "line:%d ", line);
	perror(err_string);
	exit(1);
}

//执行SQL语句，成功返回0,失败返回-1
int executesql(const char *sql){
    if(mysql_real_query(conn,sql,strlen(sql)))
    return -1;                                  //如果可用则返回-1
    return 0;
}

int init_mysql(){
	//初始化mysql
    conn = mysql_init(NULL);   //当为null时会分配一个新的对象
    //conn = mysql_init(conn);
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
        printf("成功创建person表\n");
        executesql("create table person(id INT(10) PRIMARY KEY NOT NULL UNIQUE AUTO_INCREMENT,name VARCHAR(24) NOT NULL,passwd VARCHAR(24) NOT NULL,email VARCHAR(24) NOT NULL,phone VARCHAR(24) NOT NULL,state INT(1) NOT NULL,fd INT(1) NOT NULL);");
    }
    mysql_free_result(res); //释放结果集
}





//初始化服务器
// void init_serv(){
//     //开创空间作为数据发送包
//     rvms = (msg*)malloc(MAXSIZE);
//     sdms = (msg*)malloc(MAXSIZE);

//     bzero(&servaddr,sizeof(struct sockaddr_in));
//     //初始化servaddr内部变量
//     servaddr.sin_family = AF_INET;
//     servaddr.sin_port = htons(PORT);       //调用 htons 来将端口换为网络端口
//     servaddr.sin_addr.s_addr = htonl(INADDR_ANY);  //调用 htonl 来将地址转为小端地址
//     //初始化socket
//     listenfd = socket(AF_INET,SOCK_STREAM,0);
//     if(listenfd < 0){
//         perror("socket");
//     }
//     //绑定套接字
//     if(bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) < 0)
//     perror("bind");
//     //设置监听
//     if(listen(listenfd,LINSTENNUM) < 0){
//         perror("listen");
//     }
//     //初始化mysql
//     // if(init_mysql())
//     // printf("%s",mysql_error(conn));
//     // printf("adwdwd");
//     // //选择创建数据库
//     // create_databases();
//     // //初始化表
//     // find_table();
// }


//用户注册函数
void p_sign(int connfd){
    //先定义客户端状态
    bzero(sdms,sizeof(msg));
    sdms->type = USER_SIGN;
    sdms->msglen = 0;
    int len = 0;
    if(send(connfd,sdms,sizeof(msg),0) < 0){
        perror("login_send1");
    }
    while(1){
        //接收信息
        bzero(rvms,sizeof(msg));
        bzero(&usr,sizeof(usr));
        len = recv(connfd,rvms,MAXSIZE,0);
        if(len < 0){
            perror("login_recv1");
        }else if(len == 0){
            printf("客户端与主机断开连接...\n");
            getchar();
        }

        if(rvms->type == USER_MASSAGE){
            memcpy(usr.name,rvms->data,rvms->msglen);
            //在数据库中查找客户端输入的用户名
            sprintf(sql,"select * from person where name = \'%s\';",usr.name);
            int ret1= mysql_query(conn,sql); //执行操作
            printf("%d\n",ret1);
            //统计名字出现的个数
            res = mysql_store_result(conn);
            num_rows = mysql_num_rows(res);
            printf("%d",num_rows);
            //名字若没找到，则可以注册
            if(num_rows == 0){
                sdms->type = USER_AGREE; //该选项为同意使用
                sdms->msglen = 0;
                if(send(connfd,sdms,MAXSIZE,0) < 0){
                    perror("login_send2");
                }
            }else{
                sdms->type = USER_DISAG;
                sdms->msglen = 0;
                if(send(connfd,sdms,sizeof(msg),0) < 0){
                    perror("login_send3");
                }
                return;
            }
        }
            //当可以注册时，接收用户的密码信息
            bzero(rvms,sizeof(msg));
            if((len = recv(connfd,rvms,sizeof(msg),0)) < 0){
                perror("login_recv2");
            }else if(len == 0){
                printf("客户端断开连接...\n");
            }
            if(USER_MASSAGE == rvms->type){
                memcpy(usr.password,rvms->data,rvms->msglen);
                sdms->type = USER_AGREE;
                sdms->msglen = 0;
                if(send(connfd,sdms,sizeof(msg),0) < 0){
                    perror("login_send4");
                }
            }

            //接收来自客户端的邮箱信息
            bzero(rvms,sizeof(msg));
            if((len = recv(connfd,rvms,sizeof(msg),0)) < 0){
                perror("login_recv3");
            }else if(len == 0){
                printf("客户端断开连接...\n");
            }
            if(USER_MASSAGE == rvms->type){
                memcpy(usr.email,rvms->data,rvms->msglen);
                sdms->type = USER_AGREE;
                sdms->msglen = 0;
                if(send(connfd,sdms,sizeof(msg),0) < 0){
                    perror("login_send5");
                }
            }

             //接收来自客户端的电话
            bzero(rvms,sizeof(msg));
            if((len = recv(connfd,rvms,sizeof(msg),0)) < 0){
                perror("login_recv3");
            }else if(len == 0){
                printf("客户端断开连接...\n");
            }
            if(USER_MASSAGE == rvms->type){
                memcpy(usr.phonenum,rvms->data,rvms->msglen);
                sdms->type = USER_AGREE;
                sdms->msglen = 0;
                if(send(connfd,sdms,sizeof(msg),0) < 0){
                    perror("login_send6");
                }

            //在服务器打印
            puts("name:");
            puts(usr.name);
            puts("passwd:");
            puts(usr.password);
            puts("email:");
            puts(usr.email);
            puts("phone:");
            puts(usr.phonenum);

            //将帐号等信息存入数据库
            bzero(sql , sizeof( sql ));
            sprintf(sql,"INSERT INTO person (name,passwd,email,phone,state,fd) VALUES (\'%s\',\'%s\',\'%s\',\'%s\',%d,%d)",usr.name,usr.password,usr.email,usr.phonenum,0,0);//注意字符串的格式！！！！
            printf("sql:%s\n",sql);
            int ret = mysql_query(conn,sql);
            printf("%d\n",ret);
            puts("账户信息注册成功！\n");
            break;
        }
    }
}

//用户登陆函数
void p_login(int connfd){
    int len = 0;
    //告诉用户可以登陆
    bzero(sdms,sizeof(msg));
    sdms->type = USER_AGREE;
    sdms->msglen = 0;
    if((send(connfd,sdms,MAXSIZE,0)) < 0){
        my_err("send",__LINE__);
    }
        //接收帐号信息
        bzero(&usr,sizeof(usr));
        bzero(rvms,sizeof(msg));
        if((len = recv(connfd,rvms,MAXSIZE,0)) < 0){
           my_err("recv",__LINE__);
        }else if(0 == len){
            printf("客户端已断开连接...");
            getchar();
        }
        if(rvms->type == USER_MASSAGE){
            memcpy(usr.name,rvms->data,rvms->msglen);
            //从数据库中找到客户端输入的用户名
            sprintf(sql,"select * from person where name = \'%s\';",usr.name);
            int ret2 = mysql_query(conn,sql);
            res = mysql_store_result(conn);
            num_rows = mysql_num_rows(res);
            //若在数据库中存在，则返还agree
            if(num_rows != 0){
                sdms->type = USER_AGREE;
                sdms->msglen = 0;
                if((send(connfd,sdms,MAXSIZE,0)) < 0){
                    perror("send_log2");
                }
            }else{//数据库无数据，不可登陆
                    sdms->type = USER_DISAG;
                    sdms->msglen = 0;
                    if((send(connfd,sdms,MAXSIZE,0)) < 0){
                        perror("send_log3");
                    }
                    return;
            }
        }
            //接收来自客户端的密码信息
            bzero(rvms,sizeof(msg));
            if((len = recv( connfd ,rvms,MAXSIZE,0)) < 0){
                perror("recv_log4");
            }
            if(rvms->type == USER_MASSAGE){
            memcpy(usr.password,rvms->data,rvms->msglen);
            //从数据库中找密码
            sprintf(sql,"select * from person where passwd = \'%s\';",usr.password);
            int ret5 = mysql_query(conn,sql);
            //来统计出现的个数
            res = mysql_store_result(conn);
            num_rows = mysql_num_rows(res);
            printf("%d\n",num_rows);
            //如果出现过则密码正确
            if(num_rows != 0){
                sdms->type = USER_AGREE;
                sdms->msglen = 0;
                if(send( connfd,sdms,MAXSIZE ,0 ) < 0){
                    perror("login_send4");
                }
            }else{
                sdms->type = USER_DISAG;
                sdms->msglen = 0;
                if( send(connfd,sdms,MAXSIZE,0) < 0){
                    perror("login_send5");
                }
                return;
            }
        }
        //接收登陆信号
        bzero(rvms,sizeof(msg));
        if((len = recv(connfd,rvms,MAXSIZE,0)) < 0){
            my_err("recv",__LINE__);
        }
        if(rvms->type == USER_LOGIN){
            //从数据库中找客户端输入的密码，用户名
            sprintf(sql,"select * from person where name = \'%s\' and passwd = \'%s\';",usr.name,usr.password);
            int ret6 = mysql_query(conn,sql);
            //下面两个函数一起使用，用于统计出现个数
            res = mysql_store_result(conn);
            num_rows = mysql_num_rows(res);
            bzero(sdms,sizeof(msg));
            //如果出现过,则可以登陆
            if(num_rows == 1){
            sdms->type = USER_AGREE;
            sdms->msglen = 0;
            if( send( connfd , sdms , MAXSIZE , 0 ) < 0){
                perror("login_send6");
            }
            }else{                 //否则告诉客户端用户登陆已达上限
                sdms->type = USER_DISAG;
                sdms->msglen = 0;
                if(send(connfd,sdms,MAXSIZE,0) < 0){
                    perror("login_send7");
                }
                return;
            }
            printf("用户 %s 已成功登陆！\n",usr.name);
            //将数据库用户状态设置为在线状态
            sprintf(sql,"update person set state = %d  where name = \'%s\';",1,usr.name);
            int ret7 = mysql_query(conn,sql);
            sprintf(sql,"update person set fd = %d where name = \'%s\';",connfd,usr.name);
            int ret8 = mysql_query(conn,sql);
            if(person_fg == 0){
                person_fg = 1;
                person_fd = connfd;
            }
            //进入聊天室
            // chat_menu(connfd,usr.name);
            printf("进入聊天室\n");
    }
}


//密码找回
void p_find(int connfd){
    //开始时可以更改
    int len;
    bzero(sdms,sizeof(msg));
    sdms->type = USER_AGREE;
    sdms->msglen = 0;
    if(send(connfd,sdms,MAXSIZE,0) < 0){
        perror("send_fd");
    }
    //接收来自客户端的信息
    bzero(rvms,sizeof(msg));
    bzero(&usr,sizeof(user));
    if((len = recv(connfd,rvms,MAXSIZE,0)) < 0){
    perror("recv_fd1");
    }
    if(rvms->type == USER_MASSAGE){
        memcpy(usr.name,rvms->data,rvms->msglen);
        //从数据库中找客户端输入的用户名
        sprintf(sql,"select * from person where name = \'%s\';",usr.name);
        int ret = mysql_query(conn,sql);
        //统计出现的个数
        res = mysql_store_result(conn);
        num_rows = mysql_num_rows(res);
        //如果出现过则帐号存在
        if( num_rows != 0){
            sdms->type = USER_AGREE;
            sdms->msglen = 0;
            if(send(connfd,sdms,MAXSIZE,0) < 0){
                perror("send_fd2");
            }
        }else{
            sdms->type = USER_DISAG;
            sdms->msglen = 0;
            if(send(connfd,sdms,MAXSIZE,0) < 0){
                perror("send_fd3");
            }
        }


        //接收用户的邮箱或电话来匹配
        bzero(rvms,sizeof(msg));
        if((len = recv(connfd,rvms,MAXSIZE,0)) < 0){
            perror("recv_fd2");
        }
        if(rvms->type == USER_EMCHG){
            memcpy(usr.email,rvms->data,rvms->msglen);
            //从数据库中找客户端输入的邮箱和用户名
            sprintf(sql,"select * from person where name = \'%s\' and email = \'%s\';",usr.name,usr.email);
            int ret1 = mysql_query(conn,sql);
            res = mysql_store_result(conn);
            num_rows = mysql_num_rows(res);
            //如果出现过则邮箱匹配正确
            if(num_rows == 1){
                sdms->type = USER_AGREE;
                sdms->msglen = 0;
                if(send(connfd,sdms,MAXSIZE,0) < 0){
                    perror("send_fd4");
                }
            }else{
                sdms->type = USER_DISAG;
                sdms->msglen = 0;
                if(send(connfd,sdms,MAXSIZE,0) < 0){
                    perror("send_fd5");
                }
                return;
            }
        }else if(rvms->type == USER_PHCHG){
        memcpy(usr.phonenum,rvms->data,rvms->msglen);
            //从数据库中找客户端输入的邮箱和用户名
            sprintf(sql,"select * from person where name = \'%s\' and phone = \'%s\';",usr.name,usr.phonenum);
            int ret1 = mysql_query(conn,sql);
            res = mysql_store_result(conn);
            num_rows = mysql_num_rows(res);
            //如果出现过则邮箱匹配正确
            if(num_rows == 1){
                sdms->type = USER_AGREE;
                sdms->msglen = 0;
                if(send(connfd,sdms,MAXSIZE,0) < 0){
                    perror("send_fd5");
                }
            }else{
                sdms->type = USER_DISAG;
                sdms->msglen = 0;
                if(send(connfd,sdms,MAXSIZE,0) < 0){
                    perror("send_fd6");
                }
                return;
            }
        }
        //向客户端发送密码
        bzero(sdms,sizeof(msg));
        sprintf(sql,"select * from person where name = \'%s\';",usr.name);
        int ret2 = mysql_query(conn,sql);
        res = mysql_store_result(conn);
        num_fields = mysql_num_fields(res);
        row = mysql_fetch_row(res);
        if(row[2]){
        sdms->type = USER_AGREE;
        strcpy(sdms->data,row[2]);
        sdms->msglen = strlen(row[2]);
        sdms->data[sdms->msglen] = '\0';
        }else{
            sdms->type = USER_DISAG;
            sdms->msglen = 0;
            printf("%-10s",row[2]);
        }
        if((send(connfd,sdms,MAXSIZE,0)) < 0){
            perror("recv_log4");
        }
        
    }
}

//修改密码
void p_change(int connfd){
    int len;
    //可以修改
    bzero(sdms,sizeof(msg));
    sdms->type = USER_AGREE;
    sdms->msglen = 0;
    if(send(connfd,sdms,MAXSIZE,0) < 0){
        my_err("send",__LINE__);
    }
    //接收来自客户端的信息
    bzero(rvms,sizeof(msg));
    bzero(&usr,sizeof(user));
    if((len = recv(connfd,rvms,MAXSIZE,0)) < 0){
    my_err("recv",__LINE__);
    }
    if(rvms->type == USER_MASSAGE){
        memcpy(usr.name,rvms->data,rvms->msglen);
        //从数据库中找客户端输入的用户名
        sprintf(sql,"select * from person where name = \'%s\';",usr.name);
        int ret = mysql_query(conn,sql);
        //统计出现的个数
        res = mysql_store_result(conn);
        num_rows = mysql_num_rows(res);
        //如果出现过则帐号存在
        if( num_rows != 0){
            sdms->type = USER_AGREE;
            sdms->msglen = 0;
            if(send(connfd,sdms,MAXSIZE,0) < 0){
                perror("send_chg2");
            }
        }else{
            sdms->type = USER_DISAG;
            sdms->msglen = 0;
            if(send(connfd,sdms,MAXSIZE,0) < 0){
                perror("send_chg3");
            }
        }
        //接收用户的邮箱或电话来匹配
        bzero(rvms,sizeof(msg));
        if((len = recv(connfd,rvms,MAXSIZE,0)) < 0){
            perror("recv_chg2");
        }
        if(rvms->type == USER_EMCHG){
            memcpy(usr.email,rvms->data,rvms->msglen);
            //从数据库中找客户端输入的邮箱和用户名
            sprintf(sql,"select * from person where name = \'%s\' and email = \'%s\';",usr.name,usr.email);
            int ret1 = mysql_query(conn,sql);
            res = mysql_store_result(conn);
            num_rows = mysql_num_rows(res);
            //如果出现过则邮箱匹配正确
            if(num_rows != 0){
                sdms->type = USER_AGREE;
                sdms->msglen = 0;
                if(send(connfd,sdms,MAXSIZE,0) < 0){
                    my_err("send",__LINE__);
                }
            }else{
                sdms->type = USER_DISAG;
                sdms->msglen = 0;
                if(send(connfd,sdms,MAXSIZE,0) < 0){
                    my_err("send",__LINE__);
                }
                return;
            }
        }
        
        //电话匹配
        else if(rvms->type == USER_PHCHG){
        memcpy(usr.phonenum,rvms->data,rvms->msglen);
            //从数据库中找客户端输入的邮箱和用户名
            sprintf(sql,"select * from person where name = \'%s\' and phone = \'%s\';",usr.name,usr.phonenum);
            int ret1 = mysql_query(conn,sql);
            res = mysql_store_result(conn);
            num_rows = mysql_num_rows(res);
            //如果出现过则电话匹配正确
            if(num_rows != 0){
                sdms->type = USER_AGREE;
                sdms->msglen = 0;
                if(send(connfd,sdms,MAXSIZE,0) < 0){
                    my_err("send",__LINE__);
                }
            }else{
                sdms->type = USER_DISAG;
                sdms->msglen = 0;
                if(send(connfd,sdms,MAXSIZE,0) < 0){
                    my_err("send",__LINE__);
                }
                return;
        }
    }

        //接收客户端传来的新密码
        bzero(rvms,sizeof(msg));
        if((len = recv(connfd,rvms,MAXSIZE,0)) < 0){
            perror("recv_log4");
        }else if(0 == len){
            printf("客户端断开连接...");
            getchar();
        }
        if(rvms->type == USER_MASSAGE){
            memcpy(usr.password,rvms->data,rvms->msglen);
            puts("password: ");
            puts(usr.password);
            //匹配帐号
            sprintf(sql,"update person set passwd = \'%s\' where name = \'%s\';",usr.password,usr.name);
            int ret2 = mysql_query(conn,sql);
            if(ret2 == 0){
                puts("change success");
                bzero(sdms,sizeof(msg));
                sdms->type = USER_AGREE;
                strcpy(sdms->data,"change success");
                sdms->msglen = strlen(sdms->data);
                sdms->data[sdms->msglen] = '\0';
                if(send(connfd,sdms,MAXSIZE,0) < 0 ){
                    my_err("send",__LINE__);
                }
            }else{
                puts("change failed");
                bzero(sdms,sizeof(msg));
                sdms->type = USER_DISAG;
                strcpy(sdms->data,"change failed");
                sdms->msglen = strlen(sdms->data);
                sdms->data[sdms->msglen] = '\0';
                if(send(connfd,sdms,MAXSIZE,0) < 0 ){
                    my_err("send",__LINE__);
                }
            }
        }
    }
}


//服务器菜单
void *server_meun(void *connfd){
    sock *info =(sock*)connfd;
    char ip[32];
    int flag = 0;
    printf("客户端[IP：%s],[port: %d]已链接...\n",
    inet_ntop(AF_INET,&info->servaddr.sin_addr.s_addr,ip,sizeof(ip)),
    ntohs(info->servaddr.sin_port));
    while(1){
        bzero(rvms,sizeof(msg));
        int len = recv(info->connfd,rvms,MAXSIZE,0);
        if(len < 0){
            perror("recv");
            exit(1);
        }else if(len == 0){
            printf("客户端断开连接...\n");
            getchar();
        }
    switch (rvms->type)
    {
    case 1:     //注册
        p_sign(info->connfd);
        break;
    case 2:     //登陆
        p_login(info->connfd);
        break;
    case 3:     //修改密码
        p_change(info->connfd);
        break;
    case 4:     //找回密码
        p_find(info->connfd);
    break;
    case 0:     //退出
        break;
    default:
        break;
    }
    }
    close(info->connfd);
    info->connfd = -1;
    return NULL;
}

//服务器线程创建
// void serv_ptcreate(){
//     printf("kai");
//     int l;
//     int ret = -1;
//     int i = 0;
//     int connfd;
//     //多个线程并发
//     while(start && startnum < 10){
//         cliaddr_len = sizeof(cliaddr);
//         //接收来自客户端的请求,若没有客户端请求，则在此处阻塞
//         if((connfd = accept(listenfd,(struct sockaddr*)&cliaddr,&cliaddr_len)) == -1){
//             perror("accept");
//         }
//         printf("已成功和客户端[Port:%d][Address:%s]建立链接\n",cliaddr.sin_port,cliaddr.sin_addr);
//         //创建子线程
//         if((ret = pthread_create(&tid[i++],NULL,server_meun,(void *)&connfd)) != 0){
//             perror("pthread_create");
//         }
//         startnum++;
//     }
// }

void sighandler(){
    free(rvms);
	free(sdms);
    close(listenfd);
	exit(0);
}

int main(){





    
    //int listenfd,connfd;
    struct   sockaddr_in servaddr;
    int      optval;
    signal(SIGINT,sighandler);
    //init_serv();
    //开创空间作为数据发送包
    rvms = (msg*)malloc(MAXSIZE);
    sdms = (msg*)malloc(MAXSIZE);

    //初始化socket
    listenfd = socket(AF_INET,SOCK_STREAM,0);
    if(listenfd < 0){
        perror("socket");
    }

    //来防止客户端异常退出导致address alreay in use，sock退出后可以正常使用
optval = 1;
if(setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(void*)&optval,sizeof(int)) < 0){
    my_err("setsockopt",__LINE__);
}
setsockopt(listenfd,SOL_SOCKET,SO_KEEPALIVE,(void*)&optval,sizeof(int));
    bzero(&servaddr,sizeof(struct sockaddr_in));
    //初始化servaddr内部变量
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);       //调用 htons 来将端口换为网络端口
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);  //调用 htonl 来将地址转为小端地址
    //绑定套接字
    if(bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) < 0)
    perror("bind");
    //设置监听
    listen(listenfd,LINSTENNUM);

    //初始化mysql
    if(init_mysql())
    printf("%s",mysql_error(conn));
    //选择创建数据库
    create_databases();
    //初始化表
    find_table();





int epfd = epoll_create(1);

// int max = sizeof(infos)/sizeof(infos[0]);

// for(int i = 0;i < max;i++){
//     bzero(&infos[i],sizeof(infos[i]));
//     infos[i].connfd = -1;
// }

// int infolen = sizeof(sock);
int socklen = sizeof(struct sockaddr_in);


while(1){

}



















    //serv_ptcreate();
    //多个线程并发
    // while(1){
          
    //     sock * info;
    //     for(int i = 0;i < max; i++){
    //         if(infos[i].connfd == -1){
    //             info = &infos[i];
    //             break;
    //         }
    //     }
    //     //接收来自客户端的请求,若没有客户端请求，则在此处阻塞

    //     connfd = accept(listenfd,(struct sockaddr*)&info->servaddr,&infolen);
    //     info->connfd = connfd;

    //     //创建子线程
    //     pthread_t tid;
    //     pthread_create(&tid,NULL,server_meun,(void*)info);
    //     pthread_detach(tid);
    // }
return 0;
}