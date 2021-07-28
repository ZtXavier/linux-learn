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
#include"mysql.h"


recv_datas  *send_data;
recv_datas  *recv_data;
BOX_MSG     *box;
MSG         *msg;
FRIENDS     *flist;

void my_err(const char *err_string, int line)
{
	fprintf(stderr, "line:%d ", line);
	perror(err_string);
	exit(1);
}


void first_menu(void){
        printf("\t***************************************\n");
        printf("\t************欢迎来到登录界面***********\n");
        printf("\t***************************************\n");
        printf("\t*****        1.登录               *****\n");
        printf("\t*****                             *****\n");
        printf("\t*****        2.注册               *****\n");
        printf("\t*****                             *****\n");
        printf("\t*****        3.找回密码           *****\n");
        printf("\t*****                             *****\n");
        printf("\t*****        4.退出               *****\n");
        printf("\t***************************************\n");
        printf("\t***************************************\n");
}

void second_menu(void){
        printf("\t***************************************\n");
        printf("\t********welcome to chatroom************\n");
        printf("\t***************************************\n");
        printf("\t*****        1.私聊               *****\n");
        printf("\t*****                             *****\n");
        printf("\t*****        2.加好友             *****\n");
        printf("\t*****                             *****\n");
        printf("\t*****        3.删好友             *****\n");
        printf("\t*****                             *****\n");
        printf("\t*****        3.查看好友列表       *****\n");
        printf("\t*****                             *****\n");
        printf("\t*****        8.修改密码           *****\n");
        printf("\t*****                             *****\n");
        printf("\t*****        9.退出               *****\n");
        printf("\t*****                             *****\n");
        printf("\t***************************************\n");

}

