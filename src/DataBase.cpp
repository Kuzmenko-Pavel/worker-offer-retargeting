#include <iostream>
#include <algorithm>
#include <fstream>

#include <signal.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <strings.h>
#include <stdlib.h>
#include <string.h>

#include "../config.h"

#include "Log.h"
#include "DataBase.h"
#include "KompexSQLiteException.h"
#include "Config.h"
#include "BoostHelpers.h"


#define INSERTSTATMENT "INSERT INTO Offer (id) VALUES (%lu)"

DataBase::DataBase(bool create) :
    reopen(false),
    dbFileName(cfg->dbpath_),
    dirName(cfg->db_dump_path_),
    create(create),
    pStmt(nullptr)
{
    openDb();
}

DataBase::~DataBase()
{
    pStmt->FreeQuery();

    if(pStmt)
    {
        delete pStmt;
    }
}

bool DataBase::openDb()
{
    int flags;

    try
    {
        if(pStmt)
        {
            delete pStmt;
        }

        if(!create)
        {
            #ifdef DEBUG
                printf("%s\n","Reopen");
            #endif // DEBUG
            pDatabase = new Kompex::SQLiteDatabase(dbFileName,
                                           SQLITE_OPEN_READWRITE, NULL);//SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | | SQLITE_OPEN_FULLMUTEX

            pStmt = new Kompex::SQLiteStatement(pDatabase);
            pStmt->SqlStatement("PRAGMA foreign_keys = ON;");
            pStmt->SqlStatement("PRAGMA synchronous = OFF;");
            pStmt->SqlStatement("PRAGMA count_changes = OFF;");
            pStmt->SqlStatement("PRAGMA auto_vacuum = FULL;");
            pStmt->SqlStatement("PRAGMA journal_mode = OFF;");
            pStmt->SqlStatement("PRAGMA temp_store = MEMORY;");
            pStmt->SqlStatement("PRAGMA cache_size = 20000;");
            pStmt->SqlStatement("PRAGMA read_uncommitted = 1");

            return true;
        }

        std::clog<<"open and create db: "<<dbFileName<<std::endl;
        #ifdef DEBUG
            printf("%s\n","open and create db");
        #endif // DEBUG

        flags = SQLITE_OPEN_READWRITE;

        if(dbFileName != ":memory:" && !(reopen = BoostHelpers::fileExists(dbFileName)) )
        {
                flags |= SQLITE_OPEN_CREATE;
        }

        pDatabase = new Kompex::SQLiteDatabase(dbFileName, flags, NULL);//SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | | SQLITE_OPEN_FULLMUTEX

        pStmt = new Kompex::SQLiteStatement(pDatabase);
        pStmt->SqlStatement("PRAGMA foreign_keys = ON;");
        pStmt->SqlStatement("PRAGMA synchronous = OFF;");
        pStmt->SqlStatement("PRAGMA count_changes = OFF;");
        pStmt->SqlStatement("PRAGMA auto_vacuum = FULL;");
        pStmt->SqlStatement("PRAGMA journal_mode = OFF;");
        pStmt->SqlStatement("PRAGMA temp_store = MEMORY;");
        pStmt->SqlStatement("PRAGMA cache_size = 20000;");
        pStmt->SqlStatement("PRAGMA read_uncommitted = 1");

        if(reopen)
        {
            return true;
        }

        //load db dump from directory
        readDir(dirName + "/tables");

    }
    catch(Kompex::SQLiteException &ex)
    {
        printf("cannt open sqlite database: %s error: %s\n",dbFileName.c_str(), ex.GetString().c_str());
        Log::err("DB(%s)  error: %s", dbFileName.c_str(), ex.GetString().c_str());
        exit(1);
    }

    return true;
}

long DataBase::fileSize(int fd)
{
    struct stat stat_buf;
    return fstat(fd, &stat_buf) == 0 ? stat_buf.st_size : -1;
}

void DataBase::readDir(const std::string &dname)
{
    struct dirent *sql_name;

    DIR *dir = opendir(dname.c_str());

    if (dir == NULL)
    {
        Log::err("DataBase::readDir: scandir: %s error",dname.c_str());
        return;
    }
    std::vector<std::string> files;
    while( (sql_name = readdir(dir)) != NULL )
    {
#ifdef GLIBC_2_17
        if(sql_name->d_type == DT_REG)
        {
            Log::warn("DataBase::readDir: alien file %s does not included!", sql_name->d_name);
            continue;
        }
#endif // GLIBC_2_17
        if(strstr(sql_name->d_name, ".sql") != NULL)
        {
            files.push_back(dname + "/" + sql_name->d_name);
        }
    }
    closedir(dir);
    std::sort (files.begin(), files.end());
    runSqlFiles(files);
}

bool DataBase::runSqlFiles(const std::vector<std::string> &files)
{
    for (auto it = files.begin(); it != files.end(); it++)
    {
        runSqlFile(*it);
    }

    return true;
}


//The REINDEX command is used to delete and recreate indices from scratch.
void DataBase::indexRebuild()
{
    try
    {
        pStmt->BeginTransaction();
        pStmt->SqlStatement("REINDEX;");
        pStmt->CommitTransaction();
        pStmt->BeginTransaction();
        pStmt->SqlStatement("ANALYZE;");
        pStmt->CommitTransaction();
    }
    catch(Kompex::SQLiteException &ex)
    {
        Log::err("DataBase DB error: indexRebuild: %s", ex.GetString().c_str());
    }
}

bool DataBase::runSqlFile(const std::string &file)
{
    int fd;

    if( (fd = open(file.c_str(), O_RDONLY))<2 )
    {
        Log::err("DataBase::runSqlFile: error open %s", file.c_str());
        return false;
    }

    ssize_t sz = fileSize(fd);
    char *buf = (char*)malloc(sz);

    bzero(buf,sz);

    int ret = read(fd, buf, sz);

    if( ret != sz )
    {
        printf("Error read file: %s",file.c_str());
        ::exit(1);
    }
    close(fd);

    try
    {
        pStmt->SqlStatement(buf);
    }
    catch(Kompex::SQLiteException &ex)
    {
        printf("error in file: %s: %s\n", file.c_str(), ex.GetString().c_str());
        ::exit(1);
    }
    char *p = buf;
    while(p < buf+sz)
    {
        if(*p==';' && p < buf+sz-30)
        {
            try
            {
                pStmt->SqlStatement(++p);
            }
            catch(Kompex::SQLiteException &ex)
            {
                printf("error in file: %s: %s\n", file.c_str(), ex.GetString().c_str());
                ::exit(1);
            }
        }
        else
        {
            p++;
        }
    }

    free(buf);
    return true;
}

std::string DataBase::getSqlFile(const std::string &file)
{
    int fd;
    std::string retString;

    if( (fd = open((dirName + "/" + file).c_str(), O_RDONLY))<2 )
    {
        Log::err("error open file: %s/%s",dirName.c_str(),file.c_str());
        ::exit(1);
    }
    ssize_t sz = fileSize(fd);
    char *buf = (char*)malloc(sz);

    bzero(buf,sz);

    int ret = read(fd, buf, sz);

    if( ret != sz )
    {
        printf("Error read file: %s",(dirName + "/" + file).c_str());
        ::exit(1);
    }
    close(fd);

    retString = std::string(buf,ret);

    free(buf);

    return retString;
}

void DataBase::exec(const std::string &sql)
{
   try
    {
        pStmt->SqlStatement(sql);
    }
    catch(Kompex::SQLiteException &ex)
    {
        Log::err("DB error: cmd: %s msg: %s", sql.c_str(), ex.GetString().c_str());
    }
}
