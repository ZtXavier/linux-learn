// #include<stdio.h>

// union text{
//     int a;
//     char x;
// };


// int main(void){
    
//     union text u1;

//     u1.a = 0x12345678;
//     printf("%x",u1.x);

//     return 0;
// }

#include <stdio.h>
#include<stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

int main()
{
    char *buf = NULL;
    int len;
    while(1)
    {
        buf = (char *)malloc(sizeof(char) * 256);
        buf = memset(buf,0,256);   
        buf = readline("");
        if(strcmp(buf,"exit")==0)
        exit(0);
        len = strlen(buf);
        printf("len = %d buf = %s\n",len,buf);
        add_history(buf);
    }
    return 0;
}
