#include "CClient.h"
#include "../CChain.h"
#include <stdexcept>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>


#define PCHAIN ((CChain*)chain)

namespace blockchain
{
    namespace net
    {
        CClient::CClient(void* chain, const std::string& host, uint32_t port)
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

            if(connect(mSocket, (struct sockaddr*)&mAddr, sizeof(mAddr)) < 0)
                throw std::runtime_error("Failed to connect to host.");

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
    
                while(mRunning)
                {
                    DPacket writePacket;
                    writePacket.mMessageType = EMT_PING;
                    sendPacket(&writePacket);
                    DPacket gotPacket = recvPacket();

                    if(gotPacket.mMessageType != EMT_PING)
                        throw std::runtime_error("Failed to get ping back from server.");

                    usleep(5000);
                }

            }
            catch(std::runtime_error e)
            {
                throw std::runtime_error(std::string("Client: ") + e.what());
            }
            close(mSocket);
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