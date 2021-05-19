#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>

int main(){
    FILE  *stream;

    char  *filename = "text";

    errno = 0;

    stream = fopen(filename,"r");
    if(stream == NULL){
        printf("open file %s failed,errno is %d\n",filename,errno);
    }
    else
    printf("open file %s successfully\n",filename);
    return 0;
}