//来用写发数据
void *read_mission(void*arg){
    int   connfd = *(int*)arg;
    int   choice;
    int   i;
    int   ret;
    char  ch;
    char  password1[24],password2[24];
    char buf[24];
    int sure;
    send_data = (recv_datas*)malloc(sizeof(recv_datas));

    while(1){
        printf("\t***************************************\n");
        printf("\t************欢迎来到登录界面***********\n");
        printf("\t***************************************\n");
        printf("\t*****        1.登录               *****\n");
        printf("\t*****                             *****\n");
        printf("\t*****        2.注册               *****\n");
        printf("\t*****                             *****\n");
        printf("\t*****        3.找回密码           *****\n");
        printf("\t*****                             *****\n");
        printf("\t*****        4.退出               *****\n");
        printf("\t***************************************\n");
        printf("\t***************************************\n");
        printf("请选择: ");
        scanf("%d",&choice);
        getchar();
        switch(choice){
            case 1:        //登陆
            send_data->type = USER_LOGIN;
            printf("please input id: ");
            scanf("%d",&send_data->send_id);
            getchar();
            printf("please input password: ");
            i = 0;
            while(1){
            scanf("%c",&ch);
            //printf("\b");
            if(ch == '\n'){
            send_data->read_buff[i] = '\0';
            break;
            }
            //printf("*");
            send_data->read_buff[i++] = ch;
            }
            printf("\n");
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
            my_err("send",__LINE__);
            }
            pthread_mutex_lock(&cl_mu);
            pthread_cond_wait(&cl_co,&cl_mu);
            pthread_mutex_unlock(&cl_mu);
            printf("\t--%s--\n",send_data->write_buff);
            break;


            case 2:
                i = 0;            //注册
                send_data->type = USER_SIGN;
                printf("please input you name: ");
                scanf("%s",send_data->send_name);
                getchar();
                printf("please input passwd: ");
                while(1){
                scanf("%c", &ch);
                if(ch == '\n'){
                password1[i] = '\0';
                break;
                }
                password1[i++] = ch;
                //printf("*");
                }
                printf("\n");
                printf("please input passwd again: ");
                i = 0;
                while(1){
                scanf("%c", &ch);
                if(ch == '\n'){
                password2[i] = '\0';
                break;
                }
                password2[i++] = ch;
                    //printf("*");
                }
                printf("\n");
                if(strcmp(password1,password2) == 0){
                    printf("输入一致!\n");
                    bzero(send_data->read_buff,sizeof(send_data->read_buff));
                    strcpy(send_data->read_buff,password1);
                    if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
                        my_err("send",__LINE__);
                    }
                    pthread_mutex_lock(&cl_mu);
                    pthread_cond_wait(&cl_co,&cl_mu);
                    pthread_mutex_unlock(&cl_mu);
                }else{
                    printf("两次输入不一致\n");
                    printf("按回车继续\n");
                    choice = 10;
                    getchar();
                }
        break;

        case 3:           //找回
        send_data->type = USER_FIND;
        printf("please input your id:");
        scanf("%d",&send_data->send_id);
        getchar();
        printf("please input your nickname:");
        scanf("%s",send_data->send_name);
        getchar();
        if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
            my_err("send",__LINE__);
        }
        pthread_mutex_lock(&cl_mu);
        pthread_cond_wait(&cl_co,&cl_mu);
        pthread_mutex_unlock(&cl_mu);
        printf("%s\n",send_data->write_buff);
        printf("按任意键返回...");
        getchar();
        break;
        
        case 4:
        send_data->type = USER_OUT;   //登出
        if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
        my_err("send",__LINE__);
        }
        pthread_exit(0);
        break;

        default:
        printf("非法输入！\n");
        printf("按任意键继续\n");
        getchar();
        break;

        }if((choice > 4) || (choice < 1))    continue;
        else if (choice == 1) {
            if (strcmp(send_data->write_buff, "id error") == 0) {
                printf("ID or PASSWD error!!!\n");
                printf("按任意键继续.....\n");
                getchar();
                continue;
            } else {
                printf("登陆成功!!\n");
                printf("按下回车继续......\n");
                getchar();
                break;
            }
        }
        else if (choice == 2){
            printf("%s\n",send_data->write_buff);
            printf("您的账号为:%d\n", send_data->send_id);
            printf("按下回车继续.......");
            getchar();
            continue;
        }else if(choice == 3)   continue;
    }
    while(1){
    printf("\t***************************************\n");
    printf("\t********welcome to chatroom************\n");
    printf("\t***************************************\n");
    printf("\t*****        1.私聊               *****\n");
    printf("\t*****                             *****\n");
    printf("\t*****        2.加好友             *****\n");
    printf("\t*****                             *****\n");
    printf("\t*****        3.好友请求           *****\n");
    printf("\t*****                             *****\n");
    printf("\t*****        4.删除好友           *****\n");
    printf("\t*****                             *****\n");
    printf("\t*****        5.查看好友列表       *****\n");
    printf("\t*****                             *****\n");
    printf("\t*****        6.拉黑好友           *****\n");
    printf("\t*****                             *****\n");
    printf("\t*****        7.取消拉黑           *****\n");
    printf("\t*****                             *****\n");
    printf("\t*****        8.查看好友消息       *****\n");
    printf("\t*****                             *****\n");
    printf("\t*****        9.查看聊天记录       *****\n");
    printf("\t*****                             *****\n");
    printf("\t*****        10.删除聊天记录      *****\n");
    printf("\t*****                             *****\n");
    printf("\t*****        11.创建群            *****\n");
    printf("\t*****                             *****\n");
    printf("\t*****        12.解散群(只限群主)  *****\n");
    printf("\t*****                             *****\n");
    printf("\t*****        13.加群              *****\n");
    printf("\t*****                             *****\n");
    printf("\t*****        14.退群              *****\n");
    printf("\t*****                             *****\n");
    printf("\t*****        15.设置管理员        *****\n");
    printf("\t*****                             *****\n");
    printf("\t*****        16.取消管理员        *****\n");
    printf("\t*****                             *****\n");
    printf("\t*****        17.踢人              *****\n");
    printf("\t*****                             *****\n");
    printf("\t*****                             *****\n");
    printf("\t*****                             *****\n");
    printf("\t*****                             *****\n");
    printf("\t*****                             *****\n");
    printf("\t*****                             *****\n");
    printf("\t*****        19.修改密码          *****\n");
    printf("\t*****                             *****\n");
    printf("\t*****        20.退出              *****\n");
    printf("\t*****                             *****\n");
    printf("\t***************************************\n");
    printf("请选择: ");
    scanf("%d",&choice);
    getchar();
    switch(choice){
    case 1:
    send_data->type = SEND_INFO;
    // printf("please input your friend id: ");
    while(1){
    printf("please input your friend id: ");
    scanf("%d",&send_data->recv_id);
    getchar();
    if(send_data->recv_id != send_data->send_id){
    break;
    }
    printf("正常点，不要自言自语!!!\n");
    printf("按任意键继续");
    getchar();
    }
    printf("\t--可以与->%d<-好友聊天--\n",send_data->recv_id);
    while(1){
        scanf("%s",send_data->read_buff);
        getchar();
        if(strcmp(send_data->read_buff,"#over#") == 0){
            printf("\t--与->%d<-好友聊天结束--\n",send_data->recv_id);
            break;
        }
        if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
        my_err("send",__LINE__);
        }
        pthread_mutex_lock(&cl_mu);
        pthread_cond_wait(&cl_co,&cl_mu);
        pthread_mutex_unlock(&cl_mu);
        if(strcmp(send_data->write_buff,"send fail") == 0){
            printf("没有该[id:%d]的好友!!!\n",send_data->recv_id);
            printf("按任意键继续...");
            getchar();
            break;
        }
    }
    send_data->recv_id = 0;
    break;

    case 2:    //添加好友
    send_data->type = ADD_FRIEND;
    while(1){
    printf("请输入想加好友的id：");
    scanf("%d",&send_data->recv_id);
    getchar();
    if(send_data->send_id != send_data->recv_id){
        break;
    }
    printf("没朋友？太孤单寂寞了吧！！！");
    printf("按任意键继续");
    getchar();
    }
    if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
        my_err("send",__LINE__);
    }
    pthread_mutex_lock(&cl_mu);
    pthread_cond_wait(&cl_co,&cl_mu);
    pthread_mutex_unlock(&cl_mu);
    if(strcmp(send_data->write_buff,"ok") == 0){
        printf("发送成功！等待对方确认...\n");
        printf("按任意键继续...");
        getchar();
    }else{
        printf("对方已经是你的好友或者对方不存在!!!\n");
        printf("按任意键继续...\n");
        getchar();
    }
    bzero(send_data->write_buff,sizeof(send_data->write_buff));
    break;


    case 3:    //好友请求
    pthread_mutex_lock(&cl_mu);
    send_data->type = FRIEND_PLS;
    if(!box->fri_pls_num){
        printf("没有收到好友请求\n");
        printf("按任意键继续...\n");
        getchar();
        pthread_mutex_unlock(&cl_mu);
    }else{
        for(int i = 0;i < box->fri_pls_num;i++){
            printf("%s\n",box->send_pls[i]);
            send_data->recv_id = box->fri_pls_id[i];
            printf("[如何处理：1.同意 2.拒绝 3.忽略]\n选吧(选择除了-1-2-3-其他都作为-忽略-):");
            scanf("%d",&choice);
            getchar();
            if(choice == 1){
            strcpy(send_data->read_buff,"ok");
            if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
            my_err("send",__LINE__);
            }
            }else if(choice == 2){
            strcpy(send_data->read_buff,"no");
            if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
            my_err("send",__LINE__);
            }else{
            continue;
            }
            }
        }
    }
        box->fri_pls_num = 0;
        printf("处理完成...\n");
        printf("按任意键继续...");
        pthread_mutex_unlock(&cl_mu);
        bzero(send_data->write_buff,sizeof(send_data->write_buff));
        bzero(send_data->read_buff,sizeof(send_data->read_buff));
        getchar();
    break;

    case 4:                //删除好友
    send_data->type = DEL_FRIEND;
    printf("请输入你想要删除的好友id: ");
    scanf("%d",&send_data->recv_id);
    getchar();
    if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
        my_err("send",__LINE__);
    }
    pthread_mutex_lock(&cl_mu);
    pthread_cond_wait(&cl_co,&cl_mu);
    pthread_mutex_unlock(&cl_mu);
    if(strcmp(send_data->write_buff,"delete success") == 0){
        printf("delete success!\n");
        printf("按任意键继续..");
        getchar();
    }else{
        printf("您没有[id:%d]的好友!\n",send_data->recv_id);
        printf("按任意键继续..");
        getchar();
    }
    bzero(send_data->write_buff,sizeof(send_data->write_buff));
    break;
    
    case 5:                //查看好友列表
    send_data->type = LOOK_FRI_LS;
    if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
    my_err("send",__LINE__);
    }
    pthread_mutex_lock(&cl_mu);
    pthread_cond_wait(&cl_co,&cl_mu);
    pthread_mutex_unlock(&cl_mu);
    if(strcmp(send_data->write_buff,"nice") == 0){
    printf("friend list：\n");
    for(i = 0;i < flist->friend_num;i++){
    printf("-[id:%d]-[name:%s]-",flist->friend_id[i],flist->friend_nickname[i]);
    if(flist->friend_state[i] == 1){
    printf("online\n");
    }else{
    printf("outline\n");
    }
    }
    printf("按任意键继续...\n");
    getchar();
    }else if(strcmp(send_data->write_buff,"bad") == 0){
        printf("您还没有好友...\n");
        printf("按任意键继续...");
        getchar();
    }
    break;

    case 6:              //拉黑好友
    send_data->type = BLACK_LIST;
    printf("选择要拉黑的好友id: ");
    scanf("%d",&send_data->recv_id);
    getchar();
    if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
    my_err("send",__LINE__);
    }
    pthread_mutex_lock(&cl_mu);
    pthread_cond_wait(&cl_co,&cl_mu);
    pthread_mutex_unlock(&cl_mu);
    if(strcmp(send_data->write_buff,"black success") == 0){
    printf("拉黑[id:%d]好友成功\n",send_data->recv_id);
    printf("按任意键继续...");
    getchar();
    }else{
    printf("您没有该好友...\n");
    printf("按任意键继续...");
    getchar();
    }
    bzero(send_data->write_buff,sizeof(send_data->write_buff));
    break;

    case 7:        //解除好友黑名单
    send_data->type = QUIT_BLACK;
    printf("请输入要解黑的好友id: ");
    scanf("%d",&send_data->recv_id);
    getchar();
    if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
    my_err("send",__LINE__);
    }
    pthread_mutex_lock(&cl_mu);
    pthread_cond_wait(&cl_co,&cl_mu);
    pthread_mutex_unlock(&cl_mu);
    if(strcmp(send_data->write_buff,"quit ok") == 0){
    printf("解黑[id:%d]好友成功\n",send_data->recv_id);
    printf("按任意键继续...");
    getchar();
    }else{
    printf("您没有该好友...\n");
    printf("按任意键继续...");
    getchar();
    }
    bzero(send_data->write_buff,sizeof(send_data->write_buff));
    break;

    case 8:   //查看好友消息
    if(box->recv_msgnum == 0){
        printf("没有好友消息...\n");
        printf("按任意键继续...");
        getchar();
    }else{
        for(i = 0;i < box->recv_msgnum;i++){
            printf("[id:%d]说:%s\n",box->send_id[i],box->send_msg[i]);
        }
        box->recv_msgnum = 0;
        printf("按任意键继续...");
        getchar();
    }
    break;

    case 9:    //查询聊天记录
    send_data->type = LOOK_HISTORY;
    printf("请输入要查询与ta聊天记录的好友id: ");
    scanf("%d",&send_data->recv_id);
    getchar();
    if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
    my_err("send",__LINE__);
    }
    pthread_mutex_lock(&cl_mu);
    pthread_cond_wait(&cl_co,&cl_mu);
    pthread_mutex_unlock(&cl_mu);
    if(msg->num == 0){
        printf("没有聊天记录...\n");
    }else{
    for(i = 0;i< msg->num;i++){
    printf("\t[%d 对 %d 说：%s]",msg->send_id[i],msg->recv_id[i],msg->message[i]);
    }
    }
    printf("按任意键继续..");
    getchar();
    break;

    case 10: //删除聊天记录
    send_data->type = DELE_HISTORY;
    printf("请输入要删除与ta聊天记录的好友id: ");
    scanf("%d",&send_data->recv_id);
    getchar();
    if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
    my_err("send",__LINE__);
    }
    pthread_mutex_lock(&cl_mu);
    pthread_cond_wait(&cl_co,&cl_mu);
    pthread_mutex_unlock(&cl_mu);
    if(strcmp(send_data->write_buff,"delete success") == 0){
    printf("已成功删除!\n");
    }else{
    printf("没有与他的聊天记录!\n");
    }
    printf("按任意键继续...");
    getchar();
    break;

    case 11:  //创建群
    send_data->type = CREATE_GROUP;
    bzero(buf,sizeof(buf));
    printf("请输入你要创建的名称：");
    scanf("%s",buf);
    getchar();
    printf("是否保存? 1-yes 2-no\n");
    scanf("%d",&sure);
    getchar();
    if(sure == 1){
    strcpy(send_data->recv_name,buf);
    }else{
    break;
    }
    if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
    my_err("send",__LINE__);
    }
    pthread_mutex_lock(&cl_mu);
    pthread_cond_wait(&cl_co,&cl_mu);
    pthread_mutex_unlock(&cl_mu);
    if(strcmp(send_data->write_buff,"create success") == 0){
    printf("创建成功!!\n");
    printf("您的群信息为[id:%d][name:%s]\n",send_data->recv_id,send_data->recv_name);
    }else{
    printf("创建失败！！！\n");
    }
    printf("按任意键继续...\n");
    getchar();
    break;

    case 12: //删除群
    send_data->type = DISSOLVE_GROUP;
    printf("请输入要解散的群id: ");
    scanf("%d",&send_data->recv_id);
    getchar();
    printf("是否保存? 1-yes 2-no\n");
    scanf("%d",&sure);
    getchar();
    if(sure != 1){
    break;
    }
    if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
    my_err("send",__LINE__);
    }
    pthread_mutex_lock(&cl_mu);
    pthread_cond_wait(&cl_co, &cl_mu);
    pthread_mutex_unlock(&cl_mu);
    if(strcmp(send_data->write_buff,"dissolve success") == 0){
    printf("群解散成功!!\n");
    }else if(strcmp(send_data->write_buff,"no group") == 0){
    printf("群不存在！！！\n");
    }else if(strcmp(send_data->write_buff,"no num") == 0){
    printf("我劝你善良!先加群吧>>\n");
    }else{
    printf("您不是群主，没有权限！！！\n");
    }
    printf("按任意键继续...\n");
    getchar();
    break;

    case 13:   //加群
    send_data->type = ADD_GROUP;
    printf("请输入你要加入的群id: ");
    scanf("%d",&send_data->recv_id);
    getchar();
    printf("是否保存? 1-yes 2-no\n");
    scanf("%d",&sure);
    getchar();
    if(sure != 1){
    break;
    }
    if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
    my_err("send",__LINE__);
    }
    pthread_mutex_lock(&cl_mu);
    pthread_cond_wait(&cl_co, &cl_mu);
    pthread_mutex_unlock(&cl_mu);
    if(strcmp(send_data->write_buff,"add success") == 0){
    printf("成功加入群[id:%d][群名:%s]!\n",send_data->recv_id,send_data->recv_name);
    }else if(strcmp(send_data->write_buff,"have done") == 0){
    printf("您已经加入该群！！！\n");
    }else{
    printf("该群不存在！！！\n");
    }
    printf("按任意键继续...\n");
    getchar();
    break;
    


    case 14:
    send_data->type = EXIT_GROUP;
    printf("请输入你要退出的群id: ");
    scanf("%d",&send_data->recv_id);
    getchar();
    printf("是否保存? 1-yes 2-no\n");
    scanf("%d",&sure);
    getchar();
    if(sure != 1){
    break;
    }
    if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
    my_err("send",__LINE__);
    }
    pthread_mutex_lock(&cl_mu);
    pthread_cond_wait(&cl_co, &cl_mu);
    pthread_mutex_unlock(&cl_mu);
    if(strcmp(send_data->write_buff,"exit success") == 0){
    printf("退群成功!!\n");
    }else if(strcmp(send_data->write_buff,"no group") == 0){
    printf("群不存在！！！\n");
    }else if(strcmp(send_data->write_buff,"no add") == 0){
    printf("您未在该群内\n");
    }else if(strcmp(send_data->write_buff,"host exit") == 0){
    printf("您为该群群主，您已经解散该群\n");
    }
    printf("按任意键继续...\n");
    getchar();
    break;

    case 15:            //设置管理员
    send_data->type = SET_ADMIN;
    bzero(buf, sizeof(buf));
    printf("请输入群号: ");
    scanf("%d",&send_data->recv_id);
    getchar();
    printf("请输入要设置管理员的成员id: ");
    scanf("%s",buf);
    getchar();
    if(atoi(buf) == send_data->send_id){
    printf("咋这么会玩呢,不可以...");
    getchar();
    break;
    }
    printf("是否保存? 1-yes 2-no\n");
    scanf("%d",&sure);
    getchar();
    if(sure != 1){
    break;
    }
    strcpy(send_data->read_buff,buf);
    if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
    my_err("send",__LINE__);
    }
    pthread_mutex_lock(&cl_mu);
    pthread_cond_wait(&cl_co, &cl_mu);
    pthread_mutex_unlock(&cl_mu);
    if(strcmp(send_data->write_buff,"success") == 0){
    printf("[id:%d]已设置为[群:%d]的管理员!!\n",atoi(send_data->read_buff),send_data->recv_id);
    }else if(strcmp(send_data->write_buff,"not host") == 0){
    printf("您不是群主，没有该权限！！！\n");
    }else if(strcmp(send_data->write_buff,"not mem") == 0){
    printf("ta不是群成员\n");
    }
    printf("按任意键继续...\n");
    getchar();
    break;

    case 16:
    bzero(buf,sizeof(buf));
    send_data->type = QUIT_ADMIN;
    printf("请输入群id: ");
    scanf("%d",&send_data->recv_id);
    getchar();
    printf("请输入你要取消管理员的id: ");
    scanf("%s",buf);
    getchar();
    if(atoi(buf) == send_data->send_id){
    printf("咋这么会玩呢,不可以...");
    getchar();
    break;
    }
    printf("是否保存? 1-yes 2-no\n");
    scanf("%d",&sure);
    getchar();
    if(sure != 1){
    break;
    }
    strcpy(send_data->read_buff,buf);
    if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
    my_err("send",__LINE__);
    }
    pthread_mutex_lock(&cl_mu);
    pthread_cond_wait(&cl_co, &cl_mu);
    pthread_mutex_unlock(&cl_mu);
    if(strcmp(send_data->write_buff,"success") == 0){
    printf("成功取消[id:%d]在[群:%d]管理员身份!\n",atoi(send_data->read_buff),send_data->recv_id);
    }else if(strcmp(send_data->write_buff,"not host") == 0){
    printf("您不是群主，没有该权限(或者输入信息有误)！！！\n");
    }else if(strcmp(send_data->write_buff,"not mem") == 0){
    printf("ta不是群成员\n");
    }
    printf("按任意键继续...\n");
    getchar();
    break;


    case 17:
    send_data->type = KICK_MEM;
    printf("请输入群id:");
    scanf("%d",&send_data->recv_id);
    getchar();
    printf("请输入要踢出的群成员id: ");
    scanf("%s",buf);
    getchar();
    if(atoi(buf) == send_data->send_id){
    printf("你可以选择退群...");
    getchar();
    break;
    }
    printf("是否保存? 1-yes 2-no\n");
    scanf("%d",&sure);
    getchar();
    if(sure != 1){
    break;
    }
    strcpy(send_data->read_buff,buf);
    if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
    my_err("send",__LINE__);
    }
    pthread_mutex_lock(&cl_mu);
    pthread_cond_wait(&cl_co, &cl_mu);
    pthread_mutex_unlock(&cl_mu);
    if(strcmp(send_data->write_buff,"success") == 0){
    printf("成功将[id:%d]踢出[群:%d]!\n",atoi(send_data->read_buff),send_data->recv_id);
    }else if(strcmp(send_data->write_buff,"not") == 0){
    printf("对方是管理员，您没有权限(对方不在群)！！！\n");
    }else if(strcmp(send_data->write_buff,"not power") == 0){
    printf("您没有权限(对方不在群)\n");
    }
    printf("按任意键继续...\n");
    getchar();
    break;






    case 19:         //修改密码
    send_data->type = USER_CHANGE;
    printf("please input your original passwd: ");
    i = 0;
    while(1){
    scanf("%c",&ch);
    if(ch == '\n'){
    send_data->read_buff[i] = '\0';
    break;
    }
    send_data->read_buff[i++] = ch;

    }
    printf("\n");
    printf("please input your new passwd: ");
    i = 0;
    while(1){
    scanf("%c",&ch);
    if(ch == '\n'){
    send_data->write_buff[i] = '\0';
    break;
    }
    send_data->write_buff[i++] = ch;

    }
    if((ret = send(connfd,send_data,sizeof(recv_datas),0)) < 0){
        my_err("send",__LINE__);
    }
    pthread_mutex_lock(&cl_mu);
    pthread_cond_wait(&cl_co,&cl_mu);
    pthread_mutex_unlock(&cl_mu);
    printf("%s\n",send_data->write_buff);
    if(strcmp(send_data->write_buff,"success") == 0){
        printf("change passwd success!!!\n");
        printf("按任意键继续...\n");
        getchar();
    }else if(strcmp(send_data->write_buff,"error") == 0){
        printf("original passwd error");
        printf("按任意键继续...\n");
        getchar();
    }
    bzero(send_data->write_buff,sizeof(send_data->write_buff));
    bzero(send_data->read_buff,sizeof(send_data->read_buff));
    break;

    case 20:         //二级界面退出
    send_data->type = USER_OUT;
        if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
        my_err("send",__LINE__);
        }
        pthread_exit(0);
        break;
    }
    }
}














