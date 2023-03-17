#include "INet.h"
#include <stdexcept>
#include <netinet/in.h>

namespace blockchain
{
    namespace net
    {
        INet::INet()
        {
            mSocket = 0;
        }

        DPacket INet::recvPacket()
        {
            if(mSocket == 0)
                throw std::runtime_error("INet: Socket is null.");
            DPacket packet;
            packet.mVersion = recvUInt();
            packet.mMessageType = (EMessageType)recvUInt();
            packet.mDataSize = recvUInt64();
            if(packet.mDataSize != 0)
                packet.mData = recvDataAlloc(packet.mDataSize);
            return packet;
        }

        void INet::sendPacket(DPacket* packet)
        {
            if(mSocket == 0)
                throw std::runtime_error("INet: Socket is null.");
            sendUInt(packet->mVersion);
            sendUInt(packet->mMessageType);
            sendUInt64(packet->mDataSize);
            if(packet->mDataSize != 0 && packet->mData)
                sendData(packet->mData, packet->mDataSize);
        }

        uint32_t INet::recvUInt()
        {
            uint32_t netNum = 0;
            if(recv(mSocket, (void*)&netNum, sizeof(uint32_t), 0) < 0)
                throw std::runtime_error("Failed to receive UINT32.");
            return ntohl(netNum);
        }

        void INet::sendUInt(uint32_t num)
        {
            uint32_t netNum = htonl(num);
            if(send(mSocket, (void*)&netNum, sizeof(uint32_t), 0) < 0)
                throw std::runtime_error("Failed to send UINT32.");
        }

        int32_t INet::recvInt()
        {
            uint32_t netNum = 0;
            if(recv(mSocket, (void*)&netNum, sizeof(uint32_t), 0) < 0)
                throw std::runtime_error("Failed to receive INT32.");
            return ntohl(netNum);
        }

        void INet::sendInt(int32_t num)
        {
            uint32_t netNum = htonl((uint32_t)num);
            if(send(mSocket, (void*)&netNum, sizeof(uint32_t), 0) < 0)
                throw std::runtime_error("Failed to send INT32.");
        }

        uint64_t INet::recvUInt64()
        {
            uint32_t netHigh = 0;
            uint32_t netLow = 0;
            if(recv(mSocket, (void*)&netHigh, sizeof(uint32_t), 0) < 0)
                throw std::runtime_error("Failed to receive UINT64 (H).");
            if(recv(mSocket, (void*)&netLow, sizeof(uint32_t), 0) < 0)
                throw std::runtime_error("Failed to receive UINT64 (L).");

            return (((uint64_t)ntohl(netHigh) & 0xFFFFFFFFLL) << 32) | ((uint64_t)ntohl(netLow) >> 32);
        }

        void INet::sendUInt64(uint64_t num)
        {
            uint32_t netHigh = htonl((uint32_t)(num >> 32));
            uint32_t netLow = htonl((uint32_t)(num & 0xFFFFFFFFLL));
            if(send(mSocket, (void*)&netHigh, sizeof(uint32_t), 0) < 0)
                throw std::runtime_error("Failed to send UINT64 (H).");
            if(send(mSocket, (void*)&netLow, sizeof(uint32_t), 0) < 0)
                throw std::runtime_error("Failed to send UINT64 (L).");
        }

        uint8_t* INet::recvDataAlloc(uint64_t size)
        {
            uint32_t chunkSize = mChunkSize;
            uint8_t* data = new uint8_t[size];
            uint8_t* ptr = data;
            while(ptr < (data + size))
            {
                if((ptr + chunkSize) > (data + size))
                    chunkSize = (data + size - ptr);
                ssize_t r = recv(mSocket, (void*)ptr, sizeof(uint8_t) * chunkSize, 0);
                if(r < 0)
                    throw std::runtime_error("Failed to receive data chunk.");                
                ptr += r;
            }
            return data;
        }

        void INet::sendData(uint8_t* data, uint64_t size)
        {
            uint32_t chunkSize = mChunkSize;
            uint8_t* ptr = data;
            while(ptr < (data+size))
            {
                if((ptr + chunkSize) > (data + size))
                    chunkSize = (data + size - ptr);
                ssize_t r = send(mSocket, (void*)ptr, sizeof(uint8_t) * chunkSize, 0);
                if(r < 0)
                    throw std::runtime_error("Failed to send data chunk.");
                ptr += r;
            }
        }
    }
}
