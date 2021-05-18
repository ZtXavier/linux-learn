#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <pwd.h>


#define normal          0  　 //一般的命令
#define out_redirect    1  	　//输出重定向
#define in_redirect     2  	　//输入重定向
#define have_pipe       3  　　//命令中有管道
#define out_redirec     4　　　//命令中有>>

void do_cmd(int argcout, char arglist[100][256]);  //执行ａｒｇｌｉｓｔ中保存的命令
int find_command(char *command);           //　在当前目录，以及/bin、/use/bin下查找命令的可执行程序
void explain_input(char *buf, int *argcout, char arglist[100][256]);//解析ｂｕｆ中的命令，将每个选项存到ａｒｇｌｉｓｔ

char oldpwd[300];//为实现ｃｄ　－，用ｏｌｄｐｗｄ来保存上个路径

int main(int argc, char **argv)
{

    struct passwd *name;
    signal(SIGINT, SIG_IGN);
	int i, len;
    int argcout = 0;        
	char arglist[100][256], temp[256], pwd[100]; 
	char *buf  = NULL, *s = NULL;      

	buf=(char *)malloc(sizeof(char)*256);
    if( buf == NULL ){
		perror("malloc failed");
		exit(-1);
	}
    getcwd(oldpwd, sizeof(oldpwd));//获取当前路径
	
	while(1){
        name = getpwuid(getuid());
        getcwd(pwd, sizeof(pwd) - 1);
        sprintf(temp, "[%s @myshell:\033[0;34m%s\033[0m]$ ", name->pw_name, pwd);//使输出更加美观
		memset(buf,0,256);             
        s = readline(temp);     //用readline()函数读取输入命令
        add_history(s);
        write_history(NULL);

        strcpy(buf, s);//将命令拷贝到ｂｕｆ中
        
        if ( strcmp(buf, "cd") == 0 || strcmp (buf, "cd ") == 0 ) {//若命令中有ｃｄ，则默认为ｃｄ　～
            strcat(buf, " ~");
        }

        len = strlen(buf);
        len++;
        buf[len++] = '\n';
        buf[len] = '\0';
    
       
        if( strcmp(buf,"exit") == 0 || strcmp(buf,"logout") == 0 )//输入ｅｘｉｔ或者ｌｏｇｏｕｔ退出程序
			break;
		
		for( i = 0; i < 100; i++ ) {
			arglist[i][0]='\0';           
		}

		argcout = 0;                        
		explain_input(buf,&argcout,arglist);
		do_cmd(argcout,arglist);           
	}

	if( buf != NULL ){
		free(buf);
		buf = NULL;
	}

    exit(0);
}


void explain_input(char *buf,int *argcout,char arglist[100][256])
{
	int number = 0;
	char *p = buf;
	char *q = buf;

	while(1){
		if( p[0] == '\n' )
			break;
		if( p[0] == ' ' )
			p++;
		else{
			q = p;
			number = 0;
			while( (q[0] != ' ') && (q[0] != '\n') ) {
				number++;
				q++;
			}
			strncpy(arglist[*argcout],p,number+1);   
			arglist[*argcout][number] = '\0';         
			*argcout = *argcout + 1; 
			p = q;
		}
	}
}

