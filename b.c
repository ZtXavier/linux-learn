#include<stdio.h>
#include<string.h>
#include<stdlib.h>

// int main(void){
//     int a = 0x12345678;
//     char ch = (char)a;
//     printf("%c",ch);
//     return 0;
// }

//思考下面输出的值，解释原因
void func()
{
    int num = 2021;
    int res = 5;
    for(;num != 0;)
    {
        res += 2;
        num = (num - 1) & num;
    }
    printf("Xiyou Linux 20%d",res);
}
//思考下面输出的值，解释原因
void func2()
{
    char ch = 'B';
    int i = 132;
    i >>= 1;
    unsigned int f = 88667788;
    *(int*)&f >>= 24;
    *(int*)&f = *(int*)&f + '=';
    printf("ch = %c i = %c f = %c\n",ch,i,*(int*)&f);
}

//思考下面输出的值，解释原因
void func3()
{
    int nums[3][3] = {1,2,3,4,5,6,7,8,9};
    printf("%d\n",nums[2][-2]);
    printf("%d\n",(-2)[nums][10]);
    printf("%d\n",-1[nums][1]);
    int num  = getc(stdin);
    printf("%d",num);
    char *ch = "awdfg";
    puts(ch);
}





int main(int argc, char **argv)
{
    // func();
    // func2();
    func3();
    return 0;
}