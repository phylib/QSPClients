//
// Created by phmoll on 7/22/19.
//

#ifndef QUADTREESYNCEVALUATION_HASHSTORAGE_H
#define QUADTREESYNCEVALUATION_HASHSTORAGE_H

#include <unordered_map>

#include "../QuadTreeStructs.h"

class HashStorage {
public:
    HashStorage();

public:
    void insert(size_t hash, const std::vector<Chunk> &changes);

    std::pair<size_t, const std::vector<Chunk>> get(std::size_t hash);

    std::size_t size();

    bool exists(size_t hash);

    void remove(size_t hash);

private:
    std::unordered_map<size_t, const std::vector<Chunk>> datastore;
};

#endif //QUADTREESYNCEVALUATION_HASHSTORAGE_H
