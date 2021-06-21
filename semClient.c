#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<semaphore.h>
#include<unistd.h>


int main(void){

    key_t  key;
    int    semid,semval;
    union semun {
        int    val;
        struct semid_ds *buf;
        unsigned short *array;
    }semopts;

    if((key = ftok(".",'s')) == -1){
        perror("ftok error!\n");
        exit(1);
    }

    if((semid = semget(key,1,IPC_CREAT | 0666)) == -1){
        perror("semget error!\n");
        exit(1);
    }

    while(1){

        if((semval = semctl(semid,0,GETVAL,0)) == -1){
            perror("semctl error!\n");
            exit(1);
        }
        if(semval > 0){
            printf("Still %d resource can be used\n",semval);
        }
        else
        {
            printf("No more resource can be used %d\n",semval);
            break;
        }
        sleep(3);
    }
    exit(0);
}