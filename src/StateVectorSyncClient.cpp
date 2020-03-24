//
// Created by phmoll on 3/5/20.
//

#include "StateVectorSyncClient.h"

void quadtree::StateVectorSyncClient::applyChangesOverTime()
{

    auto nextChangePublication
        = std::chrono::system_clock::now() + std::chrono::milliseconds(StateVectorSyncClient::SLEEP_TIME_MS);
    for (unsigned i = 0; i < this->changesOverTime.size(); i++) {
        auto tickChunkPair = this->changesOverTime[i];
        this->currentTick = tickChunkPair.first;
        spdlog::info("Apply changes of tick " + std::to_string(this->currentTick));

        std::vector<Point> changedPoints;
        for (const auto& chunk : tickChunkPair.second) {
            changedPoints.push_back(chunk.pos);
        }
        submitChange(changedPoints);

        std::this_thread::sleep_until(nextChangePublication);
        nextChangePublication += std::chrono::milliseconds(StateVectorSyncClient::SLEEP_TIME_MS);
    }
    spdlog::info("All changes applied, killing application");

    storeLogValues();

    // Close application when trace ended
    this->isRunning = false;
    usleep(1000);
    //    this->face.shutdown();
    exit(0);
}
void quadtree::StateVectorSyncClient::submitChange(const std::vector<quadtree::Point>& changedPoints)
{
    std::vector<Chunk> changedChunks;

    // Only apply changes in own area
    std::vector<quadtree::Point> ownPoints;
    for (const Point& changedPoint : changedPoints) {
        if (responsibleArea.isPointInRectangle(changedPoint)) {
            ownPoints.push_back(changedPoint);
        }
    }

    {
        std::unique_lock<std::mutex> lck(this->localDataAccessMutex);

        for (const Point& changedPoint : ownPoints) {
            // Change the area in the local data store
            Chunk changedChunk(changedPoint, 0);
            if (knownPoints.find(changedPoint) == knownPoints.end()) {
                knownPoints[changedPoint] = 0;
            } else {
                int version = knownPoints[changedPoint];
                version++;
                changedChunk.data = version;
                knownPoints[changedPoint] = version;
            }
            logger.logChunkUpdateProduced(changedChunk, ownPoints.size());
            changedChunks.push_back(changedChunk);
        }
    } // Release lock here

    if (changedChunks.empty()) { // No data needs to be sent when nothing changed
        return;
    }

    // Serialize changed chunks with protobuf
    quadtreesync::ChunkChanges chunkChanges;
    chunkChanges.set_hashknown(false);
    for (const Chunk& chunk : changedChunks) {
        quadtreesync::Chunk protoChunk;
        protoChunk.set_x(chunk.pos.x);
        protoChunk.set_y(chunk.pos.y);
        protoChunk.set_data(chunk.data);
        chunkChanges.mutable_chunks()->Add()->CopyFrom(protoChunk);
    }
    std::string data = chunkChanges.SerializeAsString();
    std::string compressed = GZip::compress(data);
    //    spdlog::trace(compressed);
    spdlog::debug("Publish packet of size: " + std::to_string(compressed.size()) + ", "
        + std::to_string(chunkChanges.chunks_size()) + " chunk updates");
    //    std::string testMessage("Publish " + std::to_string(changedChunks.size()) + " packets");
    //    spdlog::trace(testMessage);
    svs.publishMsg(compressed);
}

