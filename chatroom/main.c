#include <mysql/mysql.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <mysql/mysql.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "pack.h"

#define MAXEPOLL 1024


#define _MY_MYSQL_C



#include <mysql/mysql.h>
#include <stdio.h>




MYSQL accept_mysql(void) {
    MYSQL               mysql;
	
    if (NULL == mysql_init(&mysql)) {
        my_err("mysqlinit", __LINE__);
	}

    // 初始化数据库
    if (mysql_library_init(0, NULL, NULL) != 0) {
        my_err("mysqllibrary_init", __LINE__);
    }

    //连接数据库
    if (NULL == mysql_real_connect(&mysql, "127.0.0.1", "root", "520520cw...", "happy", 0, NULL, 0)) {
        my_err("mysqlrealconnect", __LINE__);
    }

    //设置中文字符集
    if (mysql_set_character_set(&mysql, "utf8") < 0) {
        my_err("mysqlsetcharacter_set", __LINE__);
    }
    
    return mysql;
}

int close_mysql(MYSQL mysql) {
    mysql_close(&mysql);
    mysql_library_end();
    
    return 0;
}









int main() {
	 int                        i;
	 int                        sock_fd;
	 int                        conn_fd;
	 int                        socklen;
	 int                        acceptcont = 0;
	 int                        kdpfd;
	 int                        curfds;
	 int                        nfds;
	 char                       need[MAXIN];
	 MYSQL                      mysql;
	 struct sockaddr_in         cli;
	 struct epoll_event         ev;
	 struct epoll_event         events[MAXEPOLL];
	 PACK                       recv_pack;
	 PACK                       *pack;
	 pthread_t                  pid;
     MYSQL_RES                  *result;
    
     pthread_mutex_init(&mutex, NULL);
	 socklen = sizeof(struct sockaddr_in);
	 mysql = accept_mysql();
	 sock_fd = my_accept_seve();

	 kdpfd = epoll_create(MAXEPOLL);

	 ev.events = EPOLLIN | EPOLLET;
	 ev.data.fd = sock_fd;

	 if(epoll_ctl(kdpfd, EPOLL_CTL_ADD, sock_fd, &ev) < 0) {
	 	my_err("epoll_ctl", __LINE__);
	 }

	 curfds = 1;

	while(1) {
	 	if((nfds = epoll_wait(kdpfd, events, curfds, -1)) < 0){
	 		my_err("epoll_wait", __LINE__);
	 }

 	for (i = 0; i < nfds; i++) { 
		if (events[i].data.fd == sock_fd) {
 			if ((conn_fd = accept(sock_fd, (struct sockaddr*)&cli, &socklen)) < 0) {
 				my_err("accept", __LINE__);
 			}
 			printf("连接成功,套接字编号%d\n", conn_fd);
 			acceptcont++;

 			ev.events = EPOLLIN | EPOLLET;
 			ev.data.fd = conn_fd;

 			if (epoll_ctl(kdpfd, EPOLL_CTL_ADD, conn_fd, &ev) < 0) {
 				my_err("epoll_ctl", __LINE__);
 			}
 			curfds++;
 			continue;
 		} else if (events[i].events & EPOLLIN) { 
			memset(&recv_pack, 0, sizeof(PACK));
 			if (recv(events[i].data.fd, &recv_pack, sizeof(PACK), MSG_WAITALL) < 0) {
 				close(events[i].data.fd);
 				perror("recv");
 				continue;
 			}
            if (recv_pack.type == EXIT) {
                if (send(events[i].data.fd, &recv_pack, sizeof(PACK), 0) < 0) {
                    my_err("send", __LINE__);
                }
                memset(need, 0, sizeof(need));
                sprintf(need, "update user_data set user_state = 0 where user_state = 1 and user_socket = %d", events[i].data.fd);
                mysql_query(&mysql, need);
                epoll_ctl(kdpfd, EPOLL_CTL_DEL, events[i].data.fd, &ev);
                curfds--;
                
                continue;
            }
			if (recv_pack.type == LOGIN) {
 		    	memset(need, 0, sizeof(need));
                sprintf(need, "select *from user_data where account = %d", recv_pack.data.send_account);
                pthread_mutex_lock(&mutex);
                mysql_query(&mysql, need);
                result = mysql_store_result(&mysql);
                if (!mysql_fetch_row(result)) {
                    recv_pack.type = ACCOUNT_ERROR;
                    memset(recv_pack.data.write_buff, 0, sizeof(recv_pack.data.write_buff));
                    printf("$$sad\n");
                    strcpy(recv_pack.data.write_buff, "password error");
                    if (send(events[i].data.fd, &recv_pack, sizeof(PACK), 0) < 0) {
                        my_err("send", __LINE__);
                    }
                    pthread_mutex_unlock(&mutex);
                    continue;
                }
 		    	memset(need, 0, sizeof(need)); 
 		    	sprintf(need, "update user_data set user_socket = %d where account = %d", events[i].data.fd, recv_pack.data.send_account);
 	    		mysql_query(&mysql, need); 
                pthread_mutex_unlock(&mutex);
            }
            recv_pack.data.recv_fd = events[i].data.fd;
 			pack = (PACK*)malloc(sizeof(PACK));
 			memcpy(pack, &recv_pack, sizeof(PACK));
 			pthread_create(&pid, NULL, deal, (void*)pack);
        }
    }
} 
}
void *deal(void *recv_pack) {
    pthread_detach(pthread_self());
	PACK               *pack;
	int                i;
    BOX                *tmp = box_head;
	MYSQL              mysql;
	mysql = accept_mysql();
	pack = (PACK*)recv_pack;
	switch(pack->type)
	{
		case LOGIN:
			{
				if (login(pack, mysql) != 0) {
                    pack->type = ACCOUNT_ERROR;
                    memset(pack->data.write_buff, 0, sizeof(pack->data.write_buff));
					strcpy(pack->data.write_buff, "password error");
                    if (send(pack->data.recv_fd, pack, sizeof(PACK), 0) < 0) {
                        my_err("send", __LINE__);
                    }
				} else {
                    memset(pack->data.write_buff, 0, sizeof(pack->data.write_buff));
                    strcpy(pack->data.write_buff, "good");
                    int ret;
                    if ((ret = send(pack->data.recv_fd, pack, sizeof(PACK), 0)) < 0) {
                        my_err("send", __LINE__);
                    }
                    while (tmp != NULL) {
                        if (tmp->recv_account == pack->data.send_account) {
                            break;
                        }
                        tmp = tmp->next;
                    }
                    if (tmp == NULL) {
                        tmp = (BOX *)malloc(sizeof(BOX));
                        tmp->recv_account = pack->data.send_account;
                        tmp->talk_number = tmp->friend_number = 0;
                        tmp->number = 0;
                        tmp->next = NULL;
                        if (box_head == NULL) {
                            box_head = box_tail = tmp;
                        } else {
                            box_tail->next = tmp;
                            box_tail = tmp;
                        }
                        if (send(pack->data.recv_fd, tmp, sizeof(BOX), 0) < 0) {
                            my_err("send", __LINE__);
                        }
                    } else {
                        if (send(pack->data.recv_fd, tmp, sizeof(BOX), 0) < 0) {
                            my_err("send", __LINE__);
                        }
                        tmp->friend_number = 0;
                        tmp->talk_number = 0;
                    }
                }
                break;
			}
        case REGISTERED:
            {
                registered(pack, mysql);
                memset(pack->data.write_buff, 0, sizeof(pack->data.write_buff));
                strcpy(pack->data.write_buff, "registered success!!");
                if (send(pack->data.recv_fd, pack, sizeof(PACK), 0) < 0) {
                    my_err("send", __LINE__);
                }
                
                break;
            }
        case CHANGE_PASSWORD:
            {
                if (change_password(pack, mysql) == 0) {
                    memset(pack->data.write_buff, 0, sizeof(pack->data.write_buff));
                    strcpy(pack->data.write_buff, "success");
                    if (send(pack->data.recv_fd, pack, sizeof(PACK), 0) < 0) {
                        my_err("send", __LINE__);
                    }
                } else {
                    memset(pack->data.write_buff, 0, sizeof(pack->data.write_buff));
                    strcpy(pack->data.write_buff, "fail");
                    if (send(pack->data.recv_fd, pack, sizeof(PACK), 0) < 0) {
                        my_err("send", __LINE__);        
                    }
                }
                break;
            }
        case ADD_FRIEND:
            {
                if (add_fir(pack, mysql) == 0) {
                    pack->type = ADD_FRIEND;
                    memset(pack->data.write_buff, 0, sizeof(pack->data.write_buff));
                    strcpy(pack->data.write_buff, "success");
                    if (send(pack->data.recv_fd, pack, sizeof(PACK) , 0) < 0) {
                        my_err("send", __LINE__);
                    }
                } else {
                    memset(pack->data.write_buff, 0, sizeof(pack->data.write_buff));
                    strcpy(pack->data.write_buff, "fail");
                    if (send(pack->data.recv_fd, pack, sizeof(PACK), 0) < 0) {
                        my_err("send", __LINE__);        
                    } 
                }
                break;
            }
        case FRIENDS_PLZ:
            {
                friends_plz(pack, mysql);
                break;
            }
        case DEL_FRIEND:
            {
                if (del_friend(pack, mysql) == 0) {
                    memset(pack->data.write_buff, 0, sizeof(pack->data.write_buff));
                    strcpy(pack->data.write_buff, "success");
                    if (send(pack->data.send_fd, pack, sizeof(PACK), 0) < 0) {
                        my_err("send", __LINE__);
                    }
                } else {
                    memset(pack->data.write_buff, 0, sizeof(pack->data.write_buff));
                    strcpy(pack->data.write_buff, "fail");
                    if (send(pack->data.send_fd, pack, sizeof(PACK), 0) < 0) {
                        my_err("send", __LINE__);
                    }
                }
                break;
            }
        case BLACK_FRIEND:
            {
                if (black_friend(pack, mysql) == 0) {
                    memset(pack->data.write_buff, 0, sizeof(pack->data.write_buff));
                    strcpy(pack->data.write_buff, "success");
                    if (send(pack->data.send_fd, pack, sizeof(PACK), 0) < 0) {
                        my_err("send", __LINE__);
                    }
                } else {
                    memset(pack->data.write_buff, 0, sizeof(pack->data.write_buff));
                    strcpy(pack->data.write_buff, "fail");
                    if (send(pack->data.send_fd, pack, sizeof(PACK), 0) < 0) {
                        my_err("send", __LINE__);
                    } 
                }
                break;
            }
        case WHITE_FRIEND:
            {
                if (white_friend(pack, mysql) == 0) {
                    memset(pack->data.write_buff, 0, sizeof(pack->data.write_buff));
                    strcpy(pack->data.write_buff, "success");
                    if (send(pack->data.send_fd, pack, sizeof(PACK), 0) < 0) {
                        my_err("send", __LINE__);
                    }
                } else {
                    memset(pack->data.write_buff, 0, sizeof(pack->data.write_buff));
                    strcpy(pack->data.write_buff, "fail");
                    if (send(pack->data.send_fd, pack, sizeof(PACK), 0) < 0) {
                        my_err("send", __LINE__);
                    } 
                }
                break;
            }
        case CARE_FRIEND:
            {
                if (care_friend(pack, mysql) == 0) {
                    memset(pack->data.write_buff, 0, sizeof(pack->data.write_buff));
                    strcpy(pack->data.write_buff, "success");
                    if (send(pack->data.send_fd, pack, sizeof(PACK), 0) < 0) {
                        my_err("send", __LINE__);
                    }
                } else {
                    memset(pack->data.write_buff, 0, sizeof(pack->data.write_buff));
                    strcpy(pack->data.write_buff, "fail");
                    if (send(pack->data.send_fd, pack, sizeof(PACK), 0) < 0) {
                        my_err("send", __LINE__);
                    } 
                }
                break;
            }
        case DISCARE_FRIEND:
            {
                if (discare_friend(pack, mysql) == 0) {
                    memset(pack->data.write_buff, 0, sizeof(pack->data.write_buff));
                    strcpy(pack->data.write_buff, "success");
                    if (send(pack->data.send_fd, pack, sizeof(PACK), 0) < 0) {
                        my_err("send", __LINE__);
                    }
                } else {
                    memset(pack->data.write_buff, 0, sizeof(pack->data.write_buff));
                    strcpy(pack->data.write_buff, "fail");
                    if (send(pack->data.send_fd, pack, sizeof(PACK), 0) < 0) {
                        my_err("send", __LINE__);
                    } 
                }
                break;
            }
        case LOOK_LIST:
            {
                FRIEND *list = look_list(pack, mysql);
                if (list->friend_number != 0) {
                    memset(pack->data.write_buff, 0, sizeof(pack->data.write_buff));
                    strcpy(pack->data.write_buff, "success");
                    if (send(pack->data.send_fd, pack, sizeof(PACK), 0) < 0) {
                        my_err("send", __LINE__);
                    }
                    if (send(pack->data.send_fd, list, sizeof(FRIEND), 0) < 0) {
                        my_err("send", __LINE__);
                    }
                } else {
                    memset(pack->data.write_buff, 0, sizeof(pack->data.write_buff));
                    strcpy(pack->data.write_buff, "fail");
                    if (send(pack->data.send_fd, pack, sizeof(PACK), 0) < 0) {
                        my_err("send", __LINE__);
                    }
                    if (send(pack->data.send_fd, list, sizeof(FRIEND), 0) < 0) {
                        my_err("send", __LINE__);
                    }
                }
                break;       
            }
        case SEND_FMES:
            {
                if (send_fmes(pack, mysql) == -1) {
                    memset(pack->data.write_buff, 0, sizeof(pack->data.write_buff));
                    strcpy(pack->data.write_buff, "#fail");
                    if (send(pack->data.send_fd, pack, sizeof(PACK), 0) < 0) {
                        my_err("send", __LINE__);
                    }
                } else {
                    memset(pack->data.write_buff, 0, sizeof(pack->data.write_buff));
                    strcpy(pack->data.write_buff, "#succss");
                    if (send(pack->data.send_fd, pack, sizeof(PACK), 0) < 0) {
                        my_err("send", __LINE__);
                    }
                }
                break;
            }
        case SEND_GMES:
            {
                if (send_gmes(pack, mysql) == -1) {
                    memset(pack->data.write_buff, 0, sizeof(pack->data.write_buff));
                    strcpy(pack->data.write_buff, "#fail");
                    if (send(pack->data.send_fd, pack, sizeof(PACK), 0) < 0) {
                        my_err("send", __LINE__);
                    }
                } else {
                    memset(pack->data.write_buff, 0, sizeof(pack->data.write_buff));
                    strcpy(pack->data.write_buff, "#succss");
                    if (send(pack->data.send_fd, pack, sizeof(PACK), 0) < 0) {
                        my_err("send", __LINE__);
                    }
                }
                break;
            }
        case READ_MESSAGE:
            {
                read_message(pack, mysql);
                break;
            }
        case DEL_MESSAGE:
            {
                if (del_message(pack, mysql) == 0) {
                    memset(pack->data.write_buff, 0, sizeof(pack->data.write_buff));
                    strcpy(pack->data.write_buff, "success");
                    if (send(pack->data.send_fd, pack, sizeof(PACK), 0) < 0) {
                        my_err("send", __LINE__);
                    }
                } else {
                    memset(pack->data.write_buff, 0, sizeof(pack->data.write_buff));
                    strcpy(pack->data.write_buff, "fail");
                    if (send(pack->data.send_fd, pack, sizeof(PACK), 0) < 0) {
                        my_err("send", __LINE__);
                    } 
                }
                break;    
            }
        case CREATE_GROUP:
            {
                if (create_group(pack, mysql) == 0) {
                    memset(pack->data.write_buff, 0, sizeof(pack->data.write_buff));
                    strcpy(pack->data.write_buff, "success");
                    printf("%s\n", pack->data.write_buff);
                    if (send(pack->data.send_fd, pack, sizeof(PACK), 0) < 0) {
                        my_err("send", __LINE__);
                    }
                } else {
                    memset(pack->data.write_buff, 0, sizeof(pack->data.write_buff));
                    strcpy(pack->data.write_buff, "fail");
                    if (send(pack->data.send_fd, pack, sizeof(PACK), 0) < 0) {
                        my_err("send", __LINE__);
                    } 
                }
                break; 
            }
        case ADD_GROUP:
            {
               if (add_group(pack, mysql) == 0) {
                    memset(pack->data.write_buff, 0, sizeof(pack->data.write_buff));
                    strcpy(pack->data.write_buff, "success");
                    printf("%s\n", pack->data.write_buff);
                    if (send(pack->data.send_fd, pack, sizeof(PACK), 0) < 0) {
                        my_err("send", __LINE__);
                    }
                } else {
                    memset(pack->data.write_buff, 0, sizeof(pack->data.write_buff));
                    strcpy(pack->data.write_buff, "fail");
                    if (send(pack->data.send_fd, pack, sizeof(PACK), 0) < 0) {
                        my_err("send", __LINE__);
                    } 
                }
                break;  
            }
        case EXIT_GROUP:
            {
                if (exit_group(pack, mysql) == 0) {
                    memset(pack->data.write_buff, 0, sizeof(pack->data.write_buff));
                    strcpy(pack->data.write_buff, "success");
                    printf("%s\n", pack->data.write_buff);
                    if (send(pack->data.send_fd, pack, sizeof(PACK), 0) < 0) {
                        my_err("send", __LINE__);
                    }
                } else {
                    memset(pack->data.write_buff, 0, sizeof(pack->data.write_buff));
                    strcpy(pack->data.write_buff, "fail");
                    if (send(pack->data.send_fd, pack, sizeof(PACK), 0) < 0) {
                        my_err("send", __LINE__);
                    } 
                }
                break; 
            }
        case SET_ADMIN:
            {
                if (set_admin(pack, mysql) == 0) {
                    memset(pack->data.write_buff, 0, sizeof(pack->data.write_buff));
                    strcpy(pack->data.write_buff, "success");
                    printf("%s\n", pack->data.write_buff);
                    if (send(pack->data.send_fd, pack, sizeof(PACK), 0) < 0) {
                        my_err("send", __LINE__);
                    }
                } else {
                    memset(pack->data.write_buff, 0, sizeof(pack->data.write_buff));
                    strcpy(pack->data.write_buff, "fail");
                    if (send(pack->data.send_fd, pack, sizeof(PACK), 0) < 0) {
                        my_err("send", __LINE__);
                    } 
                }
                break; 
            }
        case DEL_ADMIN:
            {
                if (del_admin(pack, mysql) == 0) {
                    memset(pack->data.write_buff, 0, sizeof(pack->data.write_buff));
                    strcpy(pack->data.write_buff, "success");
                    printf("%s\n", pack->data.write_buff);
                    if (send(pack->data.send_fd, pack, sizeof(PACK), 0) < 0) {
                        my_err("send", __LINE__);
                    }
                } else {
                    memset(pack->data.write_buff, 0, sizeof(pack->data.write_buff));
                    strcpy(pack->data.write_buff, "fail");
                    if (send(pack->data.send_fd, pack, sizeof(PACK), 0) < 0) {
                        my_err("send", __LINE__);
                    } 
                }
                break; 
            }
        case LOOK_MEMBER:
            {
                look_member(pack, mysql);
                break;
            }
        case LOOK_GROUP_LIST:
            {
                look_group_list(pack, mysql);
                break;
            }
        case FIND_PASSWORD:
            {
                find_password(pack, mysql);
                break;
            }
        case DEL_MEMBER:
            {
                 if (del_member(pack, mysql) == 0) {
                    memset(pack->data.write_buff, 0, sizeof(pack->data.write_buff));
                    strcpy(pack->data.write_buff, "success");
                    if (send(pack->data.send_fd, pack, sizeof(PACK), 0) < 0) {
                        my_err("send", __LINE__);
                    }
                } else {
                    memset(pack->data.write_buff, 0, sizeof(pack->data.write_buff));
                    strcpy(pack->data.write_buff, "fail");
                    if (send(pack->data.send_fd, pack, sizeof(PACK), 0) < 0) {
                        my_err("send", __LINE__);
                    } 
                }
                break;   
            }
        case SEND_FILE:
            {
                pthread_mutex_lock(&mutex);
                int fd = open("2", O_WRONLY|O_CREAT|O_APPEND, S_IRUSR|S_IWUSR|S_IXUSR);
                write(fd, pack->data.read_buff, 1023);
                close(fd);
                printf("%d\n", pack->data.cont);
                if (send(pack->data.send_fd, pack, sizeof(PACK), 0) < 0) {
                    my_err("send", __LINE__);
                }
                pthread_mutex_unlock(&mutex);
                break;
            }
        case READ_FILE:
            {
               pthread_mutex_lock(&mutex);
               int fd = open("2", O_RDONLY);
               lseek(fd, 1023*pack->data.cont, SEEK_SET);
               memset(pack->data.read_buff, 0, sizeof(pack->data.read_buff));
               if (read(fd, pack->data.read_buff, 1023) == 0) {
                   strcpy(pack->data.write_buff, "ok");
               }
               if (send(pack->data.send_fd, pack, sizeof(PACK), 0) < 0) {
                    my_err("send", __LINE__);
               }
               close(fd);
               pthread_mutex_unlock(&mutex);
               break;
            }
        case OK_FILE:
            {
                    printf("sssssss^^\n");
                ok_file(pack, mysql);
                break;
            }
       
	}
	close_mysql(mysql);
}


