#include<stdio.h>


void byteorder()
{
    union
    {
        short value;
        char union_bytes[sizeof(short)];
    }test;
    test.value = 0x0102;
    if((test.union_bytes[0]==1)&&(test.union_bytes[1] == 2))
    {
        printf("big");
    }
    else if((test.union_bytes[0]==2)&&(test.union_bytes[1] == 1))
    {
        printf("little");
    }
    else
    {
        printf("unknow");
    }
}


int main()
{
    byteorder();
    return 0;
}