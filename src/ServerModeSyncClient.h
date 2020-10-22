//
// Created by phmoll on 1/27/20.
//

#ifndef QUADTREESYNCEVALUATION_SERVERMODESYNCCLIENT_H
#define QUADTREESYNCEVALUATION_SERVERMODESYNCCLIENT_H

#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <utility>

#include "spdlog/spdlog.h"
#include <ndn-cxx/face.hpp>

#include "SyncTree.h"
#include "src/logging/ChunkLogger.h"
#include "zip/GZip.h"

using namespace std::literals;

namespace quadtree {

/**
 * This class represents a SyncClient for a multi-server scenario. A single sync tree is spanned over the whole world.
 * Every server is managing an area of the same size. Meaning that if 4 servers manage the world, every server is
 * responsible for a subtree of level 2.
 */
class ServerModeSyncClient {

public:
    ServerModeSyncClient(std::string worldPrefix, Rectangle area, Rectangle responsibleArea,
        unsigned initialRequestLevel, std::vector<std::pair<unsigned, std::vector<quadtree::Chunk>>> changesOverTime,
        const std::string& logFolder, const std::string& logFilePrefix, unsigned lowerLevels = 2,
        unsigned chunkThreshold = 200, long syncRequestInterval = 500)
        : worldPrefix(std::move(worldPrefix))
        , world(std::move(area))
        , responsibleArea(std::move(responsibleArea))
        , initialRequestLevel(initialRequestLevel)
        , lowerLevels(lowerLevels)
        , chunkThreshold(chunkThreshold)
        , syncRequestInterval(syncRequestInterval)
        , changesOverTime(std::move(changesOverTime))
        , logFolder(logFolder)
        , logFilePrefix(logFilePrefix)
        , logger(logFolder + logFilePrefix + "_chunklog.csv")
    {

    }

public:
    void submitChange(const Point& changedPoint, unsigned numChanges);

    void startSynchronization();

protected:
    void applyChangesOverTime();

    void synchronizeRemoteRegion(SyncTree* subtree);

    void storeLogValues();

    // NDN Consumer Methods
    void onSubtreeSyncResponseReceived(const ndn::Interest&, const ndn::Data& data);

    void onNack(const ndn::Interest&, const ndn::lp::Nack& nack);

    void onTimeout(const ndn::Interest& interest);

    // NDN Producer Methods
    void onSubtreeSyncRequestReceived(const ndn::InterestFilter&, const ndn::Interest& interest);

    void onRegisterFailed(const ndn::Name& prefix, const std::string& reason);

public:
    const unsigned SLEEP_TIME_MS = 500;

protected:
    std::string worldPrefix;
    SyncTree world;
    Rectangle responsibleArea;
    SyncTree* ownSubtree;
    std::vector<SyncTree*> remoteSyncTrees;
    unsigned initialRequestLevel;
    unsigned lowerLevels;
    unsigned chunkThreshold;
    long syncRequestInterval;

    ndn::Face face;
    ndn::KeyChain keyChain;

    std::vector<std::pair<unsigned, std::vector<quadtree::Chunk>>> changesOverTime;

    std::string logFolder;
    std::string logFilePrefix;
    ChunkLogger logger;

    std::atomic<bool> isRunning { true };
    std::thread publisherThread;
    std::vector<std::thread> consumerthreads;
    std::atomic<unsigned> currentTick { 0 };
    std::mutex treeAccessMutex;
    std::mutex keyChainMutex;

    long last_publish_timestamp;
    std::mutex runtimeMemoryMutex;
    std::unordered_map<std::string, long> received_data_runtimes;

    unsigned long received_chunk_responses = 0;
    unsigned long received_subtree_responses = 0;
    unsigned long received_unknown_hash_responses = 0;
};

}

#endif // QUADTREESYNCEVALUATION_SERVERMODESYNCCLIENT_H
