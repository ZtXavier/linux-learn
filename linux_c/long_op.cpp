#include<stdio.h>
#include<stdlib.h>
#include<getopt.h>


int main(int argc, char*argv []) {
    int c;
    
    int digit_optind = 0;
    while(1) {
        int lopt;
        // optind 是下一个将被处理的在argv中的下标值
        int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        static struct option long_options[] = {
          {"create",  required_argument, 0, 'c'},
          {"add", required_argument, &lopt, 1},
          {"append",  required_argument,  &lopt, 2},
          {"delete",  required_argument, &lopt,  3 },
          {"verbose", required_argument, &lopt,  4 },
          {"file",    required_argument, &lopt, 5 },
          {0, 0, 0, 0}
        };
               c = getopt_long(argc, argv, "abc:d:012",
                 long_options, &option_index);
        if (c == -1)
            break;
 
       switch (c) {
        case 0:
               switch(lopt) {
                    case 1 :
                    printf(" with arg %d", lopt);
                    break;
                    case 2:
                    printf(" with arg %d", lopt);
                    break;
               }
            printf("option %s", long_options[option_index].name);
           //  optarg：表示当前选项对应的参数值。
            if (optarg)
                printf(" with arg %s", optarg);
            printf("\n");
            break;
 
       case '0':
        case '1':
        case '2':
            if (digit_optind != 0 && digit_optind != this_option_optind)
              printf("digits occur in two different argv-elements.\n");
            digit_optind = this_option_optind;
            printf("option %c\n", c);
            break;
 
       case 'a':
            printf("option a\n");
            break;
 
       case 'b':
            printf("option b\n");
            break;
 
       case 'c':
            printf("option c with value '%s'\n", optarg);
            break;
 
       case 'd':
            printf("option d with value '%s'\n", optarg);
            break;
 
       case '?':
            break;
 
       default:
            printf("?? getopt returned character code 0%o ??\n", c);
        }
    }
 
   if (optind < argc) {
        printf("non-option ARGV-elements: ");
        while (optind < argc)
            printf("%s ", argv[optind++]);
        printf("\n");
    }
 
   exit(EXIT_SUCCESS);
}