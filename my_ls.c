#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <linux/limits.h>
#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <errno.h>
#include <signal.h>

#define PARAM_NONE 0
#define PARAM_A 1	  //参数a
#define PARAM_L 2	  //参数l
#define PARAM_I 4	  //参数i
#define PARAM_R 8	  //参数r
#define PARAM_T 16	  //参数t
#define PARAM_RR 32	  //参数R
#define PARAM_S 64	  //参数s
#define MAXROWLEN 128 //每行所用最大格数

int g_maxlen; //最长文件名长度
int g_leave_len = MAXROWLEN;
int total = 0; //文件的大小总和
int l = 0;	   //每行已输出文件名的个数，用来判断是否换行
int fpnum = 4; //一行输出文件名的个数

void list_dir(char *pathname, int param);
void display_DIR(DIR *ret_opendir, int filecolor);

/******************错误函数********************/
void my_err(const char *err_string, int line)
{
	fprintf(stderr, "line:%d ", line);
	perror(err_string);
	exit(1);
}

/*********************************************
功能 ：输出文件信息  参数  -l
参数1  文件信息stat结构体
参数2  文件名name
参数3  文件显示颜色filecolor
 *********************************************/
void display_attribute(struct stat buf, char *name, int filecolor)
{
	char  buff_link[256];
	char  colorname_link[NAME_MAX + 30];
	char colorname[NAME_MAX + 30];
	char buf_time[32];
	struct passwd *psd;
	struct group *grp;
	if (S_ISLNK(buf.st_mode))
	{ //文件类型
		printf("l");
	}
	else if (S_ISREG(buf.st_mode))
	{
		printf("-");
	}
	else if (S_ISDIR(buf.st_mode))
	{
		printf("d");
	}
	else if (S_ISCHR(buf.st_mode))
	{
		printf("c");
	}
	else if (S_ISBLK(buf.st_mode))
	{
		printf("b");
	}
	else if (S_ISFIFO(buf.st_mode))
	{
		printf("f");
	}
	else if (S_ISSOCK(buf.st_mode))
	{
		printf("s");
	}
	//文件权限
	//拥有者权限
	if (buf.st_mode & S_IRUSR)
		printf("r");
	else
		printf("-");
	if (buf.st_mode & S_IWUSR)
		printf("w");
	else
		printf("-");
	if (buf.st_mode & S_IXUSR)
	{
		printf("x");
	}
	else
		printf("-");
	//组权限
	if (buf.st_mode & S_IRGRP)
		printf("r");
	else
		printf("-");
	if (buf.st_mode & S_IWGRP)
		printf("w");
	else
		printf("-");
	if (buf.st_mode & S_IXGRP)
	{
		printf("x");
	}
	else
		printf("-");
	//其他用户权限
	if (buf.st_mode & S_IROTH)
		printf("r");
	else
		printf("-");
	if (buf.st_mode & S_IWOTH)
		printf("w");
	else
		printf("-");
	if (buf.st_mode & S_IXOTH)
	{
		printf("x");
	}
	else
		printf("-");
	printf("\t");
	//通过用户和组id得到用户的信息和其所在组的信息
	readlink(name,buff_link,sizeof(buff_link));
	psd = getpwuid(buf.st_uid);
	grp = getgrgid(buf.st_gid);
	printf("%2d ", buf.st_nlink);			//打印文件的硬链接数
	printf("%-5s", psd->pw_name);			//打印用户的名字
	printf("%-5s", grp->gr_name);			//打印用户组的名字
	printf("%6d", buf.st_size);				//打印文件大小
	strcpy(buf_time, ctime(&buf.st_mtime)); //把时间转换成普通表示格式
	buf_time[strlen(buf_time) - 1] = '\0';	//去掉换行符
	printf("  %s", buf_time);				//输出时间
	if (S_ISLNK(buf.st_mode))
	{
	sprintf(colorname, "\033[%dm%s\033[0m", filecolor, name);
	printf(" %-s -> %s\n", colorname,buff_link);
	}else{
	sprintf(colorname, "\033[%dm%s\033[0m", filecolor, name);
	printf(" %-s\n", colorname);
	}
}

