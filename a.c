#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>

#define MSGSIZE    1024
#define MAXSIZE    1032
typedef struct message{
int    type;            //消息类型
int    msglen;          //消息长度
char   data[MSGSIZE];   //消息数据
}msg;
int main(void){
   msg *r =(msg*)malloc(MAXSIZE);
   scanf("%s",r->data);
    // int i;
    // int sum = 0;
    // for(i = 0;i < 10; i++){

    //     sum += i;
    // }
    // printf("%d\n",sum);
   r->data[strlen(r->data)] = '\0';
    printf("%d\n",strlen(r->data)+1);

    return 0;
}