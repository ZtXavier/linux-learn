#include<stdio.h>
#include<stdlib.h>

int a[100000];


void init(){
    for(int i =0;i<100000;i++)
    a[i] = rand() % 5;
}



void quicksort(int l,int r)
{
    int t,i=l,j=r,temp;
    if(l>r)
    return;
    temp=a[l];
    i=l;
    j=r;
    while (i!=j)
    {
        while(a[j]>=temp&&i<j)
        j--;
        while (a[i]<=temp&&i<j)
        i++;
        if(i<j)
        {
            t=a[i];
            a[i]=a[j];
            a[j]=t;
        }
        
    }
    a[l]=a[i];
    a[i]=temp;
    quicksort(l,i-1);
    quicksort(i+1,r);
}
int main(void){
    init();
    quicksort(0,99999);
    // for(int i=0;i<1000;i++)
    // printf("%d ",a[i]);
    // printf("\n");
    return 0;
}