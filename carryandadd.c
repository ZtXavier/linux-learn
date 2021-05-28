#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>

#define N 50
#define true 1
#define addNUM 10
#define workerNum 5

int queue[N] = {0};   //任务缓冲区（初始化为0）,实现循环队列
int in = 0;           //生产的产品的位置
int out = 0;          //消费者的位置
int mutex = 1;        //互斥条件  
int empty = N;        //为空的个数
int full  = 0;        //为满的个数
int addmutex = 1;     //生产者之间的互斥条件 
int addfoodsN = 0;    //生产产品
int wmutex = 1;


void *add(void *a){
    while(true){
        // while(addmutex <= 0);
        // addmutex--;

        addfoodsN++;


        printf("生产的产品号为%d,于缓冲区的%d位置\n",addfoodsN,in);

        while(empty <= 0){
            printf("缓冲区已满\n");
            sleep(5);
        }
        empty--;
        while(mutex <= 0);  //如果mutex <= 0 说明其他线程正在工作，在这里等待
        mutex--;

        queue[in] = addfoodsN;
        in = (in + 1) % N;

        mutex++;            //解除互斥条件，让等待的线程继续工作
        full++;             //满了的缓冲区加一 
        sleep(2);           //在此睡眠1秒结束        
    }   
}


void *worker(void *w){
    while(true){
        // while(wmutex <= 0);
        // wmutex--;

        while(full <= 0){          //先判断是否缓冲区为空，若为空，则阻塞
            printf("缓冲区为空\n");
            sleep(5);
        }                   
    
    full--;                        //否则将缓冲区的物品拿走一个消费

    while(mutex <= 0);            //当多个消费线程工作时，在此进行阻塞，一个一个进行工作，以防止数据混乱
    mutex--;                      //当第一个线程开始时，在这里改变互斥条件
    
    int foods = queue[out];       //将消费的产品保存起来  
    queue[out] = 0;               //将原来的缓冲区位置归0来重复利用


    
    out = (out + 1) % N;

    mutex++;
    empty++;
    printf("\t\t\t\t 消费的产品ID为%d,缓冲区位置为%d\n",foods,out);

    sleep(2);
   }
}


int main(void){

    pthread_t threadPool[15];
    pthread_mutex_t mutex;

   
    for(int i = 0;i < 10; i++){
        pthread_t pt;
        if(pthread_create(&pt,NULL,add,NULL) == -1){
            printf("failed to create a pthread of add%d\n",i);
            exit(1);
        }

        threadPool[i] = pt;
    }
   

    for(int i = 0;i < 5; i++){
        pthread_t pt;
        if(pthread_create(&pt,NULL,worker,NULL) == -1){
            printf("failed to create a pthread of worker%d\n",i);
            exit(1);
        }

        threadPool[i+10] = pt;
    }

    void *res;
    for(int i = 0;i < 15; i++){
        if(pthread_join(threadPool[i],&res) == -1){
            printf("failed to recollect\n");
            exit(1);
        }
    }
   return 0;
}