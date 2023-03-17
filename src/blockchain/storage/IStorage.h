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