#ifndef _COMM_H_
#define _COMM_H_

#include<stdio.h>
#include<sys/types.h>
#include<sys/shm.h>
#include<sys/ipc.h>
#define PATHNAME "."
#define PROJ_ID 0x6666


int CreateShm(int size);
int DestroyShm(int shmid);
int GetShm(int size);

#endif