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

    SECTION("Test the max levels calculation") {
        Rectangle rectangle(Point(0, 0), Point(8, 8));
        SyncTree tree(rectangle);

        REQUIRE(tree.getMaxLevel() == 3);

        Rectangle largerRectangle(Point(0, 0), Point(16, 16));
        SyncTree largerTree(largerRectangle);
        largerTree.change(0,0); // Inflate subtree
        SyncTree* subtree = largerTree.getSubtree(rectangle);
        REQUIRE(subtree->getMaxLevel() == 4);
    }

    SECTION("Test the getChunkPath function")
    {

        Rectangle rectangle(Point(0, 0), Point(8, 8));
        SyncTree tree(rectangle);

        std::vector<unsigned char> path = tree.getChunkPath(0, 0);
        REQUIRE(path.size() == 3);
        REQUIRE(path.at(0) == 0);
        REQUIRE(path.at(1) == 0);
        REQUIRE(path.at(2) == 0);

        path = tree.getChunkPath(7, 7);
        REQUIRE(path.size() == 3);
        REQUIRE(path.at(0) == 3);
        REQUIRE(path.at(1) == 3);
        REQUIRE(path.at(2) == 3);

        path = tree.getChunkPath(4, 2);
        REQUIRE(path.size() == 3);
        REQUIRE(path.at(0) == 1);
        REQUIRE(path.at(1) == 2);
        REQUIRE(path.at(2) == 0);

        REQUIRE_THROWS(tree.getChunkPath(8, 3));
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
            for (unsigned i = 0; i < treeDimension; i++) {
                for (unsigned j = 0; j < treeDimension; j++) {
                    tree.change(i, j);
                }
            }
            THEN("85 nodes have to be inflated") { REQUIRE(tree.countInflatedNodes() == 85); }

            THEN("84 chunks have to be inflated") { REQUIRE(tree.countInflatedChunks() == 64); }
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

        WHEN("Chunk 0,0 and 2,2 changes")
        {
            tree.change(0, 0);
            tree.change(2, 2);

            THEN("In level 1, only one child should be inflated")
            {
                std::map<unsigned int, unsigned int> changedSubtrees = tree.countInflatedSubtreesPerLevel();
                REQUIRE(changedSubtrees[1] == 1);
            }

            THEN("In level 2, two childs should be inflated")
            {
                std::map<unsigned int, unsigned int> changedSubtrees = tree.countInflatedSubtreesPerLevel();
                REQUIRE(changedSubtrees[2] == 2);
            }
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

        //        WHEN("every single chunk in the tree changes")
        //        {
        //
        //            // Todo: Disable this test case
        //
        //            for (unsigned i = 0; i < treeDimension; i++) {
        //                for (unsigned j = 0; j < treeDimension; j++) {
        //                    tree.change(i, j);
        //                }
        //            }
        //            tree.reHash();
        //
        //            THEN("The number of changed chunks should be 65536x65536")
        //            {
        //                const std::pair<bool, std::vector<Chunk*>>& changes = tree.getChanges(initial_hash);
        //                REQUIRE(changes.first);
        //                REQUIRE(changes.second.size() == treeDimension * treeDimension);
        //            }
        //        }
    }
}

SCENARIO("Test function that delivers Hash-Values of N-Levels")
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

            THEN(
                "When requesting hash-values of the next 3 levels should deliver the non-empty root and 1 empty levels")
            {

                std::pair<std::map<unsigned, std::vector<size_t>>, int> result = tree.hashValuesOfNextNLevels(3, initial_hash);
                std::map<unsigned, std::vector<size_t>> hashValues = result.first;
                REQUIRE(hashValues.size() == 3);

                REQUIRE(hashValues[1].at(0) == initial_hash);
                for (int i = 0; i < 4; i++) {
                    REQUIRE(hashValues[2].at(i) == 0);
                }
                for (int i = 0; i < 16; i++) {
                    REQUIRE(hashValues[3].at(i) == 0);
                }
            }

            THEN("when enumerating 3 lower levels, 64 nullpointers shall be found") {
                int numNullptrs = 0;
                for (const auto subtree : tree.enumerateLowerLevel(3)) {
                    REQUIRE(subtree == nullptr);
                    numNullptrs++;
                }
                REQUIRE(numNullptrs == 64);
            }
        }

        WHEN("a change in the third quarter is performed")
        {
            tree.change(treeDimension / 2 - 10, treeDimension / 2 + 10);
            tree.reHash();

            THEN("the hash values of the other quarters should be zero")
            {

                std::pair<std::map<unsigned, std::vector<size_t>>, int> result = tree.hashValuesOfNextNLevels(4, initial_hash);
                std::map<unsigned, std::vector<size_t>> hashValues = result.first;
                REQUIRE(hashValues.size() == 4);

                REQUIRE(hashValues[1].size() == 1);
                REQUIRE(hashValues[2].size() == 4);
                REQUIRE(hashValues[3].size() == 16);
                REQUIRE(hashValues[4].size() == 64);

                REQUIRE(hashValues[2].at(0) == 0);
                REQUIRE(hashValues[2].at(1) == 0);
                REQUIRE(hashValues[2].at(2) != 0);
                REQUIRE(hashValues[2].at(3) == 0);

                for (int i = 0; i < 8; i++) {
                    REQUIRE(hashValues[3].at(i) == 0);
                }
                for (int i = 12; i < 16; i++) {
                    REQUIRE(hashValues[3].at(i) == 0);
                }

                for (int i = 0; i < 32; i++) {
                    REQUIRE(hashValues[4].at(i) == 0);
                }
                for (int i = 48; i < 64; i++) {
                    REQUIRE(hashValues[4].at(i) == 0);
                }
            }

            THEN("the number of returned changes should be one") {
                auto result = tree.hashValuesOfNextNLevels(4, initial_hash);
                REQUIRE(result.second == 1);
            }

            THEN("when enumerating 3 lower levels, one node has to be initialized, the other 63 are nullptrs") {
                int numNullptrs = 0;
                int numInit = 0;
                for (const auto subtree : tree.enumerateLowerLevel(3)) {
                    if (subtree == nullptr) {
                        numNullptrs++;
                    } else {
                        numInit++;
                    }
                }
                REQUIRE(numNullptrs == 63);
                REQUIRE(numInit == 1);
            }
        }
    }
}

