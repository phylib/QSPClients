//
// Created by phmoll on 1/27/20.
//
#include "spdlog/spdlog.h"
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <chrono>
#include <thread>

#include "ServerModeSyncClient.h"
#include "csv/CSVReader.h"

using namespace std::literals;
namespace po = boost::program_options;

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

void storeParameters(std::string logDir, std::string responsibilityArea, int treeSize, int requestLevel,
    std::string traceFile, std::string prefix)
{
    std::ofstream logfile = std::ofstream(logDir + "/SyncClientSettings.txt");
    logfile << "[Parameters]" << std::endl;
    logfile << "logDir:\t" << logDir << std::endl;
    logfile << "responsiblityArea:\t" << responsibilityArea << std::endl;
    logfile << "treeSize:\t" << treeSize << std::endl;
    logfile << "requestLevel:\t" << requestLevel << std::endl;
    logfile << "traceFile:\t" << traceFile << std::endl;
    logfile << "prefix:\t" << prefix << std::endl;
    logfile.flush();
    logfile.close();
}

int main(int argc, char* argv[])
{
    spdlog::set_level(spdlog::level::trace);

    po::options_description desc("Usage");
    int opt;
    desc.add_options()("help", "produce help message")("responsiblityArea", po::value<std::string>(),
        "Set the responsibility area of the server. String in form of x1,y1,x2,y2")(
        "treeSize", po::value<int>(&opt)->default_value(65536), "set the id of the current server")(
        "requestLevel", po::value<int>(&opt)->default_value(1), "In which level of the quadtree are requests sent")(
        "logDir", po::value<std::string>()->default_value("logs"), "Directory where log output is stored")("traceFile",
        po::value<std::string>()->default_value(
            "../QuadTreeRMAComparison/max_distance/ChunkChanges-very-distributed.csv"),
        "File where chunk changes are located")(
        "prefix", po::value<std::string>()->default_value("/world"), "Application specific prefix");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }

    std::string responsibilityAreaString;
    if (vm.count("responsiblityArea")) {
        responsibilityAreaString = vm["responsiblityArea"].as<std::string>();
    } else {
        std::cout << desc << std::endl;
        exit(-1);
    }
    // Parse responsiblityCoordinates
    std::vector<std::string> coordinates;
    boost::split(coordinates, responsibilityAreaString, boost::is_any_of(","));
    if (coordinates.size() != 4) {
        std::cout << "Invalid responsibility area!" << std::endl;
        std::cout << desc << std::endl;
        exit(-1);
    }

    unsigned treeSize = vm["treeSize"].as<int>();
    unsigned initialRequestLevel = vm["requestLevel"].as<int>();
    std::string logDir = vm["logDir"].as<std::string>();
    std::string traceFile = vm["traceFile"].as<std::string>();
    std::string prefix = vm["prefix"].as<std::string>();

    storeParameters(logDir, responsibilityAreaString, treeSize, initialRequestLevel, traceFile, prefix);

    // Parse CSV File
    auto changesOverTime = readChangesOverTime(traceFile, treeSize);

    // Create Sync Client
    quadtree::Rectangle world(quadtree::Point(0, 0), quadtree::Point(treeSize, treeSize));
    quadtree::Rectangle responsibility(quadtree::Point(std::stoi(coordinates[0]), std::stoi(coordinates[1])),
        quadtree::Point(std::stoi(coordinates[2]), std::stoi(coordinates[3])));

    quadtree::ServerModeSyncClient client(prefix, world, responsibility, initialRequestLevel, changesOverTime,
        logDir + "/Testlog_" + responsibilityAreaString + ".csv");

    // Start Sync Client
    try {
        client.startSynchronization();
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
}