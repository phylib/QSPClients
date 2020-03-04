//
// Created by phmoll on 2/3/20.
//

#include "ChunkLogger.h"

void ChunkLogger::logChunkUpdateProduced(const quadtree::Chunk& chunk)
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    long millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    {
        std::unique_lock<std::mutex> lck(this->fileMutex);
        logfile << millis << sep << "OUT" << sep << chunk.pos.x << sep << chunk.pos.y << sep << chunk.data << std::endl;
    }
}

void ChunkLogger::logChunkUpdateReceived(const quadtree::Chunk& chunk, long timestamp)
{
    {
        std::unique_lock<std::mutex> lck(this->fileMutex);
        logfile << timestamp << sep << "IN" << sep << chunk.pos.x << sep << chunk.pos.y << sep << chunk.data
                << std::endl;
    }
}
