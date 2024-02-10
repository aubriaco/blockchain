/*
 * Copyright 2023-2024 Alessandro Ubriaco. All Rights Reserved.
 * 
 * Licensed under the Apache License 2.0 (the "License").
 * You may not use this file except in the compliance with the License.
 * You may obtain a copy of the license in the file LICENSE.txt
 * in the source distribution.
*/
#ifndef __I_NET_INCLUDED__
#define __I_NET_INCLUDED__
#include "CPacket.h"

namespace blockchain
{
    namespace net
    {
        class INet
        {
        public:
            int mSocket;    // socket handle
            CPacket recvPacket();   // receive packet of data
            void sendPacket(CPacket* packet);   // send packet of data
        protected:
            INet();
        private:
            const uint32_t mChunkSize = 2048;   // chunk data size

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