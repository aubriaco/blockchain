/*
 * Copyright 2023-2024 Alessandro Ubriaco. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").
 * You may not use this file except in the compliance with the License.
 * You may obtain a copy of the license in the file LICENSE.txt
 * in the source distribution.
 */

#include <solusek/solusek.h>

namespace blockchain::api
{
    class CAPIServer
    {
    protected:
        static solusek::IServer *mServer;

        static void interruptCallback(int sig);

    public:
        CAPIServer();
        ~CAPIServer();
        void run();
    };
}