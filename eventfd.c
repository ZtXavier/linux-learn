#include <sys/eventfd.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>             /* Definition of uint64_t */
 
#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)
 
int
main(int argc, char *argv[])
{
    int efd, j;
    uint64_t u;
    ssize_t s;
    
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <num>...\n", argv[0]);
        exit(EXIT_FAILURE);
    }   
    // 作为进程间的通信符号
    efd = eventfd(0, 0); 
    if (efd == -1) 
        handle_error("eventfd");
    
    switch (fork()) {
        case 0:
            for (j = 1; j < argc; j++) {
            printf("Child writing %s to efd\n", argv[j]);
            u = strtoull(argv[j], NULL, 0); 
            /* strtoull() allows various bases */
            printf("strtoul is %ld\n",u);
            s = write(efd, &u, sizeof(uint64_t));
            if (s != sizeof(uint64_t))
                handle_error("write");
            }
            printf("Child completed write loop\n");
            exit(EXIT_SUCCESS);
        default:
            sleep(2);
 
            printf("Parent about to read\n");
            s = read(efd, &u, sizeof(uint64_t));
            if (s != sizeof(uint64_t))
                handle_error("read");
            printf("Parent read %llu (0x%llx) from efd\n",
                   (unsigned long long) u, (unsigned long long) u);
            exit(EXIT_SUCCESS);
 
        case -1:
            handle_error("fork");
  }
}