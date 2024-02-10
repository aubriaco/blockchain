/*
 * Copyright 2023-2024 Alessandro Ubriaco. All Rights Reserved.
 * 
 * Licensed under the Apache License 2.0 (the "License").
 * You may not use this file except in the compliance with the License.
 * You may obtain a copy of the license in the file LICENSE.txt
 * in the source distribution.
*/
#ifndef __C_NODE_INFO_INCLUDED__
#define __C_NODE_INFO_INCLUDED__
#include <time.h>
#include <string>

namespace blockchain
{
    namespace net
    {
        class CNodeInfo
        {
        public:
            std::string mHostName;
            uint32_t mPort;
            time_t mLastSeen;

            CNodeInfo(const std::string& hostname, uint32_t port)
            {
                mHostName = hostname;
                mPort = port;
                mLastSeen = time(0);
            }

            void seen()
            {
                mLastSeen = time(0);
            }
        };
    }
}
#endif