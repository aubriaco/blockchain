#include "CLog.h"
#include <time.h>

namespace blockchain
{
    bool CLog::sUseFiles = false;
    bool CLog::sFilesOpen = false;

    CLog::CLog(const std::string& moduleName)
    {
        mModuleName = moduleName;
    }

    CLog::~CLog() {}

    void CLog::writeLine(const std::string& str)
    {
        time_t rawtime;
        struct tm* timeinfo;
        char timestamp[32];
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(timestamp, 32, "%X", timeinfo);

        fprintf(stdout, "[%s] (%s) %s\n", timestamp, mModuleName.c_str(), str.c_str());
    }

    void CLog::errorLine(const std::string& str)
    {
        time_t rawtime;
        struct tm* timeinfo;
        char timestamp[32];
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(timestamp, 32, "%X", timeinfo);

        fprintf(stderr, "[%s] (%s) %s\n", timestamp, mModuleName.c_str(), str.c_str());
    }

    void CLog::open(bool useFiles)
    {
        if(sFilesOpen)
            return;
        sUseFiles = useFiles;
        if(sUseFiles)
        {
            freopen("output.log", "a", stdout);
            freopen("error.log", "a", stderr);
            sFilesOpen = true;
        }
    }

    void CLog::close()
    {
        if(sUseFiles)
        {
            fclose(stdout);
            fclose(stderr);
        }
        sFilesOpen = false;
    }
}