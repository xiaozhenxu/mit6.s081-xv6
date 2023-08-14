# lab: Xv6 and Unix utilities
*****
本实验所用的代码 xv6-labs-2021 与 xv6-riscv 稍有不同，可以通过日志记录来查看具体做了哪些改动
```
git log
```
除了查看提交日志，也可以通过命令 git diff 来展示最新一次 commit 后的改动，也可以通过下面的命令来查看代码与源代码某一分支的区别。origin 在本地端指向 xv6 源代码。
```
git diff origin/util
```
切换到 lab01 分支的命令如下
```
git checkout util
```

**运行 xv6 操作系统**

make qemu 成功后进入操作系统界面，在操作系统界面运行 ls 会输出当前根目录下的文件信息，这些文件是由 mkfs 创建的初始化文件系统。

xv6 没有 ps 命令，如果想要查看进程信息，可以 ctrl-p 查看，当前是两个进程 init 进程和 sh 进程。

**分数判定**

在任一条分支下都可以通过 make grade 来检查实验结果是否通过。

## 课程笔记
****
### 操作系统接口

操作系统的接口需要遵守两个原则：
	1. 接口简单明了，便于使用 
	2. 为应用程序提供复杂的特性。为实现两个看似矛盾的目的，需要***依赖一些可结合的机制实现更好的通用性***。

xv6采用传统的内核形式。内核是一个特殊的程序，一台计算机通常有多个进程，但只有一个内核，内核程序具有操纵硬件的权限，而用户程序执行的时候没有这些权限，这是硬件所实现的。

