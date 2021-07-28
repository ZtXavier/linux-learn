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

int close_mysql(MYSQL mysql) {
    mysql_close(&mysql);
    mysql_library_end();
    
    return 0;
}

int user_find(recv_datas *mybag,MYSQL mysql){
    MYSQL_RES *res = NULL;
    MYSQL_ROW  row;
    recv_datas *data = mybag;
    char       sql[MYSQL_MAX];
    pthread_mutex_lock(&mutex);
    bzero(sql,sizeof(sql));
    sprintf(sql,"select * from person where id = \'%d\';",data->send_id);
    mysql_query(&mysql,sql);
    res = mysql_store_result(&mysql);
    row = mysql_fetch_row(res);
    if(row == NULL){
    bzero(data->write_buff,sizeof(data->write_buff));
    strcpy(data->write_buff,"your id is error");
    pthread_mutex_unlock(&mutex);
    return 0;
    }else{
    if(strcmp(data->send_name,row[1]) != 0){
        bzero(data->write_buff,sizeof(data->write_buff));
        strcpy(data->write_buff,"your nickname is error");
        pthread_mutex_unlock(&mutex);
        return 0;
    }else{
        bzero(data->write_buff,sizeof(data->write_buff));
        sprintf(data->write_buff, "your passwd:%s", row[2]);
        if(send(data->recvfd,data,sizeof(recv_datas),0) < 0){
            my_err("send", __LINE__);
        }
        pthread_mutex_unlock(&mutex);
        return 1;
    }
}

}

int user_login(recv_datas *mybag,MYSQL mysql){
    int   ret;
    int   i;
    char  sql[MYSQL_MAX];
    recv_datas  *data = mybag;
    MYSQL_RES    *res  = NULL;
    MYSQL_ROW     row;
    bzero(sql,sizeof(sql));
    sprintf(sql,"select * from person where id = \'%d\';",data->send_id);
    pthread_mutex_lock(&mutex);
    //mysql_query(&mysql,sql);
    if((ret = mysql_query(&mysql,sql)) == 0){
    res = mysql_store_result(&mysql);
    row = mysql_fetch_row(res);
    if(row == NULL){
        pthread_mutex_unlock(&mutex);
        return -1;
    }else if(strcmp(row[2],data->read_buff) == 0){          //判断密码
        strcpy(data->send_name,row[1]);                     //将昵称写入缓冲区好友申请时发送给对方
        bzero(sql,sizeof(sql));
        data->recvfd = atoi(row[4]);
        sprintf(sql,"update person set state = \'1\' where id = \'%d\';",data->send_id);
        mysql_query(&mysql,sql);
        mysql_free_result(res);
        pthread_mutex_unlock(&mutex);
        return 1;
    }else{
        data->recvfd = atoi(row[4]);
        pthread_mutex_unlock(&mutex);
        return -1;
    }
    }else{
        printf("failed to query\n");
        pthread_mutex_unlock(&mutex);
        return -1;
    }
}

int user_sign(recv_datas *mybag,MYSQL mysql){
    FILE           *fp;
    recv_datas      *recv_data = mybag;
    char            sql[MYSQL_MAX];
    int             idnums;
    pthread_mutex_lock(&mutex);
    if((fp = fopen("idnums.txt","r")) == NULL){
        printf("打开文件失败\n");
        exit(1);
    }
    fread(&idnums,sizeof(int),1,fp);
    sprintf(sql,"insert into person values(\'%d\',\'%s\',\'%s\',\'%d\',\'%d\');",idnums,recv_data->send_name,recv_data->read_buff,0,recv_data->recvfd);

    recv_data->send_id = idnums;
    idnums -= 1 ;
    mysql_query(&mysql,sql);
    fclose(fp);
    if((fp = fopen("idnums.txt", "w")) == NULL){
        printf("打开文件失败\n");
        exit(1);
    }
    fwrite(&idnums,sizeof(int),1,fp);
    fclose(fp);
    pthread_mutex_unlock(&mutex);
}

