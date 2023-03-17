#ifndef __C_PACKET_INCLUDED__
#define __C_PACKET_INCLUDED__
#include "EMessageType.h"
#include <stdint.h>

namespace blockchain
{
    namespace net
    {
        class CPacket
        {
        public:
            uint32_t mVersion;
            EMessageType mMessageType;
            uint32_t mDataSize;
            uint8_t* mData;
            bool mTrackDataAlloc;

            CPacket()
            {
                mTrackDataAlloc = false;
                reset();
            }

            ~CPacket()
            {
            }

            void destroyData()
            {
                if(mTrackDataAlloc && mData)
                {
                    delete[] mData;
                    mData = 0;
                }
                mTrackDataAlloc = false;
            }

            void reset()
            {
                destroyData();
                mVersion = 1;
                mMessageType = EMT_NULL;
                mDataSize = 0;
                mData = 0;
            }

            void setData(uint8_t* data, uint64_t dataSize, bool trackAlloc = false)
            {
                mData = data;
                mDataSize = dataSize;
                mTrackDataAlloc = trackAlloc;
            }
        };
    }
}

#endif