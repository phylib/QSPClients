//
// Created by phmoll on 7/22/19.
//
#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "../src/QuadTree.h"

bool testBounds(std::pair<Point, Point> bounds, int x1, int y1, int x2, int y2) {
    return bounds.first.x == x1 && bounds.first.y == y1
           && bounds.second.x == x2 && bounds.second.y == y2;
}

bool comparePoints(Point p1, Point p2) {
    return p1.x == p2.x && p1.y == p2.y;
}

TEST_CASE("QuadTree initialization and find element", "[quadTree]") {

    SECTION("A simple 2x2 tree (no subtrees)") {
        QuadTree quadTree2x2(Point(0, 0), Point(2, 2), 1);

        REQUIRE(quadTree2x2.isInMaxLevel());

        REQUIRE(quadTree2x2.getChunk(Point(0, 0)) != nullptr);
        REQUIRE(quadTree2x2.getChunk(Point(0, 1)) != nullptr);
        REQUIRE(quadTree2x2.getChunk(Point(1, 0)) != nullptr);
        REQUIRE(quadTree2x2.getChunk(Point(1, 1)) != nullptr);

        REQUIRE(quadTree2x2.getChunk(Point(-1, 0)) == nullptr);
        REQUIRE(quadTree2x2.getChunk(Point(0, 3)) == nullptr);
        REQUIRE(quadTree2x2.getChunk(Point(2, 0)) == nullptr);
        REQUIRE(quadTree2x2.getChunk(Point(2, 1)) == nullptr);

        REQUIRE(!quadTree2x2.isChanged());

    }

    SECTION("Test a Quadtree with one level") {
        QuadTree quadTree4x4(Point(0, 0), Point(4, 4), 1);

        REQUIRE(!quadTree4x4.isInMaxLevel());

        REQUIRE(testBounds(quadTree4x4.topLeftTree->getBounds(), 0, 0, 2, 2));
        REQUIRE(testBounds(quadTree4x4.topRightTree->getBounds(), 2, 0, 4, 2));
        REQUIRE(testBounds(quadTree4x4.botLeftTree->getBounds(), 0, 2, 2, 4));
        REQUIRE(testBounds(quadTree4x4.botRightTree->getBounds(), 2, 2, 4, 4));

        REQUIRE(quadTree4x4.getChunk(Point(0, 0)) != nullptr);
        REQUIRE(quadTree4x4.getChunk(Point(3, 3)) != nullptr);
        REQUIRE(quadTree4x4.getChunk(Point(3, 0)) != nullptr);
        REQUIRE(quadTree4x4.getChunk(Point(0, 3)) != nullptr);

        REQUIRE(quadTree4x4.getChunk(Point(-1, 0)) == nullptr);
        REQUIRE(quadTree4x4.getChunk(Point(0, 4)) == nullptr);
        REQUIRE(quadTree4x4.getChunk(Point(4, 0)) == nullptr);
        REQUIRE(quadTree4x4.getChunk(Point(4, 1)) == nullptr);

        REQUIRE(!quadTree4x4.isChanged());
    }

    SECTION("Test a Quadtree with negative coordinates") {
        QuadTree quadTree4x4(Point(-4, -4), Point(0, 0), 1);

        REQUIRE(!quadTree4x4.isInMaxLevel());

        REQUIRE(testBounds(quadTree4x4.topLeftTree->getBounds(), -4, -4, -2, -2));
        REQUIRE(testBounds(quadTree4x4.topRightTree->getBounds(), -2, -4, 0, -2));
        REQUIRE(testBounds(quadTree4x4.botLeftTree->getBounds(), -4, -2, -2, 0));
        REQUIRE(testBounds(quadTree4x4.botRightTree->getBounds(), -2, -2, 0, 0));

        REQUIRE(quadTree4x4.getChunk(Point(-4, -4)) != nullptr);
        REQUIRE(quadTree4x4.getChunk(Point(-3, -1)) != nullptr);
        REQUIRE(quadTree4x4.getChunk(Point(-1, -3)) != nullptr);
        REQUIRE(quadTree4x4.getChunk(Point(-3, -3)) != nullptr);

        REQUIRE(!quadTree4x4.isChanged());
    }
}

