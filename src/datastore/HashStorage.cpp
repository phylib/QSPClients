//
// Created by phmoll on 7/22/19.
//

#include <iostream>
#include "HashStorage.h"

HashStorage::HashStorage() = default;

void HashStorage::insert(size_t hash, const std::vector<quadtree::Chunk> &changes) {
    datastore.insert(std::make_pair(hash, changes));
}

std::pair<size_t, const std::vector<quadtree::Chunk>> HashStorage::get(std::size_t hash) {
    const std::vector<quadtree::Chunk> changes = datastore[hash];
    return std::pair<size_t, const std::vector<quadtree::Chunk>>(hash, changes);
}

std::size_t HashStorage::size() {
    return datastore.size();
}

bool HashStorage::exists(size_t hash) {
    return datastore.find(hash) != datastore.end();
}

void HashStorage::remove(size_t hash) {
    auto it = datastore.find(hash);
    if (it != datastore.end()) {
        datastore.erase(it);
    }
}

