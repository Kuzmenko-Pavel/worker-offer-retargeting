#ifndef FILE_H
#define FILE_H


class File
{
    public:
        File();
        virtual ~File();
        bool read(std::string &cont);
    protected:
    private:
        long fileSize(int fd);
};

#endif // FILE_H
