//
// Created by phmoll on 1/27/20.
//

#include "P2PModeSyncClient.h"

void quadtree::P2PModeSyncClient::startSynchronization()
{
    spdlog::info("Initialize P2PModeSyncClient");
    // ndn::Interest::setDefaultCanBePrefix(true);

    // Inflate all subtrees on the initial request level
    for (int i = 0; i < pow(4, initialRequestLevel); i++) {
        world.inflateSubtree(initialRequestLevel + 1, i);
    }

    ownSubtree = world.getSubtree(responsibleArea);
    auto requestableTrees = world.getTreeCoverageBasedOnRectangle(responsibleArea, initialRequestLevel + 1);
    for (const auto& subtree : requestableTrees) {
        spdlog::debug("Start requestor for area: {}", subtree->getArea().to_string());
        if (subtree != nullptr && subtree != ownSubtree) {
            remoteSyncTrees.push_back(subtree);
        }
    }

    // This thread applies changes from CSV file, NOT the producer
    this->publisherThread = std::thread(&P2PModeSyncClient::applyChangesOverTime, this);

    // Producer listening to requests from subtree
    SyncTree* currentRegion = this->ownSubtree;
    do {
        ndn::Name ownRegionName(worldPrefix);
        ndn::Name subtreeName = currentRegion->subtreeToName();
        ownRegionName.append(subtreeName);

        this->face.setInterestFilter(ownRegionName,
            std::bind(&P2PModeSyncClient::onSubtreeSyncRequestReceived, this, _1, _2),
            ndn::RegisterPrefixSuccessCallback(), std::bind(&P2PModeSyncClient::onRegisterFailed, this, _1, _2));
        spdlog::info("P2PModeSyncClient Started -- Zone:" + ownRegionName.toUri());
        currentRegion = currentRegion->getParent();
    } while (currentRegion->getLevel() > 1);

    // Start process which requests changes from remote servers
    for (SyncTree* remoteRegion : this->remoteSyncTrees) {
        std::thread consumerThread = std::thread(&P2PModeSyncClient::synchronizeRemoteRegion, this, remoteRegion);
        this->consumerthreads.push_back(std::move(consumerThread));
    }

    // This method should be blocking -- calling Face.processEvents
    while (this->isRunning) {
        this->face.processEvents();
        usleep(1000);
    }
}

void quadtree::P2PModeSyncClient::storeLogValues()
{
    std::ofstream logfile = std::ofstream(logFolder + logFilePrefix + "_stats.txt");
    logfile << "received_chunk_responses: " << received_chunk_responses << std::endl;
    logfile << "received_subtree_responses: " << received_subtree_responses << std::endl;
    logfile << "received_unknown_hash_responses: " << received_unknown_hash_responses << std::endl;
    logfile.flush();
    logfile.close();
}

void storeParameters(std::string logDir, std::string responsibilityArea, int treeSize, std::string traceFile)
{
    std::ofstream logfile = std::ofstream(logDir + "/" + responsibilityArea + "_settings.txt");
    logfile << "[Parameters]" << std::endl;
    logfile << "logDir:\t" << logDir << std::endl;
    logfile << "responsiblityArea:\t" << responsibilityArea << std::endl;
    logfile << "treeSize:\t" << treeSize << std::endl;
    logfile << "traceFile:\t" << traceFile << std::endl;
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
        ("logDir", po::value<std::string>()->default_value("logs"), "Directory where log output is stored")
        ("traceFile", po::value<std::string>()->default_value("../QuadTreeRMAComparison/max_distance/ChunkChanges-very-distributed.csv"), "File where chunk changes are located")
        ("levelDifference", po::value<int>(&opt)->default_value(2), "How many levels to go deeper for respones with high number of chunk changes")
        ("chunkThreshold", po::value<int>(&opt)->default_value(200), "The maximum amount of chunks included in a sync response");
    /* clang-format on */

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return EXIT_FAILURE;
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
    std::string logDir = vm["logDir"].as<std::string>();
    std::string traceFile = vm["traceFile"].as<std::string>();
    int chunkThreshold = vm["chunkThreshold"].as<int>();
    int levelDifference = vm["levelDifference"].as<int>();

    storeParameters(logDir, responsibilityAreaString, treeSize, traceFile);

    // Parse CSV File
    auto changesOverTime = ChunkFileReader::readChangesOverTime(traceFile, treeSize);

    // Create Sync Client
    quadtree::Rectangle world(quadtree::Point(0, 0), quadtree::Point(treeSize, treeSize));
    quadtree::Rectangle responsibility(quadtree::Point(std::stoi(coordinates[0]), std::stoi(coordinates[1])),
        quadtree::Point(std::stoi(coordinates[2]), std::stoi(coordinates[3])));

    quadtree::P2PModeSyncClient client("/world", world, responsibility, 3, changesOverTime, logDir,
        responsibilityAreaString, levelDifference, chunkThreshold);
    client.startSynchronization();
}