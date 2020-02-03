//
// Created by phmoll on 1/27/20.
//

#include "ServerModeSyncClient.h"

void quadtree::ServerModeSyncClient::submitChange(const quadtree::Point& changedPoint)
{
    if (responsibleArea.isPointInRectangle(changedPoint)) {
        const Chunk& chunk(*world.change(changedPoint.x, changedPoint.y));
        logger.logChunkUpdateProduced(chunk);
        //        std::cout << "Changed " << changedPoint.x << "," << changedPoint.y << std::endl;
    }
    //    } else {
    //        std::cout << "OutOfRange " << changedPoint.x << "," << changedPoint.y << std::endl;
    //    }
}

void quadtree::ServerModeSyncClient::startSynchronization()
{
    this->publisherThread = std::thread(&ServerModeSyncClient::applyChangesOverTime, this);

    // Start process which requests changes from remote servers
    for (SyncTree* remoteRegion : this->remoteSyncTrees) {
        std::thread consumerThread = std::thread(&ServerModeSyncClient::synchronizeRemoteRegion, this, remoteRegion);
        this->consumerthreads.push_back(std::move(consumerThread));
    }

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

    // Close application when trace ended
    this->isRunning = false;
    this->face.shutdown();
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
        subtreeRequest.setCanBePrefix(false);
        subtreeRequest.setInterestLifetime(boost::chrono::milliseconds(ServerModeSyncClient::SLEEP_TIME_MS));

        std::cout << "Express Interest for " << subtreeRequestName.toUri() << std::endl;
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
    // Todo: Verify signature

    // Todo: Decrypt packet

    // unzip und unserialize data
    char* rawData = (char*)data.getContent().value();
    std::string receivedData = std::string(rawData, data.getContent().value_size());
    std::string decompressed = GZip::decompress(receivedData);
    quadtree::SyncResponse response;
    response.ParseFromString(decompressed);

    if (response.chunkdata()) {
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
    }

    {
        std::unique_lock<std::mutex> lck(this->treeAccessMutex);
        // Todo get subtree corresonding to ndn::Name
        SyncTree* subtree = world.getSubtreeFromName(data.getFullName());
        subtree->applySyncResponse(response);
    }

    // Todo: Decode changes and apply to sync tree
    std::cout << "Received Data " << data << std::endl;
}

void quadtree::ServerModeSyncClient::onNack(const ndn::Interest& interest, const ndn::lp::Nack& nack)
{
    // Todo: How to handle NACKs?

    std::cout << "Received Nack with reason " << nack.getReason() << " for " << interest << std::endl;
}

void quadtree::ServerModeSyncClient::onTimeout(const ndn::Interest& interest)
{
    // Todo: How to handle Timeouts?

    std::cout << "Timeout for " << interest << std::endl;
}

void quadtree::ServerModeSyncClient::onSubtreeSyncRequestReceived(
    const ndn::InterestFilter&, const ndn::Interest& interest)
{
    // Todo: Decode request and build data packet
}

void quadtree::ServerModeSyncClient::onRegisterFailed(const ndn::Name& prefix, const std::string& reason)
{
    std::cerr << "ERROR: Failed to register prefix '" << prefix << "' with the local forwarder (" << reason << ")"
              << std::endl;
    this->face.shutdown();
}
