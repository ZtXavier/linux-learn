#include<stdio.h>
#include<sys/types.h>
#include<fcntl.h>
#include<sys/stat.h>
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


recv_datas   *send_data;
recv_datas   *recv_data;
BOX_MSG      *box;
MSG          *msg;
GRP_MSG      *group_msg;
FRIENDS      *flist;
GRP_INFO     *grp_info_list;
GRP_MEM_LIST *group_mem;
FILE_INFO    *file_info;

void my_err(const char *err_string, int line){
	fprintf(stderr, "line:%d ", line);
	perror(err_string);
	exit(1);
}

char * my_passwd(char* passwd){
        int k = 0;
        system("stty -icanon");
        system("stty -echo");
        while(k < 24){
        passwd[k] = getchar();
        if(k == 0 && passwd[k] == BACKSPACE){
			k=0;
			passwd[k]='\0';
			continue;
		}
		else if(passwd[k] == '\n'){               //若按回车则直接跳出，输入结束
			passwd[k]='\0';
			break;
		}
		else if(passwd[k] == BACKSPACE){
			printf("\b \b");                    //若删除，则光标前移，输空格覆盖，再光标前移
			passwd[k]='\0';
			k -= 1;                              //返回到前一个值继续输入
			continue;                           //结束当前循环
		}
		else{
			printf("*");
		}
		k++;
        }
        system("stty echo");                      //开启回显
        system("stty icanon");                   //关闭一次性读完操作，即getchar()必须回车也能获取字符
        return passwd;
}


