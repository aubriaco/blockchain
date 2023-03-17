#include "CChain.h"
#include "storage/storage.h"
#include <stdexcept>

namespace blockchain
{
    CChain::CChain(int difficulty, storage::E_STORAGE_TYPE storageType)
    {
        mRunning = true;
        mStopped = false;
        mDifficulty = difficulty;
        mNetPort = 7698;
        mStorage = storage::createStorage(storageType);  // initialize storage
        mServer = new net::CServer(this, mNetPort);
        mClient = 0;
        CBlock* block = new CBlock(0);
        mChain.push_back(block);  // First block (genesis)
        block->mine(mDifficulty);
        mCurrentBlock = block;
        load();
        mServer->start();
    }

    CChain::CChain(bool newChain, const std::string& connectToNode, int difficulty, storage::E_STORAGE_TYPE storageType) : CChain(difficulty, storageType)
    {
        if(!newChain)
        {
            if(connectToNode.empty())
                throw std::runtime_error("When not creating a new chain, you must specify 'connectToNode'.");
            mClient = new net::CClient(this, connectToNode, mNetPort);
            mClient->start();
        }
    }

    CChain::~CChain()
    {
        if(mClient)
            delete mClient;
        delete mServer;
        mStorage->dispose();
        for(std::vector<CBlock*>::iterator it = mChain.begin(); it != mChain.end(); ++it)
        {
            delete (*it);
        }
        mRunning = false;
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
    }

    CBlock* CChain::getCurrentBlock()
    {
        return mCurrentBlock;
    }

    void CChain::load()
    {
        mStorage->loadChain(&mChain);
        mCurrentBlock = mChain.back();
        if(mChain.size() > 1)
            nextBlock(false);
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
        if(mClient)
            mClient->stop();
        mServer->stop();
        mStopped = true;
    }

    bool CChain::isRunning()
    {
        return !mStopped;
    }

}