//输出文件名
void display_single(char *name, int filecolor)
{
	char   buf[256];
	char colorname[NAME_MAX + 30];
	int i, len, j = 0;
	len = strlen(name);
	for (i = 0; i < len; i++)
	{
		if (name[i] < 0)
		{
			j++;
		}
	}
	len = len - j / 3;
	if (len < 40)
	{
		len = 40 - len;
	}
	else
	{
		printf("\n");
	}
	sprintf(colorname, "\033[%dm%s\033[0m", filecolor, name);
	printf("%-s", colorname);
	for (i = 0; i < len + 10; i++)
	{
		printf(" ");
	}
	g_leave_len = g_leave_len - 45;
	if (g_leave_len < 45)
	{
		printf("\n");
		g_leave_len = MAXROWLEN;
	}
}

/********************************************
功能：输出带有i_node的文件信息    参数 -i
 * *****************************************/
void display_st_ino(struct stat buf, char *name, int filecolor)
{
	char colorname[NAME_MAX + 30];
	int i, len, j = 0;
	l++;
	len = strlen(name);
	for (i = 0; i < len; i++)
	{
		if (name[i] < 0)
		{
			j++;
		}
	}
	len = len - j / 3;
	if (len < 40)
	{
		len = 40 - len;
	}
	else
	{
		printf("\n");
	}
	printf("%d ", buf.st_ino);
	sprintf(colorname, "\033[%dm%s\033[0m", filecolor, name);
	printf(" %-s", colorname);
	//输出少于要求，补够空格
	for (i = 0; i < len + 5; i++)
		printf(" ");
	g_leave_len = g_leave_len - 45;
	if (g_leave_len < 45)
	{
		printf("\n");
		g_leave_len = MAXROWLEN;
	}
}

/**************************************
功能：判断是否有参数a l s及其各种组合
 * ***********************************/
void display(int flag, char *pathname)
{
	int filecolor = 37;
	int i, j;
	struct stat buf; //文件信息buf
	char name[NAME_MAX + 1];
	for (i = 0, j = 0; i < strlen(pathname); i++)
	{
		if (pathname[i] == '/')
		{
			j = 0;
			continue;
		}
		name[j++] = pathname[i];
	}
	name[j] = '\0';
	if (lstat(pathname, &buf) == -1)
	{
		printf("%s open failed\n", pathname);
		return;
	}
	if (S_ISLNK(buf.st_mode))
	{
		filecolor = 35;
	}
	else if (S_ISREG(buf.st_mode))
	{
	}
	else if (S_ISDIR(buf.st_mode))
	{
		filecolor = 34;
	}
	else if (S_ISCHR(buf.st_mode))
	{
		filecolor = 33;
	}
	else if (S_ISBLK(buf.st_mode))
	{
		filecolor = 33;
	}
	else if (S_ISFIFO(buf.st_mode))
	{
	}
	else if (S_ISSOCK(buf.st_mode))
	{
	}
	if (filecolor == 37 &&
		((buf.st_mode & S_IXUSR) ||
		 (buf.st_mode & S_IXGRP) ||
		 (buf.st_mode & S_IXOTH)))
	{
		filecolor = 32;
	}
	switch (flag)
	{
	case PARAM_NONE:
		if (name[0] != '.')
		{
			display_single(name, filecolor);
		}
		break;
	case PARAM_I:
		if (name[0] != '.')
		{
			display_st_ino(buf, name, filecolor);
		}
		break;

	case PARAM_S:
		if (name[0] != '.')
		{
			printf("%2d  ", buf.st_blocks / 2);
			display_single(name, filecolor);
		}
		break;

	case PARAM_A:
		display_single(name, filecolor);
		break;
	case PARAM_L:
		if (name[0] != '.')
		{
			display_attribute(buf, name, filecolor);
		}
		break;
	case PARAM_A + PARAM_L:
		display_attribute(buf, name, filecolor);
		break;
	case PARAM_A + PARAM_I:
		display_st_ino(buf, name, filecolor);
		break;

	case PARAM_A + PARAM_S:
		printf("%2d  ", buf.st_blocks / 2);
		display_single(name, filecolor);
		break;

	case PARAM_L + PARAM_S:
		if (name[0] != '.')
		{
			printf("%d  ", buf.st_blocks / 2);
			display_attribute(buf, name, filecolor);
		}
		break;

	case PARAM_L + PARAM_I:
		if (name[0] != '.')
		{
			printf("%d ", buf.st_ino);
			display_attribute(buf, name, filecolor);
		}
		break;

	case PARAM_I + PARAM_S:
		if (name[0] != '.')
		{
			printf("%2d  ", buf.st_ino);
			printf("%2d  ", buf.st_blocks / 2);
			display_single(name, filecolor);
		}
		break;

	case PARAM_A + PARAM_L + PARAM_S:
		printf("%d  ", buf.st_blocks / 2);
		display_attribute(buf, name, filecolor);

		break;

	case PARAM_A + PARAM_I + PARAM_S:
		printf("%d  ", buf.st_ino);
		printf("%2d  ", buf.st_blocks / 2);
		display_single(name, filecolor);
		break;

	case PARAM_L + PARAM_I + PARAM_S:
		if (name[0] != '.')
		{
			printf("%d\t", buf.st_ino);
			printf("%d\t", buf.st_blocks / 2);
			display_attribute(buf, name, filecolor);
		}
		break;
	case PARAM_A + PARAM_L + PARAM_I:
		printf("%d ", buf.st_ino);
		display_attribute(buf, name, filecolor);
		break;

	case PARAM_A + PARAM_I + PARAM_L + PARAM_S:
		printf("%d  ", buf.st_ino);
		printf("%d  ", buf.st_blocks / 2);
		display_attribute(buf, name, filecolor);
		break;
	case PARAM_R:
		if (name[0] != '.')
		{
			display_single(name, filecolor);
		}
		break;
	default:
		break;
	}
}

