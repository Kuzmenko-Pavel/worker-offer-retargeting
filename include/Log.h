#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <syslog.h>
#include <stdarg.h>
#include <pthread.h>

#include <string>
#include <streambuf>

#define BUFLEN 1024*1024
#define FMTPARCE  char buffer[BUFLEN];\
  va_list args;\
  va_start (args, fmt);\
  vsprintf (buffer,fmt, args);\
  va_end (args);
//  perror (buffer);

enum LogPriority
{
    Emerg   = LOG_EMERG,   // system is unusable
    Alert   = LOG_ALERT,   // action must be taken immediately
    Crit    = LOG_CRIT,    // critical conditions
    Err     = LOG_ERR,     // error conditions
    Warning = LOG_WARNING, // warning conditions
    Notice  = LOG_NOTICE,  // normal, but significant, condition
    Info    = LOG_INFO,    // informational message
    Debug   = LOG_DEBUG    // debug-level message
};

std::ostream& operator<< (std::ostream& os, const LogPriority& log_priority);

class Log : public std::basic_streambuf<char, std::char_traits<char> >
{
public:

    explicit Log(int facility);
    virtual ~Log();
    static void err(const char *fmt, ... );
    static void err(const std::string &mes);
    static void warn(const char* fmt, ... );
    static void warn(const std::string &mes);
    static void info(const char* fmt, ... );
    static void info(const std::string &mes);
    static void gdb(const char* fmt, ... );
    static void gdb(const std::string &mes);

    static int memUsage();
    static float cpuUsage();

protected:
    int sync();
    int overflow(int c);
private:
    friend std::ostream& operator<< (std::ostream& os, const LogPriority& log_priority);
    std::string buffer_;
    int facility_;
    int priority_;

    static int parseLine(char* line);
};

#endif // LOG_H
