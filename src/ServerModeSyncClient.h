#include <utility>

//
// Created by phmoll on 1/27/20.
//

#ifndef QUADTREESYNCEVALUATION_SERVERMODESYNCCLIENT_H
#define QUADTREESYNCEVALUATION_SERVERMODESYNCCLIENT_H

#include "SyncTree.h"

namespace quadtree {

/**
 * This class represents a SyncClient for a multi-server scenario. A single sync tree is spanned over the whole world.
 * Every server is managing an area of the same size. Meaning that if 4 servers manage the world, every server is
 * responsible for a subtree of level 2.
 */
class ServerModeSyncClient {

public:
    ServerModeSyncClient(Rectangle area, Rectangle responsibleArea, unsigned initialRequestLevel)
        : world(std::move(area))
        , responsibleArea(std::move(responsibleArea))
        , initialRequestLevel(initialRequestLevel)
    {
    }

public:
    void submitChange(const Point& changedPoint);

    void startSynchronization();


protected:
    SyncTree world;
    Rectangle responsibleArea;
    unsigned initialRequestLevel;
};

}

#endif // QUADTREESYNCEVALUATION_SERVERMODESYNCCLIENT_H