/************************************
功能：实现参数R	r t
 * **********************************/
void display_dir(int flag_param, char *path)
{
	DIR *dir;
	long t;
	int count = 0;
	int i, j, len;
	struct dirent *ptr;
	int flag_param_temp;
	struct stat buf;
	struct stat name;
	char temp[PATH_MAX + 10];
	flag_param_temp = flag_param;
	dir = opendir(path);

	if (dir == NULL)
	{
		my_err("oppendir", __LINE__);
		return;
	}
	//解析文件个数，及文件名的最长值
	while ((ptr = readdir(dir)) != NULL)
	{
		int a = 0; //用来统计汉字的个数，个数 = a/3
		int b = 0; //用来统计非汉字的个数 b
		for (i = 0; i < strlen(ptr->d_name); i++)
		{
			if (ptr->d_name[i] < 0)
			{
				a++;
			}
			else
			{
				b++;
			}
		}
		len = a + b;
		if (g_maxlen < len)
		{
			g_maxlen = len;
		}
		count++; //文件个数
	}
	fpnum = g_leave_len / (g_maxlen + 15);
	if (g_maxlen > 40)
	{
		fpnum = 1;
	}

	closedir(dir);
	char **filename = (char **)malloc(sizeof(char *) * 256);
	long **filetime = (long **)malloc(sizeof(long *) * 256);
	len = strlen(path);
	dir = opendir(path);
	//得到该目录下的所有文件的路径
	for (i = 0; i < count; i++)
	{
		filename[i] = (char *)malloc(sizeof(char) * 1000);
		ptr = readdir(dir);
		if (ptr == NULL)
		{
			my_err("readdir", __LINE__);
		}
		strncpy(filename[i], path, len);
		//strcpy是个不安全的函数，尽量使用strncpy替代,strncpy不拷贝'\0'
		filename[i][len] = '\0';
		strcat(filename[i], ptr->d_name);
		filename[i][len + strlen(ptr->d_name)] = '\0';
	}
	closedir(dir);

	//插入排序
	if (flag_param & PARAM_T) //根据时间排序
	{
		flag_param -= PARAM_T;
		for (i = 0; i < count; i++)
		{
			filetime[i] = (long *)malloc(sizeof(long));
			stat(filename[i], &buf); //用buf获取文件filename[i]中的数据
			filetime[i][0] = buf.st_mtime;
		}

		for (i = 0; i < count; i++)
		{
			for (j = i; j < count; j++)
			{
				if (filetime[i][0] < filetime[j][0])
				{
					/*交换时间filetime还要叫唤文件名*/
					t = filetime[i][0];
					filetime[i][0] = filetime[j][0];
					filetime[j][0] = t;
					strcpy(temp, filename[i]);
					strcpy(filename[i], filename[j]);
					strcpy(filename[j], temp);
				}
			}
		}
	}
	// else if(flag_param & PARAM_R)
	// {
	// 	for(i=0;i<count;i++)
	// 	{
	// 		for(j=i;j<count;j++)
	// 		{
	// 			if(strcmp(filename[i],filename[j]) < 0)
	// 			{
	// 				strcpy(temp,filename[j]);
	// 				strcpy(filename[j],filename[i]);
	// 				strcpy(filename[i],temp);
	// 			}
	// 		}
	// 	}
	// }
	else
	{
		for (i = 0; i < count - 1; i++)
		{
			for (j = 0; j < count - 1 - i; j++)
			{
				if (strcmp(filename[j], filename[j + 1]) > 0)
				{
					strcpy(temp, filename[j]);
					strcpy(filename[j], filename[j + 1]);
					strcpy(filename[j + 1], temp);
				}
			}
		}
	}

	//计算总用量total
	if (flag_param & PARAM_A)
	{
		for (i = 0; i < count; i++)
		{
			stat(filename[i], &name);
			total = total + name.st_blocks / 2;
		}
	}
	else
	{
		for (i = 0; i < count; i++)
		{
			stat(filename[i], &name);
			if (filename[i][2] != '.')
			{
				total = total + name.st_blocks / 2;
			}
		}
	}
	if (flag_param & PARAM_L)
	{
		printf("总用量： %d\n", total);
	}

	/* 打印文件 */
	if (flag_param & PARAM_R)
	{
		for (i = count - 2; i > 0; i--)
		{
			display(flag_param, filename[i]);
		}
	}
	else
	{
		for (i = 0; i < count - 1; i++)
		{
			display(flag_param, filename[i]);
		}
	}
	closedir(dir);
}

