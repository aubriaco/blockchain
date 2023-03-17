#ifndef __D_PACKET_INCLUDED__
#define __D_PACKET_INCLUDED__
#include "EMessageType.h"
#include <stdint.h>

namespace blockchain
{
    namespace net
    {
        class DPacket
        {
        public:
            uint32_t mVersion;
            EMessageType mMessageType;
            uint64_t mDataSize;
            uint8_t* mData;

            DPacket()
            {
                mVersion = 1;
                mMessageType = EMT_NULL;
                mDataSize = 0;
                mData = 0;
            }
        };
    }
}

#endif