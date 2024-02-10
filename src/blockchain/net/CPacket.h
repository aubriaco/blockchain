/*
 * Copyright 2023-2024 Alessandro Ubriaco. All Rights Reserved.
 * 
 * Licensed under the Apache License 2.0 (the "License").
 * You may not use this file except in the compliance with the License.
 * You may obtain a copy of the license in the file LICENSE.txt
 * in the source distribution.
*/
#ifndef __C_PACKET_INCLUDED__
#define __C_PACKET_INCLUDED__
#include "EMessageType.h"
#include <stdint.h>
#include <openssl/sha.h>
#include <ctime>
#include <string.h>

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
            uint8_t mHash[SHA256_DIGEST_LENGTH];
            uint8_t mPrevHash[SHA256_DIGEST_LENGTH];
            time_t mCreatedTS;
            uint32_t mNonce;

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
                mNonce = 0;
                mCreatedTS = 0;
                memset(mHash, 0, SHA256_DIGEST_LENGTH);
                memset(mPrevHash, 0, SHA256_DIGEST_LENGTH);
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