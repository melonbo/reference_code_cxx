#include "os_utils.h"


int getLocalEthernetInfo(void)
{
    int fd;
    int interfaceNum = 0;
    struct ifreq buf[16];
    struct ifconf ifc;
    struct ifreq ifrcopy;
    char mac[16] = {0};
    char ip[32] = {0};
    char broadAddr[32] = {0};
    char subnetMask[32] = {0};

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket");

        close(fd);
        return -1;
    }

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = (caddr_t)buf;
    if (!ioctl(fd, SIOCGIFCONF, (char *)&ifc))
    {
        interfaceNum = ifc.ifc_len / sizeof(struct ifreq);
        printf("network interface num = %d\n", interfaceNum);
        while (interfaceNum-- > 0)
        {
            printf("\ndevice name: %s\n", buf[interfaceNum].ifr_name);

            //ignore the interface that not up or not runing
            ifrcopy = buf[interfaceNum];
            if (ioctl(fd, SIOCGIFFLAGS, &ifrcopy))
            {
                printf("ioctl: %s [%s:%d]\n", strerror(errno), __FILE__, __LINE__);

                close(fd);
                return -1;
            }

            //get the mac of this interface
            if (!ioctl(fd, SIOCGIFHWADDR, (char *)(&buf[interfaceNum])))
            {
                memset(mac, 0, sizeof(mac));
                snprintf(mac, sizeof(mac), "%02x%02x%02x%02x%02x%02x",
                    (unsigned char)buf[interfaceNum].ifr_hwaddr.sa_data[0],
                    (unsigned char)buf[interfaceNum].ifr_hwaddr.sa_data[1],
                    (unsigned char)buf[interfaceNum].ifr_hwaddr.sa_data[2],

                    (unsigned char)buf[interfaceNum].ifr_hwaddr.sa_data[3],
                    (unsigned char)buf[interfaceNum].ifr_hwaddr.sa_data[4],
                    (unsigned char)buf[interfaceNum].ifr_hwaddr.sa_data[5]);
                printf("device mac: %s\n", mac);
            }
            else
            {
                printf("ioctl: %s [%s:%d]\n", strerror(errno), __FILE__, __LINE__);
                close(fd);
                return -1;
            }

            //get the IP of this interface

            if (!ioctl(fd, SIOCGIFADDR, (char *)&buf[interfaceNum]))
            {
                snprintf(ip, sizeof(ip), "%s",
                    (char *)inet_ntoa(((struct sockaddr_in *)&(buf[interfaceNum].ifr_addr))->sin_addr));
                printf("device ip: %s\n", ip);
            }
            else
            {
                printf("ioctl: %s [%s:%d]\n", strerror(errno), __FILE__, __LINE__);
                close(fd);
                return -1;
            }

            //get the broad address of this interface

            if (!ioctl(fd, SIOCGIFBRDADDR, &buf[interfaceNum]))
            {
                snprintf(broadAddr, sizeof(broadAddr), "%s",
                    (char *)inet_ntoa(((struct sockaddr_in *)&(buf[interfaceNum].ifr_broadaddr))->sin_addr));
                printf("device broadAddr: %s\n", broadAddr);
            }
            else
            {
                printf("ioctl: %s [%s:%d]\n", strerror(errno), __FILE__, __LINE__);
                close(fd);
                return -1;
            }

            //get the subnet mask of this interface
            if (!ioctl(fd, SIOCGIFNETMASK, &buf[interfaceNum]))
            {
                snprintf(subnetMask, sizeof(subnetMask), "%s",
                    (char *)inet_ntoa(((struct sockaddr_in *)&(buf[interfaceNum].ifr_netmask))->sin_addr));
                printf("device subnetMask: %s\n", subnetMask);
            }
            else
            {
                printf("ioctl: %s [%s:%d]\n", strerror(errno), __FILE__, __LINE__);
                close(fd);
                return -1;

            }
        }
    }
    else
    {
        printf("ioctl: %s [%s:%d]\n", strerror(errno), __FILE__, __LINE__);
        close(fd);
        return -1;
    }

    close(fd);

    return 0;
}

