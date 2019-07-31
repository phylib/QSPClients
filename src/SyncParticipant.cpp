//
// Created by phmoll on 7/23/19.
//

#include "SyncParticipant.h"

namespace quadtree {

    SyncParticipant::SyncParticipant() = default;

    void SyncParticipant::setSyncAreas(std::vector<Rectangle> areas) {

        this->syncAreas = areas;

        initSyncTrees();
    }

    void SyncParticipant::initSyncTrees() {

        // Check if every chunk of all sync areas is covered by quadtrees
        for (const auto &area : syncAreas) {
            initSyncTreeForRectangle(area);
        }
    }

    void SyncParticipant::initSyncTreeForRectangle(const Rectangle &area) {
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
                    int quadTreeX = floor(point.x / (double) QUADTREE_SIZE) * QUADTREE_SIZE;
                    int quadTreeY = floor(point.y / (double) QUADTREE_SIZE) * QUADTREE_SIZE;
                    QuadTree quadTree(Point(quadTreeX, quadTreeY), Point(quadTreeX + 32, quadTreeY + 32), 1);
                    quadTree.setHashStorage(hashStorage);
                    syncTrees.insert(syncTrees.begin(), quadTree);
                }

            }
        }
    }

    void SyncParticipant::reHash() {
        for (auto &syncTree : syncTrees) {
            syncTree.hashQuadTree();
        }
    }

    void SyncParticipant::applyChange(const Point &changedPoint) {

        for (auto &syncTree : syncTrees) {
            if (syncTree.isPointInQuadTree(changedPoint)) {

                Chunk *c = syncTree.getChunk(changedPoint);
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

    ChangeResponse SyncParticipant::getChanges(const std::string &path, const size_t &knownHash) {

        std::vector<Point> points = QuadTree::splitPath(path, QUADTREE_SIZE * 2);
        Point p = *points.begin();

        // Shorten pointvector for use in quadtrees
        points.erase(points.begin());
        std::string quadTreePath = QuadTree::getPath(points.back(), QUADTREE_SIZE, points.size());

        for (QuadTree &syncTree : syncTrees) {
            if (syncTree.isPointInQuadTree(p)) {

                QuadTree *requestedChangeTree = syncTree.getSubTree(quadTreePath, QUADTREE_SIZE);

                if (requestedChangeTree->getHash() == knownHash) { // No change since knownHash
                    // Return an empty ChangeResponse
                    return ChangeResponse(path, 0);

                } else {
                    // knownHash is not the current hash any more.
                    ChangeResponse cr(path, requestedChangeTree->getHash());

                    if (hashStorage.exists(knownHash)) { // Changes since known hash are still stored
                        cr.changeVector = hashStorage.get(knownHash).second;
                        cr.delta = true;
                    } else { // Change vector is lost, Encode full subtree
                        cr.changeVector = requestedChangeTree->enumerateChunks();
                        cr.delta = false;
                    }

                    return cr;
                }
            }
        }
        //const std::vector<const std::string> pathComponents = QuadTree::splitPath(path);

        return ChangeResponse();
    }

    const std::vector<RemoteSyncArea> &SyncParticipant::getRemoteSyncAreas() const {
        return remoteSyncAreas;
    }

    void SyncParticipant::setRemoteSyncAreas(const std::vector<RemoteSyncArea> &remoteSyncAreas) {
        this->remoteSyncAreas = remoteSyncAreas;

        initSyncTrees();
    }

}
