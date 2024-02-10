#include "CClient.h"
#include "../CChain.h"
#include <stdexcept>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <iostream>

#define PCHAIN ((CChain*)mChain)

namespace blockchain
{
    namespace net
    {
        CClient::CClient(void* chain, const std::string& host, uint32_t port) : mLog("Client")
        {
            mChain = chain;
            mPort = port;
            mSocket = 0;
            mRunning = false;
            mStopped = false;
            mWorkerThread = 0;

            memset((char *)&mAddr, 0, sizeof(mAddr));
            mAddr.sin_family = AF_INET;
            mAddr.sin_port = htons(port);
            std::cout << host << "\n";
            if(inet_pton(AF_INET, host.c_str(), &mAddr.sin_addr) <= 0)
                throw std::runtime_error("Invalid host to connect to.");
        }

        CClient::~CClient()
        {
            stop();
        }

        void CClient::start()
        {
            mRunning = true;
            mSocket = socket(AF_INET, SOCK_STREAM, 0);
            if(mSocket < 0)
                throw std::runtime_error("Could not open socket.");

            mLog.writeLine("Connecting...");

            if(connect(mSocket, (struct sockaddr*)&mAddr, sizeof(mAddr)) < 0)
                throw std::runtime_error("Failed to connect to host.");

            mLog.writeLine("Connected to host!");

            startWorker();
        }

        void CClient::startWorker()
        {
            pthread_attr_t tattr;
            pthread_attr_init(&tattr);
            pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
            if(pthread_create(&mWorkerThread, &tattr, &static_worker, this) != 0)
                throw std::runtime_error("Failed to start client worker thread.");
            pthread_attr_destroy(&tattr);
        }

        void* CClient::static_worker(void* param)
        {
            CClient* client = (CClient*)param;
            client->worker();
            return 0;
        }

        void CClient::worker()
        {
            try
            {
                struct timeval timeout;
                timeout.tv_sec = 10;
                timeout.tv_usec = 0;
                if(setsockopt(mSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeval)) < 0)
                    std::runtime_error("Could not setup socket timeout.");


                bool pingConfirm = false;

                
                CPacket writePacket, gotPacket;
                writePacket.mMessageType = EMT_NODE_REGISTER;
                mLog.writeLine("Hostname size: " + std::to_string(PCHAIN->getHostName().size()));
                writePacket.setData((uint8_t*)PCHAIN->getHostName().c_str(), PCHAIN->getHostName().size());
                sendPacket(&writePacket);

                gotPacket = recvPacket();
                gotPacket.destroyData();
                if(gotPacket.mMessageType != EMT_ACK)
                    throw std::runtime_error("Server has rejected client.");

                mLog.writeLine("Server has acknoledged client.");

                writePacket.reset();
                writePacket.mMessageType = EMT_NODE_REGISTER_PORT;
                uint32_t netPort = PCHAIN->getNetPort();
                writePacket.setData((uint8_t*)&netPort, sizeof(uint32_t));
                sendPacket(&writePacket);

                gotPacket = recvPacket();
                gotPacket.destroyData();
                if(gotPacket.mMessageType != EMT_ACK)
                    throw std::runtime_error("Server has rejected client port.");

                while(mRunning)
                {
                    writePacket.mMessageType = EMT_PING;
                    sendPacket(&writePacket);
                    gotPacket = recvPacket();

                    if(gotPacket.mMessageType != EMT_PING)                    
                        throw std::runtime_error("Failed to get ping back from server.");
                    else if(!pingConfirm)
                    {
                        mLog.writeLine("Confirmed ping from server. Will continue to ping...");
                        pingConfirm = true;
                    }
                    gotPacket.destroyData();
                    usleep(5000);
                }

                gotPacket.destroyData();

            }
            catch(std::runtime_error e)
            {
                mLog.errorLine(std::string("Error: ") + e.what());
            }
            close(mSocket);
            mLog.writeLine("Closed.");
            mStopped = true;
        }

        void CClient::stop()
        {
            mRunning = false;
            while(!mStopped)
                usleep(10);
        }

    }
}