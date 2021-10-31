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

int main(int argc,char *argv[])
{
    DIR *dp;
    struct dirent *dirp;
    if(argc != 2)
    {
        err_sys("usage: Is directory_name");
    }
    if((dp = opendir(argv[1])) == NULL)
    {
        err_sys("can't open %s",argv[1]);
    }
    while((dirp = readdir(dp)) != NULL)
    {
        printf("%s\n",dirp->d_name);
    }
    closedir(dp);
    exit(0);
}

