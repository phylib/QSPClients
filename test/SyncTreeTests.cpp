//
// Created by phmoll on 11/15/19.
//
#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include <math.h>

#include "../src/SyncTree.h"

using namespace quadtree;

TEST_CASE("Basic SyncTree Structure and function", "[SyncTree]")
{

    SECTION("Constructor and Destructor")
    {

        {
            // A tree with maxlevel 0 and decruction afterwards
            Rectangle rectangle(Point(0, 0), Point(2, 2));
            SyncTree* tree = new SyncTree(rectangle);
            REQUIRE(tree->finalLevel());
            REQUIRE(tree->countInflatedNodes() == 1);
            delete (tree);
            tree = nullptr;
            REQUIRE(tree == nullptr);
        }

        {
            // A tree with maxlevel 1 and destruction afterwards
            Rectangle rectangle(Point(0, 0), Point(4, 4));
            SyncTree tree(rectangle);
            REQUIRE(!tree.finalLevel());
            REQUIRE(tree.countInflatedNodes() == 1);
        }
    }

    SECTION("Valid and invalid tree dimensions")
    {

        // Initializing sync trees with a size of 2^n is valid

        unsigned n = 4;
        Point p1(0, 0);
        Point p2(n, n);
        Rectangle rect(p1, p2);
        REQUIRE_NOTHROW(SyncTree(rect));

        n = 4;
        p1 = Point(0, 0);
        p2 = Point(n, n);
        rect = Rectangle(p1, p2);
        REQUIRE_NOTHROW(SyncTree(rect));

        n = 16;
        p1 = Point(0, 0);
        p2 = Point(n, n);
        rect = Rectangle(p1, p2);
        REQUIRE_NOTHROW(SyncTree(rect));

        n = 65536;
        p1 = Point(0, 0);
        p2 = Point(n, n);
        rect = Rectangle(p1, p2);
        REQUIRE_NOTHROW(SyncTree(rect));

        // Initializing with other values than that should raise exceptions

        n = 10;
        p1 = Point(0, 0);
        p2 = Point(n, n);
        rect = Rectangle(p1, p2);
        REQUIRE_THROWS(SyncTree(rect));

        n = 12;
        p1 = Point(0, 0);
        p2 = Point(n, n);
        rect = Rectangle(p1, p2);
        REQUIRE_THROWS(SyncTree(rect));

        n = 60000;
        p1 = Point(0, 0);
        p2 = Point(n, n);
        rect = Rectangle(p1, p2);
        REQUIRE_THROWS(SyncTree(rect));

        // Also, non square areas are not allowed
        n = 16;
        p1 = Point(0, 0);
        p2 = Point(n, n + 2);
        rect = Rectangle(p1, p2);
        REQUIRE_THROWS(SyncTree(rect));
    }
}

