#ifndef BOOSTHELPERS_H
#define BOOSTHELPERS_H

#include <string>

class BoostHelpers
{
    public:
        BoostHelpers();
        virtual ~BoostHelpers();

        static std::string getConfigDir(const std::string &filePath);
        static bool checkPath(const std::string &path_, bool checkWrite, bool isFile);
        static int getSeconds(const std::string &s);
        static std::string float2string(const float);
        static bool fileExists(const std::string &path_);
    protected:
    private:
};

#endif // BOOSTHELPERS_H
