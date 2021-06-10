#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/msg.h>
#include<sys/sem.h>
#include<sys/ipc.h>
#include<linux/sem.h>
#include<unistd.h>
#include<errno.h>

int createsem(const char*pathname,int proj_id,int members,int init_val){
    key_t    msgkey;
    int      index,sid;
    union    semun   semopts;


    /* 获取键值 */
    if(msgkey = ftok(pathname,proj_id) == -1){
        perror("ftok error!\n");
        return -1;
    } 
    if((sid = semget(msgkey,members,IPC_CREAT | 0666)) == -1){
        perror("semget call failed\n");
        return -1;
    }


    /* 初始化操作 */
    semopts.val = init_val;
    for(index = 0;index < members;index ++){
        sesmctl(sid,index,SETVAL,semopts);
    }    
    return(sid);
}


/* P操作函数 */
int sem_p(int semid,int index){
    struct sembuf buf = {0,1,IPC_NOWAIT};
    if(index < 0){
        perror("index of array cannot equals a minus value!");
        return -1;
    }
    buf.sem_num = index;
    if(semop(semid,&buf,-1) == -1){
        perror ("a wrong operation to semaphore occurred!");
        return -1;
    }
    return 0;
}
/* V操作函数 */
int sem_v (int semid,int index){
    struct sembuf buf = {0,1,IPC_NOWAIT};
    if(index < 0){
        perror("index of array cannot equals a minus value!");
        return -1;
    } 
    buf.sem_num  = index;
    if(semop(semid,&buf,1) == -1){
        perror("a wrong operation to semaphone occured!");
        return -1;
    }
    return 0;
}
/*  获取单个信号函数*/
int semval_op(int semid,int index,int cmd){
    if(index < 0){
        printf("index cannot be minus!\n");
        return -1;
    }
    if(cmd == GETVAL || cmd == SETVAL){
        return semctl(semid,index,cmd,0);
    }
    printf("function cannot surport cmd:%d\n",cmd);
    return -1;
}