//来用写发数据
void *send_mission(void*arg){
        int   connfd = *(int*)arg;
        int   record;
        int   choice;
        int   i;
        int   ret;
        char  ch;
        char  password1[24],password2[24];
        char  buf[50];
        int   sure;
        struct stat buff;
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
            printf("\t*****        0.退出               *****\n");
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
                bzero(send_data->read_buff,sizeof(send_data->read_buff));
                my_passwd(send_data->read_buff);
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
                my_passwd(password1);
                printf("\n");
                printf("please input passwd again: ");
                my_passwd(password2);
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
            }
            else{
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

            case 0:
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
            }
            if((choice >= 3) || (choice < 0))    continue;
            else if (choice == 1){
                if (strcmp(send_data->write_buff, "id error") == 0){
                    printf("ID or PASSWD error!!!\n");
                    printf("按任意键继续...\n");
                    getchar();
                    continue;
                }
                else if(strcmp(send_data->write_buff,"login success") == 0){
                    printf("按任意键继续...\n");
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
            }
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
        printf("\t*****        18.查看加入的群      *****\n");
        printf("\t*****                             *****\n");
        printf("\t*****        19.查看群成员        *****\n");
        printf("\t*****                             *****\n");
        printf("\t*****        20.群聊              *****\n");
        printf("\t*****                             *****\n");
        printf("\t*****        21.查看群消息        *****\n");
        printf("\t*****                             *****\n");
        printf("\t*****        22.发送文件          *****\n");
        printf("\t*****                             *****\n");
        printf("\t*****        23.接收文件          *****\n");
        printf("\t*****                             *****\n");
        printf("\t*****        24.修改密码          *****\n");
        printf("\t*****                             *****\n");
        printf("\t*****        0.退出               *****\n");
        printf("\t*****                             *****\n");
        printf("\t***************************************\n");
        printf("请选择: ");
        scanf("%d",&choice);
        getchar();
        switch(choice){
        case 1:
        send_data->type = SEND_INFO;
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
        printf("\t--与->%d<-好友聊天窗口--\n",send_data->recv_id);
        while(1){
            bzero(send_data->read_buff,sizeof(send_data->read_buff));
            fgets(send_data->read_buff,sizeof(send_data->read_buff),stdin);
            send_data->read_buff[strlen(send_data->read_buff)-1] = '\0';
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
        }
        else{
            printf("对方已经是你的好友或者对方不存在!!!\n");
            printf("按任意键继续...\n");
            getchar();
        }
        bzero(send_data->write_buff,sizeof(send_data->write_buff));
        send_data->recv_id = 0;
        break;


        case 3:    //好友请求
        pthread_mutex_lock(&cl_mu);
        send_data->type = FRIEND_PLS;
        if(!box->fri_pls_num)
        {
            printf("没有收到好友请求\n");
            printf("按任意键继续...\n");
            getchar();
            pthread_mutex_unlock(&cl_mu);
        }
        else
        {
            for(int i = 0;i < box->fri_pls_num;i++){
                printf("%s\n",box->send_pls[i]);
                send_data->recv_id = box->fri_pls_id[i];
                printf("如何处理[1.同意][2.拒绝]\n选吧(选择除了-1-2-其他都作为-拒绝-):");
                scanf("%d",&choice);
                getchar();
                if(choice == 1){
                    strcpy(send_data->read_buff,"ok");
                    if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
                        my_err("send",__LINE__);
                    }
                }else if(choice == 2)
                {
                    strcpy(send_data->read_buff,"no");
                    if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
                        my_err("send",__LINE__);
                    }
                    else{
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
        send_data->recv_id = 0;
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
        send_data->recv_id = 0;
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
        send_data->recv_id = 0;
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
            for(i = 0;i < msg->num;i++){
                printf("\t[id:%d]对[id:%d]:%s\n",msg->send_id[i],msg->recv_id[i],msg->message[i]);
            }
        }
        send_data->recv_id = 0;
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
        send_data->recv_id = 0;
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
        send_data->recv_id = 0;
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
        send_data->recv_id = 0;
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
        send_data->recv_id = 0;
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
        send_data->recv_id = 0;
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
        send_data->recv_id = 0;
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
        send_data->recv_id = 0;
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
        send_data->recv_id = 0;
        printf("按任意键继续...\n");
        getchar();
        break;

        case 18:     //查看已加入的群
        send_data->type = LOOK_GROUP_LS;
        if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
            my_err("send",__LINE__);
        }
        pthread_mutex_lock(&cl_mu);
        pthread_cond_wait(&cl_co, &cl_mu);
        pthread_mutex_unlock(&cl_mu);
        if(grp_info_list->number == 0){
            printf("您还没有加群哦!!!\n");
        }else{
            printf("\tgroup_list:\n");
            for(i = 0;i < grp_info_list->number;i++){
                    printf("\t--[群id:%d][群名称:%s]--",grp_info_list->group_id[i],grp_info_list->group_name[i]);
                if(grp_info_list->group_state[i] == 2){
                    printf("群主\n");
                }else if(grp_info_list->group_state[i] == 1){
                    printf("管理员\n");
                }else{
                    printf("普通成员\n");
                }
            }
        }
        send_data->recv_id = 0;
        printf("按任意键继续...\n");
        getchar();
        break;

        case 19:            //查看群成员
        send_data->type = LOOK_GROUP_MEM;
        printf("请输入你要查看的群id: ");
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
        if(strcmp(send_data->write_buff,"fail") == 0){
            printf("您没有进群,无法查看群内信息！");
        }else if(strcmp(send_data->write_buff,"success") == 0){
            printf("[群id:%d]:\n",send_data->recv_id);
            for(i = 0;i < group_mem->group_mem_num;i++){
                printf("\t--[id:%d][name:%s]--",group_mem->group_mem_id[i],group_mem->group_mem_nickname[i]);
                if(group_mem->group_mem_state[i] == 2){
                    printf("群主\n");
                }else if(group_mem->group_mem_state[i] == 1){
                    printf("管理员\n");
                }else{
                    printf("普通成员\n");
                }
            }
        }
        send_data->recv_id = 0;
        printf("按任意键继续...\n");
        getchar();
        break;

        case 20: //群聊
        send_data->type = SEND_GROUP_MSG;
        printf("请输入你要聊天的群id:");
        scanf("%d",&send_data->recv_id);
        getchar();
        printf("是否保存? 1-yes 2-no\n");
        scanf("%d",&sure);
        getchar();
        if(sure != 1){
            break;
        }
        printf("\t--->群[id:%d]<---\n",send_data->recv_id);
        while(1){
            bzero(send_data->read_buff,sizeof(send_data->read_buff));
            fgets(send_data->read_buff,sizeof(send_data->read_buff),stdin);
            send_data->read_buff[strlen(send_data->read_buff)-1] = '\0';
            if(strcmp(send_data->read_buff,"#over#") == 0){
                printf("您已退出群聊...\n");
                break;
            }
            if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
                pthread_mutex_lock(&cl_mu);
                pthread_cond_wait(&cl_co, &cl_mu);
                pthread_mutex_unlock(&cl_mu);
            if(strcmp(send_data->write_buff,"no group") == 0){
                printf("没有[id:%d]的群...\n",send_data->recv_id);
                break;
            }
        }
        send_data->recv_id = 0;
        printf("按任意键继续...");
        getchar();
        break;

        case 21:   //查群消息
        if(box->group_msg_num == 0){
            printf("没有群消息...\n");
            printf("按任意键继续...");
            getchar();
        }else{
            for(i = 0;i < box->group_msg_num;i++){
                printf("<群id:%d>[id:%d][name:%s]说:%s\n",box->group_id[i],box->group_send_id[i],box->group_mem_nikename[i],box->group_message[i]);
            }
            box->group_msg_num = 0;
            printf("按任意键继续...");
            getchar();
        }
        break;

        case 22:
        int fp;
        bzero(buf,sizeof(buf));
        send_data->type = SEND_FILE;
        send_data->cont = 0;
        while(1){
            printf("请输入对方的id: ");
            scanf("%d",&send_data->recv_id);
            getchar();
            if(send_data->send_id != send_data->recv_id){
                break;
            }
            printf("自己不可以给自己发文件哦!!!\n");
            printf("按任意键继续");
            getchar();
        }
        printf("请输入你要发送的文件的路径: ");
        scanf("%s",buf);
        getchar();
        printf("是否保存? 1-yes 2-no\n");
        scanf("%d",&sure);
        getchar();
        if(sure != 1){
            break;
        }
        send_data->type = SEND_FILE;
        if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
            my_err("send",__LINE__);
        }
        pthread_mutex_lock(&cl_mu);
        pthread_cond_wait(&cl_co, &cl_mu);
        pthread_mutex_unlock(&cl_mu);
        if(strcmp(send_data->write_buff,"no people") == 0){
            printf("没有该id,发送失败!!!\n");
            printf("请按任意键退出...\n");
            getchar();
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            break;
        }
        send_data->flag = 1;
        send_data->mun = -1;
        bzero(send_data->write_buff,sizeof(send_data->write_buff));
        if(stat(buf,&buff) != 0){
            printf("%s\n",strerror(errno));
            printf("按任意键返回...");
            getchar();
            break;
        }
        send_data->filesize = buff.st_size;
        strcpy(send_data->write_buff,buf);
        if((fp = open(send_data->write_buff,O_RDONLY)) < 0){
            printf("%s\n",strerror(errno));
            printf("按任意键返回...");
            getchar();
            break;
        }
            bzero(send_data->read_buff,sizeof(send_data->read_buff));
            printf("正在发送...\n");
            ssize_t l;
        while(1){
            if((l = read(fp,send_data->read_buff,sizeof(send_data->read_buff)-1)) < (sizeof(send_data->read_buff)-1)){
                //如果读到的字节数小于规定的包的大小，将最后一个包的大小存入l
                send_data->flag = 0;        //文件最后截止的提示
                send_data->mun = l;         //文件最后的长度
                if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
                    my_err("send",__LINE__);
                }
                pthread_mutex_lock(&cl_mu);
                pthread_cond_wait(&cl_co, &cl_mu);
                pthread_mutex_unlock(&cl_mu);
                break;
            }
            if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
            pthread_mutex_lock(&cl_mu);
            pthread_cond_wait(&cl_co, &cl_mu);
            pthread_mutex_unlock(&cl_mu);
            send_data->cont++;
        }
        close(fp);
            send_data->type = FINSH;
            if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
            pthread_mutex_lock(&cl_mu);
            pthread_cond_wait(&cl_co, &cl_mu);
            pthread_mutex_unlock(&cl_mu);
            if(strcmp(send_data->write_buff,"no people") == 0){
                printf("发送失败!!!\n");
            }
            else if(strcmp(send_data->write_buff,"nice") == 0){
                printf("已成功发送给对方!!!\n");
            }
            send_data->recv_id = 0;
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            bzero(send_data->read_buff,sizeof(send_data->read_buff));
            printf("按任意键继续...\n");
            getchar();
        break;

        case 23:
            int choose;
            printf("\t***************************************\n");
            printf("\t************文件接收选项***************\n");
            printf("\t***************************************\n");
            printf("\t*****        1.实时文件           *****\n");
            printf("\t*****                             *****\n");
            printf("\t*****        2.离线文件           *****\n");
            printf("\t*****                             *****\n");
            printf("\t*****        0.退出               *****\n");
            printf("\t***************************************\n");
            printf("\t***************************************\n");
            printf("请选择: ");
            scanf("%d",&choose);
            getchar();
            switch(choose){
            case 1:
            if(file_info->num == 0){
                    printf("没有收到文件..\n");
                    printf("按任意键继续...");
                    getchar();
                    break;
            }
            for(int j = 0;j < file_info->num; j++){
                printf("\t--[文件个数:%d]--\n",file_info->num);
                printf("[id:%d][name:%s]给你发送了[文件%s大小为%d]!",file_info->send_id[j],file_info->send_nickname[j],file_info->filepath[j],file_info->filesize[j]);
            while(1){
                    printf("处理方式:[1.接收] [2.拒绝]\n");
                    scanf("%d",&sure);
                    getchar();
                if(sure == 1){
                    send_data->cont = 0;
                    while(1){
                        send_data->type = READ_FILE;
                        if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
                            my_err("send",__LINE__);
                        }
                            printf("开始接收文件...\n");
                            pthread_mutex_lock(&cl_mu);
                            pthread_cond_wait(&cl_co, &cl_mu);
                            pthread_mutex_unlock(&cl_mu);
                            send_data->cont++;
                            printf("已接收%d个包\n",send_data->cont);
                        if(strcmp(send_data->write_buff,"finish") == 0){
                            printf("文件接收完毕!!");
                            printf("按任意键继续");
                            getchar();
                            break;
                        }
                    }
                break;
                }
                else if(sure == 2){
                    printf("你拒绝了id:%d的文件...\n",file_info->send_id[j]);
                    printf("按任意键继续");
                    getchar();
                    break;
                }
                else{
                    printf("输入无效，请重新输入!");
                    printf("按任意键继续");
                    getchar();
                    continue;
                }
            }
        }
            printf("是否清空文件缓存? 1.yes 2.no\n");
            scanf("%d",&sure);
            getchar();
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            bzero(send_data->read_buff,sizeof(send_data->read_buff));
            if(sure == 1)  bzero(file_info,sizeof(FILE_INFO));
            break;

            case 2:
                    if(box->file_num == 0){
                    printf("没有收到离线文件..\n");
                    printf("按任意键继续...");
                    getchar();
                    break;
            }
            for(int j = 0;j < box->file_num; j++){
                    printf("\t--[文件数目:%d]--\n",box->file_num);
                    printf("[id:%d][name:%s]给你发送了[文件%s大小为%d]!",box->file_send_id[j],box->file_send_nickname[j],box->file_pathname[j],box->file_size[j]);
            while(1){
                    printf("处理方式:[1.接收] [2.拒绝]\n");
                    scanf("%d",&sure);
                    getchar();
                if(sure == 1){
                    send_data->cont = 0;
                while(1){
                    send_data->type = READ_FILE;
                    if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
                        my_err("send",__LINE__);
                    }
                        printf("开始接收文件...\n");
                        pthread_mutex_lock(&cl_mu);
                        pthread_cond_wait(&cl_co, &cl_mu);
                        pthread_mutex_unlock(&cl_mu);
                        send_data->cont++;
                        printf("已接收%d个包\n",send_data->cont);
                        if(strcmp(send_data->write_buff,"finish") == 0){
                        printf("文件接收完毕!!");
                        printf("按任意键继续");
                        getchar();
                        break;
                    }
                }
                break;
                }
                else if(sure == 2){
                    printf("你拒绝了id:%d的文件...\n",box->file_send_id[j]);
                    printf("按任意键继续");
                    getchar();
                    break;
                }
                else{
                    printf("输入无效，请重新输入!\n");
                    printf("按任意键继续");
                    getchar();
                    continue;
                }
            }
        }
            printf("是否清空离线文件缓存? 1.yes 2.no\n");
            scanf("%d",&sure);
            getchar();
            if(sure == 1){
                for(int j = 0;j < box->file_num;j++){
                    bzero(box->file_pathname[j],sizeof(box->file_pathname[j]));
                    bzero(box->file_send_nickname[j],sizeof(box->file_send_nickname[j]));
                    box->file_send_id[j] = 0;
                    box->file_size[j] = 0;
                }
                box->file_num = 0;
                printf("清理缓存成功!\n");
                printf("按任意键继续");
                getchar();
            }
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            bzero(send_data->read_buff,sizeof(send_data->read_buff));
            break;

            case 0:
            break;
        }
        break;

        case 24:         //修改密码
                send_data->type = USER_CHANGE;
                printf("please input your original passwd: ");
                my_passwd(send_data->read_buff);
                printf("\n");
                printf("please input your new passwd: ");
                my_passwd(send_data->write_buff);
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
        //群聊天记录的删除有bug,需要分表，以后改
        case 25:
            send_data->type = LOOK_GRP_HISTORY;
            printf("请输入你要查询聊天记录的群id：");
            scanf("%d",&send_data->recv_id);
            getchar();
            if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
            pthread_mutex_lock(&cl_mu);
            pthread_cond_wait(&cl_co,&cl_mu);
            pthread_mutex_unlock(&cl_mu);
            if(strcmp(send_data->write_buff,"not enter") == 0){
                printf("您不再群中，请先加群!!!\n");
                printf("按任意键继续...\n");
                getchar();
                send_data->recv_id = 0;
                bzero(send_data->write_buff,sizeof(send_data->write_buff));
                break;
            }
            if(group_msg->group_msg_num == 0)
                printf("该群没有聊天记录...\n");
            else{
                printf("\t--[群id:%d][群昵称:%s]聊天记录:--\n",group_msg->group_id,group_msg->group_nikename);
                for(i = 0;i < group_msg->group_msg_num;i++){
                    printf("\t[id:%d][昵称:%s]:%s\n",group_msg->group_mem_id[i],group_msg->group_mem_nikename[i],group_msg->message[i]);
                }
            }
            send_data->recv_id = 0;
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            printf("显示完毕，按任意键继续...");
            getchar();
        break;

        case 26:
            send_data->type = DELE_GRP_HISTORY;
            printf("请输入你要删除聊天记录的群id：");
            scanf("%d",&send_data->recv_id);
            getchar();
            if(send(connfd,send_data,sizeof(recv_datas),0) < 0){
                my_err("send",__LINE__);
            }
            pthread_mutex_lock(&cl_mu);
            pthread_cond_wait(&cl_co,&cl_mu);
            pthread_mutex_unlock(&cl_mu);
            if(strcmp(send_data->write_buff,"not enter") == 0){
                printf("您不再群中，请先加群!!!\n");
                printf("按任意键继续...\n");
                getchar();
            }else if(strcmp(send_data->write_buff,"dele success") == 0){
                printf("删除成功！！\n");
                printf("按任意键继续...\n");
                getchar();
            }else if(strcmp(send_data->write_buff,"have done") == 0){
                printf("删除成功！！！\n");
                printf("按任意键继续...\n");
                getchar();
            }
            send_data->recv_id = 0;
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
        break;

        case 0:         //二级界面退出
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
    printf("\t离线收到的消息:--好友消息:%d条--好友邀请:%d条--群消息:%d条--文件:%d个\n",box->recv_msgnum,box->fri_pls_num,box->group_msg_num,box->file_num);
    pthread_exit(0);
}

