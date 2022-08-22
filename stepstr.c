#include<stdio.h>
#include<stdlib.h>



// 使用0来作为struct的起始地址来方便寻找每个字段的偏移地址
struct st_test
{
    char a;
    int b;
    char c;
};

int main(int argc,char *argv[])
{
    struct st_test *p = 0;
    struct st_test *c = NULL;
    
    printf("%p\n",&(p->c));

    return 0;
}