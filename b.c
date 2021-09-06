#include<stdio.h>
#include<string.h>
#include<stdlib.h>

int main(void){
    int a = 0x12345678;
    char ch = (char)a;
    printf("%c",ch);
    return 0;
}