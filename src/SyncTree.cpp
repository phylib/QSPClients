//
// Created by phmoll on 11/15/19.
//

#include "SyncTree.h"

namespace quadtree {

unsigned SyncTree::countInflatedNodes()
{
    unsigned numInflatedChildren = 1;

    for (SyncTree* child : childs) {
        if (child != nullptr) {
            numInflatedChildren += child->countInflatedNodes();
        }
    }

    for (Chunk* chunk : data) {
        if (chunk != nullptr) {
            numInflatedChildren++;
        }
    }

    return numInflatedChildren;
}

void SyncTree::initChilds()
{
    if (!finalLevel()) {
        for (unsigned i = 0; i < numChilds; i++) {
            childs.push_back(nullptr);
        }
    } else {
        // Todo: Adapt for larger number of childs, currently, assumption QuadTree
        for (unsigned i = 0; i < numChilds; i++) {
            data.push_back(nullptr);
        }
    }
}

Chunk* SyncTree::change(unsigned int x, unsigned int y)
{
    Chunk* c = inflateChunk(x, y, true);
    c->data++;

    return c;
}

Chunk* SyncTree::inflateChunk(unsigned x, unsigned y)
{
    return inflateChunk(x, y, false);
}

Chunk* SyncTree::inflateChunk(unsigned x, unsigned y, bool rememberChanged)
{
    if (x < area.topleft.x || x >= area.bottomRight.x || y < area.topleft.y || y >= area.bottomRight.y) {
        return nullptr;
    }

    unsigned xHalf = area.topleft.x + ((area.bottomRight.x - area.topleft.x) / 2);
    unsigned yHalf = area.topleft.y + ((area.bottomRight.y - area.topleft.y) / 2);

    unsigned index;
    bool firstXHalf = true, firstYHalf = true;

    if (x < xHalf && y < yHalf) { // topleft
        index = 0;
        firstXHalf = true;
        firstYHalf = true;
    } else if (x >= xHalf && y < yHalf) { // topright
        index = 1;
        firstXHalf = false;
        firstYHalf = true;
    } else if (x < xHalf && y >= yHalf) {
        index = 2;
        firstXHalf = true;
        firstYHalf = false;
    } else {
        index = 3;
        firstXHalf = false;
        firstYHalf = false;
    }

    if (finalLevel()) {
        if (data.at(index) == nullptr) {
            data.at(index) = new Chunk(Point(x, y), 0);
        }
        Chunk*& pChunk = data.at(index);

        // Remember changed chunk
        if (rememberChanged && std::find(changedChunks.begin(), changedChunks.end(), pChunk) == changedChunks.end()) {
            changedChunks.insert(changedChunks.end(), pChunk);
        }

        return pChunk;

    } else {
        if (childs.at(index) == nullptr) {

            Point p1(firstXHalf ? area.topleft.x : xHalf, firstYHalf ? area.topleft.y : yHalf);
            Point p2(
                p1.x + (area.bottomRight.x - area.topleft.x) / 2, p1.y + (area.bottomRight.y - area.topleft.y) / 2);
            SyncTree* child = new SyncTree(Rectangle(p1, p2), this, this->level + 1, this->numChilds);

            childs.at(index) = child;
        }
        Chunk* pChunk = childs.at(index)->inflateChunk(x, y, rememberChanged);

        // Remember changed chunk
        if (rememberChanged && std::find(changedChunks.begin(), changedChunks.end(), pChunk) == changedChunks.end()) {
            changedChunks.insert(changedChunks.end(), pChunk);
        }

        return pChunk;
    }
}

std::size_t SyncTree::getHash() { return currentHash; }

void SyncTree::reHash()
{
    std::size_t hash_value = 0;
    boost::hash_combine(hash_value, area.topleft.x);
    boost::hash_combine(hash_value, area.topleft.y);
    boost::hash_combine(hash_value, area.bottomRight.x);
    boost::hash_combine(hash_value, area.bottomRight.y);

    for (SyncTree* child : childs) {
        if (child != nullptr) {
            child->reHash();
            boost::hash_combine(hash_value, child->getHash());
        }
    }

    for (Chunk* chunk : data) {
        if (chunk != nullptr) {
            boost::hash_combine(hash_value, chunk->hashChunk());
        }
    }

    this->storedChanges = std::pair(this->currentHash, changedChunks);

    this->currentHash = hash_value;
    this->changedChunks = std::vector<Chunk*>();
}

std::pair<bool, std::vector<Chunk*>> SyncTree::getChanges(std::size_t since)
{
    if (storedChanges.first == since) {
        return std::pair<bool, std::vector<Chunk*>>(true, storedChanges.second);
    }
    return std::pair<bool, std::vector<Chunk*>>(false, std::vector<Chunk*>());
}

void SyncTree::checkDimensions(const Rectangle& rect)
{
    unsigned width = rect.bottomRight.x - rect.topleft.x;
    unsigned height = rect.bottomRight.y - rect.topleft.y;

    if (width != height) {
        throw std::invalid_argument("tree does not cover a square");
    }

    if (!isPowerOfTwo(width)) {
        throw std::invalid_argument("Height and width must be power of two");
    }

    if (width < 2) {
        throw std::invalid_argument("Tree has to be at least 2 units wide");
    }
}

void SyncTree::checkDimensions() { checkDimensions(area); }

bool SyncTree::isPowerOfTwo(ulong x) { return (x != 0) && ((x & (x - 1)) == 0); }

SyncTree* SyncTree::getSubtree(Rectangle rectangle)
{
    // Check if the given rectangle is a power of two and a sqare
    checkDimensions(rectangle);

    // The size of the requested

    // Check if the given rectangle is part of the tree
    if (rectangle.topleft.x < area.topleft.x || rectangle.bottomRight.x > area.bottomRight.x
        || rectangle.topleft.y < area.topleft.y || rectangle.bottomRight.y > area.bottomRight.y) {
        throw std::invalid_argument("Requested subtree is not part of the current tree");
    }

    // If the given rectangle belongs to this tree (checked above) and the size is the same, then
    // the current subtree is requested
    if (rectangle.bottomRight.x - rectangle.topleft.x == area.bottomRight.x - area.topleft.x) {
        return this;
    }

    unsigned xHalf = area.topleft.x + ((area.bottomRight.x - area.topleft.x) / 2);
    unsigned yHalf = area.topleft.y + ((area.bottomRight.y - area.topleft.y) / 2);
    unsigned index;
    Point topLeft = rectangle.topleft;
    if (topLeft.x < xHalf && topLeft.y < yHalf) { // topleft
        index = 0;
    } else if (topLeft.x >= xHalf && topLeft.y < yHalf) { // topright
        index = 1;
    } else if (topLeft.x < xHalf && topLeft.y >= yHalf) {
        index = 2;
    } else {
        index = 3;
    }

    // If the requested subtree is not yet inflated, return null
    if (childs.at(index) == nullptr) {
        return nullptr;
    }

    return childs.at(index)->getSubtree(rectangle);
}

std::pair<bool, std::vector<Chunk*>> SyncTree::getChanges(std::size_t since, Rectangle subtree)
{
    SyncTree* subtreePointer = getSubtree(subtree);
    if (subtreePointer == nullptr) {
        return std::pair<bool, std::vector<Chunk*>>(false, std::vector<Chunk*>());
    }

    return subtreePointer->getChanges(since);
}

}