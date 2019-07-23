//
// Created by phmoll on 7/23/19.
//

#ifndef QUADTREESYNCEVALUATION_SYNCPARTICIPANT_H
#define QUADTREESYNCEVALUATION_SYNCPARTICIPANT_H

#include <vector>

#include "QuadTree.h"

class SyncParticipant {

public:
    static const int QUADTREES_SIZE = 32;

public:
    SyncParticipant();

public:
    /**
     * Allows to update the sync areas of a SyncParticipant. Also updates the corresponding sync trees.
     *
     * @param areas New Sync areas of the sync participant
     */
    void setSyncAreas(std::vector<Rectangle> areas);

    /**
     * Generates a new version of the hash trees.
     */
    void reHash();

    void applyChange(const Point& changedPoint);

    Chunk getChunk(const Point& point);


public: // Getter and Setter

    std::vector<QuadTree>& getSyncTrees() {
        return syncTrees;
    }

private:
    void initSyncTrees();

private:
    std::vector<Rectangle> syncAreas;
    std::vector<QuadTree> syncTrees;
};


#endif //QUADTREESYNCEVALUATION_SYNCPARTICIPANT_H
