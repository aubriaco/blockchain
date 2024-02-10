/*
 * Copyright 2023-2024 Alessandro Ubriaco. All Rights Reserved.
 * 
 * Licensed under the Apache License 2.0 (the "License").
 * You may not use this file except in the compliance with the License.
 * You may obtain a copy of the license in the file LICENSE.txt
 * in the source distribution.
*/
#ifndef __C_SERVER_INCLUDED__
#define __C_SERVER_INCLUDED__
#include "INet.h"
#include "CNodeInfo.h"
#include "../CLog.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <vector>

namespace blockchain
{
    namespace net
    {
        class CServer : protected INet
        {
        private:
            void* mChain;
            std::vector<CNodeInfo> mNodeList;
            uint32_t mListenPort;
            int mListenerSocket;
            int mBacklog;
            struct sockaddr_in mServerAddr;
            bool mRunning;
            bool mStopped;
            pthread_t mWorkerThread;

            CLog mLog;
        protected:
            class CSocketPackage : public INet
            {
            public:
                CServer* mServer;
                pthread_t mThread;
                bool mRunning;

                CSocketPackage(CServer* server, int socket)
                {
                    mServer = server;
                    mSocket = socket;
                    mThread = 0;
                    mRunning = true;
                }

                ~CSocketPackage() {}
            };

            void startWorker();
            static void* static_worker(void* param);
            void worker();

            void startClient(int socket);
            static void* static_client(void* param);
            void client(CSocketPackage* pkg);

            void addNodeToList(const std::string& hostname, uint32_t port);

            void processPacket(CSocketPackage* pkg, CPacket* packet, bool* pingConfirm);

        public:
            CServer(void* chain, uint32_t listenPort);
            ~CServer();
            void start();
            void stop();
            
            
        };
    }
}

#endif