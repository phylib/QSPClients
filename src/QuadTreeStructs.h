//
// Created by phmoll on 7/22/19.
//

#ifndef QUADTREESYNCEVALUATION_QUADTREESTRUCTS_H
#define QUADTREESYNCEVALUATION_QUADTREESTRUCTS_H

#include <boost/functional/hash.hpp>

namespace quadtree {

    struct Point {
        int x;
        int y;

        Point(int _x, int _y) {
            x = _x;
            y = _y;
        }

        Point(const Point &_p2)
                : x(_p2.x), y(_p2.y) {
        }

        Point() {

        }

        static bool comparePoints(Point p1, Point p2) {
            if (p1.y == p2.y) {
                return p1.x < p2.x;
            }
            return p1.y < p2.y;
        }

    };

    struct Chunk {
        Point pos;
        int data;

        Chunk(Point _pos, int _data)
                : pos(_pos) {
            data = _data;
        }

        Chunk()
                : pos(0, 0) {
            data = 0;
        }

        std::size_t hashChunk() {
            std::size_t seed = 0;
            boost::hash_combine(seed, pos.x);
            boost::hash_combine(seed, pos.y);
            boost::hash_combine(seed, data);
            return seed;
        }

        static bool compareChunks(Chunk c1, Chunk c2) {
            return Point::comparePoints(c1.pos, c2.pos);
        }

        static bool chunkCoordMatch(Chunk c1, Chunk c2) {
            return c1.pos.x == c2.pos.x && c1.pos.y == c2.pos.y;
        }
    };

    struct ChangeResponse {
        std::string path;
        std::size_t currentHash;
        bool delta;
        std::vector<Chunk> changeVector;

        ChangeResponse() : path(""), currentHash(0), delta(false), changeVector() {}

        ChangeResponse(const std::string path, size_t currentHash)
                : path(path), currentHash(currentHash), delta(false), changeVector() {}

        bool isEmpty() {
            return currentHash == 0 && changeVector.empty();
        }
    };

    struct Rectangle {
        Point topleft;
        Point bottomRight;

        Rectangle(const Point &topleft, const Point &bottomRight) : topleft(topleft), bottomRight(bottomRight) {
        }

        Rectangle() = default;
    };

}


#endif //QUADTREESYNCEVALUATION_QUADTREESTRUCTS_H
