#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char* argv[])
{
    int parReadFd[2];
    int sonReadFd[2];
    pipe(parReadFd);
    pipe(sonReadFd);

    if(fork()!=0)
    {
        int status = 0;

        // 父进程
        close(parReadFd[1]);
        close(sonReadFd[0]);

        // 向子进程发送数据
        write(sonReadFd[1], "ping", strlen("ping")+1);
        
        // 接收子进程数据
        char readbuf[8];
        memset(readbuf, 0, sizeof(readbuf));
        read(parReadFd[0], readbuf, sizeof(readbuf));
        
        wait(&status);
        printf("%d: received %s\n",getpid(),readbuf);

    }
    else
    {
        // 子进程
        close(parReadFd[0]);
        close(sonReadFd[1]);

        // 向父进程发送数据
        write(parReadFd[1], "pong", strlen("pong")+1);
        
        // 接收子进程数据
        char readbuf[8];
        memset(readbuf, 0, sizeof(readbuf));
        read(sonReadFd[0], readbuf, sizeof(readbuf));
        
        printf("%d: received %s\n",getpid(),readbuf);
        
        exit(0);
    }
    exit(0);
}