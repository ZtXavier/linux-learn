#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/ipc.h>
#include<sys/msg.h>

#define BUF_SIZE   256
#define PROJ_ID    32
#define PTH_NAME   "."

int main(void){
    /* 用户自定义的消息缓冲 */
    struct mymsgbuf{
        long msgtype;
        char ctrlstring[BUF_SIZE];
    }msgbuffer;

    int qid;    /* 消息队列标识符 */
    int msglen;
    int msgkey;

    /* 获取键值 */
    if((msgkey = ftok(PTH_NAME,PROJ_ID)) == -1){
        perror("ftok error!\n");
        exit(1);
    }
    /* 创建消息队列 */
    if((qid = msgget(msgkey,IPC_CREAT|0660)) == -1){
        perror("msgget error!\n");
        exit(1);
    }
    
    msglen = sizeof(struct mymsgbuf) - 4;
    if(msgrcv(qid,&msgbuffer,msglen,msgbuffer.msgtype,0) == -1){ //读取数据
        perror("msgrcv error!");
        exit(1);
    }
    printf("get message %s\n",msgbuffer.ctrlstring);
    exit(0);
}