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


int close_mysql(MYSQL mysql)
{
	mysql_close(&mysql);
	mysql_library_end();
	printf("end\n");
	return 0;
}