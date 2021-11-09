#include<stdio.h>
#define SUM1(A) (A)
#define SUM2(A,B) ((A) + (B))
#define SUM3(A,B,C) ((A) + (B) + (C))

#define GET_MACRO(_1,_2,_3,NAME,...) NAME
#define SUM(...)                                       \
    GET_MACRO(__VA_ARGS__,SUM3,SUM2,SUM1)              \
    (__VA_ARGS__)
//在c语言上实现了函数重载
int main()
{
    printf("%d\n",SUM(1));
    printf("%d\n",SUM(1,2));
    printf("%d\n",SUM(1,2,3));
    return 0;
}