int get_local_ip(const char *eth_inf, char *ip)
{
    int sd;
    struct sockaddr_in sin;
    struct ifreq ifr;
#define IP_SIZE 	16

    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sd)
    {
        printf("socket error: %s\n", strerror(errno));
        return -1;
    }

    strncpy(ifr.ifr_name, eth_inf, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;

    // if error: No such device
    if (ioctl(sd, SIOCGIFADDR, &ifr) < 0)
    {
        printf("ioctl error: %s\n", strerror(errno));
        close(sd);
        return -1;
    }

    memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
    snprintf(ip, IP_SIZE, "%s", inet_ntoa(sin.sin_addr));
    printf("get local ip %s\n", ip);

    close(sd);
    return 0;
}


void signal_int(int var)
{
    switch (var)
    {
        case 1:
            printf("signal is SIGHUP\n");
            break;
        case 2:
            printf("signal is SIGINT\n");
            break;
        case 3:
            printf("signal is SIGQUIT\n");
            break;
        case 4:
            printf("signal is SIGILL\n");
            break;
        case 6:
            printf("signal is SIGABRT\n");
            break;
        case 8:
            printf("signal is SIGFPE\n");
            break;
        case 9:
            printf("signal is SIGKILL\n");
            break;
        case 11:
            printf("signal is SIGSEGV\n");
            break;
        case 13:
            printf("signal is SIGPIPE\n");
            break;
        case 14:
            printf("signal is SIGALRM\n");
            break;
        case 15:
            printf("signal is SIGTERM\n");
            break;
        case 30:
            printf("signal is SIGUSR1 30\n");
            break;
        case 10:
            printf("signal is SIGUSR1 10\n");
            break;
        case 16:
            printf("signal is SIGUSR1 16\n");
            break;
        case 31:
            printf("signal is SIGUSR2 31\n");
            break;
        case 12:
            printf("signal is SIGUSR2 12\n");
            break;
        case 17:
            printf("signal is SIGUSR2 17\n");
            break;
        default:
            printf("signal_num = %d", var);
            break;
        }
}

void regitsterInterrupt()
{
    signal(SIGHUP, signal_int);
    signal(SIGINT, signal_int);
    signal(SIGQUIT, signal_int);
    signal(SIGILL, signal_int);
    signal(SIGABRT, signal_int);
    signal(SIGFPE, signal_int);
    signal(SIGKILL, signal_int);
    signal(SIGSEGV, signal_int);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGALRM, signal_int);
    signal(SIGTERM, signal_int);
    signal(SIGUSR1, signal_int);
    signal(SIGUSR2, signal_int);
    signal(SIGSTOP, signal_int);
    signal(SIGTSTP, signal_int);
    signal(SIGTTIN, signal_int);
    signal(SIGTTOU, signal_int);
}


void initDaemon()
{
    int pid;
    int i;
    if(pid=fork())
    {
        exit(0);//是父进程，结束父进程
    }
    else if(pid< 0)
        exit(1);//fork失败,退出

    //是第一子进程,后台继续执行
    setsid();//第一子进程成为新的会话组长和进程组长,并与控制终端分离
    if(pid=fork())
        exit(0);//是第一子进程，结束第一子进程
    else if(pid< 0)
        exit(1);//fork失败,退出


    //是第二子进程,继续,第二子进程不再是会话组长
    for(i=0;i< NOFILE;++i)//关闭打开的文件描述符
        close(i);

    chdir("/home/root/app");//改变工作目录到/home/root/app

    umask(0);//重设文件创建掩模
    return;
}


//**************************************************
//功能：获取设备CPU状态，应用程序CPU使用率
//**************************************************

