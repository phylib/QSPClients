//
// Created by phmoll on 7/22/19.
//

#ifndef QUADTREESYNCEVALUATION_QUADTREESTRUCTS_H
#define QUADTREESYNCEVALUATION_QUADTREESTRUCTS_H

#include <boost/functional/hash.hpp>
#include <ostream>

namespace quadtree {

struct Point {
    int x;
    int y;

    Point(int _x, int _y)
    {
        x = _x;
        y = _y;
    }

    Point(const Point& _p2)
        : x(_p2.x)
        , y(_p2.y)
    {
    }

    Point() {}

    static bool comparePoints(Point p1, Point p2)
    {
        if (p1.y == p2.y) {
            return p1.x < p2.x;
        }
        return p1.y < p2.y;
    }

    static bool equalPointCoords(Point p1, Point p2) { return p1.x == p2.x && p1.y == p2.y; }

    bool operator==(const Point& rhs) const { return x == rhs.x && y == rhs.y; }
    bool operator!=(const Point& rhs) const { return !(rhs == *this); }
    bool operator<(const Point& rhs) const
    {
        if (x < rhs.x) {
            return true;
        } else if (x == rhs.x && y < rhs.y) {
            return true;
        }
        return false;
    }

    friend std::ostream& operator<<(std::ostream& os, const Point& point)
    {
        os << "(x: " << point.x << " y: " << point.y << ")";
        return os;
    }
};

struct Chunk {
    Point pos;
    int data;

    Chunk(Point _pos, int _data)
        : pos(_pos)
    {
        data = _data;
    }

    Chunk()
        : pos(0, 0)
    {
        data = 0;
    }

    std::size_t hashChunk()
    {
        std::size_t seed = 0;
        boost::hash_combine(seed, pos.x);
        boost::hash_combine(seed, pos.y);
        boost::hash_combine(seed, data);
        return seed;
    }

    static bool compareChunks(Chunk c1, Chunk c2) { return Point::comparePoints(c1.pos, c2.pos); }

    static bool chunkCoordMatch(Chunk c1, Chunk c2) { return c1.pos.x == c2.pos.x && c1.pos.y == c2.pos.y; }

    friend std::ostream& operator<<(std::ostream& os, const Chunk& chunk)
    {
        os << "Chunk(pos: " << chunk.pos.x << "," << chunk.pos.y << " data: " << chunk.data << ")";
        return os;
    }
};

struct ChangeResponse {
    std::string path;
    std::size_t currentHash;
    bool delta;
    std::vector<Chunk> changeVector;

    ChangeResponse()
        : path("")
        , currentHash(0)
        , delta(false)
        , changeVector()
    {
    }

    ChangeResponse(const std::string path, size_t currentHash)
        : path(path)
        , currentHash(currentHash)
        , delta(false)
        , changeVector()
    {
    }

    bool isEmpty() { return currentHash == 0 && changeVector.empty(); }
};

struct Rectangle {
    Point topleft;
    Point bottomRight;

    Rectangle(const Point& topleft, const Point& bottomRight)
        : topleft(topleft)
        , bottomRight(bottomRight)
    {
    }

    Rectangle() = default;

    bool isPointInRectangle(const Point& p) const
    {
        return p.x >= this->topleft.x && p.x < this->bottomRight.x && p.y >= this->topleft.y
            && p.y < this->bottomRight.y;
    }

    bool isOverlapping(const Rectangle& r2) const
    {

        //            if (Point::equalPointCoords(this->topleft, r2.topleft) &&
        //                Point::equalPointCoords(this->bottomRight, r2.bottomRight)) {
        //                return true;
        //            }

        // If one rectangle is on left side of other
        if ((this->topleft.x >= r2.bottomRight.x) || (this->bottomRight.x <= r2.topleft.x)
            || (this->topleft.y >= r2.bottomRight.y) || (this->bottomRight.y <= r2.topleft.y))
            return false;

        return true;
    }

    bool operator==(const Rectangle& rhs) const { return topleft == rhs.topleft && bottomRight == rhs.bottomRight; }
    bool operator!=(const Rectangle& rhs) const { return !(rhs == *this); }

    friend std::ostream& operator<<(std::ostream& os, const Rectangle& rectangle)
    {
        os << "Area: [topleft: " << rectangle.topleft << " bottomRight: " << rectangle.bottomRight << "]";
        return os;
    }

    std::string to_string() const
    {
        std::ostringstream ss;
        ss << *this;
        return std::move(ss).str(); // enable efficiencies in c++17
    }
};

}

#endif // QUADTREESYNCEVALUATION_QUADTREESTRUCTS_H
