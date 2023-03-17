#ifndef __C_CLIENT_INCLUDED__
#define __C_CLIENT_INCLUDED__
#include "INet.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdint.h>
#include <string>
#include <pthread.h>


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
        protected:
            void startWorker();
            static void* static_worker(void* param);
            void worker();
        public:
            CClient(void* chain, const std::string& host, uint32_t port);
            ~CClient();
            void start();
            void stop();
        };
    }
}

#endif