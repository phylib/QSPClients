#include <utility>

//
// Created by phmoll on 11/15/19.
//

#ifndef QUADTREESYNCEVALUATION_SYNCTREE_H
#define QUADTREESYNCEVALUATION_SYNCTREE_H

#include "QuadTreeStructs.h"
#include "proto/SyncResponse.pb.h"

#include <boost/functional/hash.hpp>
#include <map>
#include <math.h>
#include <regex>
#include <vector>

#include <ndn-cxx/name.hpp>

namespace quadtree {

typedef std::pair<std::map<unsigned, std::vector<size_t>>, int> NextNLevelsResponseType;
typedef std::pair<bool, std::vector<Chunk*>> ChangeResponseType;

struct SyncRequestResponse {
    bool containsChanges;
    NextNLevelsResponseType nextNLevelsResponse;
    ChangeResponseType changeReponse;
};

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

    unsigned getMaxLevel();

    /**
     * Traverses the tree and counts the number of inflated nodes (including parent and child chunks)
     *
     * @return Number of inflated nodes
     */
    unsigned countInflatedNodes();

    /**
     * Traverses the tree and counts the number of inflated chunks (Excludes all non-leaf nodes, only chunks)
     * @return Number of inclated chunks
     */
    unsigned countInflatedChunks();

    std::map<unsigned, unsigned> countInflatedSubtreesPerLevel();

    /**
     * Returns the area covered by the tree
     *
     * @return area covered by the tree
     */
    inline Rectangle getArea() { return area; }

    inline unsigned getLevel() { return level; }

    /**
     * Increases the version of the chunk on position x, y
     *
     * @param x X-Coordinate of the chunk to change
     * @param y Y-Coordinate of the chunk to change
     *
     * @return Pointer to the changed chunk
     */
    Chunk* change(unsigned x, unsigned y);

    /**
     * Sets the version of the chunk on position x, y to a certain value
     *
     * @param x X-Coordinate of the chunk to change
     * @param y Y-Coordinate of the chunk to change
     * @param version New version of the chunk on position x,y
     *
     * @return Pointer to the changed chunk
     */
    Chunk* change(unsigned x, unsigned y, unsigned version);

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
    std::size_t getHash() const;

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

    /**
     * Returns the path to a given chunk, based on the current tree node. The path of a chunk consists of several
     * path components, where each component represents one level of a tree. Each component is a number defining
     * the quadrant of the tree where the chunk is located.
     *
     * Example:
     * For a 8x8 tree, the name of the field 3,3 is the following:
     * /0/3/3, where 0 stands for the area 0,0-3,3; the first number 3 for the area 2,2-3,3 and the second number 3
     * for the chunk 3,3.
     *
     * @param x X-Coordinate of the chunk
     * @param y Y-Coordinate of the chunk
     * @return The name of the chunk
     */
    std::vector<unsigned char> getChunkPath(unsigned x, unsigned y);

    /**
     * Returns the hash values of the nextNLevels and the number of changes since the provided hash value, iff the
     * provided hash-value is known. If not, -1 is returned as number of changes.
     * @param nextNLevels Number of levels where hash entries are returned
     * @param since Known hash value of the subtree
     * @return Hash values of the subtrees and the number of changes since the provided hash value
     */
    NextNLevelsResponseType hashValuesOfNextNLevels(unsigned nextNLevels, size_t since);

    /**
     * Represents a sync request to the quad tree. First it is queried how many changes in the current subtree
     * where made since the given hash. If the number of changes lies below the given threshold, the changes are
     * returned. If the hash is not known, or too many changes were found, the hash-values of the next-n-values are
     * returned
     *
     * @param since Hash value since when changes are queried
     * @param nextNLevels If too many changes are found, of how many levels deeper shall hash values be returned?
     * @param threshold Number of chunks allowed in the response.
     * @return
     */
    SyncRequestResponse syncRequest(size_t since, unsigned nextNLevels, unsigned threshold);

