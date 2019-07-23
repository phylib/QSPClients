//
// Created by phmoll on 7/22/19.
//

#include <iostream>
#include "HashStorage.h"

HashStorage::HashStorage(size_t size)
        : _size(size), datastore(size) {
}

void HashStorage::insert(size_t hash, const std::vector<Point> changes) {
    datastore.put(hash, changes);
}

std::pair<size_t, const std::vector<Point>> HashStorage::get(std::size_t hash) {
    const std::vector<Point> changes = datastore.get(hash);
    return std::pair<size_t, const std::vector<Point>>(hash, changes);
}

std::size_t HashStorage::size() {
    return datastore.size();
}

bool HashStorage::exists(size_t hash) {
    return datastore.exists(hash);
}

