#include<stdio.h>
//深度优先搜索,找到n位数的全排列
int book[10],a[10],n;

void dfs(int step)
{
    int i;
    if(step == n+1)
    {
        for(i = 1;i <= n;i++)
        {
            printf("%d",a[i]);
        }
        printf("\n");
        return ;
    }

    for(i = 1;i <= n;i++)
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
    scanf("%d",&n);
    dfs(1);
    getchar();
    return 0;
}