bool get_cpu_occupy(cpu_info_t *info)
{
    FILE *fp = NULL;
    char buf[256] = {0};
    fp = fopen("/proc/stat","r");
    if(!fp)
    {
        printf("open file failed\n");
        return false;
    }

    fgets(buf, sizeof(buf),fp);
    //printf("buf = %s\n",buf);

    sscanf(buf, "%s %u %u %u %u %u %u %u", info->name, &info->user, &info->nice,
    &info->system, &info->idle, &info->iowait, &info->irq, &info->softirq);


    printf("info.name = %s, info->user = %u, info->nice = %u, info->system = %u, \
    info->idle = %u, info->iowait = %u, info->irq = %u, info->softirq = %u\n",\
    info->name, info->user, info->nice, info->system, info->idle, info->iowait, info->irq, info->softirq);
    fclose(fp);
    return true;
}

double calc_cpu_rate(cpu_info_t *old_info, cpu_info_t *new_info)
{
    double od, nd;
    double usr_dif, sys_dif, nice_dif;
    double user_cpu_rate;
    double kernel_cpu_rate;

    od = (double)(old_info->user + old_info->nice + old_info->system + old_info->idle + old_info->iowait + old_info->irq + old_info->softirq);
    nd = (double)(new_info->user + new_info->nice + new_info->system + new_info->idle + new_info->iowait + new_info->irq + new_info->softirq);

    if(nd - od)
    {
        user_cpu_rate = (new_info->user - old_info->user) / (nd - od)*100;
        kernel_cpu_rate = (new_info->system - old_info->system) / (nd - od) *100;

        return user_cpu_rate + kernel_cpu_rate;
    }

    return 0;
}

#define CPU_NUM 4
double calc_cpu_rate_app(cpu_info_t *old_info, cpu_info_t *new_info, app_info_t *old_info_app, app_info_t *new_info_app)
{
    double od, nd, od_app, nd_app;
    double usr_dif, sys_dif, nice_dif;
    double user_cpu_rate;
    double kernel_cpu_rate;
    double app_cpu_rate;

    od = (double)(old_info->user + old_info->nice + old_info->system + old_info->idle + old_info->iowait + old_info->irq + old_info->softirq);
    nd = (double)(new_info->user + new_info->nice + new_info->system + new_info->idle + new_info->iowait + new_info->irq + new_info->softirq);

    od_app = (double)(old_info_app->utime + old_info_app->stime + old_info_app->cutime + old_info_app->cstime);
    nd_app = (double)(new_info_app->utime + new_info_app->stime + new_info_app->cutime + new_info_app->cstime);

    if((nd - od) && (nd_app - od_app))
    {
        user_cpu_rate = (new_info->user - old_info->user) / (nd - od)*100;
        kernel_cpu_rate = (new_info->system - old_info->system) / (nd - od) *100;

        app_cpu_rate = (nd_app - od_app) / (nd - od)*100;
        printf("app_cpu_rate=%f, nd_app %f, od_app %f, nd %f, od %f\n", app_cpu_rate, nd_app, od_app, nd, od);

        return app_cpu_rate * CPU_NUM;
    }

    return 0;
}

char *basename(char *path)
{
    register const char *s;
    register const char *p;

    p = s = path;

    while (*s) {
        if (*s++ == '/') {
            p = s;
        }
    }

    return (char *) p;
}

 /* 根据进程名称获取PID, 比较 base name of pid_name
  * pid_list: 获取PID列表
  * list_size: 获取PID列表长度
  * RETURN值说明:
  *              < 0:
  *              >=0: 发现多少PID, pid_list 将保存已发现的PID
  */