shell 是一个普通的用户程序，从用户那里读取命令并执行它们。shell 的实现在[user/sh.c](https://gitee.com/xiaozhenxu/mit6.s081-xv6/blob/util/user/sh.c)。shell 的主要执行逻辑可以查看 main 函数，主循环调用 getcmd 函数从用户的输入中读取一行，然后调用 fork 创建一个 shell 的副本，父进程调用 wait 函数，子进程执行命令。

例如：当用户输入 echo hello 时，runcmd 函数将以 echo hello 为参数被调用来执行命令，在某一个 echo 会调用 exit，浙江导致父进程从 main 当中的 wait 返回。

一些常用的系统调用接口如下
![Fileter_transaction](https://gitee.com/xiaozhenxu/mit6.s081-xv6/blob/master/xv6%E5%AE%9E%E9%AA%8C%E8%AE%B0%E5%BD%95/fig/Pasted%20image%2020230806191719.png)
**IO和文件描述符** 

每一个进程都有一个从零开始的文件描述符的私有空间。进程从文件描述符0读取(标准输入)，将输出写入文件描述符1(标准输出)，并将错误消息写入文件描述符2(标准错误)。以下为 cat 程序的部分实现
```c
char buf[512];
int n;
for(;;)
{
	n = read(0, buf, sizeof(buf));
	if(n == 0) break;
	if(n < 0)
	{
		fprintf(2, "read error\n");
		exit(1);
	}
	if(write(1, buf, n) != n)
	{
		fprintf(2, "write error\n");
		exit(1);
	}
}

/* 可以看出的是 cat 并不知它打印的是文件还是控制台或者管道 */
char *argv[2];
argv[0] = "cat";
argv[1] = 0;
if(fork()==0)
{
	/*
	fork和exec 分开的作用就在这里，可以有更好的通用性，获得自己专有的 io 设置
	*/
	close(0);  // 关闭标准输入，以便重定向
	open("input.txt", O_RDONLY);
	exec("cat", argv);
}
```

dup 系统调用复制一个现有的文件描述符，返回一个引用自同一个底层I/O对象的新文件描述符，通过一系列 fork 和 dup 调用从同一个原始文件描述符派生出来的，共享一个偏移量。
```c
int fd = dup(1);
write(1, "hello ", 6);
write(fd, "world\n", 6);
```

**文件系统**

mknod 函数创建一个引用设备的新的设备文件，与设备文件相关联的是主设备号和次设备号( mknod 的两个参数)，它们唯一的标识了一个内核设备。当进程打开设备文件的时候，内核将使用内核设备实现 read 和 write 系统调用，而不是使用文件系统。

fstat 系统调用从文件描述符所引用的 inode 中检索信息。填充一个 stat 类型的结构体，在[kernel/stat.h](https://gitee.com/xiaozhenxu/mit6.s081-xv6/blob/util/kernel/stat.h)中定义
```c
#define T_DIR 1    // Directory
#define T_FILE 2   // File
#define T_DEVICE 3 // Device
struct stat {
    int dev;     // 文件系统的磁盘设备
    uint ino;    // Inode编号
    short type;  // 文件类型
    short nlink; // 指向文件的链接数
    uint64 size; // 文件字节数
};

```

创建一个没有名称的临时 inode 的方法，该临时 inode 将在进程关闭 fd 或退出的时候被清理。
```c
fd = open("/tmp/xyz", O_CREATE | O_RDWR);
unlink("/tmp/xyz");
```
## sleep
****
该实验目的是在用户态下实现一个 sleep 函数，该函数位于[user/sleep.c](https://gitee.com/xiaozhenxu/mit6.s081-xv6/blob/util/user/sleep.c)，只要调用系统调用函数 sleep 就可以。

注意点：  
	1. 命令行参数为字符串类型，sleep 函数在接收参数的时候需要利用 atoi 转换为整型，注意这里的 atoi 函数不是来自C标准库函数，而是 xv6 在[user/ulib.c](https://gitee.com/xiaozhenxu/mit6.s081-xv6/blob/util/user/ulib.c) 中定义  
	3. 系统调用 sleep 的申明位于[user/user.h](https://gitee.com/xiaozhenxu/mit6.s081-xv6/blob/util/user/user.h)  
	4. 用户态完成的函数需要在 Makefile UPROGS 填写，以成功编译   

代码的具体实现可以 git show f654383..0bdddcd 或者 查看 commit 记录 sleep 实现
## pingpong
该实验目的是在用户态下实现一个 pingpong 函数，该函数位于[user/pingpong.c](https://gitee.com/xiaozhenxu/mit6.s081-xv6/blob/util/user/pingpong.c)，主要考察了进程的创建、通信等简单应用。

注意点
	1. 需要用到 pipe fork getpid read 等系统调用  
	2. 用户态和内核态提供的函数都在[user/user.h](https://gitee.com/xiaozhenxu/mit6.s081-xv6/blob/util/user/user.h)中申明，其用户态的函数定义在[user/ulib.c](https://gitee.com/xiaozhenxu/mit6.s081-xv6/blob/util/user/ulib.c)，[user/printf.c](https://gitee.com/xiaozhenxu/mit6.s081-xv6/blob/util/user/printf.c)，[user/mallloc.c](https://gitee.com/xiaozhenxu/mit6.s081-xv6/blob/util/user/umalloc.c)

代码的具体实现可以 git show 0bdddcd..af9bc9a 或者 查看 commit 记录 pingpong 实现

## primes
该实验目的是利用管道实现prime sieve(素数过滤)的并发版本，该函数位于[user/primes.c](https://gitee.com/xiaozhenxu/mit6.s081-xv6/blob/util/user/primes.c)文件中。

第一个进程传输数字2-35到管道。对于每一个素数都会有一个进程通过一个管道从左邻居读入，并通过另一个管道向其右邻居写入。

注意点：  
	1. 一开始的方法是构建一个数组去存储再发送，但这并不符合并发的概念，因此现在的逻辑是接收数据同时发送数据  
	2. 在并发的概念下就需要考虑最终数据输出的有序性，父进程可以知道子进程何时结束(wait)，但是这样的化父进程需要输出从大到小输出素数，这样也不多。因此本方案的实现是依靠 read 函数，子进程在检测到父进程的写端关闭后输出素数

代码的具体实现可以 git show af9bc9a..2009e10 或者 查看 commit 记录 primes 实现

## find
该实验的目的是实现 find 命令，该函数位于[user/find.c](https://gitee.com/xiaozhenxu/mit6.s081-xv6/blob/util/user/find.c)文件中。

注意点：  
	1. 采用递归的方式深入子目录中  
	2. 遇到 ‘.' 或者 ’..' 需要停止搜索  
	3. 掌握文件存储在应用层的接口。特别是对文件夹的读取，得到的内容是一个个 entry ，其存储形式在[kernel/fs.h](https://gitee.com/xiaozhenxu/mit6.s081-xv6/blob/util/kernel/fs.h)中定义  
	4. 如何读取一个文件夹可以参考 [user/ls.c](https://gitee.com/xiaozhenxu/mit6.s081-xv6/blob/util/user/ls.c)

代码的具体实现可以 git show 2009e10..9b0ac3e 或者 查看 commit 记录 find 实现

## xargs
本实现的目的是实现 xargs 命令，该函数位于[user/xargs.c](https://gitee.com/xiaozhenxu/mit6.s081-xv6/blob/util/user/xargs.c)

首先介绍一下 xargs 命令，Unix 命令都带有参数，而有一部分命令可以接收**标准输入**作为参数，其中 grep 便可以接收标准输入，下面的命令便是将左侧命令的标准输出转换为标准输入作为 grep 的参数
```
cat /etc | grep root
```
但是 echo 命令便不能接受标准输入，如下命令是不能成功运行的，需要用 xargs 将输出转换为参数
```
echo hello world | echo
```

注意点：  
	1. 使用 fork 和 exec 在每一行输入中调用该命令，在父进程中使用 wait 以等待子进程完成命令  
	2. 要读取单个输入行，一次读取一个字符，知道出现换行符”\n”，一定要单个字符地读取  
	3. 输入 exec 函数参数项的第一个参数没有什么影响  

代码的具体实现可以 git show 9b0ac3e..b527749 或者 查看 commit 记录 xargs 实现