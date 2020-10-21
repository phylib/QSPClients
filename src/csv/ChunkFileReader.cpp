//
// Created by phmoll on 3/5/20.
//

#include "ChunkFileReader.h"

std::vector<std::pair<unsigned, std::vector<quadtree::Chunk>>> ChunkFileReader::readChangesOverTime(
    const std::string& fname, unsigned treeSize)
{
    CSVReader reader(fname);
    const std::vector<std::vector<std::string>>& lines = reader.getData();

    int minX = INT_MAX;
    int minY = INT_MAX;
    int maxX = INT_MIN;
    int maxY = INT_MIN;

    std::vector<std::pair<unsigned, std::vector<quadtree::Chunk>>> changesPerTick;

    // Iterate over all timestamps
    for (const std::vector<std::string>& item : lines) {

        std::vector<std::string> chunkStrings;
        std::vector<quadtree::Chunk> changedChunks;
        boost::split(chunkStrings, item.at(1), boost::is_any_of(";"));

        if (item.size() == 1 || !item.at(1).empty()) {
            for (std::string& chunkStr : chunkStrings) {

                std::size_t splitPos = chunkStr.find_first_of(',');
                quadtree::Point chunkPos(std::stoi(chunkStr.substr(0, splitPos)),
                    std::stoi(chunkStr.substr(splitPos + 1, chunkStr.length())));
                quadtree::Chunk chunk(chunkPos, 0);
                changedChunks.insert(changedChunks.end(), chunk);

                if (chunk.pos.x < minX) {
                    minX = chunk.pos.x;
                }
                if (chunk.pos.x > maxX) {
                    maxX = chunk.pos.x;
                }
                if (chunk.pos.y < minY) {
                    minY = chunk.pos.y;
                }
                if (chunk.pos.y > maxY) {
                    maxY = chunk.pos.y;
                }
            }
        }
        changesPerTick.insert(changesPerTick.end(),
            std::pair<unsigned, std::vector<quadtree::Chunk>>(std::stoi(item.at(0)), changedChunks));
    }

    int width = maxX - minX;
    int height = maxY - minY;
    int xCenter = minX + (width / 2);
    int yCenter = minY + (height / 2);
    int xShift = (treeSize / 2) - xCenter;
    int yShift = (treeSize / 2) - yCenter;

    // Shift the chunk coordinates to the center of the tree
    for (unsigned j = 0; j < changesPerTick.size(); j++) {
        auto tickChunkPair = changesPerTick[j];
        for (unsigned i = 0; i < tickChunkPair.second.size(); i++) {
            quadtree::Chunk chunk = tickChunkPair.second[i];
            chunk.pos.x = chunk.pos.x + xShift;
            chunk.pos.y = chunk.pos.y + yShift;
            tickChunkPair.second[i] = chunk;
        }
        changesPerTick[j] = tickChunkPair;
    }

    return changesPerTick;
}
