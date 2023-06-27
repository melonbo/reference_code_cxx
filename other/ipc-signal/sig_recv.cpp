#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

void new_op(int,siginfo_t*,void*);
int main(int argc,char**argv)
{    
    int sig = atoi(argv[1]);     
    pid_t pid = getpid();
    
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_sigaction=new_op;
    act.sa_flags=SA_SIGINFO;
    printf("listen signal %d\n", sig);

    if(sigaction(sig,&act,NULL)<0)
    {
        printf("install sigal error %d\n", errno);
    }

    while(1)
    {
        sleep(2);
        printf("wait for the signal\n");
    }
}

void new_op(int signum,siginfo_t *info,void *myact)
{
    printf("receive signal %d\n", signum);
    printf("the int value is %d\n", info->si_int);    
/*
    char *p = (char*)info->si_ptr;
    for(int i=0;i<10;i++)
    {
        printf("%c ", *(p+i));
    }
    printf("\n");
*/
}