int get_pid_by_name(char* process_name, pid_t pid_list[], int list_size)
{
    #define  MAX_BUF_SIZE       256

    DIR *dir;
    struct dirent *next;
    int count=0;
    pid_t pid;
    FILE *fp;
    char *base_pname = NULL;
    char *base_fname = NULL;
    char cmdline[MAX_BUF_SIZE];
    char path[MAX_BUF_SIZE];

    if(process_name == NULL || pid_list == NULL)
        return -EINVAL;

    base_pname = basename(process_name);
    if(strlen(base_pname) <= 0)
        return -EINVAL;

    dir = opendir("/proc");
    if (!dir)
    {
        return -EIO;
    }
    while ((next = readdir(dir)) != NULL) {
        /* skip non-number */
        if (!isdigit(*next->d_name))
            continue;

        pid = strtol(next->d_name, NULL, 0);
        sprintf(path, "/proc/%u/cmdline", pid);
        fp = fopen(path, "r");
        if(fp == NULL)
            continue;

        memset(cmdline, 0, sizeof(cmdline));
        if(fread(cmdline, MAX_BUF_SIZE - 1, 1, fp) < 0){
            fclose(fp);
            continue;
        }
        fclose(fp);
        base_fname = basename(cmdline);

        if (strcmp(base_fname, base_pname) == 0 )
        {
            if(count >= list_size){
                break;
            }else{
                pid_list[count] = pid;
                count++;
            }
        }
    }
    closedir(dir) ;
    return count;
}

/* 如果进程已经存在, return true */
int is_process_exist(char* process_name)
{
    pid_t pid;

    return (get_pid_by_name(process_name, &pid, 1) > 0);
}

#define MAX_PID_NUM     32

void get_cpu_occupy_by_pid(app_info_t *info, pid_t pid)
{
    FILE *fp = NULL;
    char buf[256] = {0};
    char app_stat[100];
    sprintf(app_stat, "/proc/%d/stat", pid);
    printf("open pid %d stat\n", pid);
    fp = fopen(app_stat,"r");
    if(!fp)
    {
        printf("open file failed\n");
        return;
    }

    fgets(buf, sizeof(buf),fp);
    //printf("buf = %s\n",buf);

    sscanf(buf, "%u %s %s %u %u %u %u %u %u %u %u %u %u %u %u %u %u  %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u",
    &info->pid, &info->comm, &info->task_state,
    &info->ppid, &info->pgid, &info->sid, &info->tty_nr, &info->tty_pgrp, &info->task_flags,
    &info->min_flt, &info->cmin_flt, &info->maj_flt, &info->cmaj_flt, &info->utime, &info->stime,
    &info->cutime, &info->cstime, &info->priority, &info->nice, &info->num_threads, &info->it_real_value,
    &info->start_time, &info->vsize, &info->rss, &info->rlim, &info->start_code, &info->end_code,
    &info->start_stack,  &info->kstkesp, &info->kstkeip, &info->pendingsig, &info->block_sig, &info->sigign, &info->sigcatch,
    &info->wchan, &info->nswap, &info->cnswap, &info->exit_signal, &info->task_cpu, &info->task_rt_priority,
    &info->task_policy);

    printf("info.pid = %u, info->utime = %u, info->stime = %u, info->cutime = %u, info->cstime = %u\n",
            info->pid, info->utime, info->stime, info->cutime, info->cstime);
    fclose(fp);
}

#define MAX_PID 8

void get_cpu_occupy_by_group_pid(app_info_t *info)
{
    char app[] = "gst-launch-1.0";
    pid_t pid[MAX_PID];
    int count;

    count=get_pid_by_name(app, pid, MAX_PID);
    if(count == 0)
    {
        printf("app %s not exist\n", app);
        return;
    }

    app_info_t info_tmp;
    printf("%s has %d pid\n", app, count);

    for(int i=0; i<count; i++)
    {
        get_cpu_occupy_by_pid(&info_tmp, pid[i]);
        info->utime += info_tmp.utime;
        info->stime += info_tmp.stime;
        info->cutime += info_tmp.cutime;
        info->cstime += info_tmp.cstime;
    }
    printf("all pid, info->utime = %u, info->stime = %u, info->cutime = %u, info->cstime = %u\n",
            info->utime, info->stime, info->cutime, info->cstime);
}