void *recv_info(void * arg){
    if(recv_data->send_id == send_data->recv_id){
    printf("[id:%d][昵称:%s]:%s\n",recv_data->send_id,recv_data->send_name,recv_data->read_buff);
    }else{
        box->send_id[box->recv_msgnum] = recv_data->send_id;
        strcpy(box->send_msg[box->recv_msgnum++],recv_data->read_buff);
        printf("您收到一条消息\n");
    }
    pthread_exit(0);
}

void *look_list(void * connfd){
    bzero(flist,sizeof(FRIENDS));
    if(recv(*(int*)connfd,flist,sizeof(FRIENDS),MSG_WAITALL) < 0){
    my_err("recv",__LINE__);
    }
    pthread_exit(0);
}

void *box_check(void * connfd){
if(recv(*(int*)connfd,box,sizeof(BOX_MSG),MSG_WAITALL) < 0){
my_err("recv",__LINE__);
}
pthread_exit(0);
}

void *look_msg(void * connfd){
if(recv(*(int*)connfd,msg,sizeof(MSG), MSG_WAITALL) < 0){
my_err("recv", __LINE__);
}
pthread_exit(0);
}
















void *write_mission(void*arg){
    int   connfd = *(int*)arg;
    int   ret;
    pthread_t tid;
    recv_data  = (recv_datas*)malloc(sizeof(recv_datas));
    box = (BOX_MSG*)malloc(sizeof(BOX_MSG));
    flist = (FRIENDS *)malloc(sizeof(FRIENDS));
    msg = (MSG*)malloc(sizeof(MSG));



    while(1){
        bzero(recv_data,sizeof(recv_datas));
        if((ret = recv(connfd,recv_data,sizeof(recv_datas),MSG_WAITALL)) < 0){
            my_err("recv",__LINE__);
        }
        switch(recv_data->type){
            case USER_OUT:
            printf("客户端退出...");
            pthread_exit(0);
            break;

            case USER_LOGIN:
            strcpy(send_data->send_name,recv_data->send_name);
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            send_data->sendfd = recv_data->recvfd;
            pthread_create(&tid,NULL,box_check,arg);
            pthread_join(tid, NULL);
            printf("\t--message:%d--fplease:%d--\n",box->recv_msgnum,box->fri_pls_num);
            pthread_mutex_lock(&cl_mu);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case USER_SIGN:
            send_data->send_id = recv_data->send_id;
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_mutex_lock(&cl_mu);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case USER_FIND:
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_mutex_lock(&cl_mu);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case ID_ERROR:
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_mutex_lock(&cl_mu);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case USER_CHANGE:
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_mutex_lock(&cl_mu);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case ADD_FRIEND:
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_mutex_lock(&cl_mu);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case FRIEND_PLS:
            pthread_mutex_lock(&cl_mu);
            box->fri_pls_id[box->fri_pls_num]= recv_data->send_id;
            strcpy(box->send_pls[box->fri_pls_num],recv_data->read_buff);
            box->fri_pls_num += 1;
            //printf("受到一条来自[帐号:%d][名称:%s]的好友请求...",recv_data->send_id,recv_data->send_name);
            pthread_mutex_unlock(&cl_mu);
            break;

            case DEL_FRIEND:
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_mutex_lock(&cl_mu);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case BLACK_LIST:
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_mutex_lock(&cl_mu);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case QUIT_BLACK:
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_mutex_lock(&cl_mu);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case SEND_INFO:
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_mutex_lock(&cl_mu);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case RECV_INFO:
            pthread_create(&tid,NULL,recv_info,arg);
            pthread_join(tid, NULL);
            break;

            case LOOK_FRI_LS:
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            bzero(flist,sizeof(FRIENDS));
            pthread_create(&tid,NULL,look_list,arg);
            pthread_join(tid, NULL);
            pthread_mutex_lock(&cl_mu);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case LOOK_HISTORY:
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_create(&tid,NULL,look_msg,arg);
            pthread_join(tid, NULL);
            pthread_mutex_lock(&cl_mu);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case DELE_HISTORY:
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_mutex_lock(&cl_mu);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case CREATE_GROUP:
            bzero(send_data->recv_name,sizeof(send_data->recv_name));
            strcpy(send_data->recv_name,recv_data->recv_name);
            send_data->recv_id = recv_data->recv_id;
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_mutex_lock(&cl_mu);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case DISSOLVE_GROUP:
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_mutex_lock(&cl_mu);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case ADD_GROUP:
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            bzero(send_data->recv_name,sizeof(send_data->recv_name));
            strcpy(send_data->recv_name,recv_data->recv_name);
            pthread_mutex_lock(&cl_mu);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;


            case EXIT_GROUP:
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_mutex_lock(&cl_mu);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case SET_ADMIN:
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_mutex_lock(&cl_mu);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case QUIT_ADMIN:
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_mutex_lock(&cl_mu);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case KICK_MEM:
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_mutex_lock(&cl_mu);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;
        }
    }
}



int main(){
    int connfd;
    pthread_t tid1, tid2;
    struct sockaddr_in servaddr;
    pthread_mutex_init(&cl_mu, NULL);
    pthread_cond_init(&cl_co, NULL);
    if((connfd = socket(AF_INET, SOCK_STREAM,0))<0){
        my_err("socket",__LINE__);
    }
    bzero(&servaddr,sizeof(struct sockaddr_in));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    inet_pton(AF_INET,"127.0.0.1",(void*)&servaddr.sin_addr.s_addr);
    printf("正在与服务器建立连接...\n");
    if((connect(connfd,(struct sockaddr*)&servaddr,sizeof(servaddr))) < 0){
        perror("connect");
    }else
    printf("与服务器链接成功！！！\n");
    pthread_create(&tid1,NULL,read_mission,(void*)&connfd);
    pthread_create(&tid2,NULL,write_mission,(void*)&connfd);
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    return 0;
}



