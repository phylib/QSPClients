//
// Created by phmoll on 11/15/19.
//

#ifndef QUADTREESYNCEVALUATION_SYNCTREE_H
#define QUADTREESYNCEVALUATION_SYNCTREE_H

#include "QuadTreeStructs.h"
#include <vector>

namespace quadtree {

    /**
     * A sync tree (either quad or more childs), which does not inflate elements unless they are required.
     */
    class SyncTree {

    public:
        SyncTree(Rectangle area, int numchilds = 4)
                : area(area), level(1), parent(nullptr), numChilds(numchilds) {
            initChilds();
        }

        SyncTree(Rectangle area, SyncTree *parent, unsigned level, int numchilds = 4)
                : area(area), level(level), parent(parent), numChilds(numchilds) {
            initChilds();
        }

        ~SyncTree() {
            // Delete all child elements
            for (SyncTree *item : childs) {
                delete (item);
                item = nullptr;
            }
            for (Chunk *item : data) {
                delete (item);
                item = nullptr;
            }
            // Delete vector itself
            childs.erase(childs.begin(), childs.end());
            data.erase(data.begin(), data.end());
        }

    public:
        /**
         * Returns true if the width of the tree is 2 or less. No more childs are possible.
         * @return True if it is the final level
         */
        inline bool finalLevel() {
            return area.bottomRight.x - area.topleft.x <= 2;
        }

        /**
         * Traverses the tree and counts the number of inflated nodes (including parent and child chunks)
         *
         * @return Number of inflated nodes
         */
        unsigned countInflatedNodes();

        /**
         * Returns the area covered by the tree
         *
         * @return area covered by the tree
         */
        inline Rectangle getArea() {
            return area;
        }

        inline unsigned getLevel() {
            return level;
        }

        /**
         * Increases the version of the chunk on position x, y
         * @param x X-Coordinate of the chunk to change
         * @param y Y-Coordinate of the chunk to change
         *
         * @return Pointer to the changed chunk
         */
        Chunk *change(unsigned x, unsigned y);

    private:
        void initChilds();

        Chunk* inflateChunk(unsigned x, unsigned y);

    protected:
        Rectangle area;
        unsigned level;
        SyncTree *parent;
        unsigned numChilds;

        std::vector<SyncTree *> childs;
        std::vector<Chunk *> data;
    };
}


#endif //QUADTREESYNCEVALUATION_SYNCTREE_H