int user_change(recv_datas *mybag,MYSQL mysql){
    MYSQL_RES  *res = NULL;
    MYSQL_ROW   row;
    recv_datas *recv_data = mybag;
    char        sql[MYSQL_MAX];
    bzero(sql,sizeof(sql));
    sprintf(sql,"select * from person where id = \'%d\';",recv_data->send_id);
    pthread_mutex_lock(&mutex);
    mysql_query(&mysql,sql);
    res = mysql_store_result(&mysql);
    if((row = mysql_fetch_row(res))){
        if(strcmp(recv_data->read_buff,row[2]) == 0){
            //recv_data->recvfd = atoi(row[4]);
            bzero(sql,sizeof(sql));
            sprintf(sql,"update person set passwd = \'%s\' where id = \'%d\';",recv_data->write_buff,recv_data->send_id);
            mysql_query(&mysql,sql);
            pthread_mutex_unlock(&mutex);
            return 1;
        }else{
            pthread_mutex_unlock(&mutex);
            return 0;
        }
    }else{
        pthread_mutex_unlock(&mutex);
        return 0;
    }


}

int add_friend(recv_datas *mybag,MYSQL mysql){
    MYSQL_RES *res = NULL;
    MYSQL_ROW  row_find,row;
    recv_datas *recv_data = mybag;
    list_box    box = NULL;
    char        sql[MYSQL_MAX];
    char        buf[100];
    bzero(sql,sizeof(sql));
    sprintf(sql,"select * from person where id = \'%d\';",recv_data->recv_id);
    pthread_mutex_lock(&mutex);
    int ret = mysql_query(&mysql,sql);
    res = mysql_store_result(&mysql);
    row_find = mysql_fetch_row(res);
    if(row_find == NULL){          //若查不到说明无此人,若查到则拿到该人的所有信息
    pthread_mutex_unlock(&mutex);
    return 0;
    }else{
    bzero(sql,sizeof(sql));
    sprintf(sql,"select * from friends where your_id = \'%d\' and friend_id = \'%d\';",recv_data->send_id,recv_data->recv_id);
    ret = mysql_query(&mysql,sql);
    printf("%d",ret);
    res = mysql_store_result(&mysql);
    row = mysql_fetch_row(res);
    if(row != NULL){               //如果已经是好友则返回
    pthread_mutex_unlock(&mutex);
    return 0;
    }else{                        //在此处要考虑对方在线情况
    bzero(buf,sizeof(buf));
    sprintf(buf,"[帐号:%d][昵称:%s]该用户向您发来好友申请",recv_data->send_id,recv_data->send_name);
    if(atoi(row_find[3]) == 1){
    recv_data->type = FRIEND_PLS;
    strcpy(recv_data->read_buff,buf);
    recv_data->sendfd = atoi(row_find[4]);
    strcpy(recv_data->recv_name,row_find[1]);
    if(send(recv_data->sendfd,recv_data,sizeof(recv_datas),0) < 0){
    my_err("send",__LINE__);
    }
    pthread_mutex_unlock(&mutex);
    return 1;
    }else{   //存消息盒子
    box = head;
    while(box != NULL){
        if(box->recv_id == recv_data->recv_id){
        box->fri_pls_id[box->fri_pls_num] = recv_data->send_id;
        strcpy(box->send_pls[box->fri_pls_num++],buf);
        }
        box = box->next;
    }if(box == NULL){
        box = (list_box)malloc(sizeof(BOX_MSG));
        box->recv_id = recv_data->recv_id;
        box->recv_msgnum = 0;
        box->fri_pls_num = 0;
        box->fri_pls_id[box->fri_pls_num] = recv_data->send_id;
        strcpy(box->send_pls[box->fri_pls_num++],buf);
        if(head== NULL){
        head = tail = box;
        tail->next = NULL;
        }else{
        tail->next = box;
        tail = box;
        tail->next = NULL;
        }
    }
    return 1;
    }
    }
}
    pthread_mutex_unlock(&mutex);
}

int friend_pls(recv_datas *mybag,MYSQL mysql){
    MYSQL_RES   *res = NULL;
    MYSQL_ROW   row;
    recv_datas  *recv_data = mybag;
    char         sql[MYSQL_MAX];
    pthread_mutex_lock(&mutex);
    
    if(strcmp(recv_data->read_buff,"ok") == 0){
    sprintf(sql,"insert into friends values(\'%d\',\'%d\',0);",recv_data->send_id,recv_data->recv_id);
    mysql_query(&mysql,sql);
    bzero(sql,sizeof(sql));
    sprintf(sql,"insert into friends values(\'%d\',\'%d\',0);",recv_data->recv_id,recv_data->send_id);
    mysql_query(&mysql,sql);
    pthread_mutex_unlock(&mutex);
    }else
    pthread_mutex_unlock(&mutex);
}