int login(PACK *pack, MYSQL mysql1) {
    int                i;
    int                ret;
    int                cont = 0;
    char               inedx[100];	
    PACK               *recv_pack = pack;
    MYSQL              mysql = mysql1;
    MYSQL_RES          *result = NULL;
    MYSQL_ROW          row;

    sprintf(inedx, "select *from user_data where account = %d", recv_pack->data.send_account);
    pthread_mutex_lock(&mutex);	
    ret = mysql_query(&mysql, inedx);

    if (!ret) {
        result = mysql_store_result(&mysql);
        row = mysql_fetch_row(result);
        if (row == NULL) {
            pthread_mutex_unlock(&mutex);
            return -1;
        }
        if (strcmp(row[2], recv_pack->data.read_buff) == 0) {
            strcpy(recv_pack->data.send_user, row[1]);
            memset(inedx, 0, sizeof(inedx));
            pack->data.recv_fd = atoi(row[4]);
            sprintf(inedx, "update user_data set user_state = 1 where account = %d", pack->data.send_account);
            mysql_query(&mysql, inedx);
            mysql_free_result(result);
            pthread_mutex_unlock(&mutex);
                
            return 0;
        } else {      
            pack->data.recv_fd = atoi(row[4]);
            pthread_mutex_unlock(&mutex);
            return -1;
        }
    } else {
        printf("query fail\n");
        pthread_mutex_unlock(&mutex);
        
        return -1;
    }    
}