TEST_CASE("Change parts of a QuadTree", "[quadTree]") {

    SECTION("Test if changing chunks works") {
        QuadTree quadTree4x4(Point(0, 0), Point(4, 4), 1);

        REQUIRE(!quadTree4x4.isChanged());
        REQUIRE(!quadTree4x4.topLeftTree->isChanged());

        Chunk c(Point(1, 1), 3);
        auto changed = quadTree4x4.markChangedChunk(c);
        REQUIRE(changed->data == 3);
        REQUIRE(quadTree4x4.isChanged());
        REQUIRE(quadTree4x4.topLeftTree->isChanged());
        REQUIRE(!quadTree4x4.topRightTree->isChanged());
        REQUIRE(!quadTree4x4.botLeftTree->isChanged());
        REQUIRE(!quadTree4x4.botRightTree->isChanged());

        Chunk c2(Point(3, 1), 3);
        quadTree4x4.markChangedChunk(c2);
        REQUIRE(quadTree4x4.getChanges().size() == 2);
        REQUIRE(quadTree4x4.topLeftTree->getChanges().size() == 1);
        REQUIRE(quadTree4x4.topRightTree->getChanges().size() == 1);
    }

}

TEST_CASE("Test QuadTree Hash Functions") {

    SECTION("The hash of a tree must change when a chunk changes") {
        QuadTree quadTree(Point(0, 0), Point(4, 4), 1);

        size_t initial_hash = quadTree.hashQuadTree();
        REQUIRE(initial_hash == quadTree.hashQuadTree(true)); // Hash stays the same when rehashing

        quadTree.markChangedChunk(Chunk(Point(0, 2), 3));
        quadTree.hashQuadTree();
        REQUIRE(initial_hash != quadTree.getHash());
    }

    SECTION("The hash of an unchanged subtree must not change when a chunk changes") {
        QuadTree quadTree(Point(0, 0), Point(4, 4), 1);

        size_t tl = quadTree.topLeftTree->hashQuadTree();
        size_t tr = quadTree.topRightTree->hashQuadTree();

        quadTree.markChangedChunk(Chunk(Point(3, 0), 3));
        quadTree.hashQuadTree();

        REQUIRE(tl == quadTree.topLeftTree->getHash());
        REQUIRE(tr != quadTree.topRightTree->getHash());
    }

    SECTION("The hash of two identical trees should be the same") {

        QuadTree tree1(Point(0, 0), Point(2, 2), 1);
        QuadTree tree2(Point(0, 0), Point(2, 2), 1);
        REQUIRE(tree1.hashQuadTree() == tree2.hashQuadTree());

        tree1.markChangedChunk(Chunk(Point(0, 2), 3));
        tree2.markChangedChunk(Chunk(Point(0, 2), 3));

        REQUIRE(tree1.hashQuadTree() == tree2.hashQuadTree());
    }

    SECTION("The changes of a QuadTree must be empty after rehashing") {
        QuadTree quadTree(Point(0, 0), Point(4, 4), 1);

        quadTree.markChangedChunk(Chunk(Point(3, 0), 3));
        quadTree.markChangedChunk(Chunk(Point(3, 3), 4));

        REQUIRE(quadTree.getChanges().size() == 2);

        quadTree.hashQuadTree();

        REQUIRE(quadTree.getChanges().size() == 0);
    }
}

TEST_CASE("Test the HashStorage", "[HashStorage]") {

    SECTION("Insert data and check if the right points are stored") {
        HashStorage storage;

        QuadTree quadTree(Point(0, 0), Point(4, 4), 1);
        quadTree.setHashStorage(storage);

        size_t rh = quadTree.getHash();
        size_t trch = quadTree.topRightTree->getHash();

        quadTree.markChangedChunk(Chunk(Point(3, 0), 3));
        quadTree.hashQuadTree();

        // Check if 3,0 is stored for root
        Chunk changed = *(storage.get(rh).second.begin());
        REQUIRE(comparePoints(changed.pos, Point(3, 0)));

        // Check if 3,0 is stored for child
        changed = *(storage.get(trch).second.begin());
        REQUIRE(comparePoints(changed.pos, Point(3, 0)));
        // No entry for childs without change should exist
        REQUIRE(!storage.exists(quadTree.botLeftTree->getHash()));
        REQUIRE(!storage.exists(quadTree.botRightTree->getHash()));

    }

    SECTION("Old hash value should be invalidated when new hash is stored") {
        HashStorage storage;

        QuadTree quadTree(Point(0, 0), Point(4, 4), 1);
        quadTree.setHashStorage(storage);

        size_t rh1 = quadTree.getHash();
        size_t trch1 = quadTree.topRightTree->getHash();

        quadTree.markChangedChunk(Chunk(Point(3, 0), 2));
        quadTree.hashQuadTree();

        size_t rh2 = quadTree.getHash();
        size_t trch2 = quadTree.topRightTree->getHash();

        quadTree.markChangedChunk(Chunk(Point(3, 0), 2));
        quadTree.hashQuadTree();

        REQUIRE(!storage.exists(rh1));
        REQUIRE(!storage.exists(trch1));

        REQUIRE(storage.exists(rh2));
        REQUIRE(storage.exists(trch2));
    }


}