void *look_msg(void * connfd){
    if(recv(*(int*)connfd,msg,sizeof(MSG), MSG_WAITALL) < 0){
        my_err("recv", __LINE__);
    }
    pthread_exit(0);
}

void *look_grp_msg(void * connfd){
    if(recv(*(int*)connfd,group_msg,sizeof(GRP_MSG),MSG_WAITALL) < 0){
        my_err("recv",__LINE__);
    }
    pthread_exit(0);
}

void *look_group_info(void *connfd){
    if(recv(*(int*)connfd,grp_info_list,sizeof(GRP_INFO), MSG_WAITALL) < 0){
        my_err("recv", __LINE__);
    }
    pthread_exit(0);
}

void *look_group_mem(void *connfd){
    if(recv(*(int*)connfd,group_mem,sizeof(GRP_MEM_LIST), MSG_WAITALL) < 0){
        my_err("recv", __LINE__);
    }
    pthread_exit(0);
}

void *recv_group_msg(void * connfd){
    if(recv_data->recv_id == send_data->recv_id){
    printf("[群id:%d][昵称:%s][id:%d][name:%s]:%s\n",recv_data->recv_id,recv_data->recv_name,recv_data->send_id,recv_data->send_name,recv_data->read_buff);
    }else{
        strcpy(box->group_message[box->group_msg_num],recv_data->read_buff); //发的消息
        box->group_send_id[box->group_msg_num] = recv_data->send_id;         //发消息的id
        strcpy(box->group_mem_nikename[box->group_msg_num],recv_data->send_name);
        box->group_id[box->group_msg_num++] = recv_data->recv_id;
        printf("您收到一条来自[群id:%d]的消息\n",recv_data->recv_id);
        recv_data->send_id = 0;
    }
    pthread_exit(0);
}