void quadtree::StateVectorSyncClient::onSyncResponseReceived(
    const ndn::svs::NodeID& senderNodeId, const std::string& msg)
{

    // Todo: Log how many subtree responses, chunk responses and hash unknown responses were received

    // Parse received msg
    spdlog::debug(
        "Received sync update of node " + std::to_string(senderNodeId) + " of size: " + std::to_string(msg.size()));
    //    spdlog::trace("Message: " + msg);
    // Todo: Verify signature

    // Todo: Decrypt packet

    // unzip und unserialize data
    //    spdlog::trace("try to unzip packet: " + msg);
    std::string decompressed = GZip::decompress(msg);
    quadtreesync::ChunkChanges chunkChanges;
    chunkChanges.ParseFromString(decompressed);
    spdlog::trace("Received sync update contained " + std::to_string(chunkChanges.chunks_size()) + " chunk changes");

    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    long millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

    // Apply the sync response to the local data store
    {
        std::unique_lock<std::mutex> lck(this->localDataAccessMutex);

        for (const auto& protoChunk : chunkChanges.chunks()) {
            Point p(protoChunk.x(), protoChunk.y());
            Chunk receivedChunk(p, protoChunk.data());
            logger.logChunkUpdateReceived(receivedChunk, millis);

            if (knownPoints.find(p) == knownPoints.end()) {
                knownPoints[p] = receivedChunk.data;
            } else {
                unsigned version = knownPoints[p];
                if (version < (unsigned)receivedChunk.data) {
                    knownPoints[p] = receivedChunk.data;
                }
            }
        }
    } // Release lock here
}

void quadtree::StateVectorSyncClient::storeLogValues()
{
    std::ofstream logfile = std::ofstream(logFolder + logFilePrefix + "_stats.txt");
    // Todo: Decide on which data is important here
    // logfile << "received_chunk_responses: " << received_chunk_responses << std::endl;
    // logfile << "received_subtree_responses: " << received_subtree_responses << std::endl;
    // logfile << "received_unknown_hash_responses: " << received_unknown_hash_responses << std::endl;
    logfile.flush();
    logfile.close();
}
void quadtree::StateVectorSyncClient::startSynchronization()
{
    // This thread applies changes from CSV file, NOT the producer
    this->publisherThread = std::thread(&StateVectorSyncClient::applyChangesOverTime, this);

    // Producer listening to requests from subtree
    svs.registerPrefix();

    // Create other thread to run
    std::thread thread_svs([this] { svs.run(); });

    spdlog::info("StateVectorSyncClient Started -- SyncPrefix:" + std::to_string(nodeId));

    // This method should be blocking -- calling Face.processEvents
    thread_svs.join();
    this->publisherThread.join();
}

// ----------------- METHODS REQURIRED FOR MAIN ------------------------

void storeParameters(
    std::string logDir, std::string responsibilityArea, int treeSize, std::string traceFile, int clientId)
{
    std::ofstream logfile = std::ofstream(logDir + "/" + responsibilityArea + "_settings.txt");
    logfile << "[Parameters]" << std::endl;
    logfile << "logDir:\t" << logDir << std::endl;
    logfile << "responsiblityArea:\t" << responsibilityArea << std::endl;
    logfile << "treeSize:\t" << treeSize << std::endl;
    logfile << "traceFile:\t" << traceFile << std::endl;
    logfile << "clientId:\t" << clientId << std::endl;
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
        ("clientId", po::value<int>(), "ID of the client (used as sync prefix)")
        ("treeSize", po::value<int>(&opt)->default_value(65536), "set the id of the current server")
        ("logDir", po::value<std::string>()->default_value("logs"), "Directory where log output is stored")
        ("traceFile", po::value<std::string>()->default_value("../QuadTreeRMAComparison/max_distance/ChunkChanges-very-distributed.csv"), "File where chunk changes are located");
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

    int clientId;
    if (vm.count("clientId")) {
        clientId = vm["clientId"].as<int>();
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

    storeParameters(logDir, responsibilityAreaString, treeSize, traceFile, clientId);

    // Parse CSV File
    auto changesOverTime = ChunkFileReader::readChangesOverTime(traceFile, treeSize);

    // Create Sync Client
    quadtree::Rectangle world(quadtree::Point(0, 0), quadtree::Point(treeSize, treeSize));
    quadtree::Rectangle responsibility(quadtree::Point(std::stoi(coordinates[0]), std::stoi(coordinates[1])),
        quadtree::Point(std::stoi(coordinates[2]), std::stoi(coordinates[3])));

    quadtree::StateVectorSyncClient client(
        world, responsibility, clientId, changesOverTime, logDir, std::to_string(clientId));
    client.startSynchronization();
}