int del_friend(recv_datas *mybag,MYSQL mysql){
    MYSQL_RES *res = NULL;
    MYSQL_ROW  row;
    recv_datas *recv_data = mybag;
    char         sql[MYSQL_MAX];
    bzero(sql,sizeof(sql));
    sprintf(sql,"select * from friends where friend_id = \'%d\' and your_id = \'%d\';",recv_data->recv_id,recv_data->send_id);
    pthread_mutex_lock(&mutex);
    int ret = mysql_query(&mysql,sql);
    res = mysql_store_result(&mysql);
    row = mysql_fetch_row(res);
    if(row == NULL){
    bzero(recv_data->write_buff,sizeof(recv_data->write_buff));       //查不到说明没有好友
    strcpy(recv_data->write_buff,"fail");
    pthread_mutex_unlock(&mutex);
    return 0;
    }else{
    int ret = mysql_query(&mysql,sql);
    res = mysql_store_result(&mysql);
    bzero(sql,sizeof(sql));
    sprintf(sql,"delete from friends where friend_id = \'%d\' and your_id = \'%d\';",recv_data->recv_id,recv_data->send_id);
    ret = mysql_query(&mysql,sql);
    bzero(sql,sizeof(sql));
    sprintf(sql,"delete from friends where your_id = \'%d\' and friend_id = \'%d\';",recv_data->recv_id,recv_data->send_id);
    ret = mysql_query(&mysql,sql);
    bzero(recv_data->write_buff,sizeof(recv_data->write_buff));
    strcpy(recv_data->write_buff,"delete success");
    pthread_mutex_unlock(&mutex);
    return 0;
    }
}

int black_list(recv_datas *mybag,MYSQL mysql){
    MYSQL_RES  *res = NULL;
    MYSQL_ROW   row;
    recv_datas *recv_data = mybag;
    char        sql[MYSQL_MAX];
    bzero(sql,sizeof(sql));
    sprintf(sql,"select * from friends where your_id = \'%d\' and friend_id = \'%d\';",recv_data->send_id,recv_data->recv_id);
    pthread_mutex_lock(&mutex);
    int ret = mysql_query(&mysql,sql);
    res = mysql_store_result(&mysql);
    row = mysql_fetch_row(res);
    if(row == NULL){
        bzero(recv_data->write_buff,sizeof(recv_data->write_buff));
        strcpy(recv_data->write_buff,"black failed");
        pthread_mutex_unlock(&mutex);
        return 0;
    }else{
    bzero(sql,sizeof(sql));
    sprintf(sql,"update friends set relation = '-1' where your_id = \'%d\' and friend_id = \'%d\';",recv_data->send_id,recv_data->recv_id);
    ret = mysql_query(&mysql,sql);
    bzero(recv_data->write_buff,sizeof(recv_data->write_buff));
    strcpy(recv_data->write_buff,"black success");
    pthread_mutex_unlock(&mutex);
    return 0;
    }
}

int quit_black(recv_datas *mybag,MYSQL mysql){
    MYSQL_RES    *res = NULL;
    MYSQL_ROW     row;
    recv_datas   *recv_data = mybag;
    char          sql[MYSQL_MAX];
    bzero(sql,sizeof(sql));
    sprintf(sql,"select * from friends where your_id = \'%d\' and friend_id = \'%d\';",recv_data->send_id,recv_data->recv_id);
    pthread_mutex_lock(&mutex);
    int ret = mysql_query(&mysql,sql);
    res = mysql_store_result(&mysql);
    row = mysql_fetch_row(res);
    if(row == NULL){
        bzero(recv_data->write_buff,sizeof(recv_data->write_buff));
        strcpy(recv_data->write_buff,"quit black failed");
        pthread_mutex_unlock(&mutex);
        return 0;
    }else{
    bzero(sql,sizeof(sql));
    sprintf(sql,"update friends set relation = '0' where your_id = \'%d\' and friend_id = \'%d\';",recv_data->send_id,recv_data->recv_id);
    ret = mysql_query(&mysql,sql);
    bzero(recv_data->write_buff,sizeof(recv_data->write_buff));
    strcpy(recv_data->write_buff,"quit ok");
    pthread_mutex_unlock(&mutex);
    return 0;
    }
}

