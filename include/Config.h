#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <list>

#include <tinyxml.h>

#include "DataBase.h"

extern unsigned long request_processed_;
extern unsigned long last_time_request_processed;
extern unsigned long offer_processed_;
extern unsigned long social_processed_;
extern unsigned long retargeting_processed_;

class Config
{
public:
    std::vector<std::string> mongo_main_host_;
    std::string mongo_main_db_;
    std::string mongo_main_set_;
    bool mongo_main_slave_ok_;
    std::string mongo_main_login_;
    std::string mongo_main_passwd_;
    //new params
    std::string server_ip_;
    std::string redirect_script_;
    std::string server_socket_path_;
    int server_children_;
    std::string dbpath_;
    std::string cookie_name_;
    std::string cookie_domain_;
    std::string cookie_path_;
    std::string db_dump_path_;

    int         instanceId;
    std::string lock_file_;
    std::string pid_file_;
    std::string user_;
    std::string group_;

    int time_update_;
    std::string mq_path_;
    std::string offerSqlStr;

    bool logCoreTime, logOutPutSize, logIP, logCountry, logRegion, logCookie,
        logContext, logSearch, logInformerId, logLocation,
        //output offer ids
        logOutPutOfferIds,
        logSphinx,
        logMonitor, logMQ, logRedis
        ;
    bool toLog()
    {
        return logCoreTime || logOutPutSize || logIP || logCountry || logRegion || logCookie
        || logContext || logSearch || logInformerId || logLocation
        || logOutPutOfferIds || logSphinx;
    }

    std::map<unsigned,std::string> Categories;

    DataBase *pDb;

    static Config* Instance();
    bool LoadConfig(const std::string fName);
    bool Load();
    void dbInit(){ pDb = new DataBase(true); }
    virtual ~Config();

    bool to_bool(std::string const& s)
    {
        return s != "false";
    }
    float to_float(std::string const& s)
    {
        return atof(s.c_str());
    }
    int to_int(std::string const& s)
    {
        return atoi(s.c_str());
    }
    void minifyhtml(std::string &s);

protected:
private:
    static Config* mInstance;
    Config();
    bool mIsInited;
    std::string mes;
    std::string mFileName;
    std::string cfgFilePath;

    int getTime(const char *p);
    std::string getFileContents(const std::string &fileName);
    void exit(const std::string &mes);
    bool checkPath(const std::string &path_, bool checkWrite, bool isFile, std::string &mes);
};

extern Config *cfg;

#endif // CONFIG_H
