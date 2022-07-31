#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>

#include<unistd.h>
#include<sys/types.h>
#include<pthread.h>

int quitflag;
sigset_t mask;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t waitloc  = PTHREAD_COND_INITIALIZER;

void *pth_fn(void *args)
{
    
    int signo;
    while(1){

        sigwait(&mask, &signo);
        switch(signo){
            case SIGINT:
                printf("\ninterrupt\n");
                
                break;

            case SIGQUIT:
                pthread_mutex_lock(&lock);
                quitflag += 1;
                printf("1 signal %d\n", signo);
                pthread_mutex_unlock(&lock);
                pthread_cond_signal(&waitloc);
                return 0;

            default:
                printf("unexpected signal %d\n", signo);
                exit(0);
        }
    }
}

void *pth_fn2(void *args)
{
    
    int signo;
    while(1){
        // sleep(3);
        sigwait(&mask, &signo);
        switch(signo){
            case SIGINT:
                printf("\ninterrupt\n");
                
                break;

            case SIGQUIT:
                pthread_mutex_lock(&lock);
                quitflag += 1;
                printf("2 signal %d\n", signo);
                pthread_mutex_unlock(&lock);
                pthread_cond_signal(&waitloc);
                
                return 0;

            default:
                printf("unexpected signal %d\n", signo);
                exit(0);
        }
    }
}
int main()
{
    sigset_t oldmask;
    pthread_t tid;
    pthread_t tid2;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGQUIT);
    sigprocmask(SIG_BLOCK, &mask, &oldmask);
    pthread_create(&tid, NULL, pth_fn, NULL);
    pthread_create(&tid2,NULL,pth_fn2,NULL);
    pthread_detach(tid2);
    pthread_detach(tid);
    pthread_mutex_lock(&lock);
    while(quitflag != 2)
        pthread_cond_wait(&waitloc, &lock);
    pthread_mutex_unlock(&lock);

    quitflag = 0;
    sigprocmask(SIG_SETMASK, &oldmask, NULL);
    

    return 0;
}