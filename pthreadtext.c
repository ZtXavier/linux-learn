#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

#define  MAX 100000



int a[MAX];

void quicksort(int l,int r);
pthread_mutex_t lock;
// void *quicksort(void *args)
// {
//     qk *my_args = (qk *)args;

//     int t,i=my_args->left,j=my_args->right,temp;
//     if(my_args->left>my_args->right)
//     return NULL;
//     temp=my_args->a[my_args->left];
//     i=my_args->left;
//     j=my_args->right;
//     while (i!=j)
//     {
//         while(my_args->a[j]>=temp&&i<j)
//         j--;
//         while (my_args->a[i]<=temp&&i<j)
//         i++;
//         if(i<j)
//         {
//             t=my_args->a[i];
//             my_args->a[i]=my_args->a[j];
//             my_args->a[j]=t;
//         }
        
//     }
//     my_args->a[my_args->left]=my_args->a[i];
//     my_args->a[i]=temp;
//     quicksort();
//     quicksort();
// }

void init(){
    for(int i =0;i<100000;i++)
    a[i] = rand() % 5;
}



void quicksort(int l,int r)
{
    int i=l,j=r,temp,t;
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

void *thread1(void* args){
    pthread_mutex_lock(&lock);
    quicksort(0,99999);
    pthread_mutex_unlock(&lock);
    return NULL;
}

// void *thread2(void* args){
//     pthread_mutex_lock(&lock);
//     quicksort(a,i+1,r);
//     pthread_mutex_unlock(&lock);
//     return NULL;
// }


int main(void)
{

    // for(int i=0;i<=9;i++)
    // printf("%d ",a[i]);
    // printf("\n");

    pthread_t th1;
    // pthread_t th2;
    pthread_mutex_init(&lock,NULL);
    // if(0 != pthread_create(&th1,NULL,quicksort,)){
    // printf("created pthread failed\n");
    // return 0;
    // }
    pthread_create(&th1,NULL,thread1,NULL);
    pthread_create(&th1,NULL,thread1,NULL);
    pthread_create(&th1,NULL,thread1,NULL);
    pthread_create(&th1,NULL,thread1,NULL);
    pthread_create(&th1,NULL,thread1,NULL);

    pthread_join(th1,NULL);
    // pthread_join(th2,NULL);
     
    // for(int i=0;i<=9;i++)
    // printf("%d ",a[i]);
    
    return 0;
}