//
// Created by phmoll on 1/27/20.
//

#include "ServerModeSyncClient.h"

void quadtree::ServerModeSyncClient::submitChange(const quadtree::Point& changedPoint)
{
    if (responsibleArea.isPointInRectangle(changedPoint)) {
        const Chunk& chunk(*world.change(changedPoint.x, changedPoint.y));
        logger.logChunkUpdateProduced(chunk);
    }
}

void quadtree::ServerModeSyncClient::startSynchronization()
{

    // This thread applies changes from CSV file, NOT the producer
    this->publisherThread = std::thread(&ServerModeSyncClient::applyChangesOverTime, this);

    // Producer listening to requests from subtree
    ndn::Name ownRegionName(worldPrefix);
    ndn::Name subtreeName = this->ownSubtree->subtreeToName();
    ownRegionName.append(subtreeName);
    this->face.setInterestFilter(ownRegionName,
        std::bind(&ServerModeSyncClient::onSubtreeSyncRequestReceived, this, _1, _2),
        ndn::RegisterPrefixSuccessCallback(), std::bind(&ServerModeSyncClient::onRegisterFailed, this, _1, _2));

    // Start process which requests changes from remote servers
    for (SyncTree* remoteRegion : this->remoteSyncTrees) {
        std::thread consumerThread = std::thread(&ServerModeSyncClient::synchronizeRemoteRegion, this, remoteRegion);
        this->consumerthreads.push_back(std::move(consumerThread));
    }

    spdlog::info("ServerModeSyncClient Started -- Zone:" + ownRegionName.toUri());

    // This method should be blocking -- calling Face.processEvents
    while (this->isRunning) {
        this->face.processEvents();
        usleep(1000);
    }
}
void quadtree::ServerModeSyncClient::applyChangesOverTime()
{

    auto nextChangePublication
        = std::chrono::system_clock::now() + std::chrono::milliseconds(ServerModeSyncClient::SLEEP_TIME_MS);
    for (unsigned i = 0; i < this->changesOverTime.size(); i++) {
        auto tickChunkPair = this->changesOverTime[i];
        this->currentTick = tickChunkPair.first;
        spdlog::info("Apply changes of tick " + std::to_string(this->currentTick));

        {
            std::unique_lock<std::mutex> lck(this->treeAccessMutex);

            for (const auto& chunk : tickChunkPair.second) {
                this->submitChange(chunk.pos);
            }
            this->world.reHash();
        }

        std::this_thread::sleep_until(nextChangePublication);
        nextChangePublication += std::chrono::milliseconds(ServerModeSyncClient::SLEEP_TIME_MS);
    }
    spdlog::info("All changes applied, killing application");

    storeLogValues();

    // Close application when trace ended
    this->isRunning = false;
    usleep(1000);
    this->face.shutdown();
    exit(0);
}

void quadtree::ServerModeSyncClient::synchronizeRemoteRegion(quadtree::SyncTree* subtree)
{

    auto nextRequest
        = std::chrono::system_clock::now() + std::chrono::milliseconds(ServerModeSyncClient::SLEEP_TIME_MS);
    while (this->isRunning) {

        // Construct name and issue Interest
        ndn::Name subtreeName;
        {
            std::unique_lock<std::mutex> lck(this->treeAccessMutex);
            subtreeName = subtree->subtreeToName(true);
        }
        ndn::Name subtreeRequestName(worldPrefix);
        subtreeRequestName.append(subtreeName);

        ndn::Interest subtreeRequest(subtreeRequestName);
        subtreeRequest.setMustBeFresh(true);
        //        subtreeRequest.setCanBePrefix(false);
        subtreeRequest.setInterestLifetime(boost::chrono::milliseconds(ServerModeSyncClient::SLEEP_TIME_MS));

        spdlog::debug("Express Interest for " + subtreeRequestName.toUri());
        this->face.expressInterest(subtreeRequest,
            std::bind(&ServerModeSyncClient::onSubtreeSyncResponseReceived, this, _1, _2),
            std::bind(&ServerModeSyncClient::onNack, this, _1, _2),
            std::bind(&ServerModeSyncClient::onTimeout, this, _1));

        std::this_thread::sleep_until(nextRequest);
        nextRequest += std::chrono::milliseconds(ServerModeSyncClient::SLEEP_TIME_MS);
    }
}

