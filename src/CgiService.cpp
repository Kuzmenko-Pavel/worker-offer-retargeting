#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/regex/icu.hpp>
#include <map>
#include <chrono>

#include "DB.h"
#include "Log.h"
#include "CgiService.h"
#include "UrlParser.h"
#include "BaseCore.h"
#include "Core.h"
#include "Cookie.h"
#include "DataBase.h"
#include "Server.h"
#include <signal.h>

#include <fcgi_stdio.h>
#include "../config.h"

#define THREAD_STACK_SIZE PTHREAD_STACK_MIN + 10 * 1024
const unsigned long STDIN_MAX = 6000000;

CgiService::CgiService()
{
    signal(SIGPIPE, SIG_IGN);
    struct sigaction actions;

    memset(&actions, 0, sizeof(actions));
    actions.sa_flags = 0;
    actions.sa_handler = SignalHandler;

    sigaction(SIGHUP,&actions,NULL);
    sigaction(SIGPIPE,&actions,NULL);

    bcore = new BaseCore();

    stat = new CpuStat();

    FCGX_Init();
    mode_t old_mode = umask(0);
    socketId = FCGX_OpenSocket(cfg->server_socket_path_.c_str(), cfg->server_children_ * 4);
    if(socketId < 0)
    {
        std::clog<<"Error open socket. exit"<<std::endl;
        ::exit(1);
    }
    umask(old_mode);

    pthread_attr_t* attributes = (pthread_attr_t*) malloc(sizeof(pthread_attr_t));
    pthread_attr_init(attributes);
    pthread_attr_setstacksize(attributes, THREAD_STACK_SIZE);

    threads = new pthread_t[cfg->server_children_ + 1];

    for(int i = 0; i < cfg->server_children_; i++)
    {
        if(pthread_create(&threads[i], attributes, &this->Serve, this))
        {
            std::clog<<"creating thread failed"<<std::endl;
            ::exit(1);
        }
    }

    pthread_attr_destroy(attributes);
    free(attributes);
}

void CgiService::run()
{
    for(;;)
    {
        //read mq and process
        bcore->ProcessMQ();
    
        #ifdef DEBUG
        if(cfg->logMonitor)
        {
            stat->cpuUsage();
        }
        #endif // DEBUG
        sleep(1);
    }
}

CgiService::~CgiService()
{
   for(int i = 0; i < cfg->server_children_; i++)
    {
        pthread_join(threads[i], 0);
    }

   delete []threads;
   delete bcore;
}

void CgiService::Response(FCGX_Request *req,
                          const std::string &content,
                          const std::string &contentType,
                          const std::string &cookie)
{
    try
    {
        if(content.empty())
        {
            return;
        }
        if(FCGX_GetError(req->err) != 0)
        {
            Log::warn("------------ ERROR ----------");
            return;
        }

        FCGX_FPrintF(req->out,"Status: 200 OK\r\n");
        FCGX_FPrintF(req->out,"Content-type: %s\r\n", contentType.c_str());
        FCGX_FPrintF(req->out,"Set-Cookie: %s\r\n", cookie.c_str());
        FCGX_FPrintF(req->out,"Pragma: no-cache\r\n");
        FCGX_FPrintF(req->out,"Expires: Fri, 01 Jan 1990 00:00:00 GMT\r\n");
        FCGX_FPrintF(req->out,"Cache-Control: no-cache, must-revalidate, no-cache=\"Set-Cookie\", private\r\n");
        FCGX_FPrintF(req->out,"\r\n");
        FCGX_FPrintF(req->out,"%s\r\n", content.c_str());
        FCGX_FFlush(req->out);
        FCGX_Finish_r(req);
    }
    catch (std::exception const &ex)
    {
        Log::err("exception %s: name: %s while processing", typeid(ex).name(), ex.what());
    }
}


