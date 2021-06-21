#include<stdio.h>
#include"mylib.h"

void welcome(){
    printf("welcome\n");
}
void outstring(const char * str){
    if(str != NULL)
    printf("%s",str);
}
