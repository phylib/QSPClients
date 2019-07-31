//
// Created by phmoll on 7/19/19.
//

#include <iostream>
#include "QuadTree.h"

void QuadTree::init() {

    if (isInMaxLevel()) {
        initChunks();
    } else {
        initChildren();
    }

    if (this->level == 1) {
        hashQuadTree(true);
    }
}

void QuadTree::initChunks() {
    for (int i = topLeft.x; i < botRight.x; i++) {
        for (int j = topLeft.y; j < botRight.y; j++) {

            chunks.insert(chunks.begin(), Chunk(Point(i, j), 0));

        }
    }
//    cout << chunks.size() << endl;
}

void QuadTree::initChildren() {
    int verticalHalf = topLeft.x + ((botRight.x - topLeft.x) / 2);
    int horizontalHalf = topLeft.y + ((botRight.y - topLeft.y) / 2);

    topLeftTree = new QuadTree(Point(topLeft.x, topLeft.y), Point(verticalHalf, horizontalHalf), level + 1, this);
    topRightTree = new QuadTree(Point(verticalHalf, topLeft.y), Point(botRight.x, horizontalHalf), level + 1, this);
    botLeftTree = new QuadTree(Point(topLeft.x, horizontalHalf), Point(verticalHalf, botRight.y), level + 1, this);
    botRightTree = new QuadTree(Point(verticalHalf, horizontalHalf), Point(botRight.x, botRight.y), level + 1, this);
}

Chunk *QuadTree::getChunk(Chunk _chunk, bool change) {
    Point point = _chunk.pos;

    if (change) {

        auto searchResult = std::find_if(this->changedChunks.begin(), this->changedChunks.end(),
                                         std::bind(Chunk::chunkCoordMatch, std::placeholders::_1, _chunk));
        if (searchResult != this->changedChunks.end()) {
            this->changedChunks.erase(searchResult);
        }
        this->changedChunks.insert(this->changedChunks.begin(), _chunk);
    }

    if (isInMaxLevel()) {
        // Replace corresponding chunk
        for (Chunk &chunk : this->chunks) {
            if (chunk.pos.x == point.x && chunk.pos.y == point.y) {

                if (change) {
                    chunk.data = _chunk.data;
                }

                return &chunk;
            }
        }
        return nullptr;


    } else {
        // Traverse until final level
        int verticalHalf = topLeft.x + ((botRight.x - topLeft.x) / 2);
        int horizontalHalf = topLeft.y + ((botRight.y - topLeft.y) / 2);

        if (point.x < verticalHalf && point.y < horizontalHalf) {
            return topLeftTree->getChunk(_chunk, change);
        } else if (point.x >= verticalHalf && point.y < horizontalHalf) {
            return topRightTree->getChunk(_chunk, change);
        } else if (point.x < verticalHalf && point.y >= horizontalHalf) {
            return botLeftTree->getChunk(_chunk, change);
        } else if (point.x >= verticalHalf && point.y >= horizontalHalf) {
            return botRightTree->getChunk(_chunk, change);
        }
    }
}

Chunk *QuadTree::markChangedChunk(Chunk changedChunk) {
    return getChunk(changedChunk, true);
}

Chunk *QuadTree::getChunk(Point point) {
    Chunk chunk(point, 0);
    return getChunk(chunk, false);
}

std::pair<Point, Point> QuadTree::getBounds() {
    return std::pair<Point, Point>(topLeft, botRight);
}

std::size_t QuadTree::hashQuadTree(bool force) {

    if (isChanged() || force) { // Only rehash if parts have changed

        if (isInMaxLevel()) {
            std::size_t seed = 0;
            for (Chunk &c : chunks) {
                boost::hash_combine(seed, c.hashChunk());
            }
            updateHash(seed);

            return seed;
        } else {

            std::size_t seed = 0;
            boost::hash_combine(seed, this->topLeftTree->hashQuadTree(force));
            boost::hash_combine(seed, this->topRightTree->hashQuadTree(force));
            boost::hash_combine(seed, this->botLeftTree->hashQuadTree(force));
            boost::hash_combine(seed, this->botRightTree->hashQuadTree(force));

            updateHash(seed);

            return seed;
        }

    } else {
        return this->hash;
    }
}

void QuadTree::setHashStorage(HashStorage &_hashStorage) {
    this->hashStorage = &_hashStorage;
    if (!isInMaxLevel()) {
        this->topLeftTree->setHashStorage(_hashStorage);
        this->topRightTree->setHashStorage(_hashStorage);
        this->botLeftTree->setHashStorage(_hashStorage);
        this->botRightTree->setHashStorage(_hashStorage);
    }
}

