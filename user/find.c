#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void find(const char* path, const char* expression);

int main(int argc,char* argv[])
{
    if(argc!=3){
        printf("usage: find [path] [key]\n");
        exit(1);
    }
    const char* path = argv[1];
    const char* expression = argv[2];

    find(path,expression);
    exit(0);
}

void find(const char* path, const char* expression)
{
    struct stat st;
    struct dirent de;
    const char* ptr;  // 用来指向最后一个'/'之后的文件名
    char* p;    // 指向buf中
    char buf[256];  // 存储目录地址
    /*获取文件信息*/
    if(stat(path, &st)<0)
    {
        /*获取文件信息失败*/
        printf("find: stat failure\n");
        exit(1);
    }

    /*打开文件描述符*/
    int fd = open(path,0);
    if(fd<0){
        printf("find: open failure\n");
        exit(1);
    }

    switch(st.type){
        case T_FILE:
            /*文件类型*/
            for(ptr=path+strlen(path);ptr>=path && *ptr!='/';ptr--);
            ptr++;

            /*比较文件名*/
            if(strcmp(ptr,expression)==0)
            {
                printf("%s\n",path);
            }
            break;
        case T_DIR:
            /*目录类型*/
            while((read(fd,&de,sizeof(de)))==sizeof(de))
            {
                if(de.inum==0){
                    continue;
                }
                //printf("de: %s\n",de.name);
                if(strcmp(de.name,".")==0 || strcmp(de.name,"..")==0){
                    continue;
                }

                /*新创建一块内存存储下一个目录*/
                strcpy(buf,path);
                //printf("buf1: %s\n",buf);
                p = buf + strlen(buf);
                *p = '/';
                p++;
                strcpy(p, de.name);
                //printf("buf2: %s\n",buf);
                p[DIRSIZ] = '\0';
                find(buf,expression);
            }
            break;

    }
    close(fd);
    return;
}