TEST_CASE("Correctly inflate subtrees") {
    GIVEN("A 64x64 area is covered by an empty SyncTree")
    {
        unsigned treeDimension = 64;
        Rectangle rectangle(Point(0, 0), Point(treeDimension, treeDimension));
        SyncTree tree(rectangle);

        REQUIRE(tree.countInflatedNodes() == 1);
        const Rectangle& area = tree.getArea();
        REQUIRE(area.bottomRight.x - area.topleft.x == treeDimension);
        REQUIRE(area.bottomRight.y - area.topleft.y == treeDimension);
        std::size_t initial_hash = tree.getHash();

        WHEN("subtree 1 of level 2 is inflated") {
            tree.inflateSubtree(2, 1);

            THEN("Subtree 32,0-64,32 should not be null") {
                Rectangle r2(Point(32, 0), Point(64, 32));
                REQUIRE(tree.getSubtree(r2) != nullptr);
            }

        }
        WHEN("subtree 1 of level 2 is inflated twice") {
            SyncTree* st1 = tree.inflateSubtree(2, 1);
            SyncTree* st2 = tree.inflateSubtree(2, 1);

            THEN("Only one inflation should have created a new node") {
                Rectangle r2(Point(32, 0), Point(64, 32));
                REQUIRE(tree.getSubtree(r2) != nullptr);
                REQUIRE(tree.getSubtree(r2) == st1);
                REQUIRE(tree.getSubtree(r2) == st2);
            }
        }



        WHEN("subtree 10 of level 3 is inflated") {
            tree.inflateSubtree(3, 10);

            THEN("Subtree 0,32-32,64 should not be null") {
                Rectangle r2(Point(0, 32), Point(32, 64));
                REQUIRE(tree.getSubtree(r2) != nullptr);
            }

            THEN("Subtree 0,48-16,64 should not be null") {
                Rectangle r2(Point(0, 48), Point(16, 64));
                REQUIRE(tree.getSubtree(r2) != nullptr);
            }

        }



        WHEN("subtree 0,1,2 of level 3 is inflated") {
            tree.inflateSubtree(3, 0);
            tree.inflateSubtree(3, 1);
            tree.inflateSubtree(3, 2);

            THEN("The coordinates of the subtree have to be correct") {
                const std::vector<SyncTree*>& subtrees = tree.enumerateLowerLevel(2);

                REQUIRE(subtrees.at(0)->getArea().topleft.x == 0);
                REQUIRE(subtrees.at(0)->getArea().topleft.y == 0);
                REQUIRE(subtrees.at(0)->getArea().bottomRight.x == 16);
                REQUIRE(subtrees.at(0)->getArea().bottomRight.y == 16);

                REQUIRE(subtrees.at(1)->getArea().topleft.x == 16);
                REQUIRE(subtrees.at(1)->getArea().topleft.y == 0);
                REQUIRE(subtrees.at(1)->getArea().bottomRight.x == 32);
                REQUIRE(subtrees.at(1)->getArea().bottomRight.y == 16);

                REQUIRE(subtrees.at(2)->getArea().topleft.x == 0);
                REQUIRE(subtrees.at(2)->getArea().topleft.y == 16);
                REQUIRE(subtrees.at(2)->getArea().bottomRight.x == 16);
                REQUIRE(subtrees.at(2)->getArea().bottomRight.y == 32);
            }

        }

        WHEN("Subtree 10 of level 1 is inflated, and the childs of the returned subtree are inflated") {
            SyncTree* st = tree.inflateSubtree(3, 10);
            st->inflateSubtree(4, 0);

            THEN("Subtree 0,48,8,56 should be inflated") {
                Rectangle r2(Point(0, 48), Point(8, 56));
                REQUIRE(tree.getSubtree(r2) != nullptr);
            }

        }
    }
}