#include "CServer.h"
#include "../CChain.h"
#include <stdexcept>
#include <unistd.h>
#include <arpa/inet.h>

#define PCHAIN ((CChain*)chain)

namespace blockchain
{
    namespace net
    {

        CServer::CServer(void* chain, uint32_t listenPort)
        {
            mChain = chain;
            mListenPort = listenPort;
            mBacklog = 5;
            mListenerSocket = 0;
            mRunning = false;
            mStopped = false;
            mWorkerThread = 0;
        }

        CServer::~CServer()
        {
            stop();
        }

        void CServer::start()
        {
            mRunning = true;
            mSocket = mListenerSocket = socket(AF_INET, SOCK_STREAM, 0);
            if(mListenerSocket < 0)
                throw std::runtime_error("Could not open listener socket.");
            memset((char *)&mServerAddr, 0, sizeof(mServerAddr));
            mServerAddr.sin_family = AF_INET;
            mServerAddr.sin_port = htons(mListenPort);
            mServerAddr.sin_addr.s_addr = INADDR_ANY;
            if(bind(mListenerSocket, (struct sockaddr*)&mServerAddr, sizeof(mServerAddr)) < 0)
                throw std::runtime_error("Could not bind to port.");

            listen(mListenerSocket, mBacklog);

            startWorker();
        }

        void CServer::stop()
        {
            mRunning = false;

            int stopSocket = socket(AF_INET, SOCK_STREAM, 0);
            struct sockaddr_in addr;
            memset((char *)&addr, 0, sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_port = htons(mListenPort);
            inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
            if(connect(stopSocket, (struct sockaddr*)&addr, sizeof(addr)) >= 0)
                close(stopSocket);

            while(!mStopped)
                usleep(10);
        }

        void CServer::startWorker()
        {
            pthread_attr_t tattr;
            pthread_attr_init(&tattr);
            pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
            if(pthread_create(&mWorkerThread, &tattr, &static_worker, this) != 0)
                throw std::runtime_error("Failed to start worker thread.");
            pthread_attr_destroy(&tattr);
        }

        void* CServer::static_worker(void* param)
        {
            CServer* server = (CServer*)param;
            server->worker();
            return 0;
        }

        void CServer::worker()
        {
            while(mRunning)
            {
                struct sockaddr clientAddr;
                socklen_t clientAddrLen = sizeof(clientAddr);
                int clientSocket = accept(mListenerSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
                if(clientSocket < 0)
                    throw std::runtime_error("Failed to accept client socket.");
                if(!mRunning)
                {
                    close(clientSocket);
                    break;
                }
                startClient(clientSocket);
                usleep(1000);
            }
            close(mListenerSocket);
            mStopped = true;
        }

        void CServer::startClient(int socket)
        {
            CSocketPackage* pkg = new CSocketPackage(this, socket);
            pthread_attr_t tattr;
            pthread_attr_init(&tattr);
            pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
            if(pthread_create(&pkg->mThread, &tattr, &static_client, pkg) != 0)
                throw std::runtime_error("Failed to start client node thread.");
            pthread_attr_destroy(&tattr);
        }

        void* CServer::static_client(void* param)
        {
            CSocketPackage* pkg = (CSocketPackage*)param;
            pkg->mServer->client(pkg);
            return 0;
        }

        void CServer::client(CSocketPackage* pkg)
        {
            try
            {
                struct timeval timeout;
                timeout.tv_sec = 10;
                timeout.tv_usec = 0;
                if(setsockopt(pkg->mSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeval)) < 0)
                    std::runtime_error("Could not setup socket timeout.");
    
                while(mRunning && pkg->mRunning)
                {
                    DPacket gotPacket = pkg->recvPacket();
                    if(gotPacket.mMessageType == EMT_PING)
                    {
                        pkg->sendPacket(&gotPacket);
                    }
                    else
                        throw std::runtime_error(std::string("Unknown packet received: ") + std::to_string(gotPacket.mMessageType));
                }
            }
            catch(std::runtime_error e)
            {
                //throw std::runtime_error(std::string("Server Node: ") + e.what());
            }
            close(pkg->mSocket);
            delete pkg;            
        }
    }
}