#include<stdio.h>
#include<string.h>
#include<stdlib.h>

char *convert(char * str)
{
    int n = strlen(str);
    char * p = (char*)malloc(sizeof(char)*(n+1));
    char *res;
    res = p;
    while((*str) != '\0')
    {
        if((*str) > 65 && (*str) < 90)
        {
            (*p) = (*str)  + 32;
        }
        else
        {
            (*p) = (*str);
        }
        p++;
        str++;
    }
    (*p) = '\0';
    return res;
}

int main(int argc,char *argv[])
{
    // int a[3][3] = {{1,2,3},{4,5,6},{7,8,9}};
    // int (*b)[3] = a;
    // b++;
    // b[1][1] = 10;
    // int *ptr = (int *)(&a+1);
    // printf("%d %d %d\n",a[2][1],*(*(a+1)-1),*(ptr - 1));

    char *str = "XiYouLinux Group 2022";
    char *temp = convert(str);
    puts(temp);
}