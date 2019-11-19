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
    Chunk* c = inflateChunk(x, y);
    c->data++;

    // Remember stored chunk
    changedChunks.insert(changedChunks.end(), c);

    return c;
}

Chunk* SyncTree::inflateChunk(unsigned x, unsigned y)
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
        return data.at(index);

    } else {
        if (childs.at(index) == nullptr) {

            Point p1(firstXHalf ? area.topleft.x : xHalf, firstYHalf ? area.topleft.y : yHalf);
            Point p2(
                p1.x + (area.bottomRight.x - area.topleft.x) / 2, p1.y + (area.bottomRight.y - area.topleft.y) / 2);
            SyncTree* child = new SyncTree(Rectangle(p1, p2), this, this->level + 1, this->numChilds);

            childs.at(index) = child;
        }
        return childs.at(index)->inflateChunk(x, y);
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
}