int registered(PACK *pack, MYSQL mysql1) {
    FILE               *fp;
    MYSQL              mysql = mysql1;	
    char               need[100];
    PACK               *recv_pack = pack;
    int                user_number;
	
    pthread_mutex_lock(&mutex);	
    /* 在user_number.txt文件中存放着用户的个数,数量从1000000开始 */
    if ((fp = fopen("user_number.txt", "r")) == NULL) {
        printf("打开文件失败\n");
        exit(-1);
    }
    fread(&user_number, sizeof(int), 1, fp);
	
    sprintf(need, "insert into user_data values(%d,\"%s\",\"%s\",%d,%d)", user_number++, recv_pack->data.send_user, recv_pack->data.read_buff, 0, recv_pack->data.recv_fd);
    recv_pack->data.send_account = user_number-1;
    mysql_query(&mysql, need);
    fclose(fp);
    if ((fp = fopen("user_number.txt", "w")) == NULL) {
        printf("打开文件失败\n");
        exit(-1);
    }
    fwrite(&user_number, sizeof(int), 1, fp);
    fclose(fp);
    pthread_mutex_unlock(&mutex);

    return 0;
}

int change_password(PACK *pack, MYSQL mysql1) {
    MYSQL_RES          *result = NULL;
    PACK               *recv_pack = pack;
    MYSQL              mysql = mysql1;
    char               need[100];
    MYSQL_ROW          row;

    sprintf(need, "select *from user_data where account = %d", recv_pack->data.send_account);
    pthread_mutex_lock(&mutex);
    mysql_query(&mysql, need);
    result = mysql_store_result(&mysql);
    row = mysql_fetch_row(result);
    if (row) {
        if (strcmp(recv_pack->data.read_buff, row[2]) == 0) {
            recv_pack->data.recv_fd = atoi(row[4]);
            memset(need, 0, sizeof(need));
            sprintf(need, "update user_data set password = \"%s\" where account = %d", recv_pack->data.write_buff, recv_pack->data.send_account);
            mysql_query(&mysql, need);
            pthread_mutex_unlock(&mutex);
            
            return 0;
        } else {
            pthread_mutex_unlock(&mutex);
            
            return -1;
        }
    } else {
        pthread_mutex_unlock(&mutex);

        return -1;
    }

    return -1;
}