int send_info(recv_datas *mybag,MYSQL mysql){
    MYSQL_RES   *res = NULL;
    MYSQL_ROW    row;
    recv_datas  *recv_data = mybag;
    char         sql[MYSQL_MAX];
    BOX_MSG      *b = NULL;
    bzero(sql,sizeof(sql));
    sprintf(sql,"select * from person where id = \'%d\';",recv_data->recv_id);
    pthread_mutex_lock(&mutex);
    int ret = mysql_query(&mysql,sql);
    res = mysql_store_result(&mysql);
    if((row = mysql_fetch_row(res)) == NULL){  //该用户没有注册
        pthread_mutex_unlock(&mutex);
        return 0;
    }else{
    strcpy(recv_data->recv_name,row[1]);  //找到该人的姓名
    if(atoi(row[3]) == 1){                //如果在线
    recv_data->type = RECV_INFO;          //将事件类型改为收消息
    recv_data->recvfd = atoi(row[4]);
    bzero(sql,sizeof(sql));
    sprintf(sql,"select * from friends where your_id = \'%d\' and friend_id = \'%d\';",recv_data->recv_id,recv_data->send_id);
    ret = mysql_query(&mysql,sql);
    res = mysql_store_result(&mysql);
    row = mysql_fetch_row(res);
    if(row == NULL){
        pthread_mutex_unlock(&mutex);
        return 0;
    }else{
        if(atoi(row[2]) == 0){           //普通好友
        if(send(recv_data->recvfd,recv_data,sizeof(recv_datas),0) < 0){
        my_err("send",__LINE__);
        }
        bzero(sql,sizeof(sql));
        sprintf(sql,"insert into massage values(\'%d\',\'%d\',\'%s\',\'1\',\'1\');",recv_data->send_id,recv_data->recv_id,recv_data->read_buff);
        ret = mysql_query(&mysql,sql);
        pthread_mutex_unlock(&mutex);
        }else if(atoi(row[2]) == -1){    //拉黑状态
        recv_data->type = SEND_INFO;
        pthread_mutex_unlock(&mutex);
        return 0;
        }
    }
}else{                   //如果对方不在线，将放到状态链表中
    b = head;
    while(NULL != b){
        if(b->recv_id == recv_data->recv_id){
            b->send_id[b->recv_msgnum] = recv_data->send_id;
            strcpy(b->send_msg[b->recv_msgnum++],recv_data->read_buff);
            bzero(sql,sizeof(sql));
            sprintf(sql,"insert into massage values(\'%d\',\'%d\',\'%s\',\'1\',\'1\');",recv_data->send_id,recv_data->recv_id,recv_data->read_buff);
            ret = mysql_query(&mysql,sql);
            break;
        }
        b = b->next;
    }
    }
    recv_data->type = SEND_INFO;
    pthread_mutex_unlock(&mutex);
    return 1;
    }
    
}

FRIENDS* look_list(recv_datas *mybag,MYSQL mysql){
    MYSQL_RES    *res = NULL;
    MYSQL_RES    *resl = NULL;
    MYSQL_ROW     row;
    MYSQL_ROW     rows;
    recv_datas   *recv_data = mybag;
    FRIENDS      *list;
    char          sql[MYSQL_MAX];
    pthread_mutex_lock(&mutex);
    list = (FRIENDS*)malloc(sizeof(FRIENDS));
    list->friend_num = 0;
    bzero(sql,sizeof(sql));
    sprintf(sql,"select * from friends where your_id = \'%d\';",recv_data->send_id);
    int ret = mysql_query(&mysql,sql);
    res = mysql_store_result(&mysql);
    while(row = mysql_fetch_row(res)){
    list->friend_id[list->friend_num] = atoi(row[1]);
    bzero(sql,sizeof(sql));
    sprintf(sql,"select * from person where id = \'%d\';",atoi(row[1]));
    ret = mysql_query(&mysql,sql);
    resl = mysql_store_result(&mysql);
    rows = mysql_fetch_row(resl);
    list->friend_state[list->friend_num] = atoi(rows[3]);
    strcpy(list->friend_nickname[list->friend_num++],rows[1]);
    }
    if(list->friend_num == 0){
    pthread_mutex_unlock(&mutex);
    return list;
    }else{
    pthread_mutex_unlock(&mutex);
    return list;
    }
}

