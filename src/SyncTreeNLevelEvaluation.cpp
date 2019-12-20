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

void writeNumRequestCSVHeader(std::ostream& outfile)
{
    outfile << "TickNo\tsubtreeRequests\tchunkRequests\ttotalRequests" << std::endl;
}

void writeNumRequests(std::ostream& outfile, unsigned tickNo, unsigned subtreeRequests, unsigned chunkRequests)
{
    outfile << tickNo << '\t' << subtreeRequests << '\t' << chunkRequests << '\t' << (subtreeRequests + chunkRequests)
            << std::endl;
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

void simulateQuadTreeSync(
    unsigned treeSize, const ChangeRecord& changeRecord, int numLevels, int chunkRequestThreshold, std::string fname)
{
    std::ofstream outfile;
    outfile.open(fname, std::ios::out | std::ios::trunc);
    writeNumRequestCSVHeader(outfile);

    // Create a sync originalTree
    const quadtree::Rectangle& rectangle
        = quadtree::Rectangle(quadtree::Point(0, 0), quadtree::Point(treeSize, treeSize));

    quadtree::SyncTree originalTree(rectangle);
    quadtree::SyncTree clonedTree(rectangle);

    // Apply all changes and rehash the originalTree
    for (const std::pair<unsigned, std::vector<quadtree::Chunk>>& item : changeRecord.changesPerTick) {
        unsigned changes = 0;
        for (const quadtree::Chunk& chunk : item.second) {
            if (rectangle.isPointInRectangle(quadtree::Point(chunk.pos.x, chunk.pos.y))) {
                originalTree.change(chunk.pos.x, chunk.pos.y);
                changes++;
            }
        }
        size_t old_hash = originalTree.getHash();
        originalTree.reHash();

        // Request the changes from the original tree
        int lowerSubtreeRequests = 0;
        int chunkRequests = 0;
        std::vector<quadtree::SyncTree*> treesToCompare;
        treesToCompare.push_back(&clonedTree);
        while (!treesToCompare.empty()) {

            quadtree::SyncTree* currentSubTree = treesToCompare.front();
            treesToCompare.erase(treesToCompare.begin());

            if (originalTree.getSubtree(currentSubTree->getArea()) == nullptr) {
                std::cout << currentSubTree->getArea().topleft.x << "," << currentSubTree->getArea().topleft.y << " "
                          << currentSubTree->getArea().bottomRight.x << "," << currentSubTree->getArea().bottomRight.y
                          << std::endl;
            }
            auto hashValuesResponse
                = originalTree.getSubtree(currentSubTree->getArea())
                      ->hashValuesOfNextNLevels(numLevels, currentSubTree->getHash()); // This is the request
            lowerSubtreeRequests++;

            if (hashValuesResponse.second > chunkRequestThreshold) {

                unsigned lowestLevel = hashValuesResponse.first.rbegin()->first;
                auto hashValues = hashValuesResponse.first[lowestLevel];
                auto treeNodes = currentSubTree->enumerateLowerLevel(lowestLevel - currentSubTree->getLevel());
                for (unsigned i = 0; i < hashValues.size(); i++) {

                    if ((treeNodes.at(i) == nullptr && hashValues.at(i) != 0)
                        || (treeNodes.at(i) != nullptr && hashValues.at(i) != treeNodes.at(i)->getHash())) {
                        if (treeNodes.at(i) == nullptr) {
                            treeNodes.at(i) = currentSubTree->inflateSubtree(lowestLevel, i);
                        }
                        treesToCompare.push_back(treeNodes.at(i));
                    }
                }

            } else {
                chunkRequests++;
                for (const auto& change :
                    originalTree.getSubtree(currentSubTree->getArea())->getChanges(currentSubTree->getHash()).second) {
                    clonedTree.change(change->pos.x, change->pos.y, change->data);
                }
            }
        }
        clonedTree.reHash();

        writeNumRequests(outfile, item.first, lowerSubtreeRequests, chunkRequests);
    }
    outfile.close();
}

int main(int argc, char* argv[])
{

    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " [treeSize] [outputFolder]" << std::endl;
        exit(-1);
    }

    std::string fname = "/home/phmoll/Coding/Minecraft/GameStateChanges/chunkChanges.csv";
    unsigned treeSize = atoi(argv[1]);
    std::string outputFolder = argv[2];
    unsigned duplicate = 1;

    // Shift all positions to the center of the Sync Tree
    ChangeRecord record = readChangesOverTime(fname);
    int xShiftValue = std::min(record.minX, (int)treeSize / 2);
    int yShiftValue = std::min(record.minY, (int)treeSize / 2);
    int traceXWidth = record.maxX - record.minX;
    int traceYWidth = record.maxY - record.minY;

    for (std::pair<unsigned, std::vector<quadtree::Chunk>>& item : record.changesPerTick) {
        for (quadtree::Chunk& chunk : item.second) {
            chunk.pos.x += std::abs(xShiftValue);
            chunk.pos.y += std::abs(yShiftValue);
        }
    }

    for (unsigned i = 1; i <= duplicate; i++) {
        for (std::pair<unsigned, std::vector<quadtree::Chunk>>& item : record.changesPerTick) {
            std::vector<quadtree::Chunk> duplicated;

            for (quadtree::Chunk& chunk : item.second) {
                duplicated.insert(duplicated.end(),
                    quadtree::Chunk(quadtree::Point(chunk.pos.x + (i * traceXWidth), chunk.pos.y), 0));
                duplicated.insert(duplicated.end(),
                    quadtree::Chunk(
                        quadtree::Point(chunk.pos.x + (i * traceXWidth), chunk.pos.y + (i * traceYWidth)), 0));
                duplicated.insert(duplicated.end(),
                    quadtree::Chunk(quadtree::Point(chunk.pos.x, chunk.pos.y + (i * traceYWidth)), 0));
            }

            item.second.insert(item.second.end(), duplicated.begin(), duplicated.end());
        }
    }

    unsigned lowerLevels = 3;
    unsigned chunkThreshold = 100;
    std::string outFileName = outputFolder + "network_requests_player_" + std::to_string(duplicate) + "_lowerLevel_"
        + std::to_string(lowerLevels) + "_chunkThreshold_" + std::to_string(chunkThreshold) + ".csv";
    simulateQuadTreeSync(treeSize, record, lowerLevels, chunkThreshold, outFileName);

    return 0;
}