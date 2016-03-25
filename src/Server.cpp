#include <fstream>

#include <signal.h>
#include <pwd.h>
#include <syslog.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <math.h>
#include <sys/mman.h>


#include "Server.h"
#include "Config.h"
#include "Log.h"

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

extern char *__progname;

Server::Server(const std::string &lockFileName, const std::string &pidFileName) :
    m_lockFileName(lockFileName),
    m_pidFileName(pidFileName)
{
	pid_t pid, sid, parent;
	int lfp = -1;
	struct stat sb;

	/* already a daemon */
	if(getppid() == 1) return;

	signal(SIGCHLD,child_handler);
	signal(SIGUSR1,child_handler);
	signal(SIGALRM,child_handler);

	pid = fork();

	if(pid < 0)
	{
		syslog(LOG_ERR, "unable to fork daemon, code=%d (%s)", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if(pid > 0)
	{
		alarm(2);
		pause();
		exit(EXIT_FAILURE);
	}

	parent = getppid();

	signal(SIGHUP,SIG_DFL);
	signal(SIGCHLD,SIG_DFL);
	signal(SIGTSTP,SIG_IGN);
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);
	signal(SIGTERM,SIG_DFL);
	signal(SIGPIPE,SIG_DFL);
	signal(SIGRTMIN,SIG_DFL);

    if(getuid() == 0 || geteuid() == 0)
	{
		struct passwd *pw = getpwnam(Config::Instance()->user_.c_str());

		if(pw)
		{
		    //pw->pw_dir
			syslog(LOG_NOTICE, "setting user to %s", Config::Instance()->user_.c_str());

			if(setuid(pw->pw_uid)!=0)
            {
                syslog(LOG_ERR,"error setuid to: %s",Config::Instance()->user_.c_str());
            }

			if(setgid(pw->pw_gid)!=0)
            {
                syslog(LOG_ERR,"error setgid to: %s",Config::Instance()->group_.c_str());
            }
		}
	}

	if(m_lockFileName.size())
	{
	    if (stat(m_lockFileName.c_str(), &sb) != -1)
	    {
			syslog(LOG_ERR, "another process running?\n");
			if( getProcIdByName(__progname) > 0)
            {
                syslog(LOG_ERR, "exit\n");
                exit(EXIT_FAILURE);
            }
            else
            {
                ::remove(m_lockFileName.c_str());
            }
        }

		lfp = open(m_lockFileName.c_str(),O_RDWR|O_CREAT,0640);

		if(lfp < 0)
		{
			syslog(LOG_ERR, "unable to create lock file %s, code=%d (%s)",
			       m_lockFileName.c_str(), errno, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}


	umask(007);

	sid = setsid();

	if(sid < 0)
	{
		syslog(LOG_ERR, "unable to create a new session, code %d (%s)",
		       errno, strerror(errno));
		//exit(EXIT_FAILURE);
	}

	if((chdir("/")) < 0)
	{
		syslog(LOG_ERR, "unable to change directory to %s, code %d (%s)",
		       "/", errno, strerror(errno));
		//exit(EXIT_FAILURE);
	}

	if(!freopen("/dev/null", "r", stdin))
        syslog(LOG_ERR,"freopen error: stdin");
	if(!freopen("/dev/null", "w", stdout))
        syslog(LOG_ERR,"freopen error: stdin");
	if(!freopen("/dev/null", "w", stderr))
        syslog(LOG_ERR,"freopen error: stdin");

	kill(parent, SIGUSR1);
/*
    struct sched_param param;
    param.sched_priority = 99;

    if(sched_setscheduler(0, SCHED_FIFO, &param) == -1)
    {
        syslog(LOG_ERR,"sched_setscheduler failed");
        //exit(-1);
    }

    if(mlockall(MCL_CURRENT|MCL_FUTURE) == -1)
    {
        syslog(LOG_ERR,"mlockall failed");
        //exit(-2);
    }
*/
	writePid(m_pidFileName);

	syslog(LOG_NOTICE, "staring: done");
}
//###########################################################################

Server::~Server()
{
    //dtor
}
//###########################################################################
void Server::child_handler(int signum)
{
	switch(signum)
	{
	case SIGALRM:
		exit(EXIT_FAILURE);
		break;

	case SIGUSR1:
		exit(EXIT_SUCCESS);
		break;

	case SIGCHLD:
		exit(EXIT_FAILURE);
		break;
    case SIGHUP:
        cfg->Load();
	}
}
//###########################################################################
bool Server::writePid(const std::string &pidFileName)
{
	if(pidFileName.size())
	{
        int fd;
		fd = open(pidFileName.c_str(),O_RDWR|O_CREAT,0640);

		if(fd < 0)
		{
			syslog(LOG_ERR, "unable to create file %s, code=%d (%s)",
			       m_lockFileName.c_str(), errno, strerror(errno));
			exit(EXIT_FAILURE);
		}
		pid_t p = getpid();
        int len = log10(p) + 2;
        char *buf = (char*)malloc(len);

        bzero(buf,len);
        snprintf(buf,len,"%d",p);
        if(write(fd, buf, len) != len)
        {
            syslog(LOG_ERR,"error write pid");
        }

        free(buf);

        close(fd);
	}

	return true;
}
//###########################################################################
int Server::getProcIdByName(const std::string &procName)
{
    int pid = -1;

    DIR *dp = opendir("/proc");
    if (dp != NULL)
    {
        struct dirent *dirp;
        while (pid < 0 && (dirp = readdir(dp)))
        {
            int id = atoi(dirp->d_name);
            if (id > 0)
            {
                std::string cmdPath = std::string("/proc/") + dirp->d_name + "/cmdline";
                std::ifstream cmdFile(cmdPath.c_str());
                std::string cmdLine;
                getline(cmdFile, cmdLine);
                if (!cmdLine.empty())
                {
                    size_t pos = cmdLine.find('\0');
                    if (pos != std::string::npos)
                        cmdLine = cmdLine.substr(0, pos);

                    pos = cmdLine.rfind('/');
                    if (pos != std::string::npos)
                        cmdLine = cmdLine.substr(pos + 1);

                    if (procName == cmdLine && id != getpid())
                        pid = id;
                }
            }
        }
    }

    closedir(dp);

    return pid;
}
