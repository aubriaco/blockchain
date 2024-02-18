/*
 * Copyright 2023-2024 Alessandro Ubriaco. All Rights Reserved.
 * 
 * Licensed under the Apache License 2.0 (the "License").
 * You may not use this file except in the compliance with the License.
 * You may obtain a copy of the license in the file LICENSE.txt
 * in the source distribution.
*/
#include "CStorageLocal.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdexcept>

namespace blockchain
{
    namespace storage
    {
        std::string CStorageLocal::mDefaultBasePath("data/");

        void CStorageLocal::setDefaultBasePath(const std::string& path)
        {
            mDefaultBasePath = path;
            if(path.size() > 1 && path[path.size()-1] != '/')
                mDefaultBasePath.push_back('/');
        }

        CStorageLocal::CStorageLocal() : mLog("Storage"), mBasePath(mDefaultBasePath)
        {
            struct stat info;
            if(stat(mBasePath.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR))
            {
                int status = mkdir(mBasePath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);   // create the directory
            }

            loadMetaData();         // attempt to load the metadata
        }

        CStorageLocal::~CStorageLocal()
        {            
        }

        void CStorageLocal::loadChain(std::vector<CBlock*>* chain)
        {
            if(mMetaData.count("LAST_BLOCK_HASH") != 0)
            {
                chain->clear();

                CBlock* block = new CBlock(0, mMetaData["LAST_BLOCK_HASH"].data());
                load(block);
                chain->push_back(block);
                CBlock* cur = block;


                while(cur->hasPrevHash())
                {
                    block = new CBlock(0, cur->getPrevHash());
                    load(block);
                    cur->setPrevBlock(block);
                    chain->insert(chain->begin(), block);
                    cur = block;
                }

                uint64_t chainSize = 0;
                memcpy(&chainSize, mMetaData["BLOCK_COUNT"].data(), sizeof(uint64_t));
                if(chain->size() != chainSize)
                    throw std::runtime_error("Manifest: Chain size does not match BLOCK_COUNT.");
            }
        }

        void CStorageLocal::load(CBlock* block)
        {
            std::string path(mBasePath + block->getHashStr());
            FILE* file = fopen(path.c_str(), "rb");
            if(file)
            {
                size_t r;
                uint32_t version = 0;
                r = fread(&version, sizeof(uint32_t), 1, file);
                if(r != 1)
                    throw std::runtime_error("Could not read version.");

                //todo: check version

                uint8_t hash[SHA256_DIGEST_LENGTH];
                r = fread(hash, sizeof(uint8_t), SHA256_DIGEST_LENGTH, file);
                if(r != SHA256_DIGEST_LENGTH)
                    throw std::runtime_error("Could not read hash.");

                uint8_t prevHash[SHA256_DIGEST_LENGTH];
                r = fread(prevHash, sizeof(uint8_t), SHA256_DIGEST_LENGTH, file);
                if(r != SHA256_DIGEST_LENGTH)
                    throw std::runtime_error("Could not read prevHash.");

                block->setPrevHash(prevHash);

                time_t createdTS = 0;
                r = fread(&createdTS, sizeof(time_t), 1, file);
                if(r != 1)
                    throw std::runtime_error("Could not read createdTS.");

                block->setCreatedTS(createdTS);

                uint32_t nonce = 0;
                r = fread(&nonce, sizeof(uint32_t), 1, file);
                if(r != 1)
                    throw std::runtime_error("Could not read nonce.");

                block->setNonce(nonce);

                uint32_t dataSize = 0; 
                r = fread(&dataSize, sizeof(uint32_t), 1, file);
                if(r != 1)
                    throw std::runtime_error("Could not read dataSize.");

                uint8_t* data = new uint8_t[dataSize];
                uint8_t* ptr = data;
                for(uint32_t n = 0; n < dataSize; n += mChunkSize)
                {
                    r = fread(ptr, sizeof(uint8_t), mChunkSize, file);
                    if(r == 0)
                        throw std::runtime_error("Could not read data chunk.");
                    ptr += r;
                }

                block->setAllocatedData(data, dataSize);

                fclose(file);
            }
            else
                throw std::runtime_error("Block file not found.");
        }

