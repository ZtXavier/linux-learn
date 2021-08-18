#include<stdio.h>
#include<sys/types.h>
#include <sys/stat.h>
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
#include"pthreadpool.h"





const int NUMBER = 3;    // 每次添加的最大线程数(定义为常量)


/* 任务结构体，任务回调函数，任务队列 */
typedef struct mission{
    void *(*func)(void* arg);     //任务回调函数:由于所存储的任务基于函数完成，所以来存储函数的地址
    void *arg;                    //回调函数的参数
}CThread_mission;


/* 线程池结构体 */
typedef struct CThread_Pool{

    CThread_mission  *mission;         /* 任务队列 */
    int queue_capacity;                /* 容量 */
    int queue_head;                    /* 队列头部->取数据 */
    int queue_rear;                    /* 队列尾部->存数据 */
    int queue_size_mission;            /* 任务队列在任务数目 */

    pthread_mutex_t  pool_mutex;       /* 互斥锁整个线程 */
    pthread_mutex_t  busy_mutex;       /* (找线程池中修改频率最多的来加锁) */
    pthread_cond_t   notFull;          /* 任务队列是否满(生产者) */
    pthread_cond_t   notEmpty;         /* 任务队列是否空(消费者) */  
    pthread_t        *threadID;        /* 工作线程ID */
    pthread_t         manageID;        /* 管理者线程ID */
    int maxthread_num;                 /* 线程池最大线程数 */
    int minthread_num;                 /* 线程池最小线程数 */
    int busynum;                       /* 工作线程个数 */
    int livenum;                       /* 存活线程个数 */
    int exitnum;                       /* 要销毁的线程个数 */
    int shutdown;                      /* 是否摧毁线程池0：不摧毁；1：摧毁 */
}threadpool;



//创建线程池并初始化
threadpool * threadPoolcreate(int max,int min,int queuesize);
//销毁线程池
int threaddestroy(threadpool *pool);
//添加任务函数
void threadpooladd(threadpool *pool,void *func(void*),void *arg);
//获取工作的线程个数
int threadbusynum(threadpool *pool);
//获取线程池中或者的线程个数
int threadalivenum(threadpool *pool);

void *worker(void* arg);
void *manager(void* arg);

void threadexit(threadpool *pool);



/* 创建线程池（初始化） */
threadpool * threadPoolcreate(int max,int min,int queuesize){
    threadpool *pool = (threadpool*)malloc(sizeof(threadpool));
    do{
    if(pool == NULL){
        printf("malloc threadpool failed\n");
        break;
    }

    pool->threadID = (pthread_t*)malloc(sizeof(pthread_t)*max);

    if(pool->threadID == NULL){
        printf("malloc threadID fail...\n");
        break;
    }

    memset(pool->threadID,0,sizeof(pthread_t)*max);

    pool->minthread_num = min;
    pool->maxthread_num = max;
    pool->busynum = 0;        //开始没有工作的线程
    pool->exitnum = 0;        //退出的线程
    pool->livenum = min;      //存活的线程，以最小个数为初始化

    if(pthread_mutex_init(&pool->pool_mutex,NULL) != 0 ||
    pthread_mutex_init(&pool->busy_mutex,NULL) != 0 ||
    pthread_cond_init(&pool->notEmpty,NULL) != 0 ||
    pthread_cond_init(&pool->notFull,NULL) != 0){
    //初始化成功返回0

    printf("pthread_init failed\n");

    break;
    }


    //任务队列
    pool->mission = (CThread_mission*)malloc(sizeof(CThread_mission)*queuesize);
    pool->queue_capacity = queuesize;
    pool->queue_head = 0;
    pool->queue_rear = 0;
    pool->queue_size_mission = 0;
    pool->shutdown = 0;

    pthread_create(&pool->manageID,NULL,manager,pool);

    for(int i = 0; i < min ; i++){
        pthread_create(&(pool->threadID[i]),NULL,worker,pool);
    }
    return pool;

   } while (0);  //循环中可以使用break来结束并且释放内存


   //释放资源
    if(pool->threadID) free(pool->threadID);
    if(pool->mission)  free(pool->mission);
    if(pool)           free(pool);
    return NULL;
}


/* 消费者线程 */
void * worker(void* arg){
    //先将函数类型转换
    threadpool *pool = (threadpool*)arg;
    //由于该函数需要不断向任务队列中取任务，所以先来个循环
    while(1){
        pthread_mutex_lock(&pool->pool_mutex);
       //当前任务队列是否为空，while是用来连续判断每个进来的线程所要的任务队列中的任务是否为空
        while((pool->queue_size_mission == 0) && !(pool->shutdown)){// 使用while而不用if是为了防止其虚假唤醒，也就是没有broadcast或signal时wait有返回值（下面的生产者阻塞也同理）
           //若为空且线程池未关闭,阻塞工作线程（线程池为空说明没有任务，所以阻塞消费者）
            pthread_cond_wait(&pool->notEmpty,&pool->pool_mutex);
           //pool->notEmpty用来唤醒线程，用来记录哪些任务被阻塞，若为零则阻塞，不为零时下次将该处阻塞的地方唤醒
            if(pool->exitnum > 0){
            pool->exitnum--;       //每次唤醒线程后，要销毁的线程数减一
            //在判断线程池中最小线程数小于存在线程后才可将存在的空闲的线程销毁
            if(pool->livenum > pool->minthread_num){

            pool->livenum--;
            pthread_mutex_unlock(&pool->pool_mutex);
            threadexit(pool);

            }
        }
    }
       // 判断线程池是否关闭
        if(pool->shutdown){
            pthread_mutex_unlock(&pool->pool_mutex);
            threadexit(pool);
        }

       //从任务队列中取出一个任务
        CThread_mission mission;
        mission.func = pool->mission[pool->queue_head].func; //取出任务函数的地址
        mission.arg = pool->mission[pool->queue_head].arg;  //取出参数
        //移动头结点
        pool->queue_head = (pool->queue_head + 1) % pool->queue_capacity;
        pool->queue_size_mission--;

       //解锁
        pthread_cond_signal(&pool->notFull);  // 在上面队列中任务已被消费后若有阻塞的生产者可以将其唤醒
        pthread_mutex_unlock(&pool->pool_mutex);

        printf("thread %ld start working\n",pthread_self());

        pthread_mutex_lock(&pool->busy_mutex);
        pool->busynum++;     //工作线程加一
        pthread_mutex_unlock(&pool->busy_mutex);

        mission.func(mission.arg);//第二种取地址来调用(*mission.func)(mission.arg);
    //    usleep(1000);
    //    free(mission.arg);
        mission.arg == NULL;

        printf("thread %ld end working\n",pthread_self());

        pthread_mutex_lock(&pool->busy_mutex);
        pool->busynum--;    //工作线程减一
        pthread_mutex_unlock(&pool->busy_mutex);
    }
    return NULL;
}

    /* 管理者线程 */
