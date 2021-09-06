#include<stdio.h>
#include<stdarg.h>


//连续加一系列值的和
int average(int n,...)
{
    va_list arg;
    int i = 0;
    int sum = 0;
    va_start(arg,n);
    for(int i = 0;i < n;i++)
    {
        sum += va_arg(arg,int);
    }
    return sum / n;
    va_end(arg);
}

int Max(int n,...)
{
    va_list arg;
    int i = 0;
    va_start(arg,n);
    int max = va_arg(arg,int);
    //令max等于可变参数的第一个参数，并且arg已经指向了下一个参数
    for(i = 1;i < n;i++)
    {
        int temp = va_arg(arg,int);
        if(temp > max)
        max = temp;
    }
    return max;
    va_end(arg);
}


int main()
{
    int a = 1;
    int b = 2;
    int c = 3;
    int avg1 = Max(2,a,b);
    int avg2 = average(3,a,b,c);
    printf("avg1=%d\n",avg1);
    printf("avg2=%d\n",avg2);
    return 0;
}