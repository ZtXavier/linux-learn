#include"comm.h"
#include<unistd.h>

int main()
{
    int shmid = CreateShm(4096);
    char * addr = shmat(shmid,NULL,0);
    sleep(5);
    int i = 0;
    while(i++ < 26)
    {
        printf("client# %s\n",addr);
        sleep(1);
    }
    shmdt(addr);
    sleep(1);
    DestroyShm(shmid);
    return 0;
}