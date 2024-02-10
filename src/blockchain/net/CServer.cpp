#include "CServer.h"
#include "../CChain.h"
#include <stdexcept>
#include <unistd.h>
#include <arpa/inet.h>

#define PCHAIN ((CChain*)mChain)

namespace blockchain
{
    namespace net
    {

        CServer::CServer(void* chain, uint32_t listenPort) : mLog("Server")
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
            mServerAddr.sin_port = htons(mListenPort);      // htons (short) -> net (short)
            mServerAddr.sin_addr.s_addr = INADDR_ANY;       // listen to any client
            if(bind(mListenerSocket, (struct sockaddr*)&mServerAddr, sizeof(mServerAddr)) < 0)  // bind function
                throw std::runtime_error(std::string("Could not bind to port.") + std::to_string(mListenPort));

            listen(mListenerSocket, mBacklog);  // listen 

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
            try
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
                }
            }
            catch(std::runtime_error e)
            {
                mLog.errorLine(std::string("Error: ") + e.what());
            }
            close(mListenerSocket);
            mLog.writeLine("Closed listener.");
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

                bool clientApproved = false;
                bool pingConfirm = false;
    
                while(mRunning && pkg->mRunning)
                {
                    CPacket gotPacket = pkg->recvPacket();
                    if(gotPacket.mMessageType == EMT_NODE_REGISTER)
                    {
                        

                        std::string clientHostName((char*)gotPacket.mData, gotPacket.mDataSize);
                        gotPacket.reset();
                        gotPacket.mMessageType = EMT_ACK;
                        pkg->sendPacket(&gotPacket);
                        mLog.writeLine("Got client hostname: " + clientHostName);

                        gotPacket = pkg->recvPacket();
                        if(gotPacket.mMessageType != EMT_NODE_REGISTER_PORT)
                        {
                            gotPacket.destroyData();
                            throw std::runtime_error("Expecting EMT_NODE_REGISTER_PORT.");
                        }

                        if(gotPacket.mDataSize != sizeof(uint32_t))
                        {
                            gotPacket.destroyData();
                            throw std::runtime_error("Expecting data size bigger than 0.");
                        }

                        uint32_t clientPort = *((uint32_t*)gotPacket.mData);

                        gotPacket.reset();
                        gotPacket.mMessageType = EMT_ACK;
                        pkg->sendPacket(&gotPacket);

                        addNodeToList(clientHostName, clientPort);

                        mLog.writeLine("Acknoledge client.");
                        clientApproved = true;
                    }
                    else
                    {
                        if(!clientApproved)
                        {
                            gotPacket.destroyData();
                            throw std::runtime_error("Client is not approved for anything except EMT_NODE_REGISTER.");
                        }

                        if(gotPacket.mMessageType == EMT_PING)
                        {
                            if(!pingConfirm)
                            {
                                mLog.writeLine("Confirmed ping from client. Pings are now silent.");
                                pingConfirm = true;
                            }
                            pkg->sendPacket(&gotPacket);
                        }
                        else
                        {
                            gotPacket.destroyData();
                            throw std::runtime_error(std::string("Unknown packet received: ") + std::to_string(gotPacket.mMessageType));
                        }
                    }
                    gotPacket.destroyData();
                }
            }
            catch(std::runtime_error e)
            {
                mLog.errorLine(std::string("Node Error: ") + e.what());
            }
            shutdown(pkg->mSocket, SHUT_RDWR);
            close(pkg->mSocket);
            mLog.writeLine("Closed node.");
            delete pkg;            
        }

        void CServer::addNodeToList(const std::string& hostname, uint32_t port)
        {
            for(std::vector<CNodeInfo>::iterator it = mNodeList.begin(); it != mNodeList.end(); ++it)
            {
                if((*it).mHostName == hostname)
                {                    
                    (*it).seen();
                    mLog.writeLine("Found node in list: " + hostname);
                    if((*it).mPort != port)
                    {
                        (*it).mPort = port;
                        mLog.writeLine("Port was updated to: " + std::to_string(port));
                    }
                    return;
                }
            }
            mNodeList.push_back(CNodeInfo(hostname, port));

            mLog.writeLine("Added new node to list: " + hostname + ":" + std::to_string(port));
            
            if(hostname != PCHAIN->getHostName() || port != PCHAIN->getNetPort())   // avoid connecting eternally to itself
                PCHAIN->connectNewClient(hostname, port);
        }
    }
}