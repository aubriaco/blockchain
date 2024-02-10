/*
 * Copyright 2023-2024 Alessandro Ubriaco. All Rights Reserved.
 * 
 * Licensed under the Apache License 2.0 (the "License").
 * You may not use this file except in the compliance with the License.
 * You may obtain a copy of the license in the file LICENSE.txt
 * in the source distribution.
*/
#ifndef __C_STORAGE_LOCAL_INCLUDED__
#define __C_STORAGE_LOCAL_INCLUDED__
#include "IStorage.h"
#include "../CBlock.h"
#include "../CChain.h"
#include "../CLog.h"
#include <string>
#include <vector>
#include <map>

namespace blockchain
{
    namespace storage
    {
        class CStorageLocal : public IStorage
        {
        private:
            const uint32_t Version = 1;
            const std::string mBasePath = std::string("data/");
            const uint32_t mChunkSize = 2048;
            std::map<std::string, std::basic_string<uint8_t>> mMetaData;

            CLog mLog;
        public:
            CStorageLocal();
            ~CStorageLocal();

            virtual void loadChain(std::vector<CBlock*>* chain);

            virtual void load(CBlock* block);
            virtual void save(CBlock* block, uint64_t blockCount);

            void loadMetaData();
            void saveMetaData();

            virtual void dispose();
        };
    }
}

#endif