#include<stdio.h>

union text{
    int a;
    char x;
};


int main(void){
    
    union text u1;

    u1.a = 0x12345678;
    printf("%x",u1.x);

    return 0;
}

