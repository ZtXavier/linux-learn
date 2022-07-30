#include<stdio.h>
#include<unistd.h>
#include<getopt.h>



int main(int argc, char*argv[])
{
    int opt;
    int digit_optind = 0;
    int option_index = 0;
    char *string = "a::b:c:d";
    static struct option long_option[] = {
        {"reqarg",required_argument,NULL,'r'},
        {"optarg",optional_argument,NULL,'o'},
        {"noarg",no_argument,NULL,'n'}

    };
    while((opt = getopt_long(argc,argv,string,long_option,&option_index)) != -1){
        printf("opt = %c\t\t", opt);
        printf("optarg = %s\t\t",optarg);
        printf("optind = %d\t\t",optind);
        printf("argv[optind] =%s\t\t", argv[optind]);
        printf("option_index = %d\n",option_index);
    }
    return 0;
}