int look_history(recv_datas *mybag,MYSQL mysql){
MYSQL_RES *res = NULL;
MYSQL_ROW  row;
recv_datas *recv_data = mybag;
MSG        *his_msg;
char        sql[MYSQL_MAX];
his_msg = (MSG*)malloc(sizeof(MSG));
his_msg->num = 0;
bzero(sql,sizeof(sql));
sprintf(sql,"select * from massage where your_id = \'%d\' and recv_id = \'%d\' and y_look = \'1\' union all select * from massage where your_id = \'%d\' and recv_id = \'%d\' and recv_look = \'1\';",recv_data->send_id,recv_data->recv_id,recv_data->recv_id,recv_data->send_id);
pthread_mutex_lock(&mutex);
int ret = mysql_query(&mysql,sql);
res = mysql_store_result(&mysql);
while((row = mysql_fetch_row(res))){
his_msg->send_id[his_msg->num] = atoi(row[0]);
his_msg->recv_id[his_msg->num] = atoi(row[1]);
strcpy(his_msg->message[his_msg->num++],row[2]);
}
if(send(recv_data->sendfd,recv_data,sizeof(recv_datas),0) < 0){
    my_err("send",__LINE__);
}
if(send(recv_data->sendfd,his_msg,sizeof(MSG),0) < 0){
    my_err("send",__LINE__);
}
pthread_mutex_unlock(&mutex);
return 1;
}

int dele_history(recv_datas *mybag,MYSQL mysql){
MYSQL_RES  *res = NULL;
MYSQL_ROW   row;
recv_datas *recv_data = mybag;
char        sql[MYSQL_MAX];

bzero(sql,sizeof(sql));
sprintf(sql,"update massage set y_look = \'0\' where your_id = \'%d\' and recv_id = \'%d\';",recv_data->send_id,recv_data->recv_id);
pthread_mutex_lock(&mutex);
int ret = mysql_query(&mysql,sql);
bzero(sql,sizeof(sql));
sprintf(sql,"update massage set recv_look = \'0\' where recv_id = \'%d\' and your_id = \'%d\';",recv_data->send_id,recv_data->recv_id);
ret = mysql_query(&mysql,sql);
bzero(sql,sizeof(sql));
sprintf(sql,"delete from massage where y_look = \'0\' and recv_look = \'0\';");
ret = mysql_query(&mysql,sql);
pthread_mutex_unlock(&mutex);
return 0;
}

int create_group(recv_datas *mybag,MYSQL mysql){
MYSQL_RES   *res = NULL;
MYSQL_ROW    row;
recv_datas  *recv_data = mybag;
char         sql[MYSQL_MAX];
FILE        *fp;
int          num;
bzero(sql,sizeof(sql));
pthread_mutex_lock(&mutex);
if((fp = fopen("group.txt","r")) == NULL){
printf("Error opening");
pthread_mutex_unlock(&mutex);
return -1;
}
fread(&num, sizeof(int), 1, fp);
fclose(fp);
sprintf(sql,"insert into groups_info values(\'%d\',\'%s\',\'1\');",num,recv_data->recv_name);
int ret = mysql_query(&mysql,sql);
bzero(sql,sizeof(sql));
sprintf(sql,"insert into groups values(\'%d\',\'%s\',\'%d\',\'%s\',\'2\');",num,recv_data->recv_name,recv_data->send_id,recv_data->send_name);
ret = mysql_query(&mysql,sql);
recv_data->recv_id = num;
num -= 1;
if((fp = fopen("group.txt","w")) == NULL){
printf("Error opening");
pthread_mutex_unlock(&mutex);
return -1;
}
fwrite(&num, sizeof(int), 1, fp);
fclose(fp);
pthread_mutex_unlock(&mutex);
return 1;
}

