#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void process(int readfd);

int main(int argc, char* argv[])
{
    int fd[2];
    pipe(fd);

    if(fork()>0)
    {
        int status = 0;
        close(fd[0]);
        /* 第一个进程发送数据2-35 */
        for(int i = 2; i<=35; i++)
        {
            write(fd[1], &i, sizeof(int));
        }
        close(fd[1]);
        wait(&status);
        exit(0);
    }
    else
    {
        /* 子进程进入递归 */
        close(fd[1]);
        process(fd[0]);
        exit(0);
    }

    exit(0);
}

uint8 isPrime(const int num)
{
    if(num==2) return 1;
    for(int i=2; i<num; i++)
    {
        if(num%i==0) return 0;
    }
    return 1;
}

void process(int readfd)
{
    int num = 0;
    if (read(readfd,&num,sizeof(int))==0)
    {
        /* 没有数据且没有写端 */
        close(readfd);
        return;
    }

    int fd[2];
    pipe(fd);

    if(fork()>0)
    {
        int prime = -1;
        int status = 0;
        close(fd[0]);
        do{
            if(prime != -1)
            {
                /* 向子进程发送数据 */
                write(fd[1],&num,sizeof(int));
            }
            else
            {
                if(isPrime(num+2)==1)
                {
                    /* 获得该进程的第一个素数 */
                    prime = num;
                }
            }
        }while(read(readfd, &num, sizeof(int))!=0);
        close(readfd);
        if(prime!=-1) printf("prime %d\n",prime);
        close(fd[1]);
        wait(&status);
    }
    else
    {
        /* 子进程 */
        close(fd[1]);
        process(fd[0]);
        exit(0);
    }

    return;
}