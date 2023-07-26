#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

// #define MSGSIZE    1024
// #define MAXSIZE    1032
// typedef struct message{
// int    type;            //消息类型
// int    msglen;          //消息长度
// char   data[MSGSIZE];   //消息数据
// }msg;
// int main(void){
//     FILE *fp;
//     int num ;
//     if((fp = fopen("group.txt","r")) == NULL){
//         printf("Error opening");
//     }
//     fread(&num,sizeof(int),1,fp);
//     fclose(fp);
//     printf("%d\n",num--);
//     if((fp = fopen("group.txt","w")) == NULL){
//         printf("Error opening");
//     }
//     fwrite(&num, sizeof(int), 1, fp);
//     fclose(fp);
//     printf("%d\n",num);
//     return 0;
// }




// int main() {
//     pid_t mypid;
//     mypid = getpid();
//     printf("my PID is %ld\n", (long)mypid);
//     return 0;
// }



int main (int argc,char * argv[]) {
    int inputFd, outputFd, openFlags;
    mode_t filePerms;
    size_t numread;
    char buf[1024];

    if(argc != 3 || strcmp(argv[1],"--help") == 0) {
        perror("error inst");
    }

    inputFd = open(argv[1], O_RDONLY);

    if(inputFd == -1) {
        perror("open failed");
    }

    openFlags = O_CREAT | O_WRONLY | O_TRUNC;
    filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    outputFd = open(argv[2], openFlags, filePerms);
    if(outputFd == -1) {
        perror("error open sd file");
    }

    while((numread = read(inputFd, buf, 1024)) > 0) {
        if(write(outputFd, buf, numread) != numread) {
            perror("could not write whole buffer");
        }
    }
    if(numread == -1) {
        exit(1);
    }

    if(close(inputFd) == -1) {
        exit(1);
    }
    if(close(outputFd) == -1) {
        exit(1);
    }
    exit(EXIT_SUCCESS);
    return 0;
}