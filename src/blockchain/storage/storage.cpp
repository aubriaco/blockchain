#include "storage.h"
#include "CStorageNone.h"
#include "CStorageLocal.h"

namespace blockchain
{
    namespace storage
    {
        IStorage* createStorage(E_STORAGE_TYPE type)
        {
            if(type == EST_LOCAL)
                return new CStorageLocal();
            else if(type == EST_NONE)
                return new CStorageNone();
            return 0;
        }
    }
}