SCENARIO("An area can be covered by a SyncTree and storedChanges can be made")
{

    GIVEN("A 8x8 area is covered by an empty SyncTree")
    {

        unsigned treeDimension = 8;
        Rectangle rectangle(Point(0, 0), Point(treeDimension, treeDimension));
        SyncTree tree(rectangle);

        REQUIRE(tree.countInflatedNodes() == 1);
        const Rectangle& area = tree.getArea();
        REQUIRE(area.bottomRight.x - area.topleft.x == treeDimension);
        REQUIRE(area.bottomRight.y - area.topleft.y == treeDimension);

        WHEN("A single chunk storedChanges")
        {
            Chunk* c = tree.change(2, 1);

            THEN("its version has to be 1")
            {
                REQUIRE(c->pos.x == 2);
                REQUIRE(c->pos.y == 1);

                REQUIRE(c->data == 1);
            }

            THEN("4 nodes must be inflated.") { REQUIRE(tree.countInflatedNodes() == 4); }
        }

        WHEN("When all chunks of the tree are inflated")
        {
            // Inflate all subtrees
            for (int i = 0; i < treeDimension; i++) {
                for (int j = 0; j < treeDimension; j++) {
                    tree.change(i, j);
                }
            }
            THEN("85 nodes have to be inflated") { REQUIRE(tree.countInflatedNodes() == 85); }
        }

        WHEN("A single chunk storedChanges twice")
        {
            Chunk* c = tree.change(2, 1);
            tree.change(2, 1);

            THEN("the chunk returned by the first change call should have an version of 2") { REQUIRE(c->data == 2); }
        }

        WHEN("nothing changed")
        {
            THEN("None of the subtree hashes should be 0") { REQUIRE(tree.getHash() != 0); }
        }
    }

    GIVEN("A 65536x65536 area is covered by an empty SyncTree")
    {

        unsigned treeDimension = 65536;
        Rectangle rectangle(Point(0, 0), Point(treeDimension, treeDimension));
        SyncTree tree(rectangle);

        REQUIRE(tree.countInflatedNodes() == 1);
        const Rectangle& area = tree.getArea();
        REQUIRE(area.bottomRight.x - area.topleft.x == treeDimension);
        REQUIRE(area.bottomRight.y - area.topleft.y == treeDimension);

        WHEN("Chunk 25000,1200 changes")
        {
            Chunk* c = tree.change(25000, 1200);

            THEN("the number of inflated nodes should be log(60000, 2) + 1")
            {
                REQUIRE(tree.countInflatedNodes() == ceil(log2(treeDimension)) + 1);
            }
        }

        WHEN("Chunk 2,1 changes")
        {
            Chunk* c = tree.change(2, 1);

            THEN("the number of inflated nodes should be log(60000, 2) + 1")
            {
                REQUIRE(tree.countInflatedNodes() == ceil(log2(treeDimension)) + 1);
            }
        }

        WHEN("Subtree 0,0 - 1,1 is requested")
        {
            THEN("An exception should be thrown because the dimension is invalid")
            {
                Point p0(0, 0);
                Point p1(1, 1);
                Rectangle r(p0, p1);
                REQUIRE_THROWS(tree.getSubtree(r));
            }
        }

        WHEN("Subtree 65536,65536 - 65538,65538 is requested")
        {
            THEN("An exception should be thrown because the rectangle is not part of the tree")
            {
                Point p0(65536, 65536);
                Point p1(65538, 65538);
                Rectangle r(p0, p1);
                REQUIRE_THROWS(tree.getSubtree(r));
            }
        }

        WHEN("Subtree 0,0 - 4,4 is requested")
        {
            Point p0(0, 0);
            Point p1(4, 4);
            Rectangle r(p0, p1);
            SyncTree* subtree = tree.getSubtree(r);

            THEN("The returned subtree should be null since it has not been inflated") { REQUIRE(subtree == nullptr); }
        }
        WHEN("chunk 5,5 changed and subtree 4,4 - 8,8 is requested")
        {
            tree.change(5, 5);

            Point p0(4, 4);
            Point p1(8, 8);
            Rectangle r(p0, p1);
            SyncTree* subtree = tree.getSubtree(r);

            THEN("The returned subtree should be valid and should have 3 inflated nodes")
            {
                REQUIRE(subtree != nullptr);
                REQUIRE(subtree->getArea().topleft.x == 4);
                REQUIRE(subtree->getArea().topleft.y == 4);
                REQUIRE(subtree->getArea().bottomRight.x == 8);
                REQUIRE(subtree->getArea().bottomRight.y == 8);

                REQUIRE(subtree->countInflatedNodes() == 3);
            }
        }
    }
}

