#ifndef CPUSTAT_H
#define CPUSTAT_H

#include <sys/types.h>

struct meterage
{
    long unsigned int utime_ticks;
    long int cutime_ticks;
    long unsigned int stime_ticks;
    long int cstime_ticks;
    long unsigned int vsize; // virtual memory size in bytes
    long unsigned int rss; //Resident  Set  Size in bytes
    long unsigned int cpu_total_time;
};

class CpuStat
{
public:
    static double cpu_user;
    static double cpu_sys;
    static long unsigned int rss;

    CpuStat();
    virtual ~CpuStat();
    void cpuUsage();
    int freeMem();
//    void _cpuUsage();

protected:
private:
    pid_t pid;
    struct meterage last_usage;
    bool getUsage(struct meterage &m);
};

#endif // CPUSTAT_H