int dissolve_group(recv_datas *mybag,MYSQL mysql){
MYSQL_RES  *res = NULL;
MYSQL_RES  *resl = NULL;
MYSQL_ROW   row,row_info;
recv_datas *recv_data = mybag;
char        sql[MYSQL_MAX];
bzero(sql,sizeof(sql));
sprintf(sql,"select * from groups_info where group_id = \'%d\';",recv_data->recv_id);
pthread_mutex_lock(&mutex);
int ret = mysql_query(&mysql,sql);
res = mysql_store_result(&mysql);
row = mysql_fetch_row(res);
if(row == NULL){             //没有此群
pthread_mutex_unlock(&mutex);
return 0;
}else{
bzero(sql,sizeof(sql));
sprintf(sql,"select * from groups where group_id  = \'%d\' and group_mem_id = \'%d\';",recv_data->recv_id,recv_data->send_id);
ret = mysql_query(&mysql,sql);
resl = mysql_store_result(&mysql);
row_info = mysql_fetch_row(resl);
if(row_info == NULL){
pthread_mutex_unlock(&mutex);
return -2;
}
if(atoi(row_info[4]) == 2){
bzero(sql,sizeof(sql));
sprintf(sql,"delete from groups where group_id = \'%d\';",recv_data->recv_id);
ret = mysql_query(&mysql,sql);
bzero(sql,sizeof(sql));
sprintf(sql,"delete from groups_info where group_id = \'%d\';",recv_data->recv_id);
ret = mysql_query(&mysql,sql);
pthread_mutex_unlock(&mutex);
return 1;
}else{
pthread_mutex_unlock(&mutex);
return -1;                       //不是群主没有权限    （群主：2,管理员：1，成员：0）
}
}
}