void *manager(void* arg){
    threadpool *pool = (threadpool *)arg;
    //当线程池没有关闭时管理者函数检测线程状态
    while(!pool->shutdown){
        //每隔三秒检测一次
        sleep(3);

       /* 取出线程池中任务的数量和当前线程的数量 */
        pthread_mutex_lock(&pool->pool_mutex);
        int queuesize = pool->queue_size_mission;
        int livenum   = pool->livenum;
        pthread_mutex_unlock(&pool->pool_mutex);

       /* 取出忙的线程的数量 */
        pthread_mutex_lock(&pool->busy_mutex);
        int busy_num = pool->busynum;
        pthread_mutex_unlock(&pool->busy_mutex);

       /* 添加线程 (对线程池中的线程操作) */
       /* 任务的个数大于存活的线程个数 && 存活的线程数小于最大线程数 */
        if((queuesize > livenum) && (livenum < pool->maxthread_num)){
            pthread_mutex_lock(&pool->pool_mutex);
            int counter = 0;
            for(int i=0;i < pool->maxthread_num && counter < NUMBER && pool->livenum < pool->maxthread_num;i++){
                if(pool->threadID[i] == 0){                   // 添加线程时需考虑每个线程id是否为零，若为零则没有创建线程
                    pthread_create(&pool->threadID[i],NULL,worker,pool);
                    counter++;          //添加的线程数+1
                    pool->livenum++;    //存活的线程+1
                }
            }
            pthread_mutex_unlock(&pool->pool_mutex);
        }

       /* 销毁线程 */
       /* 正在工作的线程*3 小于 存活的线程 && 存活的线程 大于 最小线程数 */
        if((busy_num*3 < livenum) && (livenum > pool->minthread_num)){
        pthread_mutex_lock(&pool->pool_mutex);
        pool->exitnum = NUMBER;
        pthread_mutex_unlock(&pool->pool_mutex);
        //让工作的线程自杀
            for(int i = 0;i < NUMBER;i++){
            pthread_cond_signal(&pool->notEmpty);  // 这里是将线程唤醒

            }
        }
    }
    return NULL;
}



/* 定义该函数是因为退出后其线程id没有初始化为零，下次调用会出现问题 */
void threadexit(threadpool *pool){
    pthread_t tid = pthread_self();
    for(int i = 0;i < pool->maxthread_num;i++){
        if(pool->threadID[i] == tid){
        printf("threadexit() called,%ld exiting...\n",tid);
        pool->threadID[i] = 0;
        break;
        }
    }
    pthread_exit(NULL);
}



/* 生产者线程 */
void threadpooladd(threadpool *pool,void *func(void*),void *arg){
    pthread_mutex_lock(&pool->pool_mutex);
    while((pool->queue_size_mission == pool->queue_capacity) && !(pool->shutdown)){   //满足该条件时说明线程池可以使用但是队列已经满了

        //当队列已满时，需要阻塞生产者线程
        pthread_cond_wait(&pool->notFull,&pool->pool_mutex);

    }
    if(pool->shutdown){                                                               // 判断线程池是否被关闭，若关闭进入函数退出，否则将向下执行..
        pthread_mutex_unlock(&pool->pool_mutex);
        return ;
    }

    //添加任务(从队尾添加，从队头取任务)
    pool->mission[pool->queue_rear].func = func;
    pool->mission[pool->queue_rear].arg = arg;
    pool->queue_rear = (pool->queue_rear+1) % pool->queue_capacity;
    pool->queue_size_mission++;

    //唤醒阻塞的消费者线程
    pthread_cond_signal(&pool->notEmpty);
    pthread_mutex_unlock(&pool->pool_mutex);
}



/* 工作线程计数 */
int threadbusynum(threadpool *pool){
    pthread_mutex_lock(&pool->busy_mutex);
    int busynum = pool->busynum;
    pthread_mutex_unlock(&pool->busy_mutex);
    return busynum;
}



/* 存活线程计数 */
int threadalivenum(threadpool *pool){
    pthread_mutex_lock(&pool->pool_mutex);
    int alivenum = pool->livenum;
    pthread_mutex_unlock(&pool->pool_mutex);
    return alivenum;
}



/* 销毁线程池 */
int threaddestroy(threadpool *pool){

    if(pool == NULL){
    return -1;
    }

    //关闭线程池

    pool->shutdown = 1;                 //当线程池关闭后，唤醒之后的子线程就会自动退出（内部有条件）
    return 0;

    //阻塞回收管理者线程
    pthread_join(pool->manageID,NULL);

    //唤醒消费者线程
    for(int i = 0;i < pool->livenum; i++){

        pthread_cond_signal(&pool->notEmpty);
    }
    //释放内存
    if(pool->mission){
        free(pool->mission);
        pool->mission = NULL;
    }
    if(pool->threadID){
        free(pool->threadID);
        pool->threadID = NULL;
    }
    free(pool);
    pool = NULL;

    //释放条件变量和互斥锁资源
    pthread_cond_destroy(&pool->notEmpty);
    pthread_cond_destroy(&pool->notFull);
    pthread_mutex_destroy(&pool->busy_mutex);
    pthread_mutex_destroy(&pool->pool_mutex);

    return 0;
}

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
    recv_datas  *recv_data = mybag;
    MYSQL_RES    *res  = NULL;
    MYSQL_ROW     row;
    bzero(sql,sizeof(sql));
    sprintf(sql,"select * from person where id = \'%d\';",recv_data->send_id);
    pthread_mutex_lock(&mutex);
    ret = mysql_query(&mysql,sql);
    res = mysql_store_result(&mysql);
    row = mysql_fetch_row(res);
    if(row == NULL){
    pthread_mutex_unlock(&mutex);
    return 0;
    }
    if(strcmp(row[2],recv_data->read_buff) == 0){                //判断密码
        strcpy(recv_data->send_name,row[1]);                     //将昵称写入缓冲区好友申请时发送给对方
        bzero(sql,sizeof(sql));
        sprintf(sql,"update person set state = \'1\' where id = \'%d\';",recv_data->send_id);
        ret = mysql_query(&mysql,sql);
        pthread_mutex_unlock(&mutex);
        return 1;
    }else{
        pthread_mutex_unlock(&mutex);
        return 0;
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
    int ret = mysql_query(&mysql,sql);
    res = mysql_store_result(&mysql);
    if((row = mysql_fetch_row(res))){
        if(strcmp(row[2],recv_data->read_buff) == 0){
            //recv_data->recvfd = atoi(row[4]);
            bzero(sql,sizeof(sql));
            sprintf(sql,"update person set passwd = \'%s\' where id = \'%d\';",recv_data->write_buff,recv_data->send_id);
            ret = mysql_query(&mysql,sql);
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
                pthread_mutex_unlock(&mutex);
                return 1;
            }
        }
    }
}

