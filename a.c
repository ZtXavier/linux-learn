#include<stdio.h>
#include<string.h>

int main(void){
    char array[10]="xiyou\0";
    printf("%d",strlen(array));
    return 0;
}