int find_password(PACK *pack, MYSQL mysql1) {
    MYSQL           mysql = mysql1;
    MYSQL_RES       *result;
    MYSQL_ROW       row;
    char            need[150];
    PACK            *recv_pack = pack;
    int             cont = 0;

    pthread_mutex_lock(&mutex);
    sprintf(need, "select *from friends where user = %d", recv_pack->data.send_account);
    mysql_query(&mysql, need);
    result = mysql_store_result(&mysql);
    while (row = mysql_fetch_row(result)) {
        cont++;
    }
    if (cont != recv_pack->data.recv_account) {
        strcpy(recv_pack->data.write_buff, "你找不回来你的密码了！！");
        memset(need, 0, sizeof(need));
        sprintf(need, "select *from user_data where account = %d", recv_pack->data.send_account);
        mysql_query(&mysql, need);
        result = mysql_store_result(&mysql);
        row = mysql_fetch_row(result);
        if (!row) {
            strcpy(recv_pack->data.write_buff, "你的账号都是错的哦!!");
        }
        if (send(recv_pack->data.recv_fd, recv_pack, sizeof(PACK), 0) < 0) {
            my_err("send", __LINE__);
        }
    } else {
        memset(need, 0, sizeof(need));
        sprintf(need, "select *from user_data where account = %d", recv_pack->data.send_account);
        mysql_query(&mysql, need);
        result = mysql_store_result(&mysql);
        row = mysql_fetch_row(result);
        if (!row) {
            strcpy(recv_pack->data.write_buff, "你的账号都是错的哦!!");
        } else {
            sprintf(recv_pack->data.write_buff, "你的密码是%s", row[2]);
        }
        if (send(recv_pack->data.recv_fd, recv_pack, sizeof(PACK), 0) < 0) {
            my_err("send", __LINE__);
        }
    }
    
    pthread_mutex_unlock(&mutex);
    return 0;
}