SCENARIO("Test hash functions of the sync tree")
{

    GIVEN("A 65536x65536 area is covered by an empty SyncTree")
    {
        unsigned treeDimension = 65536;
        Rectangle rectangle(Point(0, 0), Point(treeDimension, treeDimension));
        SyncTree tree(rectangle);

        REQUIRE(tree.countInflatedNodes() == 1);
        const Rectangle& area = tree.getArea();
        REQUIRE(area.bottomRight.x - area.topleft.x == treeDimension);
        REQUIRE(area.bottomRight.y - area.topleft.y == treeDimension);
        std::size_t initial_hash = tree.getHash();

        WHEN("no change is performed")
        {

            THEN("rehashing should not change the hash of the tree")
            {

                tree.reHash();
                REQUIRE(initial_hash == tree.getHash());
                tree.reHash();
                REQUIRE(initial_hash == tree.getHash());
            }
        }

        WHEN("A single chunk changes")
        {

            Chunk* c = tree.change(2, 1);

            THEN("without calling rehash, the hash value should stay the same")
            {

                REQUIRE(initial_hash == tree.getHash());
            }

            THEN("after rehashing, the hash should have changed")
            {

                tree.reHash();
                REQUIRE(initial_hash != tree.getHash());
            }

            THEN("after rehashing, the changed chunk should be returned when querying storedChanges since the initial "
                 "hash")
            {
                tree.reHash();

                const std::pair<bool, std::vector<Chunk*>>& changes = tree.getChanges(initial_hash);
                REQUIRE(changes.first);
                REQUIRE(changes.second.size() == 1);
                Chunk* changedChunk = changes.second.at(0);
                REQUIRE(changedChunk->data == 1);
                REQUIRE(changedChunk->pos.x == 2);
                REQUIRE(changedChunk->pos.y == 1);
            }

            THEN("after rehashing, querying for changes with the current hash should return false")
            {
                tree.reHash();
                REQUIRE(!tree.getChanges(tree.getHash()).first);
            }
        }

        WHEN("A single chunk changes twice")
        {

            Chunk* c = tree.change(2, 1);
            c = tree.change(2, 1);

            THEN("after rehashing, only one change should be returned when querying storedChanges since the initial "
                 "hash")
            {

                tree.reHash();

                const std::pair<bool, std::vector<Chunk*>>& changes = tree.getChanges(initial_hash);
                REQUIRE(changes.first);
                REQUIRE(changes.second.size() == 1);

                Chunk* changedChunk = changes.second.at(0);
                REQUIRE(changedChunk->data == 2);
                REQUIRE(changedChunk->pos.x == 2);
                REQUIRE(changedChunk->pos.y == 1);
            }
        }

        WHEN("all quarters are inflated and changes occur only in the first quarter")
        {
            tree.change(0, 0);
            tree.change(60000, 60000);
            tree.change(60000, 0);
            tree.change(0, 60000);
            tree.reHash();

            size_t root_hash = tree.getHash();
            size_t first_q_hash = tree.getSubtree(Rectangle(Point(0, 0), Point(32768, 32768)))->getHash();
            size_t second_q_hash = tree.getSubtree(Rectangle(Point(0, 32768), Point(32768, 65536)))->getHash();
            size_t third_q_hash = tree.getSubtree(Rectangle(Point(32768, 0), Point(65536, 32768)))->getHash();
            size_t fourth_q_hash = tree.getSubtree(Rectangle(Point(32768, 32768), Point(65536, 65536)))->getHash();

            tree.change(10, 15);
            tree.change(1000, 1050);
            tree.change(645, 1050);
            tree.reHash();

            THEN("queries for the root of the tree return all changes")
            {
                std::pair<bool, std::vector<Chunk*>> changes = tree.getChanges(root_hash);
                REQUIRE(changes.first);
                REQUIRE(changes.second.size() == 3);
            }

            THEN("queries for the first quarter should return the changes")
            {

                const std::pair<bool, std::vector<Chunk*>>& changes
                    = tree.getChanges(first_q_hash, Rectangle(Point(0, 0), Point(32768, 32768)));
                REQUIRE(changes.first);
                REQUIRE(changes.second.size() == 3);
            }

            THEN("other quarters should not appear changed")
            {

                // second quarter
                std::pair<bool, std::vector<Chunk*>> changes
                    = tree.getChanges(second_q_hash, Rectangle(Point(0, 32768), Point(32768, 65536)));
                REQUIRE(!changes.first);

                // third quarter
                changes = tree.getChanges(third_q_hash, Rectangle(Point(32768, 0), Point(65536, 32768)));
                REQUIRE(!changes.first);

                // fourth quarter
                changes = tree.getChanges(fourth_q_hash, Rectangle(Point(32768, 32768), Point(65536, 65536)));
                REQUIRE(!changes.first);
            }
        }
    }
}