void QuadTree::updateHash(size_t newHash) {

    // Invalidate old data store entry
    if (this->hashStorage != nullptr) {
        this->hashStorage->remove(this->previousHash);
    }

    this->previousHash = hash;
    this->hash = newHash;
    if (this->hashStorage != nullptr) {
        this->hashStorage->insert(this->previousHash, this->changedChunks);
    }
    this->changedChunks.clear();
}

bool QuadTree::isPointInQuadTree(Point p) {
    return p.x >= topLeft.x && p.x < botRight.x
           && p.y >= topLeft.y && p.y < botRight.y;
}

std::vector<Chunk> QuadTree::enumerateChunks() {
    std::vector<Chunk> chunkVector;

    if (isInMaxLevel()) {
        chunkVector.insert(chunkVector.begin(), this->chunks.begin(), this->chunks.end());
    } else {
        std::vector<Chunk> tmp;
        tmp = this->topLeftTree->enumerateChunks();
        chunkVector.insert(chunkVector.end(), tmp.begin(), tmp.end());
        tmp = this->topRightTree->enumerateChunks();
        chunkVector.insert(chunkVector.end(), tmp.begin(), tmp.end());
        tmp = this->botLeftTree->enumerateChunks();
        chunkVector.insert(chunkVector.end(), tmp.begin(), tmp.end());
        tmp = this->botRightTree->enumerateChunks();
        chunkVector.insert(chunkVector.end(), tmp.begin(), tmp.end());
    }

    std::sort(chunkVector.begin(), chunkVector.end(), Chunk::compareChunks);
    return chunkVector;
}

const std::vector<Point> QuadTree::splitPath(const std::string &path, int quadtreeSize) {
    std::vector<std::string> stringPathComponents;
    boost::split(stringPathComponents, path, boost::is_any_of("/"));
    stringPathComponents.erase(
            std::remove_if(
                    stringPathComponents.begin(),
                    stringPathComponents.end(),
                    [](std::string const &s) { return s.empty(); }),
            stringPathComponents.end());

    std::vector<Point> pathComponents;
    int i = 1;
    for (const std::string comp : stringPathComponents) {
        Point point = QuadTree::getLeftUpperCornerFromPathComponent(comp, i++, quadtreeSize);
        pathComponents.insert(pathComponents.begin(),
                              point);
    }
    std::reverse(pathComponents.begin(), pathComponents.end());
    return pathComponents;
}

Point
QuadTree::getLeftUpperCornerFromPathComponent(const std::string &pathComponent, unsigned level, int quadtreeSize) {
    std::vector<std::string> points;
    boost::split(points, pathComponent, boost::is_any_of(","));
    auto it = points.begin();
    int x = std::stoi(*it);
    it++;
    int y = std::stoi(*it);

    int factors = quadtreeSize / pow(2, level);

    return Point(x * factors, y * factors);
}

std::string QuadTree::getPath(const Point &point, int quadtreeSize, unsigned levels) {

    std::string path = "/";
    int max_exp = (int) log2(quadtreeSize) - 1;
    for (int i = 0; i < levels; i++) {
        path += std::to_string(int(floor(point.x / (double) pow(2, max_exp - i)))) + ","
               + std::to_string(int(floor(point.y / (double) pow(2, max_exp - i))))
               + "/";
    }

    return path;
}

QuadTree *QuadTree::getSubTree(const std::string &path, int quadtreeSize) {
    const std::vector<Point> &vector = splitPath(path, quadtreeSize);
    if (this->level - 1 == vector.size() || isInMaxLevel()) {
        return this;
    }

    Point leftUpperCorner = vector.back();

    // Traverse until final level
    int verticalHalf = topLeft.x + ((botRight.x - topLeft.x) / 2);
    int horizontalHalf = topLeft.y + ((botRight.y - topLeft.y) / 2);

    if (leftUpperCorner.x < verticalHalf && leftUpperCorner.y < horizontalHalf) {
        return topLeftTree->getSubTree(path, quadtreeSize);
    } else if (leftUpperCorner.x >= verticalHalf && leftUpperCorner.y < horizontalHalf) {
        return topRightTree->getSubTree(path, quadtreeSize);
    } else if (leftUpperCorner.x < verticalHalf && leftUpperCorner.y >= horizontalHalf) {
        return botLeftTree->getSubTree(path, quadtreeSize);
    } else if (leftUpperCorner.x >= verticalHalf && leftUpperCorner.y >= horizontalHalf) {
        return botRightTree->getSubTree(path, quadtreeSize);
    }
}