void quadtree::ServerModeSyncClient::onSubtreeSyncResponseReceived(const ndn::Interest& interest, const ndn::Data& data)
{

    // Todo: Log how many subtree responses, chunk responses and hash unknown responses were received

    spdlog::debug("Received sync update: " + interest.getName().toUri());
    // Todo: Verify signature

    // Todo: Decrypt packet

    // unzip und unserialize data
    char* rawData = (char*)data.getContent().value();
    std::string receivedData = std::string(rawData, data.getContent().value_size());
    std::string decompressed = GZip::decompress(receivedData);
    quadtree::SyncResponse response;
    response.ParseFromString(decompressed);

    if (response.chunkdata()) {
        spdlog::trace("Sync Update contains chunk data");
        received_chunk_responses++;
        // If the response contains chunks, log when the change arrived
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        long millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        for (const auto& chunk : response.chunks()) {
            Chunk c;
            c.pos.x = chunk.x();
            c.pos.y = chunk.y();
            c.data = chunk.data();
            logger.logChunkUpdateReceived(c, millis);
        }
    } else {
        spdlog::trace("Sync Update contains subtree hashes. HashKnown=" + std::to_string(response.hashknown()));
        received_subtree_responses++;
        if (!response.has_hashknown()) {
            received_unknown_hash_responses++;
        }
    }

    // Apply the sync response to the local sync tree
    std::pair<bool, std::vector<SyncTree*>> applyResult;
    {
        std::unique_lock<std::mutex> lck(this->treeAccessMutex);
        SyncTree* subtree = world.getSubtreeFromName(data.getFullName());
        applyResult = subtree->applySyncResponse(response);
    }

    // If subtrees need to be fetched, issue Interests for Subtrees
    if (!applyResult.second.empty()) {

        spdlog::trace(std::to_string(applyResult.second.size()) + " subtreerequests required");
        for (SyncTree* subtree : applyResult.second) {
            ndn::Name subtreeRequestName = ndn::Name(worldPrefix);
            {
                std::unique_lock<std::mutex> lck(this->treeAccessMutex);
                ndn::Name subtreeName(subtree->subtreeToName(true));
                subtreeRequestName.append(subtreeName);
            }
            ndn::Interest subtreeRequest(subtreeRequestName);
            subtreeRequest.setMustBeFresh(true);
            //        subtreeRequest.setCanBePrefix(false);
            subtreeRequest.setInterestLifetime(boost::chrono::milliseconds(ServerModeSyncClient::SLEEP_TIME_MS));

            spdlog::trace("Express Interest for Subtreerequest " + subtreeRequestName.toUri());
            this->face.expressInterest(subtreeRequest,
                std::bind(&ServerModeSyncClient::onSubtreeSyncResponseReceived, this, _1, _2),
                std::bind(&ServerModeSyncClient::onNack, this, _1, _2),
                std::bind(&ServerModeSyncClient::onTimeout, this, _1));
        }

    } else if (applyResult.first) { // Else, the tree is in sync
        spdlog::trace("Subtree " + interest.getName().toUri() + " is in sync");
    } else {
        spdlog::error("Subtree " + interest.getName().toUri() + " is not in sync after chunkUpdate, eliminate");
        exit(-1);
    }
}

void quadtree::ServerModeSyncClient::onNack(const ndn::Interest& interest, const ndn::lp::Nack& nack)
{
    // Todo: How to handle NACKs?

    std::string nackReason;
    switch (nack.getReason()) {
    case ndn::lp::NackReason::CONGESTION:
        nackReason = "CONGESTION";
        break;
    case ndn::lp::NackReason::NONE:
        nackReason = "NONE";
        break;
    case ndn::lp::NackReason::DUPLICATE:
        nackReason = "DUPLICATE";
        break;
    case ndn::lp::NackReason::NO_ROUTE:
        nackReason = "NO_ROUTE";
        break;
    }

    spdlog::debug("Received Nack for " + interest.getName().toUri() + " with reason " + nackReason);
}

void quadtree::ServerModeSyncClient::onTimeout(const ndn::Interest& interest)
{
    // Todo: How to handle Timeouts?

    spdlog::debug("Timeout for " + interest.getName().toUri());
}

void quadtree::ServerModeSyncClient::onSubtreeSyncRequestReceived(
    const ndn::InterestFilter&, const ndn::Interest& interest)
{
    spdlog::debug("Received Interest " + interest.getName().toUri());
    const ndn::Name& subtreeName(interest.getName());
    SyncTree* syncTree = world.getSubtreeFromName(subtreeName);

    size_t hash = 0;
    if (subtreeName.get(subtreeName.size() - 2).toUri() == "h") {
        hash = subtreeName.get(subtreeName.size() - 1).toNumber();
    }
    const SyncResponse& syncResponse = syncTree->prepareSyncResponse(hash, this->lowerLevels, this->chunkThreshold);
    std::string plain = syncResponse.SerializeAsString();
    std::string compressed = GZip::compress(plain);

    // Todo: Encrypt response

    // Create Data packet
    //    auto data = std::make_shared<ndn::Data>(interest.getName());
    ndn::Data data = ndn::Data();
    data.setName(subtreeName);
    data.setFreshnessPeriod(boost::chrono::milliseconds(ServerModeSyncClient::SLEEP_TIME_MS));
    data.setContent(reinterpret_cast<const uint8_t*>(compressed.data()), compressed.size());

    // Todo: Sign response with proper cert
    {
        std::unique_lock<std::mutex> lck(this->keyChainMutex);
        this->keyChain.sign(data);
    }
    face.put(data);
}

void quadtree::ServerModeSyncClient::onRegisterFailed(const ndn::Name& prefix, const std::string& reason)
{
    spdlog::error("ERROR: Failed to register prefix '" + prefix.toUri() + "' with the local forwarder");
    this->face.shutdown();
}
void quadtree::ServerModeSyncClient::storeLogValues()
{
    std::ofstream logfile = std::ofstream(logFolder + logFilePrefix + "_stats.txt");
    logfile << "received_chunk_responses: " << received_chunk_responses << std::endl;
    logfile << "received_subtree_responses: " << received_subtree_responses << std::endl;
    logfile << "received_unknown_hash_responses: " << received_unknown_hash_responses << std::endl;
    logfile.flush();
    logfile.close();
}
