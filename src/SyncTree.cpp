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

unsigned SyncTree::countInflatedChunks()
{
    unsigned numInflatedChunks = 0;

    for (SyncTree* child : childs) {
        if (child != nullptr) {
            numInflatedChunks += child->countInflatedChunks();
        }
    }

    for (Chunk* chunk : data) {
        if (chunk != nullptr) {
            numInflatedChunks++;
        }
    }

    return numInflatedChunks;
}

std::map<unsigned, unsigned> SyncTree::countInflatedSubtreesPerLevel()
{
    std::map<unsigned, unsigned> inflatedSubtreesPerLevel;

    unsigned subtrees = 0;
    for (SyncTree* child : childs) {
        if (child != nullptr) {
            subtrees++;

            std::map<unsigned, unsigned> tmp = child->countInflatedSubtreesPerLevel();
            for (auto const& elem : tmp) {

                if (inflatedSubtreesPerLevel.find(elem.first) == inflatedSubtreesPerLevel.end()) {
                    inflatedSubtreesPerLevel[elem.first] = elem.second;
                } else {
                    inflatedSubtreesPerLevel[elem.first] += elem.second;
                }
            }
        }
    }
    inflatedSubtreesPerLevel[getLevel()] = subtrees;

    return inflatedSubtreesPerLevel;
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

Chunk* SyncTree::change(unsigned int x, unsigned int y, unsigned version)
{
    Chunk* c = inflateChunk(x, y, true);
    c->data = version;

    return c;
}

Chunk* SyncTree::inflateChunk(unsigned x, unsigned y) { return inflateChunk(x, y, false); }

Chunk* SyncTree::inflateChunk(unsigned x, unsigned y, bool rememberChanged)
{
    if (x < (unsigned)area.topleft.x || x >= (unsigned)area.bottomRight.x || y < (unsigned)area.topleft.y
        || y >= (unsigned)area.bottomRight.y) {
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

void SyncTree::reHash(bool force)
{
    if (force || !changedChunks.empty()) {
        std::size_t hash_value = 0;
        boost::hash_combine(hash_value, area.topleft.x);
        boost::hash_combine(hash_value, area.topleft.y);
        boost::hash_combine(hash_value, area.bottomRight.x);
        boost::hash_combine(hash_value, area.bottomRight.y);

        for (SyncTree* child : childs) {
            if (child != nullptr) {
                child->reHash(force);
                boost::hash_combine(hash_value, child->getHash());
            }
        }

        for (Chunk* chunk : data) {
            if (chunk != nullptr) {
                boost::hash_combine(hash_value, chunk->hashChunk());
            }
        }

        // Only store "new revision" if something in the tree changed
        if (this->currentHash != hash_value) {

            this->storedChanges = std::pair(this->currentHash, changedChunks);

            this->currentHash = hash_value;
            this->changedChunks = std::vector<Chunk*>();
        }
    }
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

    int xHalf = area.topleft.x + ((area.bottomRight.x - area.topleft.x) / 2);
    int yHalf = area.topleft.y + ((area.bottomRight.y - area.topleft.y) / 2);
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
std::vector<unsigned char> SyncTree::getChunkPath(unsigned x, unsigned y)
{

    // Check if the given rectangle is part of the tree
    if (x < (unsigned)area.topleft.x || x >= (unsigned)area.bottomRight.x || y < (unsigned)area.topleft.y
        || y >= (unsigned)area.bottomRight.y) {
        throw std::invalid_argument("Requested chunk is not part of the current tree");
    }

    Rectangle current = this->area;
    std::vector<unsigned char> pathComponents;

    while (current.bottomRight.x - current.topleft.x >= 2) {

        unsigned xHalf = current.topleft.x + ((current.bottomRight.x - current.topleft.x) / 2);
        unsigned yHalf = current.topleft.y + ((current.bottomRight.y - current.topleft.y) / 2);
        unsigned char index;
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

        pathComponents.insert(pathComponents.end(), index);

        Point p1(firstXHalf ? current.topleft.x : xHalf, firstYHalf ? current.topleft.y : yHalf);
        Point p2(p1.x + (current.bottomRight.x - current.topleft.x) / 2,
            p1.y + (current.bottomRight.y - current.topleft.y) / 2);
        current = Rectangle(p1, p2);
    }

    return pathComponents;
}

NextNLevelsResponseType SyncTree::hashValuesOfNextNLevels(
    unsigned nextNLevels, size_t since)
{
    auto changes = getChanges(since);

    std::map<unsigned, std::vector<size_t>> hashValues = std::map<unsigned, std::vector<size_t>>();
    hashValues = hashValuesOfNextNLevels(nextNLevels, hashValues);
    return std::pair<std::map<unsigned, std::vector<size_t>>, int>(
        hashValues, changes.first ? changes.second.size() : -1);
}

std::map<unsigned, std::vector<size_t>> SyncTree::hashValuesOfNextNLevels(
    unsigned nextNLevels, std::map<unsigned, std::vector<size_t>> hashValues)
{
    if (nextNLevels <= 0) {
        return hashValues;
    } else {
        if (nextNLevels == 1) {
            if (hashValues.find(getLevel()) == hashValues.end()) {
                hashValues[getLevel()] = std::vector<size_t>();
            }
            hashValues[getLevel()].push_back(getHash());
        }

        if (nextNLevels > 1 && !finalLevel()) {

            if (nextNLevels == 2 && hashValues.find(getLevel() + 1) == hashValues.end()) {
                hashValues[getLevel() + 1] = std::vector<size_t>();
            }

            for (auto child : childs) {
                if (child == nullptr) {
                    // Add the empty hash for the child
                    if (nextNLevels == 2) {
                        hashValues[getLevel() + 1].push_back(0);
                    } else {
                        // Add empty levels for all uninitialized levels of the child node
                        if (hashValues.find(getLevel() + nextNLevels - 1) == hashValues.end()) {
                            hashValues[getLevel() + nextNLevels - 1] = std::vector<size_t>();
                        }
                        for (int j = 0; j < pow(4, nextNLevels - 2); j++) {
                            hashValues[getLevel() + nextNLevels - 1].push_back(0);
                        }
                    }

                } else {
                    hashValues = child->hashValuesOfNextNLevels(nextNLevels - 1, hashValues);
                }
            }
        }
        return hashValues;
    }
}

std::vector<SyncTree*> SyncTree::enumerateLowerLevel(unsigned n)
{
    if (n == 1) {
        return childs;
    } else if (n > 1) {
        std::vector<SyncTree*> childs;
        for (const auto& child : this->childs) {

            if (child != nullptr) {
                std::vector<SyncTree*> lowerChilds = child->enumerateLowerLevel(n - 1);
                childs.insert(childs.end(), lowerChilds.begin(), lowerChilds.end());
            } else {

                for (int i = 0; i < pow(4, n - 1); i++) {
                    childs.push_back(nullptr);
                }
            }
        }
        return childs;
    }
    return std::vector<SyncTree*>();
}
SyncTree* SyncTree::inflateSubtree(unsigned int level, int subtreeIndex)
{
    unsigned lowerLevels = level - this->getLevel();
    SyncTree* currentTree = this;

    for (unsigned i = 1; i <= lowerLevels; i++) {
        int indexToInflate = (subtreeIndex / (int)pow(4, lowerLevels - (i))) % 4;
        if (currentTree->childs.at(indexToInflate) == nullptr) {

            unsigned xHalf
                = currentTree->area.topleft.x + ((currentTree->area.bottomRight.x - currentTree->area.topleft.x) / 2);
            unsigned yHalf
                = currentTree->area.topleft.y + ((currentTree->area.bottomRight.y - currentTree->area.topleft.y) / 2);

            bool firstXHalf = true, firstYHalf = true;

            if (indexToInflate == 0) { // topleft
                firstXHalf = true;
                firstYHalf = true;
            } else if (indexToInflate == 1) { // topright
                firstXHalf = false;
                firstYHalf = true;
            } else if (indexToInflate == 2) {
                firstXHalf = true;
                firstYHalf = false;
            } else {
                firstXHalf = false;
                firstYHalf = false;
            }

            Point p1(
                firstXHalf ? currentTree->area.topleft.x : xHalf, firstYHalf ? currentTree->area.topleft.y : yHalf);
            Point p2(p1.x + (currentTree->area.bottomRight.x - currentTree->area.topleft.x) / 2,
                p1.y + (currentTree->area.bottomRight.y - currentTree->area.topleft.y) / 2);
            SyncTree* child
                = new SyncTree(Rectangle(p1, p2), currentTree, currentTree->level + 1, currentTree->numChilds);

            currentTree->childs.at(indexToInflate) = child;
        }
        currentTree = currentTree->childs.at(indexToInflate);
    }

    return currentTree;
}
unsigned SyncTree::getMaxLevel()
{
    unsigned width = this->getArea().bottomRight.x - this->getArea().topleft.x;
    unsigned levels = (unsigned)log2(width);
    return getLevel() + levels - 1;
}
SyncRequestResponse SyncTree::syncRequest(size_t since, unsigned nextNLevels, unsigned threshold)
{
    SyncRequestResponse syncRequestResponse = SyncRequestResponse();

    std::pair<bool, std::vector<Chunk*>> changeResponse = getChanges(since);

    if (getLevel() + 1 > getMaxLevel() || nextNLevels <= 1) {
        syncRequestResponse.changeReponse = changeResponse;
        syncRequestResponse.containsChanges = true;

    } else if (!changeResponse.first || changeResponse.second.size() > threshold) {

        NextNLevelsResponseType nextNLevelResponse = hashValuesOfNextNLevels(nextNLevels, since);
        syncRequestResponse.containsChanges = false;
        syncRequestResponse.nextNLevelsResponse = nextNLevelResponse;

    } else {
        syncRequestResponse.changeReponse = changeResponse;
        syncRequestResponse.containsChanges = true;
    }

    return syncRequestResponse;
}

}