#ifndef OS_UTILS_H
#define OS_UTILS_H

#include <iostream>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <execinfo.h>
#include <sys/vfs.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <regex>

typedef struct _cpu_info
{
    char name[20];
    unsigned int user;
    unsigned int nice;
    unsigned int system;
    unsigned int idle;
    unsigned int iowait;
    unsigned int irq;
    unsigned int softirq;
}cpu_info_t;

typedef struct _cpu_info_app
{
    unsigned int pid;//6873 进程(包括轻量级进程，即线程)号
    char comm[100];//a.out 应用程序或命令的名字。
    char task_state[100];//R 任务的状态，R:runnign, S:sleeping (TASK_INTERRUPTIBLE), D:disk sleep (TASK_UNINTERRUPTIBLE), T: stopped, T:tracing stop, Z:zombie, X:dead。
    unsigned int ppid;//6723 父进程ID。
    unsigned int pgid;//6873 线程组号。
    unsigned int sid;//6723 该任务所在的会话组 ID。
    unsigned int tty_nr;//34819(pts/3) 该任务的 tty 终端的设备号，INT（34817/256）=主设备号，（34817-主设备号）= 次设备号。
    unsigned int tty_pgrp;//6873 终端的进程组号，当前运行在该任务所在终端的前台任务(包括 shell 应用程序)的 PID。
    unsigned int task_flags;//8388608 进程标志位，查看该任务的特性。
    unsigned int min_flt;//77 该任务不需要从硬盘拷数据而发生的缺页（次缺页）的次数。
    unsigned int cmin_flt;//0 累计的该任务的所有的 waited-for 进程曾经发生的次缺页的次数目。
    unsigned int maj_flt;//0 该任务需要从硬盘拷数据而发生的缺页（主缺页）的次数。
    unsigned int cmaj_flt;//0 累计的该任务的所有的 waited-for 进程曾经发生的主缺页的次数目。
    unsigned int utime;//1587 该任务在用户态运行的时间，单位为 jiffies。14
    unsigned int stime;//1 该任务在核心态运行的时间，单位为 jiffies。
    unsigned int cutime;//0 累计的该任务的所有的 waited-for 进程曾经在用户态运行的时间，单位为 jiffies。
    unsigned int cstime;//0 累计的该任务的所有的 waited-for 进程曾经在核心态运行的时间，单位为 jiffies。
    unsigned int priority;//25 任务的动态优先级。
    unsigned int nice;//0 任务的静态优先级。
    unsigned int num_threads;//3 该任务所在的线程组里线程的个数。
    unsigned int it_real_value;//0 由于计时间隔导致的下一个 SIGALRM 发送进程的时延，以 jiffy 为单位。
    unsigned int start_time;//5882654 该任务启动的时间，单位为 jiffies。
    unsigned int vsize;//1409024（page） 该任务的虚拟地址空间大小。
    unsigned int rss;//56(page) 该任务当前驻留物理地址空间的大小；Number of pages the process has in real memory,minu 3 for administrative purpose. 这些页可能用于代码，数据和栈。
    unsigned int rlim;//4294967295（bytes） 该任务能驻留物理地址空间的最大值。
    unsigned int start_code;//134512640 该任务在虚拟地址空间的代码段的起始地址。
    unsigned int end_code;//134513720 该任务在虚拟地址空间的代码段的结束地址。
    unsigned int start_stack;//3215579040 该任务在虚拟地址空间的栈的结束地址。
    unsigned int kstkesp;//0 esp(32 位堆栈指针) 的当前值, 与在进程的内核堆栈页得到的一致。
    unsigned int kstkeip;//2097798 指向将要执行的指令的指针, EIP(32 位指令指针)的当前值。
    unsigned int pendingsig;//0 待处理信号的位图，记录发送给进程的普通信号。
    unsigned int block_sig;//0 阻塞信号的位图。
    unsigned int sigign;//0 忽略的信号的位图。
    unsigned int sigcatch;//082985 被俘获的信号的位图。
    unsigned int wchan;//0 如果该进程是睡眠状态，该值给出调度的调用点。
    unsigned int nswap;//被 swapped 的页数，当前没用。
    unsigned int cnswap;//所有子进程被 swapped 的页数的和，当前没用。
    unsigned int exit_signal;//17 该进程结束时，向父进程所发送的信号。
    unsigned int task_cpu;//0 运行在哪个 CPU 上。
    unsigned int task_rt_priority;//0 实时进程的相对优先级别。
    unsigned int task_policy;//0 进程的调度策略，0=非实时进程，1=FIFO实时进程；2=RR实时进程
}app_info_t;



int  getLocalEthernetInfo(void);//获取设备网卡信息
int  get_local_ip(const char *eth_inf, char *ip);//根据网卡名称获取IP地址
void regitsterInterrupt();//注册系统中断处理函数
int isNetworkPortOpened(int port);//判断一个端口是否打开
void initDaemon();//设置守护进程

bool get_cpu_occupy(cpu_info_t *info);
double calc_cpu_rate(cpu_info_t *old_info, cpu_info_t *new_info);
double calc_cpu_rate_app(cpu_info_t *old_info, cpu_info_t *new_info, app_info_t *old_info_app, app_info_t *new_info_app);
int get_pid_by_name(char* process_name, pid_t pid_list[], int list_size);
int is_process_exist(char* process_name);
void get_cpu_occupy_by_pid(app_info_t *info, pid_t pid);
void get_cpu_occupy_by_group_pid(app_info_t *info);
void get_memory_occupy();


int  getDiskVolumeTotalGB(char* path);//获取硬盘总容量，单位GB
int  getDiskVolumeFreeGB(char* path);//获取硬盘空闲容量，单位GB
int  getDiskVolumeUsedGB(char* path);//获取硬盘使用容量，单位GB
int  getDiskVolumeTotalMB(char* path);//获取硬盘总容量，单位MB
int  getDiskVolumeFreeMB(char* path);//获取硬盘空闲容量，单位MB
int  getDiskVolumeUsedMB(char* path);//获取硬盘使用容量，单位MB

#endif // OS_UTILS_H
