/*
 * Copyright 2023-2024 Alessandro Ubriaco. All Rights Reserved.
 * 
 * Licensed under the Apache License 2.0 (the "License").
 * You may not use this file except in the compliance with the License.
 * You may obtain a copy of the license in the file LICENSE.txt
 * in the source distribution.
*/
#ifndef __E_MESSAGE_TYPE_INCLUDED__
#define __E_MESSAGE_TYPE_INCLUDED__

namespace blockchain
{
    namespace net
    {
        enum EMessageType
        {
            EMT_NULL,
            EMT_PING,
            EMT_ACK,
            EMT_ERR,
            EMT_NODE_REGISTER,
            EMT_NODE_REGISTER_PORT,
            EMT_INIT_CHAIN,
            EMT_WRITE_BLOCK,
            EMT_CHAIN_NEW,
            EMT_CHAIN_INFO,
            EMT_COUNT
        };
    }
}

#endif