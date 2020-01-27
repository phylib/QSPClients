//
// Created by phmoll on 1/27/20.
//

#include "ServerModeSyncClient.h"

void quadtree::ServerModeSyncClient::submitChange(const quadtree::Point& changedPoint)
{
    if (responsibleArea.isPointInRectangle(changedPoint)) {
        world.change(changedPoint.x, changedPoint.y);
    }
}

void quadtree::ServerModeSyncClient::startSynchronization()
{
    // Todo: Start process which publishes manifest in regular intervals

    // Todo: Start process which requests changes from remote servers

    // Todo: This method should be blocking -- calling Face.processEvents
}
