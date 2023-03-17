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
            EMT_COUNT
        };
    }
}

#endif