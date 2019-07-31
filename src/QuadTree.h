//
// Created by phmoll on 7/19/19.
//

#ifndef QUADTREESYNCEVALUATION_QUADTREE_H
#define QUADTREESYNCEVALUATION_QUADTREE_H

#include <vector>
#include <string>
#include <math.h>

#include <boost/functional/hash.hpp>
#include <boost/algorithm/string.hpp>

#include "QuadTreeStructs.h"
#include "datastore/HashStorage.h"

const unsigned MAX_LEVEL = 4;


namespace quadtree {

    class QuadTree {

    public:
        QuadTree(Point _topLeft, Point _bottomRight, unsigned _level)
                : area(_topLeft, _bottomRight), level(_level), parent(nullptr) {

            init();
        }

        QuadTree(Point _topLeft, Point _bottomRight, unsigned _level, QuadTree *parent)
                : area(_topLeft, _bottomRight), level(_level), parent(nullptr) {

            init();
        }

        void setHashStorage(HashStorage &_hashStorage);

        Chunk *markChangedChunk(Chunk changedChunk);

        Chunk *getChunk(Point point);

        inline bool isInMaxLevel() {
            unsigned width = area.bottomRight.x - area.topleft.x;
            return level == MAX_LEVEL || width <= 2;
        }

        inline bool isChanged() {
            return !this->changedChunks.empty();
        }

        std::vector<Chunk> getChanges() {
            return changedChunks;
        }

        std::size_t getHash() {
            return this->hash;
        }

        unsigned getLevel() {
            return this->level;
        }

        std::vector<Chunk> enumerateChunks();

        bool isPointInQuadTree(const Point& p);

        QuadTree *getSubTree(const std::string &path, int quadtreeSize);

    public:
        static const std::vector<Point> splitPath(const std::string &path, int quadtreeSize);

        static std::string getPath(const Point &point, int quadtreeSize, unsigned levels = MAX_LEVEL);

    protected:
        void init();

        void initChunks();

        void initChildren();

        void updateHash(size_t newHash);

        Chunk *getChunk(Chunk _chunk, bool change);

        static Point
        getLeftUpperCornerFromPathComponent(const std::string &pathComponent, unsigned level, int quadtreeSize);

    public:
        std::pair<Point, Point> getBounds();

        size_t hashQuadTree(bool force = false);
        // Hold details of the boundary of this node
    protected:
        Rectangle area;
        unsigned level;

        std::size_t hash;
        std::size_t previousHash;

        std::vector<Chunk> chunks;
        std::vector<Chunk> changedChunks;

        // Parent of this tree
        QuadTree *parent;

        HashStorage *hashStorage = nullptr;


    public:
        // Children of this tree
        QuadTree *topLeftTree;
        QuadTree *topRightTree;
        QuadTree *botLeftTree;
        QuadTree *botRightTree;

    };

}

#endif //QUADTREESYNCEVALUATION_QUADTREE_H
