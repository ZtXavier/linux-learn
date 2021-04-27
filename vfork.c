#include<stdio.h>
#include<sys/types.h>
#include<unistd.h>

int globvar = 5;

int main(void){

    pid_t pid;
    int   var = 1,i;

    printf("fork is different with vfork\n");

    pid = fork();
    /* pid = vfork(); */
    switch(pid){
        case 0:
        i=3;
        while(i-- > 0){
            
        }
    }
}