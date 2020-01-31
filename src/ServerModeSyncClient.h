
#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <utility>

#include <ndn-cxx/face.hpp>

//
// Created by phmoll on 1/27/20.
//

#ifndef QUADTREESYNCEVALUATION_SERVERMODESYNCCLIENT_H
#define QUADTREESYNCEVALUATION_SERVERMODESYNCCLIENT_H

#include "SyncTree.h"

using namespace std::literals;

namespace quadtree {

/**
 * This class represents a SyncClient for a multi-server scenario. A single sync tree is spanned over the whole world.
 * Every server is managing an area of the same size. Meaning that if 4 servers manage the world, every server is
 * responsible for a subtree of level 2.
 */
class ServerModeSyncClient {

public:
    ServerModeSyncClient(Rectangle area, Rectangle responsibleArea, unsigned initialRequestLevel,
        std::vector<std::pair<unsigned, std::vector<quadtree::Chunk>>> changesOverTime)
        : world(std::move(area))
        , responsibleArea(std::move(responsibleArea))
        , initialRequestLevel(initialRequestLevel)
        , changesOverTime(std::move(changesOverTime))
    {

        // Inflate all subtrees on the initial request level
        for (int i = 0; i < pow(4, initialRequestLevel); i++) {
            world.inflateSubtree(initialRequestLevel + 1, i);
        }

        ownSubtree = world.getSubtree(responsibleArea);
        auto requestableTrees = world.enumerateLowerLevel(initialRequestLevel);
        for (const auto& subtree : requestableTrees) {
            if (subtree != nullptr && subtree != ownSubtree) {
                remoteSyncTrees.push_back(subtree);
            }
        }
    }

public:
    void submitChange(const Point& changedPoint);

    void startSynchronization();

protected:
    void applyChangesOverTime();

    void synchronizeRemoteRegion(SyncTree* subtree);

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
    SyncTree world;
    Rectangle responsibleArea;
    SyncTree* ownSubtree;
    std::vector<SyncTree*> remoteSyncTrees;
    unsigned initialRequestLevel;

    ndn::Face face;
    ndn::KeyChain keyChain;

    std::vector<std::pair<unsigned, std::vector<quadtree::Chunk>>> changesOverTime;

    std::atomic<bool> isRunning = true;
    std::thread publisherThread;
    std::vector<std::thread> consumerthreads;
    std::atomic<unsigned> currentTick = 0;
    std::mutex treeAccessMutex;
    std::mutex keyChainMutex;
};

}

#endif // QUADTREESYNCEVALUATION_SERVERMODESYNCCLIENT_H