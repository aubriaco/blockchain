/*
 * Copyright 2023-2024 Alessandro Ubriaco. All Rights Reserved.
 * 
 * Licensed under the Apache License 2.0 (the "License").
 * You may not use this file except in the compliance with the License.
 * You may obtain a copy of the license in the file LICENSE.txt
 * in the source distribution.
*/
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