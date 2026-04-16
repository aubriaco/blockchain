#include "CAPIServer.h"
#include <stdexcept>

using namespace solusek;

namespace blockchain::api
{
    solusek::IServer *CAPIServer::mServer = 0;

    solusek::MResponse endpoint_info(const void *sobj, solusek::MRequest &data)
    {
        solusek::IServer *server = (solusek::IServer *)sobj;

        solusek::MResponse resp;

        resp.Body = "TODO";

        return resp;
    }

    CAPIServer::CAPIServer()
    {
        if (mServer != 0)
            throw std::runtime_error("Server already initialized.");

        mServer = createServer();
        mServer->setInterruptCallback(interruptCallback);
        mServer->setListenPort(8080);
        mServer->registerEndpoint(new solusek::MEndpoint("/api/info", endpoint_info, "GET"));
    }

    CAPIServer::~CAPIServer()
    {
        mServer->dispose();
    }

    void CAPIServer::interruptCallback(int sig)
    {
        printf("Interrupt signal called.\n");
        mServer->stop();
    }

    void CAPIServer::run()
    {
        mServer->run();
    }

}