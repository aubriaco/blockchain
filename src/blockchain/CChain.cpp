/*
 * Copyright 2023-2024 Alessandro Ubriaco. All Rights Reserved.
 * 
 * Licensed under the Apache License 2.0 (the "License").
 * You may not use this file except in the compliance with the License.
 * You may obtain a copy of the license in the file LICENSE.txt
 * in the source distribution.
*/
#include "CChain.h"
#include "net/CPacket.h"
#include "storage/storage.h"
#include <stdexcept>
#include <unistd.h>

namespace blockchain
{
    CChain::CChain(const std::string& hostname, int difficulty, storage::E_STORAGE_TYPE storageType, uint32_t hostPort) : mLog("Chain")
    {
        CLog::open(false);
        mRunning = true;
        mStopped = false;
        mHostName = hostname;
        mDifficulty = difficulty;
        mNetPort = hostPort;
        mStorage = storage::createStorage(storageType);  // initialize storage
        mServer = new net::CServer(this, mNetPort);
        CBlock* block = new CBlock(0);
        mChain.push_back(block);  // First block (genesis)
        block->mine(mDifficulty);
        mCurrentBlock = block;
        load();
        mServer->start();
        mReady = true;
    }

    CChain::CChain(const std::string& hostname, bool newChain, const std::string& connectToNode, int difficulty, storage::E_STORAGE_TYPE storageType, uint32_t hostPort, uint32_t connectPort) : CChain(hostname, difficulty, storageType, hostPort)
    {
        if(!newChain)
        {
            if(connectToNode.empty())
                throw std::runtime_error("When not creating a new chain, you must specify 'connectToNode'.");
            net::CClient* client = connectNewClient(connectToNode, connectPort);
            while(!client->isReady())
                usleep(1);
            mReady = true;
            mLog.writeLine("Chain ready!");
        }
    }

    CChain::~CChain()
    {
        if(mClients.size() != 0)
        {
            for(std::vector<net::CClient*>::iterator it = mClients.begin(); it != mClients.end(); ++it)
            {
                delete (*it);
            }
            mClients.clear();
        }
        delete mServer;
        mStorage->dispose();
        for(std::vector<CBlock*>::iterator it = mChain.begin(); it != mChain.end(); ++it)
        {
            delete (*it);
        }
        mChain.clear();
        CLog::close();
        mRunning = false;
        mLog.writeLine("Cleanup completed.");
    }

    void CChain::appendToCurrentBlock(uint8_t* data, uint32_t size)
    {
        mCurrentBlock->appendData(data, size);
    }

    void CChain::nextBlock(bool save)
    {
        mCurrentBlock->calculateHash();
        if(save)
            mStorage->save(mCurrentBlock, mChain.size());
        CBlock* block = new CBlock(mCurrentBlock);
        mChain.push_back(block);
        block->mine(mDifficulty);
        
        mCurrentBlock = block;
        distributeBlock(block);
    }

    void CChain::distributeBlock(CBlock* block)
    {
        for(std::vector<net::CClient*>::iterator it = mClients.begin(); it != mClients.end(); ++it)
        {
            (*it)->sendBlock(block);
        }
    }

    CBlock* CChain::getCurrentBlock()
    {
        return mCurrentBlock;
    }

    CBlock* CChain::getGenesisBlock()
    {
        if(mChain.empty())
            return 0;
        return mChain[0];
    }

    void CChain::load()
    {
        mStorage->loadChain(&mChain);
        mCurrentBlock = mChain.back();
        if(mChain.size() > 1)
            nextBlock(false);
    }

    std::vector<CBlock*>* CChain::getChainPtr()
    {
        return &mChain;
    }

    size_t CChain::getBlockCount()
    {
        return mChain.size();
    }

    bool CChain::isValid()
    {
        CBlock* cur = mCurrentBlock;
        while(cur = cur->getPrevBlock())
        {
            if(!cur->isValid())
                return false;
        }
        return true;
    }

    void CChain::stop()
    {
        mRunning = false;
        /*
        if(mClients.size() != 0)
        {
            for(std::vector<net::CClient*>::iterator it = mClients.begin(); it != mClients.end(); ++it)
            {
                (*it)->stop();
                mLog.writeLine("Waiting for client to stop...");
                while(!(*it)->isStopped())
                    sleep(1);
                mLog.writeLine("Stopped.");
            }
        }
        */
        mServer->stop();
        mStopped = true;
    }

    bool CChain::isRunning()
    {
        return !mStopped;
    }

    std::string CChain::getHostName()
    {
        return mHostName;
    }

    uint32_t CChain::getNetPort()
    {
        return mNetPort;
    }

    net::CClient* CChain::connectNewClient(const std::string& hostname, uint32_t port, bool child)
    {
        net::CClient* client = new net::CClient(this, hostname, port, child);
        mClients.push_back(client);
        mLog.writeLine("Connect Client: " + hostname + ":" + std::to_string(port));
        client->start();
        return client;
    }

    std::vector<net::CClient*>* CChain::getClientsPtr()
    {
        return &mClients;
    }

    bool CChain::isReady()
    {
        return mReady;
    }

    void CChain::insertBlock(CBlock* block)
    {
        if(mChain.empty())
            mCurrentBlock = block;
        mChain.insert(mChain.begin(), block);
    }

    void CChain::pushBlock(CBlock* block)
    {
        if(!mChain.empty())
        {
            block->setPrevBlock(mCurrentBlock);
            block->setPrevHash(mCurrentBlock->getPrevHash());
        }
        mChain.push_back(block);
        mCurrentBlock = block;
    }

    void CChain::clear()
    {
        for(std::vector<CBlock*>::iterator it = mChain.begin(); it != mChain.end(); ++it)
        {
            delete (*it);
        }
        mChain.clear();
    }
}