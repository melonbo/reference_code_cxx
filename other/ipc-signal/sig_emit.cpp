#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

main(int argc,char**argv)
{ 
    int signum = atoi(argv[1]);;
    pid_t pid = (pid_t)atoi(argv[2]);;

    char data[10];
    memset(data,0,sizeof(data));
    for(int i=0; i < 5; i++)
        data[i]='2';
    

    union sigval mysigval;
    mysigval.sival_int=8;
 //   mysigval.sival_ptr=data;//可以被用来在同一进程的不同上下文中传递数据，但是不能直接在不同进程之间传递数据。

    if(sigqueue(pid,signum,mysigval)==-1)
        printf("send error\n");
        
    sleep(2);
}
