
#include <stdio.h>
//close
#include <unistd.h>
//open
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<ctype.h>
#include <stdlib.h>
//#include <pthread.h>
#include<wait.h>
#include<string.h>
#include<signal.h>
int arg=0;//命令个数
char buf[1024];//读入字符串
char *command[100];//切分后的字符串数组
int pid;//设为全局变量，方便获得子进程的进程号
char *f="temp.txt";//共享文件
void sigcat(){
	kill(pid,SIGINT);
}
//read
void read_command() {
	char *temp=strtok(buf," ");
	int i=0;
	while(temp) {
		command[i++]=temp;
		temp=strtok(NULL," ");
	}
	arg=i;
	command[i]=0;//命令形式的字符串数组最后一位必须是NULL
}

int flag[100][2];//管道的输入输出重定向标记
char *file[100][2]={0};//对应两个重定向的文件
char *argv[100][100];//参数
int ar=0;//管道个数
//解析命令
void analazy_command() {
	ar=0;
	for(int i=0;i<100;i++) {
		flag[i][0]=flag[i][1]=0;
		file[i][0]=file[i][1]=0;
		for(int j=0;j<100;j++) {
			argv[i][j]=0;
		}
	}
	for(int i=0;i<arg;i++) argv[0][i]=command[i];//初始化第一个参数
	argv[0][arg]=NULL;
	int a=0;//当前命令参数的序号
	for(int i=0;i<arg;i++) {
        //判断是否存在管道
		if(strcmp(command[i],"|")==0) {//c语言中字符串比较只能用strcmp函数
			//printf("遇到 | 符号\n");
			argv[ar][a++]=NULL;
			ar++;
			a=0;
		}
		else if(strcmp(command[i],"<")==0) {//存在输入重定向
			flag[ar][0]=1;
			file[ar][0]=command[i+1];
			argv[ar][a++]=NULL;
		}
		else if(strcmp(command[i],">")==0) {//没有管道时的输出重定向
			flag[ar][1]=1;
			file[ar][1]=command[i+1];
			argv[ar][a++]=NULL;//考虑有咩有输入重定向的情况
		}
        else argv[ar][a++]=command[i];
	}
}

//创建子进程，执行命令
int do_command() {
	//printf("seccesee||\n");
	pid=fork();//创建的子进程
	if(pid<0) {
		perror("fork error\n");
        exit(0);
	}
	//先判断是否存在管道，如果有管道，则需要用多个命令参数，并且创建新的子进程。否则一个命令即可
	else if(pid==0) {
		if(!ar) {//没有管道
			if(flag[0][0]) {//判断有无输入重定向
				close(0);
				int fd=open(file[0][0],O_RDONLY);
			}
			if(flag[0][1]) {//判断有无输出重定向
				close(1);
				int fd2=open(file[0][1],O_WRONLY|O_CREAT|O_TRUNC,0666);
			}
			execvp(argv[0][0],argv[0]);
		}
		else {//有管道
            int tt;//记录当前遍历到第几个命令
			for(tt=0;tt<ar;tt++) {
				int pid2=fork();
				if(pid2<0) {
					perror("fork error\n");
					exit(0);
				}
				else if(pid2==0) {
					if(tt) {//如果不是第一个命令，则需要从共享文件读取数据
                        close(0);
					    int fd=open(f,O_RDONLY);//输入重定向
                    }
                    if(flag[tt][0]) {
						close(0);
						int fd=open(file[tt][0],O_RDONLY);
					}
					if(flag[tt][1]) {
						close(1);
						int fd=open(file[tt][1],O_WRONLY|O_CREAT|O_TRUNC,0666);
					}		
                    close(1);
                    remove(f);//由于当前f文件正在open中，会等到解引用后才删除文件
                    int fd=open(f,O_WRONLY|O_CREAT|O_TRUNC,0666);
					if(execvp(argv[tt][0],argv[tt])==-1) {
                        perror("execvp error!\n");
                        exit(0);
                    }
				}
				else {//管道后的命令需要使用管道前命令的结果，因此需要等待
					waitpid(pid2,NULL,0);
				}
			}
            //接下来需要执行管道的最后一条命令
			close(0);
			int fd=open(f,O_RDONLY);//输入重定向
			if(flag[tt][1]) {
				close(1);
				int fd=open(file[tt][1],O_WRONLY|O_CREAT|O_TRUNC,0666);
			}
			execvp(argv[tt][0],argv[tt]);
		}
	}
	//father
	else {
		waitpid(pid,NULL,0);
	}
	return 1;
}
int main(int argc, char *argv[]) {
	signal(SIGINT, &sigcat);//注册信号响应函数
	while(gets(buf))  {    //读入一行字符串
		//初始化
		for(int i=0;i<100;i++) command[i]=0;
		arg=0;//初始化参数个数
		read_command();//将字符串拆分成命令形式的字符串数组
		analazy_command();//分析字符串数组中的各种格式，如管道或者重定向
		do_command();//创建子进程执行命令
	}
	return 0;
} 

