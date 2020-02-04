//
// Created by phmoll on 1/27/20.
//
#include <chrono>
#include <thread>

#include "ServerModeSyncClient.h"
#include "csv/CSVReader.h"

using namespace std::literals;

std::vector<std::pair<unsigned, std::vector<quadtree::Chunk>>> readChangesOverTime(
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

    int width = maxX - minX;
    int height = maxY - minY;
    int xCenter = minX + (width / 2);
    int yCenter = minY + (width / 2);
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

int main(int argc, char* argv[])
{

    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " serverNum" << std::endl;
        exit(-1);
    }
    unsigned servernum = std::stoi(argv[1]);

    unsigned treeSize = 65536;
    unsigned initialRequestLevel = 1;

    std::string traceFile = "../QuadTreeRMAComparison/max_distance/ChunkChanges-very-distributed.csv";
    // Parse CSV File
    auto changesOverTime = readChangesOverTime(traceFile, treeSize);

    // Create Sync Client
    quadtree::Rectangle world(quadtree::Point(0, 0), quadtree::Point(treeSize, treeSize));
    quadtree::Rectangle responsibility(quadtree::Point(0, 0), quadtree::Point(treeSize / 2, treeSize / 2));

    if (servernum == 0) {
        responsibility = quadtree::Rectangle(quadtree::Point(0, 0), quadtree::Point(treeSize / 2, treeSize / 2));
    } else if (servernum == 1) {
        responsibility = quadtree::Rectangle(quadtree::Point(treeSize / 2, 0), quadtree::Point(treeSize, treeSize / 2));
    } else if (servernum == 2) {
        responsibility = quadtree::Rectangle(quadtree::Point(0, treeSize / 2), quadtree::Point(treeSize / 2, treeSize));
    } else if (servernum == 3) {
        responsibility
            = quadtree::Rectangle(quadtree::Point(treeSize / 2, treeSize / 2), quadtree::Point(treeSize, treeSize));
    }

    quadtree::ServerModeSyncClient client("/world", world, responsibility, initialRequestLevel, changesOverTime,
        "logs/Testlog_" + std::to_string(servernum) + ".csv");

    // Start Sync Client
    try {
        client.startSynchronization();
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
}