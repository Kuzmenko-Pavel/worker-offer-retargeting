#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <ostream>

#include "Log.h"

extern char *__progname;
static char *buffer;

Log::Log(int facility)
{
    facility_ = facility;
    priority_ = LOG_DEBUG;
    openlog(__progname, LOG_PID, facility_);
    buffer = new char[BUFLEN];
}

Log::~Log()
{
    closelog();
    delete []buffer;
}

void Log::err(const char* fmt, ... )
{
    va_list args;

    va_start (args, fmt);
    vsprintf(buffer, fmt, args);
    va_end (args);

    syslog(LOG_ERR, "%s", buffer);
};

void Log::err(const std::string &mes)
{
    syslog(LOG_ERR, "%s", mes.c_str());
};

void Log::warn(const char* fmt, ... )
{
    va_list args;

    va_start (args, fmt);
    vsprintf(buffer, fmt, args);
    va_end (args);

    syslog(LOG_WARNING, "%s", buffer);
};

void Log::warn(const std::string &mes)
{
    syslog(LOG_WARNING, "%s", mes.c_str());
};


void Log::info(const char* fmt, ... )
{
    va_list args;

    va_start (args, fmt);
    vsprintf(buffer, fmt, args);
    va_end (args);

    syslog(LOG_INFO, "%s", buffer);
};

void Log::info(const std::string &mes)
{
    syslog(LOG_INFO, "%s", mes.c_str());
};


void Log::gdb(const char* fmt, ... )
{
#ifdef DEBUG
    va_list args;

    va_start (args, fmt);
    vsprintf(buffer, fmt, args);
    va_end (args);

    syslog(LOG_DEBUG, "%s", buffer);
#endif // DEBUG
};

void Log::gdb(const std::string &mes)
{
#ifdef DEBUG
    syslog(LOG_DEBUG, "%s", mes.c_str());
#endif // DEBUG
};

int Log::sync()
{
    if (buffer_.length())
    {
        syslog(priority_, "%s",buffer_.c_str());
        buffer_.erase();
        //priority_ = LOG_DEBUG; // default to debug for each message
    }
    return 0;
}

int Log::overflow(int c)
{
    if (c != EOF)
    {
        buffer_ += static_cast<char>(c);
    }
    else
    {
        sync();
    }
    return c;
}

std::ostream& operator<< (std::ostream& os, const LogPriority& log_priority)
{
    static_cast<Log *>(os.rdbuf())->priority_ = (int)log_priority;
    return os;
}

int Log::parseLine(char* line)
{
    int i = strlen(line);
    while (*line < '0' || *line > '9') line++;
    line[i-3] = '\0';
    i = atoi(line);
    return i;
}


int Log::memUsage()
{
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];


    while (fgets(line, 128, file) != NULL)
    {
        if (strncmp(line, "VmSize:", 7) == 0)
        {
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return result;
}
