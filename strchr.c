#include<stdio.h>
#include<string.h>


int main()
{
    char * ch = "reh\tsfkhete";
    // 第一次出现到结束
    char *ch2 = strchr(ch,'e');
    // 最后一次出现到结束
    char *ch3 = strrchr(ch,'e');
    printf("%s\n",ch2);
    printf("%s\n",ch3);
    return 0;
}