//
// Created by phmoll on 7/23/19.
//

#include "SyncParticipant.h"

SyncParticipant::SyncParticipant() = default;

void SyncParticipant::setSyncAreas(std::vector<Rectangle> areas) {

    this->syncAreas = areas;

    initSyncTrees();
}

void SyncParticipant::initSyncTrees() {

    // Check if every chunk of all sync areas is covered by quadtrees
    for (const auto &area : syncAreas) {
        for (int i = area.topleft.x; i < area.bottomRight.x; i++) {
            for (int j = area.topleft.y; j < area.bottomRight.y; j++) {

                const Point &point = Point(i, j);
                bool pointInTrees = false;
                for (QuadTree syncTree : syncTrees) {
                    if (syncTree.isPointInQuadTree(point)) {
                        pointInTrees = true;
                        break;
                    }

                }

                // For points not beeing covered, a new quadtree is build
                if (!pointInTrees) {
                    // Init QuadTree
                    int quadTreeX = floor(point.x / (double) QUADTREES_SIZE) * QUADTREES_SIZE;
                    int quadTreeY = floor(point.y / (double) QUADTREES_SIZE) * QUADTREES_SIZE;
                    QuadTree quadTree(Point(quadTreeX, quadTreeY), Point(quadTreeX + 32, quadTreeY + 32), 1);
                    this->syncTrees.insert(this->syncTrees.begin(), quadTree);
                }

            }
        }
    }
}

void SyncParticipant::reHash() {
    for (auto &syncTree : syncTrees) {
        syncTree.hashQuadTree();
    }
}

void SyncParticipant::applyChange(const Point& changedPoint) {

    for (auto &syncTree : syncTrees) {
        if (syncTree.isPointInQuadTree(changedPoint)) {

            Chunk* c = syncTree.getChunk(changedPoint);
            c->data++;
            syncTree.markChangedChunk(*c);

            return;
        }
    }

}

Chunk SyncParticipant::getChunk(const Point &point) {
    for (auto &syncTree : syncTrees) {
        if (syncTree.isPointInQuadTree(point)) {
            return *syncTree.getChunk(point);
        }
    }
}