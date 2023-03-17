#ifndef __C_LOG_INCLUDED__
#define __C_LOG_INCLUDED__
#include <stdio.h>
#include <string>

namespace blockchain
{
    class CLog
    {
    private:
        std::string mModuleName;
        static bool sUseFiles;
        static bool sFilesOpen;
    public:
        CLog(const std::string& moduleName);
        ~CLog();

        void writeLine(const std::string& str);
        void errorLine(const std::string& str);

        static void open(bool useFiles = false);
        static void close();

    };
}

#endif