void do_cmd(int argcout,char arglist[100][256])
{
	int flag=0;
	int how =0;//用于只是命令中是否含有>,<,>>,|
	int backgroud = 0;//用于命令中是否有&
	int status;
	int i;
	int fd;
	char *arg[argcout+1];
	char *argnext[argcout+1];
	char *file;
	pid_t pid;

	for( i = 0; i < argcout; i++ ) 
		arg[i]=(char *)arglist[i];    

	arg[argcout] = NULL;	

    for( i = 0; i < argcout; i++ ) {
        if( strncmp(arg[i],"&",1) == 0 ) {
            if( i == argcout-1 ) {
                backgroud = 1;
                arg[argcout - 1] =NULL;
                break;
            }
            else {
                printf("wrong command\n");
                return ;
            }
        }

        if ( strcmp(arg[i],"cd") == 0 ) {//判断命令中是否有ｃｄ
            if (  !strcmp(arg[i+1], "~" ) ) {
                strcpy(arg[i+1], "/home/crista");
            }

            if ( strcmp(arg[i+1], "-") == 0 ) {
                strcpy(arg[i+1], oldpwd);
            }
            getcwd(oldpwd, sizeof(oldpwd));
            chdir(arg[i+1]);
            return ;
        }
    }

    for( i = 0; arg[i] != NULL; i++ ){
        if( strcmp(arg[i],">") == 0 ){
            flag++;
            how=out_redirect; 
			if (arg[i+1] == NULL)
				flag++;
		}
		if( strcmp(arg[i],"<") == 0 ){
			flag++;
			how=in_redirect;
			if(i == 0)
				flag++;
		}
		if( strcmp(arg[i],"|") == 0 ) {
			flag++;
			how = have_pipe;
			if(arg[i+1] == NULL)
				flag++;
			if( i == 0)
				flag++;
		}
        if ( strcmp(arg[i],">>") == 0 ){
            flag++;
            how=out_redirec;
            if (arg[i+1] == NULL)
                flag++;
        }
	}

	if(flag > 1){
		printf("wrong command !\n");//命令中只支持一个<,>,>>,|
		return ;
	}

	if(how==out_redirect){
		for( i = 0; arg[i]!=NULL; i++ ) {
			if(strcmp(arg[i],">") == 0) {
				file = arg[i+1];
				arg[i] = NULL;
			}
		}
	}

	if(how == in_redirect){
		for(i=0; arg[i] != NULL;i++){
			if(strcmp(arg[i],"<") == 0){
				file = arg[i+1];
				arg[i] = NULL;
			}
		}
	}

	if(how == have_pipe){
		for( i=0; arg[i]!=NULL ;i++){
			if(strcmp(arg[i],"|") == 0){
				arg[i] = NULL;				
				int j;
				for(j=i+1;arg[j]!=NULL;j++){
					argnext[j-i-1] = arg[j];
				}
				argnext[j-i-1] = arg[j];
				break;
			}
		}
	}

	if( how ==  out_redirec){
		for( i = 0; arg[i]!=NULL; i++ ) {
			if(strcmp(arg[i],">>") == 0) {
				file = arg[i+1];
				arg[i] = NULL;
			}
		}
	}


	if( (pid = fork()) < 0 ) {
		printf("fork error\n");
		return ;
	}
	
	switch(how){
	case 0:
		if(pid == 0){
			if( !(find_command(arg[0])) ){
				printf("%s :command not found !\n",arg[0]);
				exit(0);
			}
			execvp(arg[0],arg);
			exit(0);
		}
		break;
	case 1:
		if(pid == 0){
			if( !(find_command(arg[0])) ){
				printf("%s :command not found !\n",arg[0]);
				exit(0);
			}
			fd = open(file,O_RDWR | O_CREAT | O_TRUNC,0644);
            dup2(fd,1);

			execvp(arg[0],arg);
			exit(0);
		}
		break;
	case 2:
		if(pid == 0){
			if( !(find_command(arg[0])) ){
				printf("%s :command not found !\n",arg[0]);
				exit(0);
			}
			fd = open(file,O_RDONLY);
            dup2(fd,0);
			execvp(arg[0],arg);
            close(fd);
			exit(0);
		}
		break;
	case 3:
		if(pid == 0){
			int pid2;
			int status2;
			int fd2;

			if( (pid2 = fork()) < 0){
				printf("fork2 error\n");
				return ;
			}
			else if(pid2 == 0){
				if(!(find_command)(arg[0])){
                    printf("%s :command not found !\n",arg[0]);
                    exit(0);
                }


                fd2 = open("/tmp/youdonotknowfile",O_WRONLY | O_CREAT | O_TRUNC ,0644);
                dup2(fd2,1);
                execvp(arg[0],arg);
                exit(0);
            }


            if(waitpid(pid2,&status2,0) == -1){
                printf("wait for child process error\n");
            }

            if(!(find_command)(argnext[0])){
                printf("%s :command not found !\n",argnext[0]);
                exit(0);
            }

            fd2 = open("/tmp/youdonotknowfile",O_RDONLY);
            dup2(fd2,0);
            execvp(argnext[0],argnext);

            if( remove("/tmp/youdonotknowfile") )
                printf("remove error\n");
            exit(0);

        }
        break;
    case 4:
        if(pid == 0){
            if( !(find_command(arg[0])) ){
                printf("%s :command not found !\n",arg[0]);
                exit(0);
            }
            fd = open(file,O_RDWR | O_CREAT | O_APPEND,0644);
            dup2(fd,1);

            execvp(arg[0],arg);
            exit(0);
        }
        break;
    default :
        break;
    }
	
    if( backgroud ==1 ) {
        printf("[process id %d]\n",pid);//父进程退出，不等待子进程，表示后台执行
        exit(0);
    }


    if(waitpid(pid,&status,0) == -1){
        printf("wait for child process error!\n");//父进程等待子进程结束
    }


}


int find_command(char *command)
{
	DIR *dp;
	struct dirent*  dirp;
	char *path[] = {"./","/bin","/usr/bin",NULL};

	if(strncmp(command,"./",2) == 0)
		command =command + 2;

	int i=0;
	while(path[i]!=NULL){
		if((dp = opendir(path[i])) == NULL){
            printf("can not open /bin\n");
		}
		while((dirp = readdir(dp)) != NULL){      
			if(strcmp(dirp -> d_name,command) == 0){
				closedir(dp);
				return 1;
			}
		}
		closedir(dp);   
		i++;
	}   
	return 0;
}

