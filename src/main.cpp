#include <sys/types.h>
#include <signal.h>
#include "CgiService.h"
#include "Log.h"
#include "Server.h"
/** \mainpage workerd

    \section usage_section Использование

    \b workerd --- it's deamon, to return html(inframe) with offers
    install:
    #aclocal -I m4 && autoconf && automake
    #make
    run:
    ./worker
*/

//static objects
Config *cfg;

int main(int argc, char *argv[])
{
    struct sigaction sa_ign;
    sa_ign.sa_handler = SIG_IGN;
    sa_ign.sa_flags = 0;
    sigemptyset(&sa_ign.sa_mask);
    sigaction(SIGPIPE, &sa_ign, NULL);
    //Log(LOG_LOCAL0);
    std::clog.rdbuf(new Log(LOG_LOCAL0));

    std::string config = "config.xml";
    std::string sock_path;
    int ret;

    bool fPrintPidFile = false;
    bool fTestConfig = false;

    while ( (ret = getopt(argc,argv,"tpc:s:")) != -1)
    {
        switch (ret)
        {
        case 'c':
            config = optarg;
            break;
        case 's':
            sock_path = optarg;
            break;
        case 'p':
            fPrintPidFile = true;
            break;
        case 't':
            fTestConfig = true;
            break;
        default:
            printf("Error found! %s -c config_file -s socket_path\n",argv[0]);
            ::exit(1);
        };
    };


    cfg = Config::Instance();
    cfg->LoadConfig(config);

    if(fTestConfig)
    {
        std::cout<<"Config: Ok"<<std::endl;
        ::exit(0);
    }

    if(fPrintPidFile)
    {
        std::cout<<cfg->pid_file_<<std::endl;
        ::exit(0);
    }

#ifndef DEBUG
    new Server(cfg->lock_file_, cfg->pid_file_);
#endif

    if( sock_path.size() > 8 )
    {
        cfg->server_socket_path_ = sock_path;
    }

    cfg->dbInit();

    CgiService().run();
    return 0;
}