void CgiService::Response(FCGX_Request *req, unsigned status)
{
    try
    {
        if(FCGX_GetError(req->err) != 0)
        {
            Log::warn("------------ ERROR ----------");
            return;
        }
        if(req && req->out)
        {
            FCGX_FPrintF(req->out,"Status: %d OK\r\n",status);
            FCGX_FPrintF(req->out,"Content-type: text/html\r\n");
            FCGX_FPrintF(req->out,"\r\n\r\n");
            FCGX_FFlush(req->out);
            FCGX_Finish_r(req);
        }
    }
    catch (std::exception const &ex)
    {
        Log::err("exception %s: name: %s while processing", typeid(ex).name(), ex.what());
    }
}


void *CgiService::Serve(void *data)
{
    signal(SIGPIPE, SIG_IGN);
    CgiService *csrv = (CgiService*)data;

    Core *core = new Core();
    int count_req = 0;

    FCGX_Request request;

    if(FCGX_InitRequest(&request, csrv->socketId, 0) != 0)
    {
        std::clog<<"Can not init request"<<std::endl;
        return nullptr;
    }

    static pthread_mutex_t accept_mutex = PTHREAD_MUTEX_INITIALIZER;

    for(;;)
    {
        pthread_mutex_lock(&accept_mutex);
        int rc = FCGX_Accept_r(&request);
        pthread_mutex_unlock(&accept_mutex);

        if(rc < 0)
        {
            std::clog<<"Can not accept new request"<<std::endl;
            break;
        }
        csrv->ProcessRequest(&request, core);
        if (count_req == 10000000)
        {
            delete core;
            core = new Core();
        }
        count_req++;
    }

    std::clog<<"thread: "<<pthread_self()<<" exit.";

    delete core;

    return nullptr;
}


