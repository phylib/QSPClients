#include "QuadTreeStructs.h"
#include "SyncTree.h"
#include "csv/CSVReader.h"
#include "proto/ChunkChanges.pb.h"
#include "proto/LowerLevelHashes.pb.h"
#include "zip/GZip.h"
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <numeric>

struct ChangeRecord {
    std::vector<std::pair<unsigned, std::vector<quadtree::Chunk>>> changesPerTick;
    int minX = INT_MAX;
    int minY = INT_MAX;
    int maxX = INT_MIN;
    int maxY = INT_MIN;
};

void writeNumRequestCSVHeader(std::ostream& outfile)
{
    outfile << "TickNo\tchangesPerTick\tsubtreeRequests\tchunkRequests\ttotalRequests\tsubTreeRequestSizesMin\tsubTreeR"
               "equestSizesMax\tsubTreeRequestSizesMean\tchunkRequestSizesMin\tchunkRequestSizesMax\tchunkRequestSizesMean"
            << std::endl;
}

void writeNumRequests(std::ostream& outfile, unsigned tickNo, unsigned changesPerTick, unsigned subtreeRequests,
    unsigned chunkRequests, std::vector<unsigned> subtreeRequestSizes, std::vector<unsigned> chunkRequestSizes)
{
    outfile << tickNo << '\t' << changesPerTick << '\t' << subtreeRequests << '\t' << chunkRequests << '\t'
            << (subtreeRequests + chunkRequests) << '\t'
            << *min_element(subtreeRequestSizes.begin(), subtreeRequestSizes.end()) << '\t'
            << *max_element(subtreeRequestSizes.begin(), subtreeRequestSizes.end()) << '\t'
            << std::accumulate(subtreeRequestSizes.begin(), subtreeRequestSizes.end(), 0.0) / subtreeRequestSizes.size() << '\t'
            << *min_element(chunkRequestSizes.begin(), chunkRequestSizes.end()) << '\t'
            << *max_element(chunkRequestSizes.begin(), chunkRequestSizes.end()) << '\t'
            << std::accumulate(chunkRequestSizes.begin(), chunkRequestSizes.end(), 0.0) / chunkRequestSizes.size()
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

unsigned getLowerSubtreeResponseSize(std::pair<std::map<unsigned, std::vector<size_t>>, int> changeResponse)
{

    unsigned lowestLevel = changeResponse.first.rbegin()->first;

    quadtreesync::LowerLevelHashes lowerSubtreeResponse;
    lowerSubtreeResponse.set_treelevel(lowestLevel);
    lowerSubtreeResponse.set_numchanges(changeResponse.second);
    for (size_t hashValue : changeResponse.first[lowestLevel]) {
        lowerSubtreeResponse.mutable_hashvalues()->add_hashvalues(hashValue);
    }
    std::string encoded = lowerSubtreeResponse.SerializeAsString();
    std::string zipped = GZip::compress(encoded);
    // std::cout << lowerSubtreeResponse.ByteSize() << '-' << zipped.length() << std::endl;

    return zipped.length();
}

unsigned getChunkRequestResponseSize(std::pair<bool, std::vector<quadtree::Chunk*>> chunkRequestResponse)
{

    quadtreesync::ChunkChanges chunkChanges;
    chunkChanges.set_hashknown(chunkRequestResponse.first);
    for (const quadtree::Chunk* chunk : chunkRequestResponse.second) {

        quadtreesync::Chunk protoChunk;
        protoChunk.set_data(chunk->data);
        protoChunk.set_x(chunk->pos.x);
        protoChunk.set_y(chunk->pos.y);
        chunkChanges.mutable_chunks()->Add()->CopyFrom(protoChunk);
    }
    std::string zipped = GZip::compress(chunkChanges.SerializeAsString());
    return zipped.length();
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
        std::vector<unsigned> lowerSubtreeRequestSizes;
        std::vector<unsigned> chunkRequestSizes;
        std::vector<quadtree::SyncTree*> treesToCompare;
        treesToCompare.push_back(&clonedTree);
        while (!treesToCompare.empty()) {

            quadtree::SyncTree* currentSubTree = treesToCompare.front();
            treesToCompare.erase(treesToCompare.begin());

            auto hashValuesResponse
                = originalTree.getSubtree(currentSubTree->getArea())
                      ->hashValuesOfNextNLevels(numLevels, currentSubTree->getHash()); // This is the request
            lowerSubtreeRequests++;
            lowerSubtreeRequestSizes.push_back(getLowerSubtreeResponseSize(hashValuesResponse));

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
                const std::pair<bool, std::vector<quadtree::Chunk*>>& chunkRequest
                    = originalTree.getSubtree(currentSubTree->getArea())->getChanges(currentSubTree->getHash());
                chunkRequestSizes.push_back(getChunkRequestResponseSize(chunkRequest));
                for (const auto& change : chunkRequest.second) {
                    clonedTree.change(change->pos.x, change->pos.y, change->data);
                }
            }
        }
        clonedTree.reHash();

        writeNumRequests(outfile, item.first, changes, lowerSubtreeRequests, chunkRequests, lowerSubtreeRequestSizes,
            chunkRequestSizes);
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

    //    unsigned playerDuplication[] = { 0, 1, 2 };
    //    unsigned lowerLevels[] = { 1, 2, 3, 4, 5, 6, 7 };
    //    unsigned chunkThresholds[] = { 10, 20, 50, 100, 200, 500, 1000 };
    unsigned playerDuplication[] = { 1 };
    unsigned lowerLevels[] = { 3 };
    unsigned chunkThresholds[] = { 100 };

    for (unsigned duplicate : playerDuplication) {
        for (unsigned lowerLevel : lowerLevels) {
            for (unsigned chunkThreshold : chunkThresholds) {

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
                                    quadtree::Point(chunk.pos.x + (i * traceXWidth), chunk.pos.y + (i * traceYWidth)),
                                    0));
                            duplicated.insert(duplicated.end(),
                                quadtree::Chunk(quadtree::Point(chunk.pos.x, chunk.pos.y + (i * traceYWidth)), 0));
                        }

                        item.second.insert(item.second.end(), duplicated.begin(), duplicated.end());
                    }
                }

                std::cout << "Start evaluation (player duplication " << duplicate << ", lowerLevels " << lowerLevel
                          << ", chunkThreshold " << chunkThreshold << ")" << std::endl;
                std::string outFileName = outputFolder + "network_requests_player_" + std::to_string(duplicate)
                    + "_lowerLevel_" + std::to_string(lowerLevel) + "_chunkThreshold_" + std::to_string(chunkThreshold)
                    + ".csv";
                simulateQuadTreeSync(treeSize, record, lowerLevel + 1, chunkThreshold, outFileName);
            }
        }
    }

    return 0;
}