        void CStorageLocal::save(CBlock* block, uint64_t blockCount)
        {
            std::string path(mBasePath + block->getHashStr());
            FILE* file = fopen(path.c_str(), "wb");
            if(file)
            {
                fwrite(&Version, sizeof(uint32_t), 1, file);
                fwrite(block->getHash(), sizeof(uint8_t), SHA256_DIGEST_LENGTH, file);
                fwrite(block->getPrevHash(), sizeof(uint8_t), SHA256_DIGEST_LENGTH, file);
                time_t createdTS = block->getCreatedTS();
                fwrite(&createdTS, sizeof(time_t), 1, file);
                uint32_t nonce = block->getNonce();
                fwrite(&nonce, sizeof(uint32_t), 1, file);
                uint32_t dataSize = block->getDataSize();
                fwrite(&dataSize, sizeof(uint32_t), 1, file);
                fwrite(block->getData(), sizeof(uint8_t), dataSize, file);
                fclose(file);

                
                mMetaData["LAST_BLOCK_HASH"] = std::basic_string<uint8_t>((uint8_t*)block->getHash(), SHA256_DIGEST_LENGTH);
                mMetaData["LAST_BLOCK_HASH_STR"] = std::basic_string<uint8_t>((uint8_t*)block->getHashStr().data(), block->getHashStr().size());
                mMetaData["BLOCK_COUNT"] = std::basic_string<uint8_t>((uint8_t*)&blockCount, sizeof(uint64_t));
                saveMetaData();
            }
        }

        void CStorageLocal::loadMetaData()
        {
            std::string metaDataFn(mBasePath + "metadata");
            FILE* file = fopen(metaDataFn.c_str(), "rb");
            if(file)
            {
                size_t r = 0;

                uint32_t version = 0;
                r = fread(&version, sizeof(uint32_t), 1, file);
                if(r != 1)
                    throw std::runtime_error("Could not read version.");

                //todo: check version

                uint64_t varCount = 0;
                r = fread(&varCount, sizeof(uint64_t), 1, file);
                if(r != 1)
                    throw std::runtime_error("Could not read varCount.");

                for(uint64_t n = 0; n < varCount; n++)
                {

                    uint32_t varSize = 0;
                    r = fread(&varSize, sizeof(uint32_t), 1, file);
                    if(r != 1)
                        throw std::runtime_error("Could not read varSize.");

                    std::string varName;
                    varName.resize(varSize);

                    r = fread((void*)varName.data(), sizeof(char), varSize, file);
                    if(r != varSize)
                        throw std::runtime_error("Could not read varName.");

                    uint32_t valSize = 0;
                    r = fread(&valSize, sizeof(uint32_t), 1, file);
                    if(r != 1)
                        throw std::runtime_error("Could not read valSize.");

                    std::basic_string<uint8_t> varVal;
                    if(valSize != 0)
                    {
                        varVal.resize(valSize);

                        r = fread((void*)varVal.data(), sizeof(uint8_t), valSize, file);
                        if(r != valSize)
                            throw std::runtime_error("Could not read varVal.");
                    }

                    mMetaData[varName] = varVal;
                }

                fclose(file);
            }
        }

        void CStorageLocal::saveMetaData()
        {
            std::string metaDataFn(mBasePath + "metadata");
            FILE* file = fopen(metaDataFn.c_str(), "wb");
            if(file)
            {
                fwrite(&Version, sizeof(uint32_t), 1, file);
                uint64_t varCount = mMetaData.size();
                fwrite(&varCount, sizeof(uint64_t), 1, file);
                if(varCount != 0)
                {
                    for(std::map<std::string, std::basic_string<uint8_t>>::iterator it = mMetaData.begin(); it != mMetaData.end(); ++it)
                    { 
                        std::string varName(it->first);
                        uint32_t varSize = varName.size();
                        if(varSize == 0)
                            continue;
                        fwrite(&varSize, sizeof(uint32_t), 1, file);
                        fwrite(varName.c_str(), sizeof(char), varName.size(), file);
                        std::basic_string<uint8_t> varVal(it->second);
                        uint32_t valSize = varVal.size();
                        fwrite(&valSize, sizeof(uint32_t), 1, file);
                        if(valSize != 0)
                            fwrite(varVal.c_str(), sizeof(uint8_t), varVal.size(), file);
                    }
                }
                fclose(file);
            }
        }

        void CStorageLocal::dispose()
        {
            delete this;
        }
    }
}