void *ser_deal(void *arg){
    int i;
    MYSQL mysql;
    mysql = init_mysql();
    FRIENDS  *friend_list = NULL;
    list_box  box = head;
    recv_datas *recv_buf = (recv_datas*)arg;
    int choice = recv_buf->type;
    switch(choice){
        case USER_LOGIN://登录后看消息盒子，包
        if(!(user_login(recv_buf,mysql))){
        recv_buf->type = ID_ERROR;
        bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
        strcpy(recv_buf->write_buff,"error");
        if (send(recv_buf->recvfd, recv_buf, sizeof(recv_datas), 0) < 0) {
        my_err("send", __LINE__);
        }
        }else{
        bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
        strcpy(recv_buf->write_buff,"sucess");
        if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
        my_err("send",__LINE__);
        }
        while(box != NULL){
            if(box->recv_id == recv_buf->send_id){
                break;
            }
            box = box->next;
        }
        if(box == NULL){
        box = (list_box)malloc(sizeof(BOX_MSG));
        box->recv_id = recv_buf->send_id;
        box->recv_msgnum = 0;
        box->fri_pls_num = 0;         //记得初始化群信息
        box->next = NULL;
        if(head == NULL){
        head = box;
        tail = box;
        }else{
        tail->next = box;
        tail= box;
        }
        if(send(recv_buf->recvfd,box,sizeof(BOX_MSG),0) < 0){
        my_err("send",__LINE__);
        }
        }else{
        if(send(recv_buf->recvfd,box,sizeof(BOX_MSG),0) < 0){
        my_err("send",__LINE__);
        }
        box->recv_msgnum = 0;
        box->fri_pls_num = 0;
        }
        }
        break;

        case USER_SIGN:
        user_sign(recv_buf,mysql);
        bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
        strcpy(recv_buf->write_buff,"sign success");
        if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
        my_err("send",__LINE__);
        }
        break;

        case USER_FIND:
        if(!user_find(recv_buf,mysql)){
        if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
        my_err("send",__LINE__);
        }
        }
        break;

        case USER_CHANGE:
        if(!user_change(recv_buf,mysql)){
        strcpy(recv_buf->write_buff,"error");
        }else{
        strcpy(recv_buf->write_buff,"success");
        }
        if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
        my_err("send",__LINE__);
        }
        break;

        case ADD_FRIEND:
        if(add_friend(recv_buf,mysql)){
        recv_buf->type = ADD_FRIEND;
        bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
        strcpy(recv_buf->write_buff,"ok");
        if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
        my_err("send",__LINE__);
        }
        }else{
        recv_buf->type = ADD_FRIEND;
        bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
        strcpy(recv_buf->write_buff,"no");
        if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
        my_err("send",__LINE__);
        }
        }
        break;

        case FRIEND_PLS:
        friend_pls(recv_buf,mysql);
        break;

        case DEL_FRIEND:
        del_friend(recv_buf,mysql);
        if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
        my_err("send",__LINE__);
        }
        break;

        case BLACK_LIST:
        black_list(recv_buf,mysql);
        if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
        my_err("send",__LINE__);
        }
        break;

        case QUIT_BLACK:
        quit_black(recv_buf,mysql);
        if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
        my_err("send",__LINE__);
        }
        break;

        case SEND_INFO:
        if(send_info(recv_buf,mysql)){
        bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
        strcpy(recv_buf->write_buff,"send ok");
        if(send(recv_buf->sendfd,recv_buf,sizeof(recv_datas),0) < 0){
        my_err("send",__LINE__);
        }
        }else{
        bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
        strcpy(recv_buf->write_buff,"send fail");
        if(send(recv_buf->sendfd,recv_buf,sizeof(recv_datas),0) < 0){
        my_err("send",__LINE__);
        }
        }
        break;

        case LOOK_FRI_LS:
        friend_list = look_list(recv_buf,mysql);
        if(friend_list->friend_num){
        bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
        strcpy(recv_buf->write_buff,"nice");
        if(send(recv_buf->sendfd,recv_buf,sizeof(recv_datas),0) < 0){
        my_err("send",__LINE__);
        }
        if(send(recv_buf->sendfd,friend_list,sizeof(FRIENDS),0) < 0){
        my_err("send",__LINE__);
        }
        }else{
        bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
        strcpy(recv_buf->write_buff,"bad");
        if(send(recv_buf->sendfd,recv_buf,sizeof(recv_datas),0) < 0){
        my_err("send",__LINE__);
        }
        if(send(recv_buf->sendfd,friend_list,sizeof(FRIENDS),0) < 0){
        my_err("send",__LINE__);
        }
        }
        break;

        case LOOK_HISTORY:
        look_history(recv_buf,mysql);
        break;

        case DELE_HISTORY:
        if(dele_history(recv_buf,mysql) == 0){
            bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
            strcpy(recv_buf->write_buff,"delete success");
            if(send(recv_buf->sendfd,recv_buf,sizeof(recv_datas),0) < 0){
            my_err("send",__LINE__);
            }
        }else{
            bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
            strcpy(recv_buf->write_buff,"delete fail");
            if(send(recv_buf->sendfd,recv_buf,sizeof(recv_datas),0) < 0){
            my_err("send",__LINE__);
            }
        }
        break;

        case CREATE_GROUP:  //建群
        if(create_group(recv_buf,mysql)){
            bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
            strcpy(recv_buf->write_buff,"create success");
            if(send(recv_buf->sendfd,recv_buf,sizeof(recv_datas),0) < 0){
            my_err("send",__LINE__);
            }
        }else{
             bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
            strcpy(recv_buf->write_buff,"create fail");
            if(send(recv_buf->sendfd,recv_buf,sizeof(recv_datas),0) < 0){
            my_err("send",__LINE__);
            }
        }
        break;


        case DISSOLVE_GROUP: //解散群
        if(dissolve_group(recv_buf,mysql) == 1){
        bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
        strcpy(recv_buf->write_buff,"dissolve success");
        if(send(recv_buf->sendfd,recv_buf,sizeof(recv_datas),0) < 0){
        my_err("send",__LINE__);
        }
        }else if(dissolve_group(recv_buf,mysql) == 0){
        bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
        strcpy(recv_buf->write_buff,"no group");
        if(send(recv_buf->sendfd,recv_buf,sizeof(recv_datas),0) < 0){
        my_err("send",__LINE__);
        }
        }else if(dissolve_group(recv_buf,mysql) == -2){
        bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
        strcpy(recv_buf->write_buff,"no num");
        if(send(recv_buf->sendfd,recv_buf,sizeof(recv_datas),0) < 0){
        my_err("send",__LINE__);
        }
        }else{
        bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
        strcpy(recv_buf->write_buff,"no host");
        if(send(recv_buf->sendfd,recv_buf,sizeof(recv_datas),0) < 0){
        my_err("send",__LINE__);
        }
        }
        break;


        case ADD_GROUP:

        break;


    }
close_mysql(mysql);
}




