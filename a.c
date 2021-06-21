#include<stdio.h>
#include<string.h>

int main(void){
    int i;
    int sum = 0;
    for(i = 0;i < 10; i++){

        sum += i;
    }
    printf("%d\n",sum);

    return 0;
}