void *recv_file(void*connfd){
    file_info->send_id[file_info->num] = recv_data->send_id;
    file_info->filesize[file_info->num] = recv_data->filesize;
    strcpy(file_info->filepath[file_info->num],recv_data->write_buff);
    strcpy(file_info->send_nickname[file_info->num],recv_data->send_name);
    file_info->num += 1;
    pthread_exit(0);
}

void *recv_mission(void*arg){
    int   connfd = *(int*)arg;
    int   ret;
    int   fp;
    pthread_t tid;
    msg = (MSG*)malloc(sizeof(MSG));
    group_msg = (GRP_MSG*)malloc(sizeof(GRP_MSG));
    box = (BOX_MSG*)malloc(sizeof(BOX_MSG));
    flist = (FRIENDS *)malloc(sizeof(FRIENDS));
    file_info = (FILE_INFO *)malloc(sizeof(FILE_INFO));
    file_info->num = 0;
    grp_info_list = (GRP_INFO *)malloc(sizeof(GRP_INFO));
    recv_data  = (recv_datas*)malloc(sizeof(recv_datas));
    group_mem = (GRP_MEM_LIST *)malloc(sizeof(GRP_MEM_LIST));
    while(1){
        bzero(recv_data,sizeof(recv_datas));
        if((ret = recv(connfd,recv_data,sizeof(recv_datas),MSG_WAITALL)) < 0){
            my_err("recv",__LINE__);
        }
        switch(recv_data->type){
            case USER_OUT:
            printf("客户端退出...");
            free(msg);
            free(box);
            free(flist);
            free(file_info);
            free(grp_info_list);
            free(recv_data);
            free(group_mem);
            pthread_exit(0);
            break;

            case USER_LOGIN:
            pthread_mutex_lock(&cl_mu);
            strcpy(send_data->send_name,recv_data->send_name);
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            //send_data->sendfd = recv_data->recvfd;
            pthread_create(&tid,NULL,box_check,arg);
            pthread_join(tid, NULL);
            //printf("\t离线收到的消息:--好友消息:%d条--好友邀请:%d条--群消息:%d条\n",box->recv_msgnum,box->fri_pls_num,box->group_msg_num);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case USER_SIGN:
            pthread_mutex_lock(&cl_mu);
            send_data->send_id = recv_data->send_id;
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case USER_FIND:
            pthread_mutex_lock(&cl_mu);
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case ID_ERROR:
            pthread_mutex_lock(&cl_mu);
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case USER_CHANGE:
            pthread_mutex_lock(&cl_mu);
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case ADD_FRIEND:
            pthread_mutex_lock(&cl_mu);
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case FRIEND_PLS:
            pthread_mutex_lock(&cl_mu);
            box->fri_pls_id[box->fri_pls_num]= recv_data->send_id;
            strcpy(box->send_pls[box->fri_pls_num],recv_data->read_buff);
            box->fri_pls_num += 1;
            printf("您收到一条好友请求!!!\n");
            pthread_mutex_unlock(&cl_mu);
            break;

            case DEL_FRIEND:
            pthread_mutex_lock(&cl_mu);
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case BLACK_LIST:
            pthread_mutex_lock(&cl_mu);
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case QUIT_BLACK:
            pthread_mutex_lock(&cl_mu);
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case SEND_INFO:
            pthread_mutex_lock(&cl_mu);
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case RECV_INFO:
            pthread_create(&tid,NULL,recv_info,arg);
            pthread_join(tid, NULL);
            break;

            case LOOK_FRI_LS:
            pthread_mutex_lock(&cl_mu);
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            bzero(flist,sizeof(FRIENDS));
            pthread_create(&tid,NULL,look_list,arg);
            pthread_join(tid, NULL);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case LOOK_HISTORY:
            pthread_mutex_lock(&cl_mu);
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_create(&tid,NULL,look_msg,arg);
            pthread_join(tid, NULL);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case DELE_HISTORY:
            pthread_mutex_lock(&cl_mu);
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case CREATE_GROUP:
            pthread_mutex_lock(&cl_mu);
            bzero(send_data->recv_name,sizeof(send_data->recv_name));
            strcpy(send_data->recv_name,recv_data->recv_name);
            send_data->recv_id = recv_data->recv_id;
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case DISSOLVE_GROUP:
            pthread_mutex_lock(&cl_mu);
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case ADD_GROUP:
            pthread_mutex_lock(&cl_mu);
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            bzero(send_data->recv_name,sizeof(send_data->recv_name));
            strcpy(send_data->recv_name,recv_data->recv_name);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;


            case EXIT_GROUP:
            pthread_mutex_lock(&cl_mu);
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case SET_ADMIN:
            pthread_mutex_lock(&cl_mu);
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case QUIT_ADMIN:
            pthread_mutex_lock(&cl_mu);
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case KICK_MEM:
            pthread_mutex_lock(&cl_mu);
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case LOOK_GROUP_LS:
            pthread_mutex_lock(&cl_mu);
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            bzero(grp_info_list,sizeof(GRP_INFO));
            pthread_create(&tid,NULL,look_group_info,arg);
            pthread_join(tid, NULL);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case LOOK_GROUP_MEM:
            pthread_mutex_lock(&cl_mu);
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            bzero(group_mem,sizeof(GRP_MEM_LIST));
            pthread_create(&tid,NULL,look_group_mem,arg);
            pthread_join(tid, NULL);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case SEND_GROUP_MSG:
            pthread_mutex_lock(&cl_mu);
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case RECV_GROUP_MSG:
            pthread_create(&tid,NULL,recv_group_msg,arg);
            pthread_join(tid, NULL);
            break;

            case SEND_FILE:
            pthread_mutex_lock(&cl_mu);
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;


            case RECV_FILE:
            pthread_create(&tid,NULL,recv_file,arg);
            pthread_join(tid, NULL);
            printf("你收到了一份文件!\n");
            break;

            case READ_FILE:
            pthread_mutex_lock(&cl_mu);
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            if((fp = open("recv_file",O_WRONLY|O_CREAT|O_APPEND,0775)) < 0){
                my_err("open",__LINE__);
            }
            if(recv_data->flag == 0){
                write(fp,recv_data->read_buff,recv_data->mun);
                close(fp);
            }else{
                write(fp,recv_data->read_buff,sizeof(recv_data->read_buff)-1);
                close(fp);
            }
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case LOOK_GRP_HISTORY:
            pthread_mutex_lock(&cl_mu);
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
            pthread_create(&tid,NULL,look_grp_msg,arg);
            pthread_join(tid, NULL);
            pthread_cond_signal(&cl_co);
            pthread_mutex_unlock(&cl_mu);
            break;

            case DELE_GRP_HISTORY:
            pthread_mutex_lock(&cl_mu);
            bzero(send_data->write_buff,sizeof(send_data->write_buff));
            strcpy(send_data->write_buff,recv_data->write_buff);
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
    inet_pton(AF_INET,"127.0.0.1",(void*)&servaddr.sin_addr.s_addr); //将其转化为网络地址
    printf("正在与服务器建立连接...\n");
    if((connect(connfd,(struct sockaddr*)&servaddr,sizeof(servaddr))) < 0){
        my_err("connect",__LINE__);
    }else
    printf("与服务器链接成功！！！\n");
    pthread_create(&tid1,NULL,send_mission,(void*)&connfd);
    pthread_create(&tid2,NULL,recv_mission,(void*)&connfd);
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    return 0;
}