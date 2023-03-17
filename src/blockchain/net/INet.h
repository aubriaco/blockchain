#ifndef __I_NET_INCLUDED__
#define __I_NET_INCLUDED__
#include "DPacket.h"

namespace blockchain
{
    namespace net
    {
        class INet
        {
        public:
            int mSocket;
            DPacket recvPacket();
            void sendPacket(DPacket* packet);
        protected:
            INet();
        private:
            const uint32_t mChunkSize = 2048;

            uint32_t recvUInt();
            void sendUInt(uint32_t num);
            int32_t recvInt();
            void sendInt(int32_t num);
            uint64_t recvUInt64();
            void sendUInt64(uint64_t num);
            uint8_t* recvDataAlloc(uint64_t size);
            void sendData(uint8_t* data, uint64_t size);
        };
    }
}

#endif