int add_fir(PACK *pack, MYSQL mysql1) {
    PACK            *recv_pack = pack;
    MYSQL           mysql = mysql1;
    MYSQL_RES       *result;
    MYSQL_ROW       row, row1;
    BOX             *tmp = box_head;
    int             ret;
    char            need[100];
    
    sprintf(need, "select *from user_data where account = %d", recv_pack->data.recv_account);
    pthread_mutex_lock(&mutex);
    ret = mysql_query(&mysql, need);
    if (!ret) {
        result = mysql_store_result(&mysql);
        row = mysql_fetch_row(result);
        if (row == NULL) {
            pthread_mutex_unlock(&mutex);
            return -1;
        }
        memset(need, 0, sizeof(need));
        sprintf(need, "select *from friends where user = %d and friend_user = %d", recv_pack->data.send_account, recv_pack->data.recv_account);
        ret = mysql_query(&mysql, need);
        if (!ret) {
            // 如果可以查到数据说明对方已经是好友了 
            result = mysql_store_result(&mysql);
            row1 = mysql_fetch_row(result);
            if (row1 != NULL) {
                pthread_mutex_unlock(&mutex);
                return -1;
            }
            memset(need, 0, sizeof(need));
            sprintf(need, "账号为%d,昵称为%s的用户发来好友请求\n", recv_pack->data.send_account, recv_pack->data.send_user);
            if (atoi(row[3]) == 0) {
                while (tmp) {
                    if (tmp->recv_account == recv_pack->data.recv_account) {
                        break;
                    }
                    tmp = tmp->next;
                }
                if (tmp != NULL) {
                    tmp->plz_account[tmp->friend_number] = recv_pack->data.send_account;
                    strcpy(tmp->write_buff[tmp->friend_number], need);
                    tmp->friend_number++;
                } else {
                    tmp = (BOX *)malloc(sizeof(BOX));
                    tmp->recv_account = recv_pack->data.recv_account;
                    tmp->friend_number = 0;
                    tmp->talk_number = 0;
                    strcpy(tmp->write_buff[tmp->friend_number], need);
                    tmp->plz_account[tmp->friend_number++] = recv_pack->data.send_account;
                    if (box_head == NULL) {
                        box_head = box_tail = tmp;
                        box_tail->next = NULL;
                    } else {
                        box_tail->next = tmp;
                        box_tail = tmp;
                        box_tail->next = NULL;
                    }
                }
                pthread_mutex_unlock(&mutex);
                return 0;
            } else {
                recv_pack->data.send_fd = atoi(row[4]);
                strcpy(recv_pack->data.recv_user, row[1]);
                strcpy(recv_pack->data.read_buff, need);
                recv_pack->type = FRIENDS_PLZ;
                if (send(recv_pack->data.send_fd, recv_pack, sizeof(PACK), 0) < 0) {
                    my_err("send", __LINE__);
                } 
                pthread_mutex_unlock(&mutex);
                return 0;
            }
        }
    }
}

