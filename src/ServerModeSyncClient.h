
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
    }

public:
    void submitChange(const Point& changedPoint);

    void startSynchronization();

protected:
    void applyChangesOverTime();

protected:
    SyncTree world;
    Rectangle responsibleArea;
    unsigned initialRequestLevel;

    ndn::Face face;

    std::vector<std::pair<unsigned, std::vector<quadtree::Chunk>>> changesOverTime;

    std::atomic<bool> isRunning = true;
    std::thread publisherThread;
    std::atomic<unsigned> currentTick = 0;
    std::mutex treeAccessMutex;
};

}

#endif // QUADTREESYNCEVALUATION_SERVERMODESYNCCLIENT_H
