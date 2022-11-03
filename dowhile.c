#include<stdio.h>
#include<string.h>

#define bin(x)  (x) = (x) + 1
#define lll(y) (y) = (y) - 1


// 一般宏定义都用do while 来代替中括号
// #define foo(x,y) {bin(x);lll(y);}
#define foo(x,y) do{bin(x);lll(y);}while(0)

int main() {
    int a,b;
    a = 2;
    b = 3;
    if(3)
    foo(a,b);
     else 
    a= a*a;
    
    return 0;
}