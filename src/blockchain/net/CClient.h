/*
 * Copyright 2023-2024 Alessandro Ubriaco. All Rights Reserved.
 * 
 * Licensed under the Apache License 2.0 (the "License").
 * You may not use this file except in the compliance with the License.
 * You may obtain a copy of the license in the file LICENSE.txt
 * in the source distribution.
*/
#ifndef __C_CLIENT_INCLUDED__
#define __C_CLIENT_INCLUDED__
#include "INet.h"
#include "../CLog.h"
#include "../CBlock.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdint.h>
#include <string>
#include <pthread.h>
#include <queue>


namespace blockchain
{
    namespace net
    {
        class CClient : protected INet
        {
        private:
            void* mChain;
            uint32_t mPort;
            bool mRunning;
            bool mStopped;
            struct sockaddr_in mAddr;
            pthread_t mWorkerThread;
            bool mPingConfirm;

            CLog mLog;

            std::string mHost;

            std::queue<CPacket> mQueue;
        protected:
            void startWorker();
            static void* static_worker(void* param);
            void worker();
            void processPacket(CPacket* packet, EMessageType responseTo);
            void init();
        public:
            CClient(void* chain, const std::string& host, uint32_t port);
            ~CClient();
            void start();
            void stop();
            void sendBlock(CBlock* block);
            std::string getHost() { return mHost; }
            uint32_t getPort() { return mPort; }
        };
    }
}

#endif