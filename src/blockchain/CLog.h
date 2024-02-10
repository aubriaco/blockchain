/*
 * Copyright 2023-2024 Alessandro Ubriaco. All Rights Reserved.
 * 
 * Licensed under the Apache License 2.0 (the "License").
 * You may not use this file except in the compliance with the License.
 * You may obtain a copy of the license in the file LICENSE.txt
 * in the source distribution.
*/
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