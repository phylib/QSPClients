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
#include "csv/ChunkFileReader.h"

using namespace std::literals;
namespace po = boost::program_options;

void storeParameters(std::string logDir, std::string responsibilityArea, int treeSize, int requestLevel,
    std::string traceFile, std::string prefix, int chunkThreshold, int levelDifference, int syncRequestInterval)
{
    std::ofstream logfile = std::ofstream(logDir + "/" + responsibilityArea + "_settings.txt");
    logfile << "[Parameters]" << std::endl;
    logfile << "logDir:\t" << logDir << std::endl;
    logfile << "responsiblityArea:\t" << responsibilityArea << std::endl;
    logfile << "treeSize:\t" << treeSize << std::endl;
    logfile << "requestLevel:\t" << requestLevel << std::endl;
    logfile << "traceFile:\t" << traceFile << std::endl;
    logfile << "prefix:\t" << prefix << std::endl;
    logfile << "chunkThreshold:\t" << chunkThreshold << std::endl;
    logfile << "levelDifference:\t" << levelDifference << std::endl;
    logfile << "syncRequestInterval:\t" << syncRequestInterval << std::endl;
    logfile.flush();
    logfile.close();
}

int main(int argc, char* argv[])
{
    spdlog::set_level(spdlog::level::trace);

    po::options_description desc("Usage");
    int opt;
    /* clang-format off */
    desc.add_options()
        ("help", "produce help message")
        ("responsiblityArea", po::value<std::string>(), "Set the responsibility area of the server. String in form of x1,y1,x2,y2")
        ("treeSize", po::value<int>(&opt)->default_value(65536), "set the id of the current server")
        ("requestLevel", po::value<int>(&opt)->default_value(1), "In which level of the quadtree are requests sent")
        ("logDir", po::value<std::string>()->default_value("logs"), "Directory where log output is stored")
        ("traceFile", po::value<std::string>()->default_value("../QuadTreeRMAComparison/max_distance/ChunkChanges-very-distributed.csv"), "File where chunk changes are located")
        ("prefix", po::value<std::string>()->default_value("/world"), "Application specific prefix")
        ("levelDifference", po::value<int>(&opt)->default_value(2), "How many levels to go deeper for respones with high number of chunk changes")
        ("chunkThreshold", po::value<int>(&opt)->default_value(200), "The maximum amount of chunks included in a sync response")
        ("syncRequestInterval", po::value<int>(&opt)->default_value(500), "Interval in which sync requests are sent");
    /* clang-format on */

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
    int chunkThreshold = vm["chunkThreshold"].as<int>();
    int levelDifference = vm["levelDifference"].as<int>();
    int syncRequestInterval = vm["syncRequestInterval"].as<int>();

    storeParameters(logDir, responsibilityAreaString, treeSize, initialRequestLevel, traceFile, prefix, chunkThreshold,
        levelDifference, syncRequestInterval);

    // Parse CSV File
    auto changesOverTime = ChunkFileReader::readChangesOverTime(traceFile, treeSize);

    // Create Sync Client
    quadtree::Rectangle world(quadtree::Point(0, 0), quadtree::Point(treeSize, treeSize));
    quadtree::Rectangle responsibility(quadtree::Point(std::stoi(coordinates[0]), std::stoi(coordinates[1])),
        quadtree::Point(std::stoi(coordinates[2]), std::stoi(coordinates[3])));

    quadtree::ServerModeSyncClient client(prefix, world, responsibility, initialRequestLevel, changesOverTime,
        logDir + "/", responsibilityAreaString, levelDifference, chunkThreshold, syncRequestInterval);

    // Start Sync Client
    try {
        client.startSynchronization();
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
}