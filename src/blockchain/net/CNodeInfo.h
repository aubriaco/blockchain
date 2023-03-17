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