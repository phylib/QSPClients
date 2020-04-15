//
// Created by phmoll on 1/27/20.
//

#ifndef QUADTREESYNCEVALUATION_P2PMODESYNCCLIENT_H
#define QUADTREESYNCEVALUATION_P2PMODESYNCCLIENT_H

#include <atomic>
#include <boost/program_options.hpp>
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <utility>

#include "spdlog/spdlog.h"
#include <ndn-cxx/face.hpp>

#include "ServerModeSyncClient.h"
#include "SyncTree.h"
#include "csv/ChunkFileReader.h"
#include "src/logging/ChunkLogger.h"
#include "zip/GZip.h"

namespace po = boost::program_options;

using namespace std::literals;

namespace quadtree {

/**
 * This class represents a SyncClient for a multi-server scenario. A single sync tree is spanned over the whole world.
 * Every server is managing an area of the same size. Meaning that if 4 servers manage the world, every server is
 * responsible for a subtree of level 2.
 */
class P2PModeSyncClient : public ServerModeSyncClient {

public:
    P2PModeSyncClient(std::string worldPrefix, Rectangle area, Rectangle responsibleArea, unsigned initialRequestLevel,
        std::vector<std::pair<unsigned, std::vector<quadtree::Chunk>>> changesOverTime, const std::string& logFolder,
        const std::string& logFilePrefix, unsigned lowerLevels = 2, unsigned chunkThreshold = 200)
        : ServerModeSyncClient(worldPrefix, area, responsibleArea, initialRequestLevel,
            changesOverTime, logFolder, logFilePrefix, lowerLevels, chunkThreshold)
    {
    }

public:
    void startSynchronization();

protected:
    void storeLogValues();

public:

protected:
};

}

#endif // QUADTREESYNCEVALUATION_SERVERMODESYNCCLIENT_H
