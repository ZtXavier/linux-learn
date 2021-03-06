#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<unistd.h>


#define MAX_RESOURCE   10

int main(void){

    key_t  key;
    int    semid,semval;
    struct sembuf sbuf = {0,-1,IPC_NOWAIT};
    union semun {
        int val;
        struct semid_ds *buf;
        unsigned short *array;
        struct seminfo *_buf;
    }semopts;

    if((key = ftok(".",'s')) == -1){
        perror("ftok error!\n");
        exit(1);
    }
    if((semid = semget(key,1,IPC_CREAT | 0666)) == -1){
        perror("semget error!\n");
        exit(1);
    }

    semopts.val = MAX_RESOURCE;
    if((semctl(semid,0,SETVAL,semopts)) == -1){
        perror("semctl error\n");
        exit(1);
    }
    while(1){
        if((semval = semop(semid,&sbuf,1)) == -1){
            perror("semop error!\n");
            exit(1);
        }
        sleep(3);
    }
    exit(0);
}