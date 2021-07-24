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
    char ch;
    char name[10];
    int i = 0;
    while(1){
    scanf("%c",&ch);
    printf("\b");
    if(ch == '\n'){
        printf("js");
        break;
    }
    name[i++] = ch;
    }
    return 0;
}