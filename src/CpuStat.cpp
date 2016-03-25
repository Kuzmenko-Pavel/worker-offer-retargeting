#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <sys/sysinfo.h>

#include "CpuStat.h"

double CpuStat::cpu_user = 0;
double CpuStat::cpu_sys = 0;
long unsigned int CpuStat::rss = 0;

CpuStat::CpuStat()
{
    pid = getpid();
    getUsage(last_usage);
}

CpuStat::~CpuStat()
{
    //dtor
}

bool CpuStat::getUsage(struct meterage &m)
{
    //convert  pid to string
    char pid_s[20];
    snprintf(pid_s, sizeof(pid_s), "%d", pid);
    char stat_filepath[30] = "/proc/"; strncat(stat_filepath, pid_s,
            sizeof(stat_filepath) - strlen(stat_filepath) -1);
    strncat(stat_filepath, "/stat", sizeof(stat_filepath) -
            strlen(stat_filepath) -1);

    FILE *fpstat = fopen(stat_filepath, "r");
    if (fpstat == NULL) {
        return false;
    }

    FILE *fstat = fopen("/proc/stat", "r");
    if (fstat == NULL) {
        fclose(fstat);
        return false;
    }

    bzero(&m,sizeof(m));

    //read values from /proc/pid/stat
    long int rss;
    if (fscanf(fpstat, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu"
                "%lu %ld %ld %*d %*d %*d %*d %*u %lu %ld",
                &m.utime_ticks, &m.stime_ticks,
                &m.cutime_ticks, &m.cstime_ticks, &m.vsize,
                &rss) == EOF) {
        fclose(fpstat);
        return false;
    }
    fclose(fpstat);
    m.rss = rss * getpagesize();

    //read+calc cpu total time from /proc/stat
    long unsigned int cpu_time[10];
    bzero(cpu_time, sizeof(cpu_time));
    if (fscanf(fstat, "%*s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
                &cpu_time[0], &cpu_time[1], &cpu_time[2], &cpu_time[3],
                &cpu_time[4], &cpu_time[5], &cpu_time[6], &cpu_time[7],
                &cpu_time[8], &cpu_time[9]) == EOF) {
        fclose(fstat);
        return false;
    }

    fclose(fstat);

    int i;
    for(i=0; i < 10;i++)
        m.cpu_total_time += cpu_time[i];

    return true;
}


//calculates the elapsed CPU usage between 2 measuring points. in percent
void CpuStat::cpuUsage()
{
    struct meterage cur_usage;

    getUsage(cur_usage);

    const long unsigned int total_time_diff = cur_usage.cpu_total_time -
                                              last_usage.cpu_total_time;

    cpu_user = 1000 * (((cur_usage.utime_ticks + cur_usage.cutime_ticks)
                    - (last_usage.utime_ticks + last_usage.cutime_ticks))
                    / (double) total_time_diff);

    cpu_sys = 1000 * ((((cur_usage.stime_ticks + cur_usage.cstime_ticks)
                    - (last_usage.stime_ticks + last_usage.cstime_ticks))) /
                    (double) total_time_diff);
    last_usage = cur_usage;
    rss = last_usage.rss;

    syslog(LOG_INFO, "cpu: sys:%02.2f user:%02.2f vs:%lu", cpu_sys, cpu_user,last_usage.rss/1024);
}

int CpuStat::freeMem()
{
    struct sysinfo info;

    if( sysinfo(&info) == 0 )
    {
        return info.freeram * 100 /info.totalram;
    }

    return -1;
}

/*
//calculates the elapsed CPU usage between 2 measuring points in ticks
void CpuStat::_cpuUsage()
{
    struct meterage cur_usage;

    getUsage(cur_usage);

    cpu_user = (cur_usage.utime_ticks + cur_usage.cutime_ticks) -
                  (last_usage.utime_ticks + last_usage.cutime_ticks);

    cpu_sys = (cur_usage.stime_ticks + cur_usage.cstime_ticks) -
                  (last_usage.stime_ticks + last_usage.cstime_ticks);

    last_usage = cur_usage;
}
*/
