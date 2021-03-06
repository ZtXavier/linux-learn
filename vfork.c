#include<stdio.h>
#include<sys/types.h>
#include<unistd.h>
#include<stdlib.h>


int globvar = 5;

int main(void){

    pid_t pid;
    int   var = 1,i;

    printf("fork is different with vfork\n");

    /* pid = fork(); */
    pid = vfork();
    switch(pid){
        case 0:
        i = 3;
        while(i-- > 0){
            printf("Child process is running\n");
            globvar++;
            var++;
            sleep(1);
        }
            printf("Child's globvar = %d,var = %d\n",globvar,var);
        break;
        case -1:
            perror("Process creation failed\n");
            exit(0);
        default:
        i = 5;
        while(i-- >0){
            printf("Parent process is running\n");
            globvar++;
            var++;
            sleep(1);
        }
            printf("Parent's globvar = %d,var = %d\n",globvar,var);
            
    }
    exit(0);
}