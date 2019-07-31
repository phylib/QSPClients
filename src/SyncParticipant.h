//
// Created by phmoll on 7/23/19.
//

#ifndef QUADTREESYNCEVALUATION_SYNCPARTICIPANT_H
#define QUADTREESYNCEVALUATION_SYNCPARTICIPANT_H

#include <vector>
#include <iostream>

#include "QuadTree.h"


namespace quadtree {

    /**
     * A remote sync area consists of a string identifying the server hosting it and a list of rectangles.
     */
    typedef std::tuple<std::string, std::vector<Rectangle>> RemoteSyncArea;

/**
 * A SyncParticipant is a player managing one or multiple areas of the game world.
 *
 * When using this sync variant, the whole world is divided into QuadTrees of size <code>QUADTREE_SIZE</code>. The
 * origin of the world is chunk (0,0). This chunk is the upper left element of the Quadtree with the name prefix /0,0/,
 * where the name component is build of /{x},{y}/ coordinates.
 *
 * Each level of the quadtrees adds an additional name component to the name. So for instance, the chunk (10, 63) is
 * represented by the following name: /0,1/0,3/1,7/2,15/5/31/10,63/, where all x,y coordinates are absolute coordinates
 * referring to the origin (0,0) and not relative in the quad tree.
 */
    class SyncParticipant {

    public:
        static const int QUADTREE_SIZE = 32;

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
         * Get the remote sync areas observed by the current sync client
         * @return
         */
        const std::vector<RemoteSyncArea> &getRemoteSyncAreas() const;

        void setRemoteSyncAreas(const std::vector<RemoteSyncArea> &remoteSyncAreas);

        /**
         * Generates a new version of the hash trees.
         */
        void reHash();

        void applyChange(const Point &changedPoint);

        Chunk getChunk(const Point &point);

        ChangeResponse getChanges(const std::string &path, const size_t &knownHash);


    public: // Getter and Setter

        std::vector<QuadTree> &getSyncTrees() {
            return syncTrees;
        }

    private:
        void initSyncTrees();

        void initSyncTreeForRectangle(const Rectangle &area);

    private:
        std::vector<Rectangle> syncAreas;
        std::vector<QuadTree> syncTrees;
        HashStorage hashStorage;

        std::vector<RemoteSyncArea> remoteSyncAreas;
    };

}

#endif //QUADTREESYNCEVALUATION_SYNCPARTICIPANT_H