int friends_plz(PACK *pack, MYSQL mysql1) {
    char            need[100]; 
    MYSQL           mysql = mysql1;
    PACK            *recv_pack = pack;
    
    pthread_mutex_lock(&mutex);
    sprintf(need, "insert into friends values(%d,%d,0)", recv_pack->data.send_account, recv_pack->data.recv_account);
    mysql_query(&mysql, need);
    memset(need, 0, sizeof(need));
    sprintf(need, "insert into friends values(%d,%d,0)", recv_pack->data.recv_account, recv_pack->data.send_account);
    mysql_query(&mysql, need);
    pthread_mutex_unlock(&mutex);
    
    return 0;
}   
     
int del_friend(PACK *pack, MYSQL mysql1) {
    PACK            *recv_pack = pack;
    MYSQL           mysql = mysql1;
    char            need[100];
    MYSQL_ROW       row;
    MYSQL_RES       *result;

    pthread_mutex_lock(&mutex);
    sprintf(need, "select *from friends where user = %d and friend_user = %d", recv_pack->data.send_account, recv_pack->data.recv_account);
    mysql_query(&mysql, need);
    result = mysql_store_result(&mysql);
    row = mysql_fetch_row(result);
    if (row == NULL) {
        pthread_mutex_unlock(&mutex);    
        return -1;
    }
    memset(need, 0, sizeof(need));
    sprintf(need, "delete from friends where user = %d and friend_user = %d", recv_pack->data.send_account, recv_pack->data.recv_account);
    mysql_query(&mysql, need);
    memset(need, 0, sizeof(need));
    sprintf(need, "delete from friends where user = %d and friend_user = %d", recv_pack->data.recv_account, recv_pack->data.send_account);
    mysql_query(&mysql, need);
    pthread_mutex_unlock(&mutex);
    
    return 0;
}

