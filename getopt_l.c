#include<stdio.h>
#include<getopt.h>
#include<string.h>
#include<unistd.h>


int main(int argc, char*argv[])
{
    int opt;
    char *string = "ab:c:d";
    while((opt = getopt(argc,argv,string)) != -1)
    {
        printf("opt = %c\t\t",opt);
        printf("optarg = %s\t\t",optarg);
        printf("optind = %d\t\t",optind);
        printf("argc[optind] = %s\n",argv[optind]);
    }
    return 0;
}

   