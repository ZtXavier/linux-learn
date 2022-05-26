// #include<stdio.h>
// #include<unistd.h>
// #include<sys/types.h>
// #include<sys/stat.h>
// #include<syslog.h>
// #include<signal.h>
// #include<sys/param.h>
// #include<fcntl.h>
// #include<stdlib.h>
// #include<time.h>

// int daemon_t()
// {
//     pid_t pid;
//     int fd;
//     int i;
//     signal(SIGTTOU,SIG_IGN);
//     signal(SIGTTIN,SIG_IGN);
//     signal(SIGTSTP,SIG_IGN);
//     signal(SIGHUP,SIG_IGN);

//     printf("ppid = %d\n",getppid());
//     pid = fork();
//     fd = open("/dev/null",O_RDONLY);
//     printf("fd = %d\n",fd);
//     close(fd);
//     if(pid > 0)
//     {
//         printf("Parent proc pid = %d\n",getpid());
//         exit(0);
//     }
//     else if(pid < 0)
//     {
//         printf("error!");
//         return -1;
//     }
//     printf("first Child process pid = %d\n",getpid());
//     printf("pgid = %d\n",getpgid(getpid()));
//     setsid();
//     printf("new pgid = %d\n",getpgid(getpid()));
//     fd = open("/dev/null",O_RDONLY);
//     printf("fd = %d\n",fd);
//     close(fd);

//     pid = fork();
//     fd = open("/dev/null",O_RDONLY);
//     printf("fd = %d\n",fd);
//     close(fd);
//     if(pid > 0)
//     {
//         printf("Parent proc pid = %d\n",getpid());
//         exit(0);
//     }
//     else if(pid < 0)
//     {
//         printf("error!");
//         return -1;
//     }




//     for(i = 0;i < NOFILE;close(i++));
//     chdir("/");
//     umask(0);
//     signal(SIGCHLD,SIG_IGN);
//     return 0;
// }


// int main()
// {
//     time_t now;
//     daemon_t();
//     syslog(LOG_USER | LOG_INFO,"测试守护进程\n");
//     while(1)
//     {
//         sleep(8);
//         time(&now);
//         syslog(LOG_USER|LOG_INFO,"系统时间:\t%s\t\t\n",ctime(&now));
//     }
//     return 0;
// }


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>
int daemon_init(void)
{
  pid_t pid;
  if ((pid = fork()) < 0)
    return (-1);
  else if (pid != 0)
    exit(0); /* parent exit */
 
  /* child continues */
  setsid();   /* become session leader */
  chdir("/"); /* change working directory */
  umask(0);   /* clear file mode creation mask */
  close(0);   /* close stdin */
  close(1);   /* close stdout */
  close(2);   /* close stderr */
  return (0);
}
 
void sig_term(int signo) {
  if (signo == SIGTERM)
  /* catched signal sent by kill(1) command */
  {
    syslog(LOG_INFO, "program terminated.");
    closelog();
    exit(0);
  }
}
 
int main(void) {
  if (daemon_init() == -1) {
    printf("can't fork self\n");
    exit(0);
  }
 
  openlog("daemontest", LOG_PID, LOG_USER);
  syslog(LOG_INFO, "program started.");
  signal(SIGTERM, sig_term); /* arrange to catch the signal */
 
  while (1) {
    sleep(1); /* put your main program here */
  }
 
  return (0);
}