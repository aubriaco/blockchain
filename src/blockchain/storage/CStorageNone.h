/*
 * Copyright 2023-2024 Alessandro Ubriaco. All Rights Reserved.
 * 
 * Licensed under the Apache License 2.0 (the "License").
 * You may not use this file except in the compliance with the License.
 * You may obtain a copy of the license in the file LICENSE.txt
 * in the source distribution.
*/
#ifndef __C_STORAGE_NONE_INCLUDED__
#define __C_STORAGE_NONE_INCLUDED__
#include "IStorage.h"
#include "../CBlock.h"

namespace blockchain
{
    namespace storage
    {
        class CStorageNone : public IStorage
        {
        public:
            virtual void loadChain(std::vector<CBlock*>* chain) {};

            virtual void load(CBlock* block) {}
            virtual void save(CBlock* block, uint64_t blockCount) {}

            virtual void dispose() { delete this; }
        };
    }
}

#endif