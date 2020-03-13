//
// Created by phmoll on 3/5/20.
//

#ifndef QUADTREESYNCEVALUATION_STATEVECTORSYNCCLIENT_H
#define QUADTREESYNCEVALUATION_STATEVECTORSYNCCLIENT_H

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <mutex>
#include <thread>

#include "spdlog/spdlog.h"
#include <ndn-cxx/face.hpp>
#include <src/svs/svs.hpp>

#include "QuadTreeStructs.h"
#include "csv/CSVReader.h"
#include "csv/ChunkFileReader.h"
#include "logging/ChunkLogger.h"
#include "proto/ChunkChanges.pb.h"
#include "zip/GZip.h"

namespace po = boost::program_options;

namespace quadtree {

class StateVectorSyncClient {

public:
    StateVectorSyncClient(Rectangle area, Rectangle responsibleArea, ndn::svs::NodeID nodeId,
        std::vector<std::pair<unsigned, std::vector<quadtree::Chunk>>> changesOverTime, const std::string& logFolder,
        const std::string& logFilePrefix)
        : world(std::move(area))
        , responsibleArea(std::move(responsibleArea))
        , nodeId(nodeId)
        , svs(nodeId, std::bind(&StateVectorSyncClient::onSyncResponseReceived, this, std::placeholders::_1, std::placeholders::_2))
        , changesOverTime(std::move(changesOverTime))
        , logFolder(logFolder)
        , logFilePrefix(logFilePrefix)
        , logger(logFolder + "/" + logFilePrefix + "_chunklog.csv")
    {

        spdlog::info("Initialize StateVectorSyncClient");
    }

public:
    void submitChange(const std::vector<Point>& changedPoints);

    void startSynchronization();

protected:
    void applyChangesOverTime();

    void storeLogValues();

    // NDN Consumer Methods
    void onSyncResponseReceived(const ndn::svs::NodeID& senderNodeId, const std::string& msg);

public:
    const unsigned SLEEP_TIME_MS = 500;

protected:
    Rectangle world;
    Rectangle responsibleArea;

    ndn::svs::NodeID nodeId;
    ndn::svs::SVS svs;

    std::vector<std::pair<unsigned, std::vector<quadtree::Chunk>>> changesOverTime;
    std::map<Point, unsigned> knownPoints;

    std::string logFolder;
    std::string logFilePrefix;
    ChunkLogger logger;

    std::atomic<bool> isRunning { true };
    std::thread publisherThread;
    std::atomic<unsigned> currentTick { 0 };
    std::mutex localDataAccessMutex;
    std::mutex keyChainMutex;
};
}

#endif // QUADTREESYNCEVALUATION_STATEVECTORSYNCCLIENT_H
