//
// Created by phmoll on 2/3/20.
//

#ifndef QUADTREESYNCEVALUATION_CHUNKLOGGER_H
#define QUADTREESYNCEVALUATION_CHUNKLOGGER_H

#include "QuadTreeStructs.h"

#include <boost/chrono/chrono.hpp>

#include <chrono>
#include <iostream>
#include <fstream>

class ChunkLogger {

public:
    explicit ChunkLogger(std::string logfilename, std::string sep="\t")
        : logfilename(std::move(logfilename))
        , sep(std::move(sep))
    {
        logfile = std::ofstream(this->logfilename);
    }

    ~ChunkLogger()
    {
        if (this->logfile.is_open()) {
            this->logfile.flush();
            this->logfile.close();
        }
    }

    void logChunkUpdateProduced(const quadtree::Chunk& chunk);

    void logChunkUpdateReceived(const quadtree::Chunk& chunk, long timestamp);

protected:
    std::string logfilename;
    std::ofstream logfile;
    std::string sep;
};

#endif // QUADTREESYNCEVALUATION_CHUNKLOGGER_H
