#include<stdio.h>

typedef union {
    long l;
    int a[5];
    char c;
}UNION;


typedef struct {
    int like;
    UNION coin;
    double collect;
} A;


int main() {
    printf("%zu",sizeof(A));
    printf("%zu",sizeof(UNION));
    return 0;
}