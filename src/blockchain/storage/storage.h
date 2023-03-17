#ifndef __STORAGE_INCLUDED__
#define __STORAGE_INCLUDED__
#include "IStorage.h"
#include "EStorageType.h"

namespace blockchain
{
    namespace storage
    {
        IStorage* createStorage(E_STORAGE_TYPE type);
    }
}

#endif