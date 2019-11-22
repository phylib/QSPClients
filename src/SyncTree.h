#include <utility>

#include <utility>

//
// Created by phmoll on 11/15/19.
//

#ifndef QUADTREESYNCEVALUATION_SYNCTREE_H
#define QUADTREESYNCEVALUATION_SYNCTREE_H

#include "QuadTreeStructs.h"

#include <boost/functional/hash.hpp>
#include <vector>

namespace quadtree {

/**
 * A sync tree (either quad or more childs), which does not inflate elements unless they are required.
 */
class SyncTree {

public:
    explicit SyncTree(Rectangle area, int numchilds = 4)
        : area(std::move(area))
        , level(1)
        , parent(nullptr)
        , numChilds(numchilds)
        , currentHash(0)
        , storedChanges(0, std::vector<Chunk*>())
    {
        checkDimensions();
        initChilds();
        reHash(true);
    }

    SyncTree(Rectangle area, SyncTree* parent, unsigned level, int numchilds = 4)
        : area(std::move(area))
        , level(level)
        , parent(parent)
        , numChilds(numchilds)
        , currentHash(0)
        , storedChanges(0, std::vector<Chunk*>())
    {
        checkDimensions();
        initChilds();
        reHash(true);
    }

    ~SyncTree()
    {
        // Delete all child elements
        for (SyncTree* item : childs) {
            delete (item);
            item = nullptr;
        }
        for (Chunk* item : data) {
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
    inline bool finalLevel() { return area.bottomRight.x - area.topleft.x <= 2; }

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
    inline Rectangle getArea() { return area; }

    inline unsigned getLevel() { return level; }

    /**
     * Increases the version of the chunk on position x, y
     * @param x X-Coordinate of the chunk to change
     * @param y Y-Coordinate of the chunk to change
     *
     * @return Pointer to the changed chunk
     */
    Chunk* change(unsigned x, unsigned y);

    /**
     * Returns the current hash of the sync tree node. The hash includes the following fields:
     *
     * - Location covered by the tree (area)
     * - Hash value of all initialized childs (in case of non-leave nodes)
     * - Hash value of the version of all initialized chunks (in case of leave nodes)
     *
     * Note that the hash-value is not updated automatically. The rehash method needs to be called for rehashing
     * the tree.
     *
     * @return The hash value of the tree node
     */
    std::size_t getHash();

    /**
     * Rehashes the sync tree node and all its childs nodes.
     *
     * For performance reasons, it rehash is only performed if chunks of the subtree changed, except the force flag
     * is set.
     *
     * For descriptions about the hash, see the getHash(..) method.
     */
    void reHash(bool force = false);

    /**
     * Returns pointers to changed chunks since the given hash
     *
     * The function returns a tuple, where the first entry is a boolean and indicates of the queried hash value is
     * known. If an too old hash value is queried, the hash value is not known any more and false is returned.
     *
     * Only the most recent state is stored.
     *
     * @param since Hash which is used for querying
     * @return Boolean if query was successful and Pointers to the changed chunks
     */
    std::pair<bool, std::vector<Chunk*>> getChanges(std::size_t since);

    /**
     * Returns pointers to changed chunks in the given region since the given hash.
     *
     * The function returns a tuple, where the first entry is a boolean and indicates of the queried hash value is
     * known. If an too old hash value is queried, the hash value is not known any more and false is returned.
     *
     * Only the most recent state is stored.
     *
     * @param since Hash which is used for querying
     * @param subtree Region to query
     * @return Boolean if query was successful and Pointers to the changed chunks
     */
    std::pair<bool, std::vector<Chunk*>> getChanges(std::size_t since, Rectangle subtree);

    /**
     * Returns a pointer to the subtree which covers the given region. Null, if the subtree is not inflated yet
     *
     * @param rectangle Region to query
     * @return Pointer to the SyncTree covering the given region
     */
    SyncTree* getSubtree(Rectangle rectangle);

private:
    void initChilds();

    static bool isPowerOfTwo(ulong x);

    void checkDimensions();

    static void checkDimensions(const Rectangle& rect);

    Chunk* inflateChunk(unsigned x, unsigned y);

    Chunk* inflateChunk(unsigned x, unsigned y, bool rememberChanged);

protected:
    Rectangle area;
    unsigned level;
    SyncTree* parent;
    unsigned numChilds;

    std::size_t currentHash;
    std::pair<std::size_t, std::vector<Chunk*>> storedChanges;
    std::vector<Chunk*> changedChunks;

    std::vector<SyncTree*> childs;
    std::vector<Chunk*> data;
};
}

#endif // QUADTREESYNCEVALUATION_SYNCTREE_H
