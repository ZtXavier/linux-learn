#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

typedef struct qsort{
    int left;
    int right;
    int *a;
} qk ;

void *quicksort(void *args)
{
    qk *my_args = (qk *)args;

    int t,i=my_args->left,j=my_args->right,temp;
    if(my_args->left>my_args->right)
    return NULL;
    temp=my_args->a[my_args->left];
    i=my_args->left;
    j=my_args->right;
    while (i!=j)
    {
        while(my_args->a[j]>=temp&&i<j)
        j--;
        while (my_args->a[i]<=temp&&i<j)
        i++;
        if(i<j)
        {
            t=my_args->a[i];
            my_args->a[i]=my_args->a[j];
            my_args->a[j]=t;
        }
        
    }
    my_args->a[my_args->left]=my_args->a[i];
    my_args->a[i]=temp;
    quicksort();
    quicksort();
}


int main(void)
{
    // int n;
    // scanf("%d",&n);
    // int a[n];
    // for(int i=0;i<n;i++)
    // scanf("%d",&a[i]);
    // quicksort(a,0,n-1);
    // for(int i=0;i<n;i++)
    // printf("%d ",a[i]);
    qk q1,q2;
    int num;
    
    scanf("%d",&num);
    q1.left=0;
    q1.right=num/2;
    q2.left=num/2+1;
    q2.right=num;

    int a[num];

    for(int i=0;i<num;i++)
    scanf("%d",&a[i]);

    q1.a = a;
    q2.a = a;
    
    pthread_t th1;
    pthread_t th2;

    pthread_create(&th1,NULL,quicksort,&q1);
    pthread_create(&th2,NULL,quicksort,&q2);


    pthread_join(th1,NULL);
    pthread_join(th2,NULL);

    return 0;
}