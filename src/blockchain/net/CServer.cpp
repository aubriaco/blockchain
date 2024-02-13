/*
 * Copyright 2023-2024 Alessandro Ubriaco. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").
 * You may not use this file except in the compliance with the License.
 * You may obtain a copy of the license in the file LICENSE.txt
 * in the source distribution.
 */
#include "CServer.h"
#include "../CChain.h"
#include <stdexcept>
#include <unistd.h>
#include <arpa/inet.h>
#include <algorithm>
#include <signal.h>

#define PCHAIN ((CChain *)mChain)

namespace blockchain
{
    namespace net
    {

        CServer::CServer(void *chain, uint32_t listenPort) : mLog("Server")
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
            if (mListenerSocket < 0)
                throw std::runtime_error("Could not open listener socket.");

            struct timeval timeout;
            timeout.tv_sec = 10;
            timeout.tv_usec = 0;
            if (setsockopt(mListenerSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeval)) < 0)
                std::runtime_error("Could not setup socket send timeout.");
            if (setsockopt(mListenerSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeval)) < 0)
                std::runtime_error("Could not setup socket receive timeout.");

            memset((char *)&mServerAddr, 0, sizeof(mServerAddr));
            mServerAddr.sin_family = AF_INET;
            mServerAddr.sin_port = htons(mListenPort);                                           // htons (short) -> net (short)
            mServerAddr.sin_addr.s_addr = INADDR_ANY;                                            // listen to any client
            if (bind(mListenerSocket, (struct sockaddr *)&mServerAddr, sizeof(mServerAddr)) < 0) // bind function
                throw std::runtime_error(std::string("Could not bind to port.") + std::to_string(mListenPort));

            listen(mListenerSocket, mBacklog); // listen

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
            if (connect(stopSocket, (struct sockaddr *)&addr, sizeof(addr)) >= 0)
                close(stopSocket);

            
        }

        void CServer::startWorker()
        {
            pthread_attr_t tattr;
            pthread_attr_init(&tattr);
            pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
            if (pthread_create(&mWorkerThread, &tattr, &static_worker, this) != 0)
                throw std::runtime_error("Failed to start worker thread.");
            pthread_attr_destroy(&tattr);
        }

        void *CServer::static_worker(void *param)
        {
            CServer *server = (CServer *)param;
            server->worker();
            return 0;
        }

        void CServer::worker()
        {
            try
            {
                while (mRunning)
                {
                    struct sockaddr clientAddr;
                    socklen_t clientAddrLen = sizeof(clientAddr);
                    int clientSocket = accept(mListenerSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
                    if (clientSocket < 0)
                    {
                        // mLog.writeLine("Failed to accept client socket. Continue...");
                        continue;
                    }
                    if (!mRunning)
                    {
                        close(clientSocket);
                        break;
                    }
                    startClient(clientSocket);
                }
            }
            catch (std::runtime_error e)
            {
                mLog.errorLine(std::string("Error: ") + e.what());
            }
            close(mListenerSocket);
            mLog.writeLine("Closed listener.");
            mStopped = true;
        }

        void CServer::startClient(int socket)
        {
            CSocketPackage *pkg = new CSocketPackage(this, socket);
            mSocketPackages.push_back(pkg);
            pthread_attr_t tattr;
            pthread_attr_init(&tattr);
            pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
            if (pthread_create(&pkg->mThread, &tattr, &static_client, pkg) != 0)
                throw std::runtime_error("Failed to start client node thread.");
            pthread_attr_destroy(&tattr);
        }

        void *CServer::static_client(void *param)
        {
            CSocketPackage *pkg = (CSocketPackage *)param;
            pkg->mServer->client(pkg);
            return 0;
        }

        void CServer::client(CSocketPackage *pkg)
        {
            try
            {
                struct timeval timeout;
                timeout.tv_sec = 10;
                timeout.tv_usec = 0;
                if (setsockopt(pkg->mSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeval)) < 0)
                    std::runtime_error("Could not setup socket send timeout.");
                if (setsockopt(pkg->mSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeval)) < 0)
                    std::runtime_error("Could not setup socket receive timeout.");

                bool clientApproved = false;
                bool pingConfirm = false;

                while (mRunning && pkg->mRunning)
                {
                    CPacket gotPacket = pkg->recvPacket();
                    if (gotPacket.mMessageType == EMT_NODE_REGISTER)
                    {

                        std::string clientHostName((char *)gotPacket.mData, gotPacket.mDataSize);
                        gotPacket.reset();
                        gotPacket.mMessageType = EMT_ACK;
                        pkg->sendPacket(&gotPacket);
                        mLog.writeLine("Got client hostname: " + clientHostName);

                        gotPacket = pkg->recvPacket();
                        if (gotPacket.mMessageType != EMT_NODE_REGISTER_PORT)
                        {
                            gotPacket.destroyData();
                            throw std::runtime_error("Expecting EMT_NODE_REGISTER_PORT.");
                        }

                        if (gotPacket.mDataSize != sizeof(uint32_t))
                        {
                            gotPacket.destroyData();
                            throw std::runtime_error("Expecting data size bigger than 0.");
                        }

                        uint32_t clientPort = *((uint32_t *)gotPacket.mData);

                        gotPacket.reset();
                        gotPacket.mMessageType = EMT_ACK;
                        pkg->sendPacket(&gotPacket);

                        addNodeToList(clientHostName, clientPort);

                        mLog.writeLine("Acknoledge client.");
                        clientApproved = true;
                    }
                    else
                    {
                        if (!clientApproved)
                        {
                            gotPacket.destroyData();
                            throw std::runtime_error("Client is not approved for anything except EMT_NODE_REGISTER.");
                        }

                        try
                        {
                            processPacket(pkg, &gotPacket, &pingConfirm);
                        }
                        catch (std::runtime_error ex)
                        {
                            gotPacket.destroyData();
                            throw std::runtime_error(std::string("Runtime error: ") + ex.what());
                        }
                    }
                    gotPacket.destroyData();
                }
            }
            catch (std::runtime_error e)
            {
                mLog.errorLine(std::string("Node Error: ") + e.what());
            }
            shutdown(pkg->mSocket, SHUT_RDWR);
            close(pkg->mSocket);
            mLog.writeLine("Closed node.");
            std::vector<CSocketPackage *>::iterator f = std::find(mSocketPackages.begin(), mSocketPackages.end(), pkg);
            if (f != mSocketPackages.end())
                mSocketPackages.erase(f);
            delete pkg;
            mLog.writeLine("There are currently " + std::to_string(mSocketPackages.size()) + " active nodes.");
        }

        void CServer::processPacket(CSocketPackage *pkg, CPacket *packet, bool *pingConfirm)
        {
            if (packet->mMessageType == EMT_PING)
            {
                if (!*pingConfirm)
                {
                    mLog.writeLine("Confirmed ping from client. Pings are now silent.");
                    *pingConfirm = true;
                }
                pkg->sendPacket(packet);
            }
            else if (packet->mMessageType == EMT_INIT_CHAIN)
            {
                if (PCHAIN->getChainPtr()->size() > 0)
                {
                    CPacket initPacket;
                    initPacket.mMessageType = EMT_CHAIN_INFO;
                    memcpy(initPacket.mHash, PCHAIN->getCurrentBlock()->getHash(), SHA256_DIGEST_LENGTH);
                    pkg->sendPacket(&initPacket);
                }
                else
                {
                    CPacket initPacket;
                    initPacket.mMessageType = EMT_CHAIN_NEW;
                    pkg->sendPacket(&initPacket);
                }

                if (memcmp(packet->mHash, PCHAIN->getCurrentBlock()->getHash(), SHA256_DIGEST_LENGTH) == 0)
                {
                    mLog.writeLine("Both chains have same hash, no sync required.");
                    CPacket respPacket;
                    respPacket.mMessageType = EMT_ACK;
                    pkg->sendPacket(&respPacket);
                }
                else
                {
                    CBlock *block = PCHAIN->getCurrentBlock();
                    CPacket respPacket;
                    do
                    {
                        respPacket.mMessageType = EMT_WRITE_BLOCK;
                        respPacket.mData = block->getData();
                        respPacket.mDataSize = block->getDataSize();
                        respPacket.mCreatedTS = block->getCreatedTS();
                        respPacket.mNonce = block->getNonce();
                        memcpy(respPacket.mHash, block->getHash(), SHA256_DIGEST_LENGTH);
                        memcpy(respPacket.mPrevHash, block->getPrevHash(), SHA256_DIGEST_LENGTH);
                        pkg->sendPacket(&respPacket);
                    } while (block = block->getPrevBlock());
                    respPacket.reset();
                    respPacket.mMessageType = EMT_ACK;
                    pkg->sendPacket(&respPacket);
                }
                
            }
            else if (packet->mMessageType == EMT_WRITE_BLOCK)
            {
            }
            else
            {
                throw std::runtime_error(std::string("Unknown packet received: ") + std::to_string(packet->mMessageType));
            }
        }

        void CServer::addNodeToList(const std::string &hostname, uint32_t port)
        {
            for (std::vector<CClient *>::iterator it = PCHAIN->getClientsPtr()->begin(); it != PCHAIN->getClientsPtr()->end(); ++it)
            {
                if ((*it)->getHost() == hostname && (*it)->getPort() == port)
                {
                    mLog.writeLine("Already connected to this node (avoiding double-connect): " + hostname);
                    return;
                }
            }

            for (std::vector<CNodeInfo>::iterator it = mNodeList.begin(); it != mNodeList.end(); ++it)
            {
                if ((*it).mHostName == hostname && (*it).mPort == port)
                {
                    (*it).seen();
                    mLog.writeLine("Found node in list: " + hostname);
                    if ((*it).mPort != port)
                    {
                        (*it).mPort = port;
                        mLog.writeLine("Port was updated to: " + std::to_string(port));
                    }
                    return;
                }
            }
            mNodeList.push_back(CNodeInfo(hostname, port));

            mLog.writeLine("Added new node to list: " + hostname + ":" + std::to_string(port));

            if (hostname != PCHAIN->getHostName() || port != PCHAIN->getNetPort()) // avoid connecting eternally to itself
                PCHAIN->connectNewClient(hostname, port, true);
        }
    }
}