void CgiService::ProcessRequest(FCGX_Request *req, Core *core)
{
    #ifdef DEBUG
        auto start = std::chrono::high_resolution_clock::now();
        printf("%s\n","------------------------------------------------------------------");
    #endif // DEBUG
    if(FCGX_GetError(req->err) != 0)
    {
        Log::warn("------------ ERROR ----------");
        return;
    }
    char *tmp_str = nullptr;
    std::string query, ip, script_name, cookie_value, postq, xhr;
    bool ajax = false;
    boost::u32regex replaceSymbol;


    if (!(tmp_str = FCGX_GetParam("QUERY_STRING", req->envp)))
    {
        Log::warn("query string is not set");
    }
    else
    {
        query = std::string(tmp_str);
    }
    
    tmp_str = nullptr;
    unsigned long content_length = STDIN_MAX;
    if ((tmp_str = FCGX_GetParam("CONTENT_LENGTH", req->envp))) {
        content_length = strtol(tmp_str, &tmp_str, 10);
        if (*tmp_str) {
            Log::warn("Can't Parse 'CONTENT_LENGTH'");
        }
        if (content_length > STDIN_MAX) {
            content_length = STDIN_MAX;
        }
        if (content_length > 0)
        {
            char * content = new char[content_length];
            memset(content, ' ', content_length);
            FCGX_GetStr(content, content_length, req->in);
            postq.clear();
            postq.resize (content_length,' ');
            postq = std::string(content);
            postq.resize (content_length);
            delete [] content;
            content_length = 0;
        }
    }

    tmp_str = nullptr;
    if( !(tmp_str = FCGX_GetParam("REMOTE_ADDR", req->envp)) )
    {
        Log::warn("remote address is not set");
        Response(req, 200);
        query.clear();
        ip.clear();
        script_name.clear();
        cookie_value.clear();
        postq.clear();
        return;
    }
    else
    {
        ip = std::string(tmp_str);
    }

    tmp_str = nullptr;
    if (!(tmp_str = FCGX_GetParam("SCRIPT_NAME", req->envp)))
    {
        Log::warn("script name is not set");
        Response(req, 200);
        query.clear();
        ip.clear();
        script_name.clear();
        cookie_value.clear();
        postq.clear();
        return;
    }
    else
    {
        script_name = std::string(tmp_str);
    }
    tmp_str = nullptr;
    if ((tmp_str = FCGX_GetParam("HTTP_X_REQUESTED_WITH", req->envp)))
    {
        xhr = std::string(tmp_str);
        std::transform(xhr.begin(), xhr.end(), xhr.begin(), ::tolower);
        if(xhr == "xmlhttprequest")
        {
            ajax = true;
        }
    }
    else
    {
        Log::err(tmp_str);
    }

    tmp_str = nullptr;
    if (!(tmp_str = FCGX_GetParam("HTTP_COOKIE", req->envp)))
    {
        //Log::warn("HTTP_COOKIE is not set");
    }
    else
    {
        std::string resp = std::string(tmp_str);
        std::vector<std::string> strs;
        boost::split(strs, resp, boost::is_any_of(";"));

        for (unsigned int i=0; i<strs.size(); i++)
        {
            if(strs[i].find(cfg->cookie_name_) != string::npos)
            {
                std::vector<std::string> name_value;
                boost::split(name_value, strs[i], boost::is_any_of("="));
                if (name_value.size() == 2)
                    cookie_value = name_value[1];
            }
        }
    }

    UrlParser *url;
    UrlParser *post;
    url = new UrlParser(query);
    post = new UrlParser(postq);

    if (url->param("show") == "status")
    {
        if( server_name.empty() && (tmp_str = FCGX_GetParam("SERVER_NAME", req->envp)) )
        {
            server_name = std::string(tmp_str);
        }

        Response(req, bcore->Status(server_name), "text/html", "");
        delete url;
        delete post;
        query.clear();
        ip.clear();
        script_name.clear();
        cookie_value.clear();
        postq.clear();
        return;
    }
    if (ajax)
    {
        try
        {
            Params prm = Params()
                         .get(query)
                         .post(postq)
                         .cookie_id(cookie_value)
                         .json(postq)
                         .parse();


            std::string result;
            result = core->Process(&prm);

            ClearSilver::Cookie c = ClearSilver::Cookie(cfg->cookie_name_,
                          prm.getCookieId(),
                          ClearSilver::Cookie::Credentials(
                                    ClearSilver::Cookie::Authority(cfg->cookie_domain_),
                                    ClearSilver::Cookie::Path(cfg->cookie_path_),
                                ClearSilver::Cookie::Expires(boost::posix_time::second_clock::local_time() + boost::gregorian::years(1))));
            if (result.empty())
            {
                Log::err("empty request %s: ", query.c_str());
                Response(req, 500);
            }
            else
            {
                try
                {
                    Response(req, result, "application/json", c.to_string());
                    #ifdef DEBUG
                        auto elapsed = std::chrono::high_resolution_clock::now() - start;
                        long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
                        printf("Time %s taken: %lld \n", __func__,  microseconds);
                        printf("%s\n","------------------------------------------------------------------");
                    #endif // DEBUG
                    core->ProcessSaveResults();
                }
                catch (std::exception const &ex)
                {
                    Log::err(c.to_string().c_str());
                    Log::err(result.c_str());
                    Log::err("exception %s: name: %s while processing send response: %s", typeid(ex).name(), ex.what(), query.c_str());
                    Response(req, 503);
                }
            }
        }
        catch (std::exception const &ex)
        {
            Log::err("exception %s: name: %s while processing: %s", typeid(ex).name(), ex.what(), query.c_str());
            Response(req, 503);
        }
    }
    else
    {
        Response(req, 200);
    }
    delete url;
    delete post;
    query.clear();
    ip.clear();
    script_name.clear();
    cookie_value.clear();
    postq.clear();
}

void CgiService::SignalHandler(int signum)
{
    switch(signum)
    {
    case SIGHUP:
        std::clog<<"CgiService: sig hup"<<std::endl;
        break;
    case SIGPIPE:
        std::clog<<"CgiService: sig pipe "<< "sig=" << signum << " tid=" << pthread_self() <<std::endl;
        break;
    }
}