    /**
     * Enumerate the tree level which lies N levels under the current node
     * @param n Specifies how many levels under the current node shall be traversed
     * @return Vector with pointers to tree nodes
     */
    std::vector<SyncTree*> enumerateLowerLevel(unsigned n);

    /**
     * Inflate a given subtree of the current synctree. This is neccesary, when the current tree is compared to the
     * hashes of another tree
     * @param level Defines the level until where the subtree needs to be inflated
     * @param subtreeIndex Index of the subtree in the given level
     */
    SyncTree* inflateSubtree(unsigned int level, int subtreeIndex);

    /**
     * This method applies a sync response packet to the quadtree.
     *
     * When the sync response contains chunk changes, the chunk changes are applied. If the sync response contains
     * hash values of lower level subtrees, than the hash values are compared.
     *
     * When the quadtree is up to date according to the given SyncResponse, true is returned. Otherwise, a list of
     * out of sync subtrees is returned.
     *
     * @param syncResponse Received Sync Response packet
     * @return (inSync: bool, subtreesToSync: List of synctrees)
     */
    std::pair<bool, std::vector<SyncTree*>> applySyncResponse(const SyncResponse& syncResponse);

    /**
     * Prepares a SyncResponse for the given hash value
     * @param hashValue Given hash value
     * @param lowerLevels Number of lower levels for subtree hashes
     * @param chunkThreshold If more than chunkThreshold chunks changed, a response containing lower chunk levels
     * is sent
     * @return SyncResponse for request with given hashValue
     */
    SyncResponse prepareSyncResponse(size_t hashValue, unsigned lowerLevels, unsigned chunkThreshold);

    /**
     * Returns a name representing the path to the current subtree. When includeSubtreeHash is true, then a name
     * component representing the hash is included.
     * @param includeSubtreeHash Include name components representing the current hash of the subtree
     * @return The path to the subtree as ndn::Name
     */
    ndn::Name subtreeToName(bool includeSubtreeHash = false) const;

    /**
     * Returns the subtree of the synctree, which is identified by the given NDN name. If the subtree is not inflated
     * yet, an exception is thrown.
     * @param subtreeName ndn::Name of the subtree to return
     * @return The corresponding subtree
     */
    SyncTree* getSubtreeFromName(const ndn::Name& subtreeName) const;

    std::vector<SyncTree*> getNeighboursForRectangle(const Rectangle requestedArea);

    std::vector<SyncTree*> getTreeCoverageBasedOnRectangle(const Rectangle requestedArea, unsigned maxLevel);

protected:
    void initChilds();

    static bool isPowerOfTwo(ulong x);

    void checkDimensions();

    static void checkDimensions(const Rectangle& rect);

    bool isRectInTree(const Rectangle& rect) const;

    Chunk* inflateChunk(unsigned x, unsigned y);

    Chunk* inflateChunk(unsigned x, unsigned y, bool rememberChanged);

    std::vector<SyncTree*> getTreeCoverageBasedOnRectangleRecursive(const Rectangle requestedArea, unsigned maxLevel, std::vector<SyncTree*> currentNeighbours);

    /**
     * Returns the hash-values of N-levels of the SyncTree. For implementation, the method calls itself iteratively.
     *
     * One characteristic of the method is that every level of the tree is fully added to the hashMap. Otherwise,
     * a mapping between tree-node and hash-value can not be created any more.
     *
     * @param nextNLevels Number of levels where hash entries are returned.
     * @param hashValues The hash-map containing the syncTree's hash-values
     * @return The hash-map containing the syncTree's hash-values
     */
    std::map<unsigned, std::vector<size_t>> hashValuesOfNextNLevels(unsigned nextNLevels,
        std::map<unsigned, std::vector<size_t>> hashValues = std::map<unsigned, std::vector<size_t>>());

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