/* 递归输出*/
void display_RR(int flag_param, char *path)
{
	DIR *dir;
	struct dirent *ptr;
	int count = 0;
	int filecolor;
	int y = 0;
	int flag_param_temp;
	char filenames[256][PATH_MAX + 1], temp[PATH_MAX + 1];
	char muluname[256][PATH_MAX + 1];
	long filetime[256][1];
	long t;
	struct stat buf;
	struct stat name;
	flag_param_temp = flag_param;
	//获取该目录下文件总数和最长的文件名
	dir = opendir(path);
	if (dir == NULL)
	{
		my_err("opendir", __LINE__);
	}
	while ((ptr = readdir(dir)) != NULL)
	{
		if (g_maxlen < strlen(ptr->d_name))
			g_maxlen = strlen(ptr->d_name);
		count++;
	}
	closedir(dir);
	if (count > 256)
		my_err("too many files under this dir", __LINE__);

	int i, j, len = strlen(path);
	//获取该目录下所有的文件名
	dir = opendir(path);
	for (i = 0; i < count; i++)
	{
		ptr = readdir(dir);
		if (ptr == NULL)
		{
			my_err("readdir", __LINE__);
		}
		strncpy(filenames[i], path, len);
		filenames[i][len] = '\0';
		strcat(filenames[i], ptr->d_name);
		filenames[i][len + strlen(ptr->d_name)] = '\0';
	}
	//使用冒泡法对文件名进行排序，排序后文件名按照字母顺序存储于filenames
	if (flag_param & PARAM_T)
	{
		flag_param -= PARAM_T;
		for (i = 0; i < count; i++)
		{
			stat(filenames[i], &buf);
			filetime[i][0] = buf.st_mtime;
		}
		for (i = 0; i < count; i++)
			for (j = i; j < count; j++)
			{
				if (filetime[i][0] < filetime[j][0])
				{
					t = filetime[j][0];
					filetime[j][0] = filetime[i][0];
					filetime[i][0] = t;
					strcpy(temp, filenames[j]);
					strcpy(filenames[j], filenames[i]);
					strcpy(filenames[i], temp);
				}
			}
	}
	else if (flag_param & PARAM_R)
	{
		flag_param -= PARAM_R;
		for (i = 0; i < count - 1; i++)
			for (j = 0; j < count - 1 - i; j++)
			{
				if (strcmp(filenames[j], filenames[j + 1]) < 0)
				{
					strcpy(temp, filenames[j + 1]);
					temp[strlen(filenames[j + 1])] = '\0';
					strcpy(filenames[j + 1], filenames[j]);
					filenames[j + 1][strlen(filenames[j])] = '\0';
					strcpy(filenames[j], temp);
					filenames[j][strlen(temp)] = '\0';
				}
			}
	}
	else
	{
		for (i = 0; i < count - 1; i++)
			for (j = 0; j < count - 1 - i; j++)
			{
				if (strcmp(filenames[j], filenames[j + 1]) > 0)
				{
					strcpy(temp, filenames[j + 1]);
					temp[strlen(filenames[j + 1])] = '\0';
					strcpy(filenames[j + 1], filenames[j]);
					filenames[j + 1][strlen(filenames[j])] = '\0';
					strcpy(filenames[j], temp);
					filenames[j][strlen(temp)] = '\0';
				}
			}
	}

	//计算总用量total
	if (flag_param & PARAM_A)
	{
		for (i = 0; i < count; i++)
		{
			stat(filenames[i], &name);
			total = total + name.st_blocks / 2;
		}
	}
	else
	{
		for (i = 0; i < count; i++)
		{
			stat(filenames[i], &name);
			if (filenames[i][2] != '.')
			{
				total = total + name.st_blocks / 2;
			}
		}
	}
	printf("总用量:%d", total);
	printf("\n%s:\n", path);
	for (i = 0; i < count; i++)
	{
		stat(filenames[i], &buf);
		if (S_ISDIR(buf.st_mode))
		{
			len = strlen(filenames[i]);
			//-R时只有./根目录打开，其他../     ./.    等等目录不打开
			if ((filenames[i][len - 1] == '.' && filenames[i][len - 2] == '/') || (filenames[i][len - 1] == '.' && filenames[i][len - 2] == '.' && filenames[i][len - 3] == '/'))
				continue;
			strncpy(muluname[y], filenames[i], len);
			len = strlen(muluname[y]);
			muluname[y][len] = '/';
			muluname[y][len + 1] = '\0';
			y++;
		}
		 display(flag_param,filenames[i]);
	}
	for (i = 2; i < y; i++)
	{
		list_dir(muluname[i], flag_param);
	}
}

