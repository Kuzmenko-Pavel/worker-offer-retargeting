#ifndef SERVER_H
#define SERVER_H

#include <string>

class Server
{
    public:
        Server(const std::string&, const std::string&);
        virtual ~Server();
        static int getProcIdByName(const std::string &procName);
    protected:
    private:
        std::string m_lockFileName;
        std::string m_pidFileName;
        std::string m_logFileName;

        static void child_handler(int signum);
        bool writePid(const std::string &pidFileName);
        static void stack_prefault(void);
};

#endif // SERVER_H