int black_friend(PACK *pack, MYSQL mysql1) {
    PACK            *recv_pack = pack;
    MYSQL           mysql = mysql1;
    char            need[100];
    MYSQL_ROW       row;
    MYSQL_RES       *result;
    
    pthread_mutex_lock(&mutex);
    sprintf(need, "select *from friends where user = %d and friend_user = %d", recv_pack->data.send_account, recv_pack->data.recv_account);
    mysql_query(&mysql, need);
    result = mysql_store_result(&mysql);
    row = mysql_fetch_row(result);
    if (row == NULL) {
        pthread_mutex_unlock(&mutex);    
        return -1;
    }
    memset(need, 0, sizeof(need));
    sprintf(need, "update friends set realtion = -1 where user = %d and friend_user = %d", recv_pack->data.send_account, recv_pack->data.recv_account);
    mysql_query(&mysql, need);
    pthread_mutex_unlock(&mutex);

    return 0;
}

int white_friend(PACK *pack, MYSQL mysql1) {
    PACK            *recv_pack = pack;
    MYSQL           mysql = mysql1;
    char            need[100];
    MYSQL_ROW       row;
    MYSQL_RES       *result;

    pthread_mutex_lock(&mutex);
    sprintf(need, "select *from friends where user = %d and friend_user = %d", recv_pack->data.send_account, recv_pack->data.recv_account);
    mysql_query(&mysql, need);
    result = mysql_store_result(&mysql);
    row = mysql_fetch_row(result);
    if (row == NULL) {
        pthread_mutex_unlock(&mutex);    
        return -1;
    }
    if (atoi(row[2]) == OK) {
        pthread_mutex_unlock(&mutex);
        return 0;
    }
    memset(need, 0, sizeof(need));
    sprintf(need, "update friends set realtion = 0 where user = %d and friend_user = %d", recv_pack->data.send_account, recv_pack->data.recv_account);
    mysql_query(&mysql, need);
    pthread_mutex_unlock(&mutex);

    return 0;
}

