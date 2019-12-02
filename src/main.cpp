#include "QuadTreeStructs.h"
#include "SyncTree.h"
#include "csv/CSVReader.h"
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <iostream>

struct ChangeRecord {
    std::vector<std::pair<unsigned, std::vector<quadtree::Chunk>>> changesPerTick;
    int minX = INT_MAX;
    int minY = INT_MAX;
    int maxX = INT_MIN;
    int maxY = INT_MIN;
};

std::string printCSVHeader()
{
    std::string header = "TickNo\tchangedChunks\trequiredChunksDelivery\trequiredChunksDeliveryNoHist\tinflatedNodes";
    return header;
}

ChangeRecord readChangesOverTime(const std::string& fname)
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

        for (std::string& chunkStr : chunkStrings) {

            std::size_t splitPos = chunkStr.find_first_of(',');
            quadtree::Point chunkPos(
                std::stoi(chunkStr.substr(0, splitPos)), std::stoi(chunkStr.substr(splitPos + 1, chunkStr.length())));
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
        changesPerTick.insert(changesPerTick.end(),
            std::pair<unsigned, std::vector<quadtree::Chunk>>(std::stoi(item.at(0)), changedChunks));
    }

    ChangeRecord record;
    record.minX = minX;
    record.maxX = maxX;
    record.minY = minY;
    record.maxY = maxY;
    record.changesPerTick = changesPerTick;

    return record;
}

void simulateQuadTreeSync(unsigned treeSize, const ChangeRecord& changeRecord)
{

    std::cout << printCSVHeader() << std::endl;

    // Create a sync tree
    const quadtree::Rectangle& rectangle
        = quadtree::Rectangle(quadtree::Point(0, 0), quadtree::Point(treeSize, treeSize));
    quadtree::SyncTree tree(rectangle);

    // Apply all changes and rehash the tree
    for (const std::pair<unsigned, std::vector<quadtree::Chunk>>& item : changeRecord.changesPerTick) {
        unsigned changes = 0;
        for (const quadtree::Chunk& chunk : item.second) {
            if (rectangle.isPointInRectangle(quadtree::Point(chunk.pos.x, chunk.pos.y))) {
                tree.change(chunk.pos.x, chunk.pos.y);
                changes++;
            }
        }
        size_t old_hash = tree.getHash();
        tree.reHash();
        std::cout << item.first << "\t" << changes << "\t" << tree.getChanges(old_hash).second.size() << "\t"
                  << tree.countInflatedChunks() << "\t" << tree.countInflatedNodes()

                  << std::endl;
    }
}

void simulateRMA(unsigned treeSize, const ChangeRecord& changeRecord)
{

    std::cout << printCSVHeader() << std::endl;

    const quadtree::Rectangle& rectangle
        = quadtree::Rectangle(quadtree::Point(0, 0), quadtree::Point(treeSize, treeSize));

    for (const std::pair<unsigned, std::vector<quadtree::Chunk>>& item : changeRecord.changesPerTick) {

        unsigned changes = 0;
        for (const quadtree::Chunk& chunk : item.second) {
            if (rectangle.isPointInRectangle(quadtree::Point(chunk.pos.x, chunk.pos.y))) {
                changes++;
            }
        }

        std::cout << item.first << "\t" << changes << "\t" << treeSize * treeSize << "\t" << treeSize * treeSize
                  << std::endl;
    }
}

int main(int argc, char* argv[])
{

    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " [treeSize] [syncType] [duplicate]" << std::endl;
        exit(-1);
    }

    std::string fname = "/home/phmoll/Coding/Minecraft/GameStateChanges/chunkChanges.csv";
    unsigned treeSize = atoi(argv[1]);
    std::string syncType(argv[2]);
    unsigned duplicate = atoi(argv[3]);

    // Shift all positions to the center of the Sync Tree
    ChangeRecord record = readChangesOverTime(fname);
    int xShiftValue = std::min(record.minX, (int)treeSize / 2);
    int yShiftValue = std::min(record.minY, (int)treeSize / 2);
    int xMultiplayerShift = record.maxX - record.minX;
    int yMultiplayerShift = record.maxY - record.minY;

    for (std::pair<unsigned, std::vector<quadtree::Chunk>>& item : record.changesPerTick) {
        for (quadtree::Chunk& chunk : item.second) {
            chunk.pos.x += std::abs(xShiftValue);
            chunk.pos.y += std::abs(yShiftValue);
        }
    }

    for (int i = 1; i <= duplicate; i++) {
        for (std::pair<unsigned, std::vector<quadtree::Chunk>>& item : record.changesPerTick) {
            std::vector<quadtree::Chunk> duplicated;

            for (quadtree::Chunk& chunk : item.second) {
                duplicated.insert(duplicated.end(), quadtree::Chunk(quadtree::Point(chunk.pos.x + (i * xMultiplayerShift), chunk.pos.y), 0));
                duplicated.insert(duplicated.end(), quadtree::Chunk(quadtree::Point(chunk.pos.x + (i * xMultiplayerShift), chunk.pos.y + (i * yMultiplayerShift)), 0));
                duplicated.insert(duplicated.end(), quadtree::Chunk(quadtree::Point(chunk.pos.x, chunk.pos.y + (i * yMultiplayerShift)), 0));
            }

            item.second.insert(item.second.end(), duplicated.begin(), duplicated.end());
        }
    }

    if (syncType == "quadtree") {

        simulateQuadTreeSync(treeSize, record);

    } else if (syncType == "rma") {

        simulateRMA(treeSize, record);
    }

    return 0;
}