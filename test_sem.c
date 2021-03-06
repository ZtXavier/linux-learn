#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>

#define PATHNAME "."
#define PROJ_ID 0x6666

union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};

int createSemSet(int nums);
int initSem(int semid, int nums, int initVal);
int getSemSet(int nums);
int P(int semid, int who);
int V(int semid, int who);
int destroySemSet(int semid);




static int commSemSet(int nums, int flags)
{
    key_t _key = ftok(PATHNAME, PROJ_ID);
    if(_key < 0){
		perror("ftok error");
		return -1;
    }
    int semid = semget(_key, nums, flags);
    if(semid < 0){
		perror("semget error");
		return -2;
    }
    return semid;
}

int createSemSet(int nums)
{
    return commSemSet(nums, IPC_CREAT|IPC_EXCL|0666);
}
int getSemSet(int nums)
{
    return commSemSet(nums, IPC_CREAT);
}
int initSem(int semid, int nums, int initVal)
{
    union semun _un;
    _un.val = initVal;
    if(semctl(semid, nums, SETVAL, _un)<0){
		perror("semctl error");
		return -1;
    }
    return 0;
}
static int commPV(int semid, int who, int op)
{
    struct sembuf _sf;
    _sf.sem_num = who;
    _sf.sem_op = op;
    _sf.sem_flg = 0;
    if(semop(semid, &_sf, 1) < 0){
		perror("semop error");
		return -1;
    }
    return 0;
}
int P(int semid, int who)
{
    return commPV(semid, who, -1);
}
int V(int semid, int who)
{
    return commPV(semid, who, 1);
}
int destroySemSet(int semid)
{
    if(semctl(semid, 0, IPC_RMID) < 0){
		perror("semctl error");
		return -1;
    }
}
int main()
{
    int semid = createSemSet(1);
    initSem(semid, 0, 1);
    pid_t id = fork();
    if(id == 0){
	//child
	int _semid = getSemSet(0);
	while(1){
	    P(_semid, 0);
	    printf("A");
	    fflush(stdout);
	    usleep(123456);
	    printf("A ");
	    fflush(stdout);
	    usleep(321456);
	    V(_semid, 0);
	}
    }
    else{
	//father
	while(1){
	    P(semid, 0);
	    printf("B");
	    fflush(stdout);
	    usleep(223456);
	    printf("B ");
	    fflush(stdout);
	    usleep(121456);
	    V(semid, 0);
	}
	wait(NULL);
    }
    destroySemSet(semid);
    return 0;
}