//**************************************************
//功能：获取设备内存状态
//**************************************************
typedef struct _mem_info_t
{
    char name[20];
    unsigned long total;
    char name2[20];
}mem_info_t;

void get_memory_occupy()
{
    FILE *fp = NULL;
    char buf[256] = {0};
    mem_info_t info;
    double mem_total,mem_used_rate;
    fp = fopen("/proc/meminfo","r");
    if(!fp)
        return;

    fgets(buf,sizeof(buf),fp);
    sscanf(buf,"%s%lu%s\n",info.name,&info.total,&info.name2);
    mem_total = info.total;

    printf("info.name = %s\n",info.name);
    printf("info.total = %lu\n",info.total);
    printf("info.name2 = %s\n",info.name2);

    fgets(buf,sizeof(buf),fp);
    sscanf(buf,"%s%lu%s\n",info.name,&info.total,&info.name2);


    printf("info.name = %s\n",info.name);
    printf("info.total = %lu\n",info.total);
    printf("info.name2 = %s\n",info.name2);

    mem_used_rate = (1 - info.total/mem_total)*100;
    mem_total = mem_total / (1024*1024);
    printf("内存大小为:%.01fG,占用率为:%.1f\n",mem_total, mem_used_rate);

    fclose(fp);
}


int  getDiskVolumeTotalGB(char* path)
{
    if(access(path, F_OK) < 0)
    {
        return 0;
    }

    struct statfs diskInfo;
    statfs(path, &diskInfo);

    int num = diskInfo.f_blocks * (diskInfo.f_bsize / 1024) / 1024 / 1024;
    printf("disk %s, total volume is %dGB\n", path, num);

    return num;
}

int  getDiskVolumeFreeGB(char* path)
{
    if(access(path, F_OK) < 0)
    {
        return 0;
    }

    struct statfs diskInfo;
    statfs(path, &diskInfo);

    int num = diskInfo.f_bfree * (diskInfo.f_bsize / 1024) / 1024 / 1024;
    printf("disk %s, free volume is %dGB\n", path, num);

    return num;
}

int  getDiskVolumeUsedGB(char* path)
{
    if(access(path, F_OK) < 0)
    {
        return 0;
    }

    struct statfs diskInfo;
    statfs(path, &diskInfo);

    int num = (diskInfo.f_blocks - diskInfo.f_bfree) * (diskInfo.f_bsize / 1024) / 1024 / 1024;
    printf("disk %s, used volume is %dGB\n", path, num);

    return num;
}

int  getDiskVolumeTotalMB(char* path)
{
    printf("get disk %s volume\n", path);
    if(access(path, F_OK) < 0)
    {
        return 0;
    }

    struct statfs diskInfo;
    statfs(path, &diskInfo);

    int num = diskInfo.f_blocks * (diskInfo.f_bsize / 1024) / 1024;
    printf("disk %s, total volume is %dMB\n", path, num);

    return num;
}

int  getDiskVolumeFreeMB(char* path)
{
    if(access(path, F_OK) < 0)
    {
        return 0;
    }

    struct statfs diskInfo;
    statfs(path, &diskInfo);

    int num = diskInfo.f_bfree * (diskInfo.f_bsize / 1024) / 1024;
    printf("disk %s, free volume is %dMB\n", path, num);

    return num;
}

int  getDiskVolumeUsedMB(char* path)
{
    if(access(path, F_OK) < 0)
    {
        return 0;
    }

    struct statfs diskInfo;
    statfs(path, &diskInfo);

    int num = (diskInfo.f_blocks - diskInfo.f_bfree) * (diskInfo.f_bsize / 1024) / 1024;
    printf("disk %s, used volume is %dMB\n", path, num);

    return num;
}

int isNetworkPortOpened(int port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int ret = bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret < 0) {
        perror("bind");
        return -1;
    }

    close(sockfd);
    return 0;
}
