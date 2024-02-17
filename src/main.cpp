/*
 * Copyright 2023-2024 Alessandro Ubriaco. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").
 * You may not use this file except in the compliance with the License.
 * You may obtain a copy of the license in the file LICENSE.txt
 * in the source distribution.
 */
#include "blockchain/CChain.h"
#include <iostream>
#include <ctime>
#include <unistd.h>
#include <signal.h>
#include <map>

using namespace std;
using namespace blockchain;

CChain *gChain;

void interruptCallback(int sig)
{
    cout << "\n";
    gChain->stop();
}

bool tobool(std::string str)
{
    for (int n = 0; n < str.size(); n++)
        str[n] = std::tolower(str[n]);

    if (str == "true" || str == "t" || str == "1")
        return true;
    return false;
}

void printChain(CChain* chain) {
    CBlock *cur = chain->getCurrentBlock();
    do
    {
        time_t ts = cur->getCreatedTS();
        string tstr(ctime(&ts));
        tstr.resize(tstr.size() - 1);
        if(cur == chain->getCurrentBlock())
            cout << "Block " << cur->getHashStr() << "\tTimeStamp " << tstr << "\tData Size " << cur->getDataSize() << "\t(CURRENT)\n";
        else
            cout << "Block " << cur->getHashStr() << "\tTimeStamp " << tstr << "\tData Size " << cur->getDataSize() << "\n";
    } while (cur = cur->getPrevBlock());
}

int main(int argc, char **argv)
{
    signal(SIGPIPE, SIG_IGN);
    if (argc == 0)
    {
        cout << "Error: no binary parameter passed by system.\n";
        return 1;
    }
    string binName(argv[0]);
    if (argc == 1)
    {
        cout << "Usage:\n"
             << binName + " -hYOURHOST -cCONNECTTO -nFALSE\n\n-h\tHOSTNAME\tYour host entry point.\n-c\tHOSTNAME\tConnect to node entrypoint hostname.\n-n\ttrue | false\tIs this a new chain or not.\n\n";
        return 1;
    }

    map<string, string> params;

    for (int n = 0; n < argc; n++)
    {
        string param(argv[n]);
        if (param.size() > 2 && param[0] == '-')
        {
            string varName(param.substr(1, 1));
            params[varName] = param.substr(2);
        }
    }

    if (params.count("h") == 0)
    {
        cout << "You must specify host entrypoint for your node using -h:\nExample: " + binName + " -h127.0.0.1\n\n";
        return 1;
    }

    if (params.count("n") == 0)
    {
        if (params.count("c") == 0)
            params["n"] = "true";
        else
            params["n"] = "false";
    }
    bool isNewChain = tobool(params["n"]);

    if (!isNewChain && params.count("c") == 0)
    {
        cout << "If this is an existing chain. You must specify which node to connect to using -c:\nExample: " + binName + " -cchain.solusek.com\n\n";
        return 1;
    }

    uint32_t hostPort = 7698, connectPort = 7698;
    std::string host(params["h"]), connectTo(params["c"]);
    size_t pos = params["h"].find(':');
    if (pos != std::string::npos)
    {
        host = params["h"].substr(0, pos);
        hostPort = (uint32_t)std::stoi(params["h"].substr(pos + 1));
    }
    pos = params["c"].find(':');
    if (pos != std::string::npos)
    {
        connectTo = params["c"].substr(0, pos);
        connectPort = (uint32_t)std::stoi(params["c"].substr(pos + 1));
    }

    storage::E_STORAGE_TYPE storageType(storage::EST_LOCAL);

    if (params.count("s") != 0)
    {
        if (params["s"] == "none")
            storageType = storage::EST_NONE;
    }

    cout << "Start.\n";

    CChain chain(host, isNewChain, connectTo, 1, storageType, hostPort, connectPort);
    gChain = &chain;

    cout << "Chain intialized!\n";
    cout << "Current block count: " << chain.getBlockCount() << "\n";

    if (chain.isValid())
        cout << "Chain is valid!\n";
    else
    {
        cout << "INVALID CHAIN\n";
        return 1;
    }

    CBlock *current = chain.getCurrentBlock();

    if (isNewChain)
    {
        uint8_t *garbage = new uint8_t[32];
        for (uint32_t n = 0; n < 32; n++)
            garbage[n] = clock() % 255;

        cout << "Garbage generated.\n";

        chain.appendToCurrentBlock(garbage, 32);
        delete[] garbage;

        cout << "Garbage appended to current block.\n";

        chain.nextBlock();

        cout << "Next block mined.\n";

        cout << "Current Hash: " << chain.getCurrentBlock()->getPrevBlock()->getHashStr() << "\nNonce: " << chain.getCurrentBlock()->getNonce() << "\n";

        garbage = new uint8_t[32];
        for (uint32_t n = 0; n < 32; n++)
            garbage[n] = clock() % 255;

        cout << "Garbage generated.\n";

        chain.appendToCurrentBlock(garbage, 32);
        delete[] garbage;

        cout << "Garbage appended to current block.\n";

        chain.nextBlock();

        cout << "Next block mined.\n";

        cout << "Previous Hash: " << chain.getCurrentBlock()->getPrevBlock()->getHashStr() << "\nNonce: " << chain.getCurrentBlock()->getNonce() << "\n";
    }
    else
    {
        uint8_t* garbage = new uint8_t[32];
        for(uint32_t n = 0; n < 32; n++)
            garbage[n] = clock() % 255;
        chain.appendToCurrentBlock(garbage, 32);
        delete[] garbage;

        cout << "Garbage appended to current block.\n";	

        chain.nextBlock();

        cout << "Next block mined.\n";

        cout << "Previous Hash: " << chain.getCurrentBlock()->getPrevBlock()->getHashStr() << "\nNonce: " << chain.getCurrentBlock()->getNonce() << "\n";
    }
    cout << "Current block count: " << chain.getBlockCount() << "\n";

    cout << "\n"
         << "## BLOCK LIST (Descending)"
         << "\n";

    printChain(&chain);

    // Interrupt Signal
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = interruptCallback;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);
    sigaction(SIGQUIT, &sigIntHandler, NULL);

    CBlock* printedBlock = chain.getCurrentBlock();

    while (chain.isRunning()) {

        usleep(5000);
        if(printedBlock != chain.getCurrentBlock())
        {
            printChain(&chain);
            printedBlock = chain.getCurrentBlock();
        }
    }

    cout << "\nExit.\n";

    return 0;
}