int main(void){
    MYSQL *myconn = NULL;
    MYSQL_RES *res = NULL;
    MYSQL_ROW  row;
    int   connfd,sockfd;
    int   optval;
    int   flag_recv = 0;
    int   names;
    int   connects = 0;
    char  sql[MYSQL_MAX];
    char   ip[32];
    pthread_t tid;
    recv_datas recv_buf;
    size_t ret;
    socklen_t clen;
    struct sockaddr_in cliaddr,servaddr;
    
    clen = sizeof(struct sockaddr_in);
    myconn  = mysql_init(NULL);
    if(!mysql_real_connect(myconn,"127.0.0.1","root","123456","chat",0,NULL,0)){
       fprintf(stderr, "Failed to connect to database: Error: %s\n",
        mysql_error(myconn));
    }
    if(mysql_set_character_set(myconn, "utf8") < 0){
		my_err("mysql_set_character_set", __LINE__);
	}
    pthread_mutex_init(&mutex, NULL);
    signal(SIGPIPE, SIG_IGN);

    int epfd,npfd;
    struct epoll_event ev;
    struct epoll_event events[MAXEVENTS];

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
    ev.events = EPOLLIN | EPOLLET;

    epoll_ctl(epfd,EPOLL_CTL_ADD,sockfd,&ev);
    connects++;

    while(1){

        npfd = epoll_wait(epfd,events,MAXEVENTS,-1);
        for(int i = 0;i < npfd;i++){
            connects++;
            if(events[i].data.fd == sockfd){
                if(connects > MAXEVENTS){
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
                ev.events = EPOLLIN  | EPOLLET;
                epoll_ctl(epfd,EPOLL_CTL_ADD,connfd,&ev); //新增套接字
            }
            /* 用户正常发来消息请求 */
            else if(events[i].events & EPOLLIN){
                bzero(&recv_buf,sizeof(recv_datas));
                if((ret = recv(events[i].data.fd,&recv_buf,sizeof(recv_datas),MSG_WAITALL)) < 0){
                    my_err("recv",__LINE__);
                    close(events[i].data.fd);
                    continue;
                }
                if(recv_buf.type == USER_OUT){
                    if(send(events[i].data.fd,&recv_buf,sizeof(recv_datas),0) < 0){
                        my_err("send",__LINE__);
                    }
                printf("客户端ip[%d]已断开连接...\n",events[i].data.fd);
                // bzero(sql,sizeof(sql));
                // sprintf(sql,"select * from person where fd = \'%d\';",events[i].data.fd);
                // mysql_query(&mysql,sql);
                // res = mysql_store_result(&mysql);
                // row = mysql_fetch_row(res);
                bzero(sql,sizeof(sql));
                sprintf(sql,"update person set state = \'0\' where state = \'1\' and fd = \'%d\';",events[i].data.fd);
                mysql_query(myconn,sql);
                //mysql_free_result(res);
                continue;
                }
                if(recv_buf.type == USER_LOGIN){
                memset(sql,0,sizeof(sql));
                sprintf(sql,"select * from person where id = \'%d\';",recv_buf.send_id);
                pthread_mutex_lock(&mutex);
                mysql_query(myconn,sql);
                res = mysql_store_result(myconn);
                if(mysql_fetch_row(res) == NULL){
                    recv_buf.type = ID_ERROR;
                    bzero(recv_buf.write_buff,sizeof(recv_buf.write_buff));
                    printf("login failed...\n");
                    strcpy(recv_buf.write_buff,"id error");
                    if(send(events[i].data.fd,&recv_buf,sizeof(recv_datas),0)<0){
                    my_err("send",__LINE__);
                    }
                    pthread_mutex_unlock(&mutex);
                    continue;
                    }
                    bzero(sql,sizeof(sql));
                    sprintf(sql,"update person set fd = \'%d\' where id = \'%d\';",events[i].data.fd,recv_buf.send_id);
                    mysql_query(myconn,sql);
                    pthread_mutex_unlock(&mutex);
            }
                    recv_buf.recvfd = events[i].data.fd;
                    recv_datas *bf;
                    bf = (recv_datas*)malloc(sizeof(recv_datas));
                    memcpy(bf,&recv_buf,sizeof(recv_datas));
                    pthread_create(&tid,NULL,(void*)ser_deal,(void*)bf);
                    pthread_detach(tid);
        }
    }
  }
}

