#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>


#include "BoostHelpers.h"
#include "Log.h"

boost::regex timeRegex("(\\d+):(\\d+):(\\d+)");

BoostHelpers::BoostHelpers()
{
    //ctor
}

BoostHelpers::~BoostHelpers()
{
    //dtor
}

std::string BoostHelpers::getConfigDir(const std::string &filePath)
{
    try
    {
        boost::filesystem::path p;

        if(!boost::filesystem::is_regular_file(filePath))
        {
            std::clog<<"does not regular file: "<<filePath<<std::endl;
            return "";
        }

        p = boost::filesystem::path(filePath);

        if(p.has_parent_path())
        {
            return p.parent_path().string() + "/";
        }
        else
        {
            return "./";
        }
    }
    catch (const boost::filesystem::filesystem_error& ex)
    {
        std::clog<<__func__<<" error: "<<ex.what()<< std::endl;
    }

    return "";
}

bool BoostHelpers::fileExists(const std::string &path_)
{
    return boost::filesystem::exists(path_);
}

bool BoostHelpers::checkPath(const std::string &path_, bool checkWrite, bool isFile)
{
    boost::filesystem::path path, test;
    boost::system::error_code errcode;
    boost::filesystem::path::iterator toEnd;
    struct stat info;
    uid_t uid;
    gid_t gid;

    uid = getuid();
    gid = getgid();


    path = boost::filesystem::path(path_);

    toEnd = path.end();
    if(isFile)
    {
        toEnd--;
    }

    for (boost::filesystem::path::iterator it = path.begin(); it != toEnd; ++it)
    {
        test /= *it;

        if(test.string()=="" || test.string()=="/")
            continue;

        if(boost::filesystem::exists(test))
        {
            ::stat(test.string().c_str(), &info);

            if (boost::filesystem::is_regular_file(test))
            {
                if(info.st_uid != uid && info.st_gid != gid)
                {
                    if(info.st_mode &(S_IROTH | S_IWOTH))
                    {
                        continue;
                    }
                    else
                    {
                        std::clog<<__func__<<" error: cann't write path: "<<test<<std::endl;
                        return false;
                    }
                }
                else if(info.st_uid == uid && info.st_gid != gid)
                {
                    if(info.st_mode & (S_IRUSR | S_IWUSR))
                    {
                        continue;
                    }
                    else
                    {
                        std::clog<<__func__<<" error: cann't write path: "<<test<<std::endl;
                        return false;
                    }
                }
                else if(info.st_uid != uid && info.st_gid == gid)
                {
                    if(info.st_mode & (S_IRGRP | S_IWGRP))
                    {
                        continue;
                    }
                    else
                    {
                        std::clog<<__func__<<" error: cann't write path: "<<test<<std::endl;
                        return false;
                    }
                }
                else
                {
                    if(info.st_mode & (S_IRUSR | S_IWUSR))
                    {
                        continue;
                    }
                    else
                    {
                        std::clog<<__func__<<" error: cann't write path: "<<test<<std::endl;
                        return false;
                    }
                }
            }
            else if (boost::filesystem::is_directory(test))
            {
                if(info.st_uid != uid && info.st_gid != gid)
                {
                    if(info.st_mode & (S_IROTH | S_IXOTH))
                    {
                        continue;
                    }
                    else
                    {
                        std::clog<<__func__<<" error: cann't write path: "<<test<<std::endl;
                        return false;
                    }
                }
                else if(info.st_uid == uid && info.st_gid != gid)
                {
                    if(info.st_mode & (S_IRUSR | S_IXUSR))
                    {
                        continue;
                    }
                    else
                    {
                        std::clog<<__func__<<" error: cann't write path: "<<test<<std::endl;
                        return false;
                    }
                }
                else if(info.st_uid != uid && info.st_gid == gid)
                {
                    if(info.st_mode & (S_IRGRP | S_IXGRP))
                    {
                        continue;
                    }
                    else
                    {
                        std::clog<<__func__<<" error: cann't write path: "<<test<<std::endl;
                        return false;
                    }
                }
                else
                {
                    if(info.st_mode & (S_IRUSR | S_IXUSR))
                    {
                        continue;
                    }
                    else
                    {
                        std::clog<<__func__<<" error: cann't write path: "<<test<<std::endl;
                        return false;
                    }
                }
                continue;
            }
            else
            {
                if(checkWrite)
                {
                    try
                    {
                        if(!boost::filesystem::create_directories(test))
                        {
                            return false;
                        }
                    }
                    catch(const boost::filesystem::filesystem_error &ex)
                    {
                        std::clog<<__func__<<" path: "<<test<<" error: "<<ex.what()<<std::endl;
                        return false;
                    }
                }
                else
                {
                    return false;
                }
            }
        }
        else//does not exists
        {
            if(checkWrite)
            {
                try
                {
                    if(!boost::filesystem::create_directories(test))
                    {
                        return false;
                    }
                }
                catch(const boost::filesystem::filesystem_error &ex)
                {
                    std::clog<<__func__<<" path: "<<test<<" error: "<<ex.what()<<std::endl;
                    return false;
                }
            }
            else
            {
                std::clog<<__func__<<" path: "<<test<<" error: does not exists"<<std::endl;
                return false;
            }
        }
    }

    if(isFile && checkWrite)
    {
        int lfp = open((path_+"test").c_str(),O_RDWR|O_CREAT,0640);

		if(lfp < 0)
		{
			std::clog<<__func__<<" unable to create file: "<<path_<<" error: "<<strerror(errno)<<std::endl;
		}
		else
        {
            close(lfp);
            unlink((path_+"test").c_str());
        }
    }

    return true;
}
//---------------------------------------------------------------------------------------------------------------
/*
                stat = boost::filesystem::status(test, errcode);
                if(errcode)
                {
                  Log::err("file system error: object: %s value: %d message: %s",
                           test.string().c_str(),
                           errcode.value(),
                           errcode.message().c_str());
                    return false;
                }

*/

int BoostHelpers::getSeconds(const std::string &s)
{
    boost::cmatch tres;

    if(boost::regex_match(s.c_str(), tres, timeRegex))
    {
        return std::atoi(tres[2].first);//only seconds
    }
    else
    {
        return -1;
    }
}

std::string BoostHelpers::float2string(const float p)
{
    return boost::lexical_cast<std::string>(p);
}