void list_dir(char *pathname, int param)
{
	char nextpath[PATH_MAX + 1];
	DIR *ret_opendir = opendir(pathname); //打开目录
	if (ret_opendir == NULL)
		my_err("ret_opendir", __LINE__);
	printf("%s:\n", pathname);	  //显示pathname的路径
	display_DIR(ret_opendir, 33); //显示pathname目录下所有非隐藏的文件名称
	//display(param,pathname);
	struct dirent *ret_readdir = NULL;			 //定义readdir函数返回的结构体变量
	while ((ret_readdir = readdir(ret_opendir))) //判断是否读取到目录尾
	{
		char *filename = ret_readdir->d_name; //获取文件名
		int end = 0;						  //优化显示路径（处理./text/与./text)
		while (pathname[end])
		end++;
		strcpy(nextpath, pathname);
		if (pathname[end - 1] != '/')
			strcat(nextpath, "/");
		strcat(nextpath, filename);
		struct stat file_message = {};				   //定义stat函数返回结构体变量
		int ret_stat = lstat(nextpath, &file_message); //获取文件信息
		//len = strlen(filename);
		if (ret_stat == -1) //stat读取文件错误则显示提示信息
			printf("%s error!", filename);
		else if (S_ISDIR(file_message.st_mode) && filename[0] != '.') //筛选‘.''..'与隐藏文件
		{
			list_dir(nextpath, param);
		}
	}
	closedir(ret_opendir);
}

