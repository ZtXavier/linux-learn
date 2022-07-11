#include"libcal.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>


int main()
{
    pthread_t id1[4];
    pthread_t id2[4];
    int i;
    int ret;

    sem_mutex_init();
    
    /*create the producer thread*/
    for(int i = 0;i < 4;i++)
    {
        if((ret = pthread_create(&id1[i], NULL, producer, NULL)) != 0)
        {
            printf("producer creation failed \n");
            exit(1);
        }
    }
    
    /*create the consumer thread*/
    for(int i = 0;i < 4;i++)
    {
        
        if((ret = pthread_create(&id2[i], NULL, consumer, NULL)) != 0)
        {
            printf("consumer creation failed \n");
            exit(1);
        }
    }
    for(int i = 0;i < 4;i++)
    {
        pthread_join(id1[i],NULL);
        pthread_join(id2[i],NULL);
    }
    exit(0);
}