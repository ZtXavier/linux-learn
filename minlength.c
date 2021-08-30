#include<stdio.h>

int n,m,p,q,min = 9999999;
int a[51][51],book[51][51];

void dfs(int x,int y,int step)
{
    int next[4][2] = {{0,1},{1,0},{0,-1},{-1,0}}; //逆时针（右，下，左，上）
    int tx,ty,k;
    if(x == p && y == q)
    {
        //更新最小值
        if(step < min)
        min = step;
        return ;
    }

    for(int k = 0;k <= 3;k++)
    {
        tx = x + next[k][0];
        ty = y + next[k][1];
        //判断是否越界
        if(tx < 1 || tx > n || ty < 1 || ty > m)
        {
            continue;
        }
        if(a[tx][ty] == 0 && book[tx][ty] == 0)
        {
            book[tx][ty] = 1; //标记走过这个点
            dfs(tx,ty,step + 1); //继续下一步
            book[tx][ty] = 0; //取消对这个点的标记
        }
    }
    return ;
}

int main(void)
{
    int i,j,startx,starty;
    printf("请输入迷宫的大小:");
    scanf("%d %d",&n,&m);   //设置迷宫的大小
    for(i = 1;i <= n;i++)
        for(j = 1;j <= m;j++)
        scanf("%d",&a[i][j]);   //读入迷宫，设置
        printf("请输入起点和终点:");
        scanf("%d %d %d %d",&startx,&starty,&p,&q);
        book[startx][starty] = 1;
        dfs(startx,starty,0);
        printf("%d",min);
        getchar();
        return 0;
}