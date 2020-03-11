//
// Created by phmoll on 3/5/20.
//

#ifndef QUADTREESYNCEVALUATION_CHUNKFILEREADER_H
#define QUADTREESYNCEVALUATION_CHUNKFILEREADER_H

#include "../QuadTreeStructs.h"
#include "CSVReader.h"

class ChunkFileReader {

public:
    static std::vector<std::pair<unsigned, std::vector<quadtree::Chunk>>> readChangesOverTime(
        const std::string& fname, unsigned treeSize);
};

#endif // QUADTREESYNCEVALUATION_CHUNKFILEREADER_H
