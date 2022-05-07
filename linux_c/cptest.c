#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<string.h>
#include<unistd.h>

void CpFile(char *src,char *des,int flag)
{
    int fdr = open(src,O_RDONLY);
    if(fdr < 0)
    {
        printf("open failed\n");
    }

    char desk[128] = { 0 };
    //  这里的标志是说明最后一个是目录
    if(flag)
    {   
        char*filename = NULL;
        if(strstr(src,"/") == NULL)
        {
            filename  = src;
        }
        else
        {
            filename = src + strlen(src);
            while(*filename != '/')
            {
                filename--;
            }
            filename++;
        }
        strcat(desk,des);
        strcat(desk,"/");
        strcat(desk,filename);
    }
    else
    {
        strcat(desk,des);
    }
    int fdw = open(desk,O_WRONLY | O_CREAT | O_TRUNC,0664);
    if(fdw < 0)
    {
        printf("error to open\n");
        exit(0);
    }

    while(1)
    {
        char buf[128] = {0};
        int n = read(fdr,buf,127);
        if(n<=0) break;
        write(fdw,buf,n);
    }
    close(fdr);
    close(fdw);
}

int main(int argc,char *argv[])
{
    if(argc < 3)
    {
        printf("not enough\n");
        return 0;
    }
    struct stat st ;
    stat(argv[argc -1],&st);
    if(argc == 3 && !S_ISDIR(st.st_mode))
    {
        CpFile(argv[1],argv[2],0);
        return 0;
    }
    if(!S_ISDIR(st.st_mode))
    {
        printf("%s not a dir\n",argv[argc - 1]);
        return 0;
    }
    for(int i = 1;i < argc -1;i++)
    {
        CpFile(argv[i],argv[argc - 1],1);
    }
    //还有要讨论多文件的判断是否是目录或者是文件

    return 0;
}