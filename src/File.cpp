#include "File.h"

File::File(const std::string &file):
    name(file)
{
    //ctor
}

File::~File()
{
    //dtor
}

bool File::read(std::string &cont)
{
    int fd;

    if( (fd = open(file.c_str(), O_RDONLY))<2 )
    {
        return false;
    }

    ssize_t sz = fileSize(fd);
    char *buf = (char*)malloc(sz);

    bzero(buf,sz);

    int ret = read(fd, buf, sz);

    if( ret != sz )
    {
        Log::err("Error read file: %s",file.c_str());
        free(buf);
        close(fd);
        return false;
    }

    cont = std::string(buf);

    free(buf);
    close(fd);
    return true;
}
