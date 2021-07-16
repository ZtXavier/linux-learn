#include<stdio.h>
#include<string.h>
#include<syslog.h>
#include<mysql.h>



/* 用固定的用户名和密码链接固定的数据库
   mysql:数据库句柄
   成功返回 0,失败返回 -1
 */
int mysql_fun_connect(MYSQL*mysql){
    if(!mysql_init(mysql)){
        syslog(LOG_ERR,"file:%s,line:%d,Mysql_init error",__FILE__,__LINE__);
        return  -1;
    }
    if(!mysql_real_connect(mysql,"127.0.0.1","root","ZHENGTIAN827625","example",0,NULL,0)){
        syslog(LOG_ERR,"file %s,line:%d,connect mysql error",__FILE__,__LINE__);
        return -1;
    }
    return 0;
}

/* 对mysql释放空间的api进行封装 */
void mysql_free_res(MYSQL_RES * res){
    mysql_free_result(res);
}

//对mysql关闭进行封装
void mysql_fun_close(MYSQL * mysql){
    syslog(LOG_DEBUG,"file:%s,line:%d.In the mysql_my_close",__FILE__,__LINE__);
    mysql_close(mysql);
}


/* 发送mysql客户端指令
   mysql：mysql句柄
   command:数据库客户端指令
    成功返回 0，失败返回-1 
*/

int mysql_fun_send(MYSQL * mysql,const char *command){
    int ret = 0;
    ret =  mysql_query(mysql,command);
    if(ret){
        syslog(LOG_ERR,"file:%s,line:%d,mysql_query_error%s",__FILE__,__LINE__,mysql_error(mysql));
        return -1;
    } 
    return 0;
}

/* 从数据库中获取结果并将其化为整数
   mysql：数据库句柄
   command：发送给数据库的sql命令
   成功返回整数结果，错误返回 -1
 */

int mysql_fun_get_int_res(MYSQL *mysql,const char *command){
    MYSQL_RES* res = NULL;
    MYSQL_ROW row;
    int ret = 0;

    ret = mysql_query(mysql,command);
    if(ret)
    syslog(LOG_ERR,"file:%ss,line:%d.Mysql query error:%s",__FILE__,__LINE__,mysql_error(mysql));
    else{
        res = mysql_store_result(mysql);
        if(res){
            row = mysql_fetch_row(res);
            if(row && (*row)){
                mysql_fun_free_res(res);
                res = NULL;
                return ((int)atoi(*row));
            }
            mysql_fun_free_res(res);
            res = NULL;
        }
        if(mysql_errno(mysql))
        syslog(LOG_ERR,"file:%s,line:%d.Retrive error :%s",__FILE__,__LINE__,mysql_errno(mysql));
    }
    return -1;
}


/* 从数据库中获取字符串结果
   mysql:数据库句柄
   command：符合要求的mysql客户端指令
   str：接受结果的字符串空间指针
   成功返回空间首地址，失败返回空
    */

   char *mysql_fun_get_char_res(MYSQL * mysql,char * command,char * str){
       MYSQL_RES * res = NULL;
       MYSQL_ROW row;
       int ret = 0;

       ret = mysql_query(mysql,command);
       if(ret)
       syslog(LOG_ERR,"file:%s,line:%d.Mysql_query error",__FILE__,__LINE__);
       else{
           res = mysql_store_result(mysql);
           if(res){
               row = mysql_fetch_row(res);
               if(row && (*row)){
                   strcpy(str,*row);
                   mysql_fun_free_res(res);
                   res = NULL;
                   return str;
               }
               mysql_fun_free_res(res);  //缺少该处可能会造成内存泄漏
               res = NULL;
           }
           if(mysql_errno(mysql))
            syslog(LOG_ERR,"file:%s,line:%d.Retrvie error:%s",__FILE__,__LINE__,mysql_errno(mysql));
       }
       return NULL;
   }





   /* 获取结果集中结果的数量
      mysql：数据库句柄 
      command:数据库指令
      成功返回结果集数量，失败返回-1
    */

   int mysql_fun_get_num(MYSQL*mysql,char * command){
       MYSQL_RES * res = NULL;
       int ret         =0;
       int num         =0;

       ret = mysql_query(mysql,command);
       if(ret){
           syslog(LOG_ERR,"file:%s,line:%d.selected error:%s",__FILE__,__LINE__,mysql_errno(mysql));
           return -1;
       }
       res = mysql_store_result(mysql);
       if(res){
           num = (int)mysql_num_rows(res);
           mysql_fun_free_res(res);
           res = NULL;
       }
       return num;
   }