void display_DIR(DIR *ret_opendir, int filecolor)
{
	char colorname[NAME_MAX + 30];
	int loop = 0;
	struct dirent *ret_readdir = NULL;		   //定义readir函数返回结构体变量
	while (ret_readdir = readdir(ret_opendir)) //判断是否读取到目录尾
	{
		char *filename = ret_readdir->d_name;
		if (filename[0] != '.')
		{ //不输出当前目录，上一级目录与隐藏文件
		
			sprintf(colorname, "\033[%dm%s\033[0m", filecolor, filename);
			printf("%-s\t", colorname); //打印文件名
		}
		loop++;
		if (!(loop % 5))
		{
			printf("\n");
			loop = 0;
		}
	}
	rewinddir(ret_opendir);//回溯到起始位置
	puts(" \t");
}

void sighandler(int signum)
{
}

int main(int argc, char *argv[])
{
	int i, j, n, num;
	char path[PATH_MAX + 1];
	char param[32];
	int flag_param = PARAM_NONE;
	struct stat buf;
	j = 0;
	num = 0;
	signal(SIGINT, sighandler);
	//将参数提取到param中
	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			for (n = 1; n < strlen(argv[i]); j++, n++)
			{
				param[j] = argv[i][n];
			}
			num++;
		}
	}

	//将参数变形以数字形式保存进flag_param
	for (i = 0; i < j; i++)
	{
		if (param[i] == 'a')
		{
			flag_param |= PARAM_A;
		}
		else if (param[i] == 'l')
		{
			flag_param |= PARAM_L;
		}
		else if (param[i] == 'R')
		{
			flag_param |= PARAM_RR;
		}
		else if (param[i] == 'r')
		{
			flag_param |= PARAM_R;
		}
		else if (param[i] == 't')
		{
			flag_param |= PARAM_T;
		}
		else if (param[i] == 'i')
		{
			flag_param |= PARAM_I;
		}
		else if (param[i] == 's')
		{
			flag_param |= PARAM_S;
		}
		else
		{
			printf("ls:failed operation -- %s", param[i]);
			exit(1);
		}
	}

	if (flag_param & PARAM_RR)
	{
		flag_param -= PARAM_RR;
		//判断是否有目录输入，没有则打开根目录./
		if (num + 1 == argc)
		{
			strcpy(path, "./");
			path[2] = '\0';
			display_RR(flag_param, path);
			return 0;
		}

		i = 1;
		do
		{
			if (argv[i][0] == '-')
			{
				i++;
				continue;
			}
			else
			{
				//得到具体路径(目录名)
				strcpy(path, argv[i]);
				if (stat(path, &buf) == -1)
					my_err("stat", __LINE__);
				//判断是否为目录文件
				if (S_ISDIR(buf.st_mode))
				{
					//如果目录最后忘记了/则加上
					if (path[strlen(argv[i]) - 1] != '/')
					{
						path[strlen(argv[i])] = '/';
						path[strlen(argv[i]) + 1] = '\0';
					}
					else
						path[strlen(argv[i])] = '\0';
					display_RR(flag_param, path);
				}
			}
		} while (i < argc);
	}

	else
	{
		//判断是否有目录输入，没有则打开根目录./
		if (num + 1 == argc)
		{
			strcpy(path, "./");
			path[2] = '\0';
			display_dir(flag_param, path);
			return 0;
		}

		i = 1;
		do
		{
			if (argv[i][0] == '-')
			{
				i++;
				continue;
			}
			else
			{
				//得到具体路径(目录名)
				strcpy(path, argv[i]);
				if (stat(path, &buf) == -1)
					my_err("stat", __LINE__);
				//判断是否为目录文件
				if (S_ISDIR(buf.st_mode))
				{
					//如果目录最后忘记了/则加上
					if (path[strlen(argv[i]) - 1] != '/')
					{
						path[strlen(argv[i])] = '/';
						path[strlen(argv[i]) + 1] = '\0';
					}
					else
						path[strlen(argv[i])] = '\0';
					display_dir(flag_param, path); //按照目录输出
					i++;
				}
				else
				{
					//按照文件输出
					display(flag_param, path);
					i++;
				}
			}
		} while (i < argc);
	}
	return 0;
}
