/*
 * Copyright 2023-2024 Alessandro Ubriaco. All Rights Reserved.
 * 
 * Licensed under the Apache License 2.0 (the "License").
 * You may not use this file except in the compliance with the License.
 * You may obtain a copy of the license in the file LICENSE.txt
 * in the source distribution.
*/
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
