//
// Created by phmoll on 1/27/20.
//

#include "ServerModeSyncClient.h"

void quadtree::ServerModeSyncClient::submitChange(const quadtree::Point& changedPoint)
{
    if (responsibleArea.isPointInRectangle(changedPoint)) {
        world.change(changedPoint.x, changedPoint.y);
        std::cout << "Changed " << changedPoint.x << "," << changedPoint.y << std::endl;
    } else {
        std::cout << "OutOfRange " << changedPoint.x << "," << changedPoint.y << std::endl;
    }
}

void quadtree::ServerModeSyncClient::startSynchronization()
{
    this->publisherThread = std::thread(&ServerModeSyncClient::applyChangesOverTime, this);

    // Todo: Start process which requests changes from remote servers

    // Todo: This method should be blocking -- calling Face.processEvents
    while (this->isRunning) {
        face.processEvents();
    }
}
void quadtree::ServerModeSyncClient::applyChangesOverTime()
{

    auto nextChangePublication = std::chrono::system_clock::now() + 500ms;
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
        std::cout << std::endl;

        std::this_thread::sleep_until(nextChangePublication);
        nextChangePublication += 500ms;
    }

    // Close application when trace ended
    this->isRunning = false;
    this->face.shutdown();
}
