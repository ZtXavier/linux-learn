#include<stdio.h>
#include<malloc.h>

int main() {
    int *mask = (int*)malloc(sizeof(int));
    *mask = 0;
    for(int i  = 0; i < 10; ++i) {
        *mask |= 1 << i;
        printf("%d\n", sizeof(mask));
    }
    return 0;
}
