/*
 * Copyright 2023-2024 Alessandro Ubriaco. All Rights Reserved.
 * 
 * Licensed under the Apache License 2.0 (the "License").
 * You may not use this file except in the compliance with the License.
 * You may obtain a copy of the license in the file LICENSE.txt
 * in the source distribution.
*/
#ifndef __I_STORAGE_INCLUDED__
#define __I_STORAGE_INCLUDED__
#include "../CBlock.h"
#include <vector>

namespace blockchain
{
    namespace storage
    {
        class IStorage
        {
        public:
            virtual void loadChain(std::vector<CBlock*>* chain) = 0;    // Load chain into memory

            virtual void load(CBlock* block) = 0;                       // Load block
            virtual void save(CBlock* block, uint64_t blockCount) = 0;  // Save block

            virtual void dispose() = 0;                                 // dispose 
        };
    }
}

#endif