int care_friend(PACK *pack, MYSQL mysql1) {
    PACK            *recv_pack = pack;
    MYSQL           mysql = mysql1;
    char            need[100];
    MYSQL_RES       *result;
    MYSQL_ROW       row;

    pthread_mutex_lock(&mutex);
    sprintf(need, "select *from friends where user = %d and friend_user = %d", recv_pack->data.send_account, recv_pack->data.recv_account);
    mysql_query(&mysql, need);
    result = mysql_store_result(&mysql);
    row = mysql_fetch_row(result);
    if (row == NULL) {
        pthread_mutex_unlock(&mutex);    
        return -1;
    }
    memset(need, 0, sizeof(need));
    sprintf(need, "update friends set realtion = 1 where user = %d and friend_user = %d", recv_pack->data.send_account, recv_pack->data.recv_account);
    mysql_query(&mysql, need);
    pthread_mutex_unlock(&mutex);
}

int discare_friend(PACK *pack, MYSQL mysql1) {
    PACK            *recv_pack = pack;
    MYSQL           mysql = mysql1;
    char            need[100];
    MYSQL_RES       *result;
    MYSQL_ROW       row;

    pthread_mutex_lock(&mutex);
    sprintf(need, "select *from friends where user = %d and friend_user = %d", recv_pack->data.send_account, recv_pack->data.recv_account);
    mysql_query(&mysql, need);
    result = mysql_store_result(&mysql);
    row = mysql_fetch_row(result);
    if (row == NULL) {
        pthread_mutex_unlock(&mutex);    
        return -1;
    }
    memset(need, 0, sizeof(need));
    sprintf(need, "update friends set realtion = 0 where user = %d and friend_user = %d", recv_pack->data.send_account, recv_pack->data.recv_account);
    mysql_query(&mysql, need);
    pthread_mutex_unlock(&mutex);
}

FRIEND *look_list(PACK *pack, MYSQL mysql1) {
    PACK            *recv_pack = pack;
    MYSQL           mysql = mysql1;
    char            need[100];
    MYSQL_RES       *result, *result1;
    MYSQL_ROW       row, row1;
    FRIEND          *list;

    list = (FRIEND *)malloc(sizeof(FRIEND));
    list->friend_number = 0;
    pthread_mutex_lock(&mutex);
    sprintf(need, "select *from friends where user = %d", recv_pack->data.send_account);
    mysql_query(&mysql, need);
    result = mysql_store_result(&mysql);
    while (row = mysql_fetch_row(result)) {
        list->friend_account[list->friend_number] = atoi(row[1]);
        memset(need, 0, sizeof(need));
        sprintf(need, "select *from user_data where account = %d", atoi(row[1]));
        mysql_query(&mysql, need);
        result1 = mysql_store_result(&mysql);
        row1 = mysql_fetch_row(result1);
        strcpy(list->friend_nickname[list->friend_number], row1[1]);
        list->friend_state[list->friend_number++] = atoi(row1[3]);
    }
    if (list->friend_number == 0) {
        pthread_mutex_unlock(&mutex);
        return list;
    } else {
        pthread_mutex_unlock(&mutex);
        return list;
    }
}