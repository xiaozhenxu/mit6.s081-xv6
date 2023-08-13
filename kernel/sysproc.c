#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

/* 该文件的其他系统调用函数的参数都是void，因为获取参数使用的是argcint()方法，argint()用于读取在a0-a5寄存器中传递的系统调用参数 */
/* myproc()函数可以获取当前进程的进程控制块（PCB）*/
/* 该函数通过在__proc结构体__中的新变量中记住其参数来实现新的系统调用 */
uint64
sys_trace(void)
{
  /* 获得传进来的参数->掩码 */
  int mask;
  if(argint(0, &mask) < 0)
  {
    return -1;
  }

  /* 把掩码赋值给进程控制块中的新变量 ---->  trace_mask */
  myproc()->tracemask = mask;

  return 0;
}

uint64
sys_sysinfo(void)
{
  printf("sysinfo\n");
  return 0;
}