int friend_pls(recv_datas *mybag,MYSQL mysql){
    MYSQL_RES   *res = NULL;
    MYSQL_ROW   row;
    recv_datas  *recv_data = mybag;
    char         sql[MYSQL_MAX];
    if(strcmp(recv_data->read_buff,"ok") == 0){
        sprintf(sql,"insert into friends values(\'%d\',\'%d\',0);",recv_data->send_id,recv_data->recv_id);
        mysql_query(&mysql,sql);
        bzero(sql,sizeof(sql));
        sprintf(sql,"insert into friends values(\'%d\',\'%d\',0);",recv_data->recv_id,recv_data->send_id);
        mysql_query(&mysql,sql);
    }
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
    int ret = mysql_query(&mysql,sql);
    res = mysql_store_result(&mysql);
    if((row = mysql_fetch_row(res)) == NULL){  //该用户没有注册
        return 0;
    }else{
    strcpy(recv_data->recv_name,row[1]);  //找到该人的姓名
    if(atoi(row[3]) == 1){                //如果在线
        recv_data->type = RECV_INFO;          //将事件类型改为收消息
        recv_data->sendfd = atoi(row[4]);
        bzero(sql,sizeof(sql));
        sprintf(sql,"select * from friends where your_id = \'%d\' and friend_id = \'%d\';",recv_data->recv_id,recv_data->send_id);
        ret = mysql_query(&mysql,sql);
        res = mysql_store_result(&mysql);
        row = mysql_fetch_row(res);
        if(row == NULL){
        recv_data->type = SEND_INFO;
        return 0;
        }else{
            if(atoi(row[2]) == 0){           //普通好友
                if(send(recv_data->sendfd,recv_data,sizeof(recv_datas),0) < 0){
                    my_err("send",__LINE__);
                }
                bzero(sql,sizeof(sql));
                sprintf(sql,"insert into massage values(\'%d\',\'%d\',\'%s\',\'1\',\'1\');",recv_data->send_id,recv_data->recv_id,recv_data->read_buff);
                ret = mysql_query(&mysql,sql);
            }else if(atoi(row[2]) == -1){    //拉黑状态
                recv_data->type = SEND_INFO;
                return 0;
            }
        }
    }else{                   //如果对方不在线，将放到消息盒子中
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
        return 1;
    }
}

void look_list(recv_datas *mybag,MYSQL mysql){
    MYSQL_RES    *res = NULL;
    MYSQL_RES    *resl = NULL;
    MYSQL_ROW     row;
    MYSQL_ROW     rows;
    recv_datas   *recv_data = mybag;
    FRIENDS      *list;
    char          sql[MYSQL_MAX];
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
        bzero(recv_data->write_buff,sizeof(recv_data->write_buff));
        strcpy(recv_data->write_buff,"bad");
        if(send(recv_data->recvfd,recv_data,sizeof(recv_datas),0) < 0){
            my_err("send",__LINE__);
        }
        if(send(recv_data->recvfd,list,sizeof(FRIENDS),0) < 0){
            my_err("send",__LINE__);
        }
    }else{
        bzero(recv_data->write_buff,sizeof(recv_data->write_buff));
        strcpy(recv_data->write_buff,"nice");
        if(send(recv_data->recvfd,recv_data,sizeof(recv_datas),0) < 0){
            my_err("send",__LINE__);
        }
        if(send(recv_data->recvfd,list,sizeof(FRIENDS),0) < 0){
        my_err("send",__LINE__);
        }
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
    int ret = mysql_query(&mysql,sql);
    res = mysql_store_result(&mysql);
    while((row = mysql_fetch_row(res))){
        his_msg->send_id[his_msg->num] = atoi(row[0]);
        his_msg->recv_id[his_msg->num] = atoi(row[1]);
        strcpy(his_msg->message[his_msg->num++],row[2]);
    }
    if(send(recv_data->recvfd,recv_data,sizeof(recv_datas),0) < 0){
        my_err("send",__LINE__);
    }
    if(send(recv_data->recvfd,his_msg,sizeof(MSG),0) < 0){
        my_err("send",__LINE__);
    }
    return 1;
}

int dele_history(recv_datas *mybag,MYSQL mysql){
    MYSQL_RES  *res = NULL;
    MYSQL_ROW   row;
    recv_datas *recv_data = mybag;
    char        sql[MYSQL_MAX];

    bzero(sql,sizeof(sql));
    sprintf(sql,"update massage set y_look = \'0\' where your_id = \'%d\' and recv_id = \'%d\';",recv_data->send_id,recv_data->recv_id);
    int ret = mysql_query(&mysql,sql);
    bzero(sql,sizeof(sql));
    sprintf(sql,"update massage set recv_look = \'0\' where recv_id = \'%d\' and your_id = \'%d\';",recv_data->send_id,recv_data->recv_id);
    ret = mysql_query(&mysql,sql);
    bzero(sql,sizeof(sql));
    sprintf(sql,"delete from massage where y_look = \'0\' and recv_look = \'0\';");
    ret = mysql_query(&mysql,sql);
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
    if((fp = fopen("group.txt","r")) == NULL){
        printf("Error opening");
        return 0;
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
        return 0;
    }
    fwrite(&num, sizeof(int), 1, fp);
    fclose(fp);
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

int add_group(recv_datas *mybag,MYSQL mysql){
    MYSQL_RES   *res = NULL;
    MYSQL_ROW    row;
    recv_datas  *recv_data = mybag;
    char         sql[MYSQL_MAX];
    int          mem_num;

    bzero(sql,sizeof(sql));
    sprintf(sql,"select * from groups_info where group_id = \'%d\';",recv_data->recv_id);
    pthread_mutex_lock(&mutex);
    int ret = mysql_query(&mysql,sql);
    res = mysql_store_result(&mysql);
    row = mysql_fetch_row(res);
    if(row == NULL){                //没有该群
        pthread_mutex_unlock(&mutex);
        return 0;
    }
    strcpy(recv_data->recv_name,row[1]);
    mem_num = atoi(row[2]);
    bzero(sql,sizeof(sql));
    sprintf(sql,"select * from groups where group_id = \'%d\' and group_mem_id = \'%d\';",recv_data->recv_id,recv_data->send_id);
    ret = mysql_query(&mysql,sql);
    res = mysql_store_result(&mysql);
    row = mysql_fetch_row(res);
    if(row != NULL){
        pthread_mutex_unlock(&mutex);
        return -1;
    }else{
        bzero(sql,sizeof(sql));
        sprintf(sql,"insert into groups values(\'%d\',\'%s\',\'%d\',\'%s\',\'0\');",recv_data->recv_id,recv_data->recv_name,recv_data->send_id,recv_data->send_name);
        ret = mysql_query(&mysql,sql);
        pthread_mutex_unlock(&mutex);
        bzero(sql,sizeof(sql));
        //mem_num += 1;
        sprintf(sql,"update groups_info set group_mem_num = \'%d\' where group_id = \'%d\';",++mem_num,recv_data->recv_id);
        ret = mysql_query(&mysql,sql);
        pthread_mutex_unlock(&mutex);
        return 1;
    }
}

int exit_group(recv_datas *mybag,MYSQL mysql){
    MYSQL_RES   *res = NULL;
    MYSQL_ROW    row;
    recv_datas  *recv_data = mybag;
    char         sql[MYSQL_MAX];
    int           mem_num;
    bzero(sql,sizeof(sql));
    sprintf(sql,"select * from groups_info where group_id = \'%d\';",recv_data->recv_id);
    pthread_mutex_lock(&mutex);
    int ret = mysql_query(&mysql,sql);
    res = mysql_store_result(&mysql);
    row = mysql_fetch_row(res);
    if(row == NULL){              //没有该群
        pthread_mutex_unlock(&mutex);
        return 2;
    }else{
        mem_num = atoi(row[2]);
        bzero(sql,sizeof(sql));
        sprintf(sql,"select * from groups where group_id = \'%d\' and group_mem_id = \'%d\';",recv_data->recv_id,recv_data->send_id);
        ret = mysql_query(&mysql,sql);
        res = mysql_store_result(&mysql);
        row = mysql_fetch_row(res);
        if(row == NULL){               //不再该群内
            pthread_mutex_unlock(&mutex);
            return 0;
        }if(atoi(row[4]) == 2){
            bzero(sql,sizeof(sql));
            sprintf(sql,"delete from groups_info where group_id  = \'%d\';",recv_data->recv_id);
            ret = mysql_query(&mysql,sql);
            bzero(sql,sizeof(sql));
            sprintf(sql,"delete from groups where group_id  = \'%d\';",recv_data->recv_id);
            ret = mysql_query(&mysql,sql);
            pthread_mutex_unlock(&mutex);
            return -1;                       //群主退出返回不对
        }else{
            bzero(sql,sizeof(sql));
            sprintf(sql,"delete from groups where group_id = \'%d\' and group_mem_id = \'%d\';",recv_data->recv_id,recv_data->send_id);
            ret = mysql_query(&mysql,sql);
            bzero(sql,sizeof(sql));
            sprintf(sql,"update groups_info set group_mem_num = \'%d\' and group_id = \'%d\';",--mem_num,recv_data->recv_id);
            ret = mysql_query(&mysql,sql);
            pthread_mutex_unlock(&mutex);
            return 1;
        }
    }
}

int set_admin(recv_datas *mybag,MYSQL mysql){
    MYSQL_RES   *res = NULL;
    MYSQL_ROW    row;
    recv_datas  *recv_data = mybag;
    char         sql[MYSQL_MAX];
    int          idnum;

    idnum = atoi(recv_data->read_buff);
    bzero(sql,sizeof(sql));
    sprintf(sql,"select * from groups where group_id = \'%d\' and group_mem_id = \'%d\' and group_state = \'2\';",recv_data->recv_id,recv_data->send_id);
    pthread_mutex_lock(&mutex);
    int ret = mysql_query(&mysql,sql);
    res = mysql_store_result(&mysql);
    row = mysql_fetch_row(res);
    if(row == NULL){             //不是群主
        pthread_mutex_unlock(&mutex);
        return -1;
    }else{
        bzero(sql,sizeof(sql));
        sprintf(sql,"select * from groups where group_id = \'%d\' and group_mem_id = \'%d\';",recv_data->recv_id,idnum);
        ret = mysql_query(&mysql,sql);
        res = mysql_store_result(&mysql);
        row = mysql_fetch_row(res);
        if(row == NULL){            //没有该成员
            pthread_mutex_unlock(&mutex);
            return 0;
        }
        bzero(sql,sizeof(sql));
        sprintf(sql,"update groups set group_state = \'1\' where group_id = \'%d\' and group_mem_id = \'%d\';",recv_data->recv_id,idnum);
        ret = mysql_query(&mysql,sql);
        pthread_mutex_unlock(&mutex);
        return 1;                    //成功设置
    }
}

int quit_admin(recv_datas *mybag,MYSQL mysql){
    MYSQL_RES   *res = NULL;
    MYSQL_ROW    row;
    recv_datas  *recv_data = mybag;
    char         sql[MYSQL_MAX];
    int          idnum;

    idnum = atoi(recv_data->read_buff);
    bzero(sql,sizeof(sql));
    sprintf(sql,"select * from groups where group_id = \'%d\' and group_mem_id = \'%d\' and group_state = \'2\';",recv_data->recv_id,recv_data->send_id);
    pthread_mutex_lock(&mutex);
    int ret = mysql_query(&mysql,sql);
    res = mysql_store_result(&mysql);
    row = mysql_fetch_row(res);
    if(row == NULL){                     //自己若不是群主
        pthread_mutex_unlock(&mutex);
        return -1;
    }else{
        bzero(sql,sizeof(sql));
        sprintf(sql,"select * from groups where group_id = \'%d\' and group_mem_id = \'%d\';",recv_data->recv_id,idnum);
        ret = mysql_query(&mysql,sql);
        res = mysql_store_result(&mysql);
        row = mysql_fetch_row(res);
        if(row == NULL){                  //对方没加群
            pthread_mutex_unlock(&mutex);
            return 0;
        }else{
            bzero(sql,sizeof(sql));
            sprintf(sql,"update groups set group_state = \'0\' where group_id = \'%d\' and group_mem_id = \'%d\';",recv_data->recv_id,idnum);
            ret = mysql_query(&mysql,sql);
            pthread_mutex_unlock(&mutex);
            return 1;
        }
    }
}

int kick_mem(recv_datas *mybag,MYSQL mysql){
    MYSQL_RES   *res = NULL;
    MYSQL_ROW    row;
    recv_datas  *recv_data = mybag;
    char         sql[MYSQL_MAX];
    int          idnum;
    int           mem_num;
    idnum = atoi(recv_data->read_buff);
    bzero(sql,sizeof(sql));
    sprintf(sql,"select * from groups where group_id = \'%d\' and group_mem_id = \'%d\';",recv_data->recv_id,recv_data->send_id);
    pthread_mutex_lock(&mutex);
    int ret = mysql_query(&mysql,sql);
    res = mysql_store_result(&mysql);
    row = mysql_fetch_row(res);
    if(row == NULL || atoi(row[4]) == 0){                     //自己若不是群主
        pthread_mutex_unlock(&mutex);
        return -1;
    }else if(atoi(row[4]) == 2){
        bzero(sql,sizeof(sql));
        sprintf(sql,"select * from groups where group_id = \'%d\' and group_mem_id = \'%d\';",recv_data->recv_id,idnum);
        ret = mysql_query(&mysql,sql);
        res = mysql_store_result(&mysql);
        row = mysql_fetch_row(res);
    if(row == NULL){
        pthread_mutex_unlock(&mutex);
        return 0;
    }
        bzero(sql,sizeof(sql));
        sprintf(sql,"delete from groups where group_id = \'%d\' and group_mem_id = \'%d\';",recv_data->recv_id,idnum);
        ret = mysql_query(&mysql,sql);
        bzero(sql,sizeof(sql));
        sprintf(sql,"select * from groups_info where group_id = \'%d\';",recv_data->recv_id);
        ret = mysql_query(&mysql,sql);
        res = mysql_store_result(&mysql);
        row = mysql_fetch_row(res);
        mem_num = atoi(row[2]);
        bzero(sql,sizeof(sql));
        sprintf(sql,"update groups_info set group_mem_num = \'%d\' where group_id = \'%d\';",--mem_num,recv_data->recv_id);
        ret = mysql_query(&mysql,sql);
        pthread_mutex_unlock(&mutex);
        return 1;
    }else if(atoi(row[4]) == 1){
        bzero(sql,sizeof(sql));
        sprintf(sql,"select * from groups where group_id = \'%d\' and group_mem_id = \'%d\';",recv_data->recv_id,idnum);
        int ret = mysql_query(&mysql,sql);
        res = mysql_store_result(&mysql);
        row = mysql_fetch_row(res);
        if(row == NULL || (atoi(row[4]) >= 1)){
            pthread_mutex_unlock(&mutex);
            return 0;
        }
        bzero(sql,sizeof(sql));
        sprintf(sql,"delete from groups where group_id = \'%d\' and group_mem_id = \'%d\';",recv_data->recv_id,idnum);
        ret = mysql_query(&mysql,sql);
        bzero(sql,sizeof(sql));
        sprintf(sql,"select * from groups_info where group_id = \'%d\';",recv_data->recv_id);
        ret = mysql_query(&mysql,sql);
        res = mysql_store_result(&mysql);
        row = mysql_fetch_row(res);
        mem_num = atoi(row[2]);
        bzero(sql,sizeof(sql));
        sprintf(sql,"update groups_info set group_mem_num = \'%d\' where group_id = \'%d\';",--mem_num,recv_data->recv_id);
        ret = mysql_query(&mysql,sql);
        pthread_mutex_unlock(&mutex);
        return 1;
    }
}

void look_group_ls(recv_datas *mybag,MYSQL mysql){
    MYSQL_RES   *res = NULL;
    MYSQL_ROW    row;
    recv_datas  *recv_data = mybag;
    char         sql[MYSQL_MAX];
    GRP_INFO     *group_list;

    bzero(sql,sizeof(sql));
    sprintf(sql,"select * from groups where group_mem_id = \'%d\'",recv_data->send_id);
    int ret = mysql_query(&mysql,sql);
    group_list = (GRP_INFO*)malloc(sizeof(GRP_INFO));
    group_list->number = 0;
    res = mysql_store_result(&mysql);
    while(row = mysql_fetch_row(res)){
        group_list->group_id[group_list->number] = atoi(row[0]);
        group_list->group_state[group_list->number] = atoi(row[4]);
        strcpy(group_list->group_name[group_list->number++],row[1]);
    }
    if(send(recv_data->recvfd,recv_data,sizeof(recv_datas),0) < 0){
        my_err("send",__LINE__);
    }
    if(send(recv_data->recvfd,group_list,sizeof(GRP_INFO),0) < 0){
        my_err("send",__LINE__);
    }
}

int look_group_mem(recv_datas *mybag,MYSQL mysql){
    MYSQL_RES    *res = NULL;
    MYSQL_ROW     row;
    recv_datas   *recv_data = mybag;
    GRP_MEM_LIST *group_mem = NULL;
    char          sql[MYSQL_MAX];

    group_mem = (GRP_MEM_LIST*)malloc(sizeof(GRP_MEM_LIST));
    group_mem ->group_mem_num = 0;
    bzero(sql,sizeof(sql));
    sprintf(sql,"select * from groups where group_mem_id = \'%d\' and group_id = \'%d\';",recv_data->send_id,recv_data->recv_id);
    int ret = mysql_query(&mysql,sql);
    res = mysql_store_result(&mysql);
    row = mysql_fetch_row(res);
    if(row == NULL){
    bzero(recv_data->write_buff,sizeof(recv_data->write_buff));
    strcpy(recv_data->write_buff,"fail");
    if(send(recv_data->recvfd,recv_data,sizeof(recv_datas),0) < 0){
    my_err("send",__LINE__);
    }
    if(send(recv_data->recvfd,group_mem,sizeof(GRP_MEM_LIST),0) < 0){
    my_err("send",__LINE__);
    }
    return 0;
    }else{
    bzero(sql,sizeof(sql));
    sprintf(sql,"select * from groups where group_id = \'%d\';",recv_data->recv_id);
    ret = mysql_query(&mysql,sql);
    res = mysql_store_result(&mysql);
    while(row = mysql_fetch_row(res)){
        group_mem->group_mem_id[group_mem->group_mem_num] = atoi(row[2]);
        group_mem->group_mem_state[group_mem->group_mem_num] = atoi(row[4]);
        strcpy(group_mem->group_mem_nickname[group_mem->group_mem_num++],row[3]);
    }
    bzero(recv_data->write_buff,sizeof(recv_data->write_buff));
    strcpy(recv_data->write_buff,"success");
    if(send(recv_data->recvfd,recv_data,sizeof(recv_datas),0) < 0){
    my_err("send",__LINE__);
    }
    if(send(recv_data->recvfd,group_mem,sizeof(GRP_MEM_LIST),0) < 0){
    my_err("send",__LINE__);
    }
    return 0;
    }
}

int send_group_msg(recv_datas *mybag,MYSQL mysql){
    MYSQL_RES   *res = NULL;
    MYSQL_RES   *result = NULL;
    MYSQL_ROW    row,rowl;
    recv_datas  *recv_data = mybag;
    BOX_MSG     *box = head;
    char         sql[MYSQL_MAX];

    pthread_mutex_lock(&mutex);
    recv_data->type = RECV_GROUP_MSG;
    bzero(sql,sizeof(sql));
    sprintf(sql,"select * from groups_info where group_id = \'%d\';",recv_data->recv_id);
    int ret = mysql_query(&mysql,sql);
    res = mysql_store_result(&mysql);
    row = mysql_fetch_row(res);
    if(row == NULL){
        recv_data->type = SEND_GROUP_MSG;
        pthread_mutex_unlock(&mutex);
        return -1;
    }else{
        bzero(sql,sizeof(sql));
        sprintf(sql,"select * from groups where group_id = \'%d\' and group_mem_id = \'%d\';",recv_data->recv_id,recv_data->send_id);
        ret = mysql_query(&mysql,sql);
        res = mysql_store_result(&mysql);
        row = mysql_fetch_row(res);
        if(row == NULL){
            recv_data->type = SEND_GROUP_MSG;
            pthread_mutex_unlock(&mutex);
            return -1;
        }

        bzero(sql,sizeof(sql));
        sprintf(sql,"insert into grps_message values(\'%d\',\'%d\',\'%s\',\'%s\',\'1\');",recv_data->recv_id,recv_data->send_id,recv_data->send_name,recv_data->read_buff);
        ret = mysql_query(&mysql,sql);

        bzero(sql,sizeof(sql));
        sprintf(sql,"select * from groups where group_id = \'%d\';",recv_data->recv_id);
        ret = mysql_query(&mysql,sql);
        res = mysql_store_result(&mysql);
        while((row = mysql_fetch_row(res)) != NULL){
            if(recv_data->send_id != atoi(row[2])){
                bzero(sql,sizeof(sql));
                sprintf(sql,"select * from person where id = \'%d\';",atoi(row[2]));
                ret = mysql_query(&mysql,sql);
                result = mysql_store_result(&mysql);
                rowl = mysql_fetch_row(result);
                if(atoi(rowl[3]) == 1){
                    if(send(atoi(rowl[4]), recv_data, sizeof(recv_datas), 0) < 0){
                        my_err("send",__LINE__);
                    }
                }else{
                    while(box != NULL){
                        if(box->recv_id == atoi(rowl[0])){
                            break;
                        }
                        box = box->next;
                    }
                    if(box == NULL){
                        box = (list_box)malloc(sizeof(BOX_MSG));
                        box->group_msg_num = 0;
                        box->recv_id = atoi(rowl[0]);    //存入收消息的id
                        strcpy(box->group_message[box->group_msg_num],recv_data->read_buff); //发的消息
                        box->group_send_id[box->group_msg_num] = recv_data->send_id;         //发消息的id
                        strcpy(box->group_mem_nikename[box->group_msg_num],recv_data->send_name);
                        box->group_id[box->group_msg_num++] = recv_data->recv_id;            //发消息的群id
                        if(head == NULL){
                        head = box;
                        tail = box;
                        tail->next = NULL;
                    }else{
                        tail->next = box;
                        box ->next = NULL;
                        tail = box;
                    }
                    }else{
                        strcpy(box->group_message[box->group_msg_num],recv_data->read_buff); //发的消息
                        box->group_send_id[box->group_msg_num] = recv_data->send_id;         //发消息的id
                        strcpy(box->group_mem_nikename[box->group_msg_num],recv_data->send_name);
                        box->group_id[box->group_msg_num++] = recv_data->recv_id;
                    }
                }
            }
        }
    }
recv_data->type = SEND_GROUP_MSG;
pthread_mutex_unlock(&mutex);
return 0;
}


int look_grp_history(recv_datas *mybag,MYSQL mysql){
    MYSQL_RES  *res = NULL;
    MYSQL_RES  *resl = NULL;
    MYSQL_ROW   row,row2;
    recv_datas *recv_data = mybag;
    GRP_MSG     *group_msg = NULL;
    char         sql[MYSQL_MAX];
    group_msg = (GRP_MSG*)malloc(sizeof(GRP_MSG));
    group_msg->group_msg_num = 0;
    bzero(sql,sizeof(sql));
    sprintf(sql,"select * from groups where group_id = \'%d\'and group_mem_id = \'%d\';",recv_data->recv_id,recv_data->send_id);
    int ret = mysql_query(&mysql,sql);
    resl = mysql_store_result(&mysql);
    row2 = mysql_fetch_row(resl);
    if(row2 == NULL){
        recv_data->type = LOOK_GRP_HISTORY;
        bzero(recv_data->write_buff,sizeof(recv_data->write_buff));
        strcpy(recv_data->write_buff,"not enter");
        if(send(recv_data->recvfd,recv_data,sizeof(recv_datas),0) < 0){
            my_err("send",__LINE__);
        }
        if(send(recv_data->recvfd,group_msg,sizeof(GRP_MSG),0) < 0){
            my_err("send",__LINE__);
        }
        return 0;               //不在群中
    }
    else{
        bzero(sql,sizeof(sql));
        sprintf(sql,"select * from grps_message where grp_id = \'%d\' and grp_mem_id = \'%d\' and look = \'1\';",recv_data->recv_id,recv_data->send_id);
        ret = mysql_query(&mysql,sql);
        res = mysql_store_result(&mysql);
        row = mysql_fetch_row(res);
        if(!row){
            if(send(recv_data->recvfd,recv_data,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
            if(send(recv_data->recvfd,group_msg,sizeof(GRP_MSG),0) < 0){
                my_err("send",__LINE__);
            }
            return -1;     //没有消息记录
        }else{
            strcpy(group_msg->group_nikename,row2[1]);
            group_msg->group_id = atoi(row2[0]);
            bzero(sql,sizeof(sql));
            sprintf(sql,"select * from grps_message where grp_id = \'%d\';",recv_data->recv_id);
            ret = mysql_query(&mysql,sql);
            res = mysql_store_result(&mysql);
            while((row = mysql_fetch_row(res)) != NULL){
                group_msg->group_mem_id[group_msg->group_msg_num] = atoi(row[1]);
                strcpy(group_msg->group_mem_nikename[group_msg->group_msg_num],row[2]);
                strcpy(group_msg->message[group_msg->group_msg_num++],row[3]);
            }
        }
            if(send(recv_data->recvfd,recv_data,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
            if(send(recv_data->recvfd,group_msg,sizeof(GRP_MSG),0) < 0){
                my_err("send",__LINE__);
            }
    }
    return 1;
}

int dele_grp_history(recv_datas *mybag,MYSQL mysql){
    MYSQL_RES   *res = NULL;
    MYSQL_ROW    row;
    recv_datas  *recv_data = mybag;
    char         sql[MYSQL_MAX];

    bzero(sql,sizeof(sql));
    sprintf(sql,"select * from groups where group_id = \'%d\' and group_mem_id = \'%d\';",recv_data->recv_id,recv_data->send_id);
    int ret = mysql_query(&mysql,sql);
    res = mysql_store_result(&mysql);
    row = mysql_fetch_row(res);
    if(row == NULL){
        bzero(recv_data->write_buff,sizeof(recv_data->write_buff));
        strcpy(recv_data->write_buff,"not enter");
        if(send(recv_data->recvfd,recv_data,sizeof(recv_datas),0) < 0){
            my_err("send",__LINE__);
        }
        return 0;
    }else{
        bzero(sql,sizeof(sql));
        sprintf(sql,"update grps_message set look = \'0\' where grp_id = \'%d\' and grp_mem_id = \'%d\';",recv_data->recv_id,recv_data->send_id);
        ret = mysql_query(&mysql,sql);
        bzero(sql,sizeof(sql));
        sprintf(sql,"select * from grps_message where look = \'1\' and grp_id = \'%d\';",recv_data->recv_id);
        ret = mysql_query(&mysql,sql);
        res = mysql_store_result(&mysql);
        row = mysql_fetch_row(res);
        if(row == NULL){
            bzero(sql,sizeof(sql));
            sprintf(sql,"delete from grps_message where look = \'0\' and grp_id = \'%d\';",recv_data->recv_id);
            ret = mysql_query(&mysql,sql);
            bzero(recv_data->write_buff,sizeof(recv_data->write_buff));
            strcpy(recv_data->write_buff,"dele success");
            if(send(recv_data->recvfd,recv_data,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
        }else{
            bzero(recv_data->write_buff,sizeof(recv_data->write_buff));
            strcpy(recv_data->write_buff,"have done");
            if(send(recv_data->recvfd,recv_data,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
            return 0;
        }
    }
}


int finsh(recv_datas *mybag,MYSQL mysql){
    MYSQL_RES   *res = NULL;
    MYSQL_ROW    row;
    recv_datas  *recv_data = mybag;
    BOX_MSG      *box = head;
    char         sql[MYSQL_MAX];
    bzero(sql,sizeof(sql));
    sprintf(sql,"select * from person where id = \'%d\';",recv_data->recv_id);
    int ret = mysql_query(&mysql,sql);
    res = mysql_store_result(&mysql);
    row = mysql_fetch_row(res);
    if(row == NULL){
        return -1;
    }else{
        if(atoi(row[3]) == 1){
            recv_data->sendfd = atoi(row[4]);
            recv_data->type = RECV_FILE;
            if(send(recv_data->sendfd,recv_data,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
            return 1;
        }else{                           //如果对方不在线放入消息盒子
            while(box != NULL){
                if(box->recv_id == atoi(row[0])){
                    break;
                }
                box = box->next;
            }
            if(box == NULL){
                box = (list_box)malloc(sizeof(BOX_MSG));
                box->file_num = 0;
                box->recv_id = atoi(row[0]);    //存入收消息的id
                strcpy(box->file_pathname[box->file_num],recv_data->write_buff);      //发的文件路径(名称)
                box->file_send_id[box->file_num] = recv_data->send_id;                //发消息的id
                strcpy(box->file_send_nickname[box->file_num],recv_data->send_name);  //发送文件的人的昵称
                box->file_size[box->file_num++] = recv_data->filesize;                //发送的文件的大小
                if(head == NULL){
                head = box;
                tail = box;
                tail->next = NULL;
            }else{
                tail->next = box;
                box ->next = NULL;
                tail = box;
            }
            }else{
                strcpy(box->file_pathname[box->file_num],recv_data->write_buff);      //发的文件路径(名称)
                box->file_send_id[box->file_num] = recv_data->send_id;                //发消息的id
                strcpy(box->file_send_nickname[box->file_num],recv_data->send_name);  //发送文件的人的昵称
                box->file_size[box->file_num++] = recv_data->filesize;
            }
            return 0;
        }
    }
}

int send_file(recv_datas *mybag,MYSQL mysql){
    int fp;
    recv_datas *recv_data = mybag;
    MYSQL_RES   *res = NULL;
    MYSQL_ROW    row;
    BOX_MSG      *box = head;
    char         sql[MYSQL_MAX];
    bzero(sql,sizeof(sql));
    sprintf(sql,"select * from person where id = \'%d\';",recv_data->recv_id);
    int ret = mysql_query(&mysql,sql);
    res = mysql_store_result(&mysql);
    row = mysql_fetch_row(res);
    if(row == NULL){
        return -1;
    }
    if((fp = open("file",O_WRONLY|O_CREAT/* 0_APPEND */,0775)) < 0){  //0_APPEND追加不能用同一个文件名，即使删除了也会先将上一次保存的大小为空再追加新的东西
        printf("%s\n",strerror(errno));
    }if(recv_data->flag == 0){
           lseek(fp,(sizeof(recv_data->read_buff)-1)*recv_data->cont,SEEK_SET);
        write(fp,recv_data->read_buff,recv_data->mun);
        close(fp);
        if(send(recv_data->recvfd,recv_data,sizeof(recv_datas),0) < 0){
            my_err("send",__LINE__);
        }
    }else{
           lseek(fp,(sizeof(recv_data->read_buff)-1)*recv_data->cont,SEEK_SET);
        write(fp,recv_data->read_buff,sizeof(recv_data->read_buff)-1);
        close(fp);
        if(send(recv_data->recvfd,recv_data,sizeof(recv_datas),0) < 0){
            my_err("send",__LINE__);
        }
    }
    return 0;
}

int read_file(recv_datas *mybag,MYSQL mysql){
    int fp;
    recv_datas  *recv_data = mybag;
    ssize_t l;
    if((fp = open("file",O_RDONLY)) < 0){
        printf("%s\n",strerror(errno));
    }
    recv_data->flag = 1;
    recv_data->mun = -1;
    bzero(recv_data->read_buff,sizeof(recv_data->read_buff));
    lseek(fp,(sizeof(recv_data->read_buff)-1)*recv_data->cont,SEEK_SET);
    if((l = read(fp,recv_data->read_buff,(sizeof(recv_data->read_buff)-1))) < (sizeof(recv_data->read_buff)-1)){
        recv_data->flag = 0;
        recv_data->mun = l;
        strcpy(recv_data->write_buff,"finish");
    }
    close(fp);
    recv_data->type = READ_FILE;
    if(send(recv_data->recvfd,recv_data,sizeof(recv_datas),0) < 0){
        my_err("send",__LINE__);
    }
}

void *ser_deal(void *arg){
    printf("进入函数选择任务类型\n");
    int i;
    int fp;
    int ret;
    MYSQL mysql;
    list_box  box = head;
    recv_datas *recv_buf = (recv_datas*)arg;
    mysql = init_mysql();
    int choice = recv_buf->type;
    switch(choice){
        case USER_LOGIN://登录后看消息盒子，包
        if(!(user_login(recv_buf,mysql))){
            recv_buf->type = ID_ERROR;
            bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
            strcpy(recv_buf->write_buff,"id error");
            if (send(recv_buf->recvfd,recv_buf,sizeof(recv_datas), 0) < 0) {
            my_err("send", __LINE__);
            }
        }else{
            bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
            strcpy(recv_buf->write_buff,"login success");
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
                box->fri_pls_num = 0;
                box->group_msg_num = 0;
                box->file_num = 0;
                box->next = NULL;
            if(head == NULL){
                head = box;
                tail = box;
                tail->next = NULL;
            }else{
                tail->next = box;
                tail= box;
                tail->next = NULL;
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
        pthread_mutex_lock(&mutex);
        friend_pls(recv_buf,mysql);
        pthread_mutex_unlock(&mutex);
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
        pthread_mutex_lock(&mutex);
        if(send_info(recv_buf,mysql)){
            bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
            strcpy(recv_buf->write_buff,"send ok");
            if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
        }else{
            bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
            strcpy(recv_buf->write_buff,"send fail");
            if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
        }
        pthread_mutex_unlock(&mutex);
        break;

        case LOOK_FRI_LS:
        pthread_mutex_lock(&mutex);
        look_list(recv_buf,mysql);
        pthread_mutex_unlock(&mutex);
        break;

        case LOOK_HISTORY:
        pthread_mutex_lock(&mutex);
        look_history(recv_buf,mysql);
        pthread_mutex_unlock(&mutex);
        break;

        case DELE_HISTORY:
        pthread_mutex_lock(&mutex);
        if(dele_history(recv_buf,mysql) == 0){
            bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
            strcpy(recv_buf->write_buff,"delete success");
            if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
        }else{
            bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
            strcpy(recv_buf->write_buff,"delete fail");
            if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
        }
        pthread_mutex_unlock(&mutex);
        break;

        case CREATE_GROUP:  //建群
        if(create_group(recv_buf,mysql)){
            bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
            strcpy(recv_buf->write_buff,"create success");
            if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
        }else{
            bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
            strcpy(recv_buf->write_buff,"create fail");
            if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
        }
        break;


        case DISSOLVE_GROUP: //解散群
        ret = dissolve_group(recv_buf,mysql);
        if(ret == 1){
            bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
            strcpy(recv_buf->write_buff,"dissolve success");
            if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
        }else if(ret == 0){
            bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
            strcpy(recv_buf->write_buff,"no group");
            if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
        }else if(ret == -2){
            bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
            strcpy(recv_buf->write_buff,"no num");
            if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
        }else{
            bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
            strcpy(recv_buf->write_buff,"no host");
            if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
            }
        break;


        case ADD_GROUP:
        ret = add_group(recv_buf,mysql);
        if(ret == 1){
            bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
            strcpy(recv_buf->write_buff,"add success");
            if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
        }else if(ret == -1){
            bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
            strcpy(recv_buf->write_buff,"have done");
            if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
        }else{
            bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
            strcpy(recv_buf->write_buff,"error");
            if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
        }
        break;

        case EXIT_GROUP:
        ret = exit_group(recv_buf,mysql);
        if(ret == 1){
            bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
            strcpy(recv_buf->write_buff,"exit success");
            if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
        }else if(ret == -1){
            bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
            strcpy(recv_buf->write_buff,"host exit");
            if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
        }else if(ret == 2){
            bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
            strcpy(recv_buf->write_buff,"no group");
            if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
        }else{
            bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
            strcpy(recv_buf->write_buff,"no add");
            if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
        }
        break;

        case SET_ADMIN:
        ret = set_admin(recv_buf,mysql);
        if(ret == 1){
            bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
            strcpy(recv_buf->write_buff,"success");
            if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
        }else if(ret == -1){
            bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
            strcpy(recv_buf->write_buff,"not host");
            if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
        }else if(ret == 0){
            bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
            strcpy(recv_buf->write_buff,"not mem");
            if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
        }
        break;

        case QUIT_ADMIN:
        ret = quit_admin(recv_buf,mysql);
        if(ret == 1){
            bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
            strcpy(recv_buf->write_buff,"success");
            if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
        }else if(ret == 0){
            bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
            strcpy(recv_buf->write_buff,"not mem");
            if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
        }else if(ret == -1){
            bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
            strcpy(recv_buf->write_buff,"not host");
            if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
        }
        break;

        case KICK_MEM:
        ret = kick_mem(recv_buf,mysql);
        if(ret == 1){
            bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
            strcpy(recv_buf->write_buff,"success");
            if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
        }else if(ret == 0){
            bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
            strcpy(recv_buf->write_buff,"not");
            if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
        }else if(ret == -1){
            bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
            strcpy(recv_buf->write_buff,"not power");
            if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
        }
        break;

        case LOOK_GROUP_LS:
        pthread_mutex_lock(&mutex);
        look_group_ls(recv_buf,mysql);
        pthread_mutex_unlock(&mutex);
        break;

        case LOOK_GROUP_MEM:
        pthread_mutex_lock(&mutex);
        look_group_mem(recv_buf,mysql);
        pthread_mutex_unlock(&mutex);
        break;

        case SEND_GROUP_MSG:
        if(send_group_msg(recv_buf,mysql) == -1){
            bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
            strcpy(recv_buf->write_buff,"no group");
            if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
        }else{
            bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
            strcpy(recv_buf->write_buff,"success");
            if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
        }
        break;

        case SEND_FILE:
        pthread_mutex_lock(&mutex);
        ret = send_file(recv_buf,mysql);
        if(ret == -1){
            recv_buf->type = SEND_FILE;
            bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
            strcpy(recv_buf->write_buff,"no people");
            if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
        }
        pthread_mutex_unlock(&mutex);
        break;


        case FINSH:
        pthread_mutex_lock(&mutex);
        ret = finsh(recv_buf,mysql);
        if(ret < 0){
            recv_buf->type = SEND_FILE;
            bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
            strcpy(recv_buf->write_buff,"no people");
            if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
        }else{
            recv_buf->type = SEND_FILE;
            bzero(recv_buf->write_buff,sizeof(recv_buf->write_buff));
            strcpy(recv_buf->write_buff,"nice");
            if(send(recv_buf->recvfd,recv_buf,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
        }
        pthread_mutex_unlock(&mutex);
        break;

        case READ_FILE:
        pthread_mutex_lock(&mutex);
        read_file(recv_buf,mysql);
        pthread_mutex_unlock(&mutex);
        break;

        case LOOK_GRP_HISTORY:
        pthread_mutex_lock(&mutex);
        look_grp_history(recv_buf,mysql);
        pthread_mutex_unlock(&mutex);
        break;

        case DELE_GRP_HISTORY:
        pthread_mutex_lock(&mutex);
        dele_grp_history(recv_buf,mysql);
        pthread_mutex_unlock(&mutex);

    }
close_mysql(mysql);
}

int main(void){
    int   connfd,sockfd;
    int   optval;
    int   flag_recv = 0;
    int   names;
    int   connects = 0;
    char  sql[MYSQL_MAX];
    char   ip[32];
    MYSQL *myconn = NULL;
    MYSQL_RES *res = NULL;
    MYSQL_ROW  row;
    pthread_t tid;
    recv_datas recv_buf;
    size_t ret;
    socklen_t clen;
    struct sockaddr_in cliaddr,servaddr;
    clen = sizeof(struct sockaddr_in);
    recv_datas *bf;
    bf = (recv_datas*)malloc(sizeof(recv_datas));

    myconn  = mysql_init(NULL);
    pthread_mutex_init(&mutex, NULL);
   threadpool *pool = threadPoolcreate(20,3,100);

    if(!mysql_real_connect(myconn,"127.0.0.1","root","123456","chat",0,NULL,0)){
    fprintf(stderr, "Failed to connect to database: Error: %s\n",
    mysql_error(myconn));
    }
    if(mysql_set_character_set(myconn, "utf8") < 0){
		my_err("mysql_set_character_set", __LINE__);
	}

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

    epfd = epoll_create(MAXEVENTS);
    ev.data.fd = sockfd;
    ev.events = EPOLLIN  | EPOLLET | EPOLLRDHUP | EPOLLERR;

    epoll_ctl(epfd,EPOLL_CTL_ADD,sockfd,&ev);
    connects++;

for(;;){
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
            inet_ntop(AF_INET,&cliaddr.sin_addr.s_addr,ip,sizeof(ip)),ntohs(cliaddr.sin_port));  //将网络地址格式反向转化
            if(connfd <= 0){
                my_err("accpet",__LINE__);
                continue;
            }
            ev.data.fd = connfd;
            ev.events = EPOLLIN  | EPOLLET | EPOLLRDHUP | EPOLLERR;
            epoll_ctl(epfd,EPOLL_CTL_ADD,connfd,&ev); //新增套接字
        }
        /* 用户非正常挂断 */
        else if((events[i].events & EPOLLIN) && (events[i].events & EPOLLRDHUP) || (events[i].events & EPOLLIN) && (events[i].events & EPOLLERR)){
            printf("客户端ip[%d]非正常断开连接...\n",events[i].data.fd);
            bzero(sql,sizeof(sql));
            sprintf(sql,"update person set state = \'0\' where state = \'1\' and fd = \'%d\';",events[i].data.fd);
            mysql_query(myconn,sql);
            epoll_ctl(epfd,EPOLL_CTL_DEL,events[i].data.fd,0); //删去套接字
            close(events[i].data.fd);
            connects--;
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
            bzero(sql,sizeof(sql));
            sprintf(sql,"update person set state = \'0\' where state = \'1\' and fd = \'%d\';",events[i].data.fd);
            mysql_query(myconn,sql);
            epoll_ctl(epfd,EPOLL_CTL_DEL,events[i].data.fd,0);
            close(events[i].data.fd);
            connects--;
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
        memcpy(bf,&recv_buf,sizeof(recv_datas));
        threadpooladd(pool,ser_deal,(void*)bf);      //引入线程池
        //pthread_create(&tid,NULL,(void*)ser_deal,(void*)bf);
        //pthread_detach(tid);
        }
    }
}
}

