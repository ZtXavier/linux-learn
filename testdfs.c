#include<stdio.h>
int book[10],a[10],n = 0;

void dfs(int step)
{
    int i;
    if(step == 10)
    {
        if(a[1]*100+a[2]*10+a[3] + a[4]*100+a[5]*10+a[6] == a[7]*100+a[8]*10+a[9])
        {
            n++;
            printf("%d%d%d + %d%d%d = %d%d%d\n",a[1],a[2],a[3],a[4],a[5],a[6],a[7],a[8],a[9]);

        }
        return ;
    }

    for(i = 1;i <= 9;i++)
    {
        //先判断该牌是否还在手上
        if(book[i] == 0)
        {
            a[step] = i;
            book[i] = 1;
            dfs(step+1);
            book[i] = 0;     //将盒子中的牌拿出来
        }
    }
    return ;
}

int main(void)
{
    dfs(1);
    printf("total = %d\n",n/2);
    getchar();
    return 0;
}