#include<stdio.h>
#include<stdarg.h>
#include<stdlib.h>
#include<string.h>



static unsigned long m_pow(int x,int y)
{
    unsigned long res = 1;
    while(y--)
    {
        res *= x;
    }
    return res;
}

void m_itoa(int n,char s[],int sysofnum)
{
    int tep,d;
    int i = 0;
    do
    {
        tep = n % sysofnum;
        if(tep > 9)
        s[i++] = tep - 10 + 'A';
        else
        s[i++] = tep + '0';
    }while(n /= sysofnum);
    s[i] = '\0';
    i -= 1;
    for(int j = 0;j <= i;j++)
    {
        d = s[i];
        s[i] = s[j];
        s[j] = d;
        i--;
    }
}



int m_printf(const char *str,...)
{
    va_list ap;            //先初始化一个char类型的指针
    int num;
    char c,ch,s[30];     //用来接收%后的参数
    va_start(ap,str);      //相当于char*ap = (char*)&str + sizeof(int);此时正好指向该函数*str后的第一个参数
    while(c = *str)
    {
        switch(c)
        {
            case '%':
                ch = *(++str);
            switch(ch)
            {
                case 'd':
                    num = va_arg(ap,int);
                    m_itoa(num,s,10);
                    fputs(s,stdout);
                    bzero(s,sizeof(s));
                break;

                case 'c':
                    putchar(va_arg(ap,int));
                break;

                case 'x':
                    num = va_arg(ap,int);
                    m_itoa(num,s,16);
                    fputs(s,stdout);
                break;

                case 's':
                    char *p = va_arg(ap,char *);
                    fputs(p,stdout);
                break;

                case 'f':
                    double f = va_arg(ap,double);
                    num = f;
                    m_itoa(num,s,10);
                    fputs(s,stdout);
                    bzero(s,sizeof(s));
                    putchar('.');
                    num = (f - num)*1000000;
                    m_itoa(num,s,10);
                    fputs(s,stdout);
                    bzero(s,sizeof(s));
                break;

                case '%':
                    putchar('%');
                break;

                default:
                    fputs("info invalid!\n",stdout);
                break;
            }
            break;

            case ' ':
                putchar(' ');
            break;
        }
        str++;
    }
    va_end(ap);
}

int main(void)
{
    int a = 124;
    char ch = 'A';
    char s[4]  = "asd";
    double de = 111.111;
    m_printf("%d %x %s %c %f",a,a,s,ch,de);
    return 0;
}
