//
// Created by phmoll on 11/15/19.
//
#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include <math.h>

#include "../src/SyncTree.h"

using namespace quadtree;

TEST_CASE("Basic SyncTree Structure and function", "[SyncTree]") {

    SECTION("Constructor and Destructor") {

        {
            // A tree with maxlevel 0 and decruction afterwards
            Rectangle rectangle(Point(0, 0), Point(2, 2));
            SyncTree *tree = new SyncTree(rectangle);
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

}

SCENARIO("An area can be covered by a SyncTree and changes can be made") {

    GIVEN("A 8x8 area is covered by am empty SyncTree") {

        unsigned treeDimension = 8;
        Rectangle rectangle(Point(0, 0), Point(treeDimension, treeDimension));
        SyncTree tree(rectangle);

        REQUIRE(tree.countInflatedNodes() == 1);
        const Rectangle &area = tree.getArea();
        REQUIRE(area.bottomRight.x - area.topleft.x == treeDimension);
        REQUIRE(area.bottomRight.y - area.topleft.y == treeDimension);

        WHEN("A single chunk changes") {
            Chunk *c = tree.change(2, 1);

            THEN("its version has to be 1") {
                REQUIRE(c->pos.x == 2);
                REQUIRE(c->pos.y == 1);

                REQUIRE(c->data == 1);
            }

            THEN("4 nodes must be inflated.") {
                REQUIRE(tree.countInflatedNodes() == 4);
            }
        }

        WHEN("When all chunks of the tree are inflated") {
            // Inflate all subtrees
            for (int i = 0; i < treeDimension; i++) {
                for (int j = 0; j < treeDimension; j++) {
                    tree.change(i, j);
                }
            }
            THEN("85 nodes have to be inflated") {
                REQUIRE(tree.countInflatedNodes() == 85);
            }

        }

        WHEN("A single chunk changes twice") {
            Chunk *c = tree.change(2, 1);
            tree.change(2, 1);

            THEN("the chunk returned by the first change call should have an version of 2") {
                REQUIRE(c->data == 2);
            }
        }
    }

    GIVEN("A 60000x60000 area is covered by am empty SyncTree") {

        unsigned treeDimension = 60000;
        Rectangle rectangle(Point(0, 0), Point(treeDimension, treeDimension));
        SyncTree tree(rectangle);

        REQUIRE(tree.countInflatedNodes() == 1);
        const Rectangle &area = tree.getArea();
        REQUIRE(area.bottomRight.x - area.topleft.x == treeDimension);
        REQUIRE(area.bottomRight.y - area.topleft.y == treeDimension);

        WHEN("A single chunk changes") {
            Chunk *c = tree.change(25000, 1200);

            THEN("the number of inflated nodes should be log(60000, 2) + 1") {
                REQUIRE(tree.countInflatedNodes() == ceil(log2(treeDimension)) + 1);
            }
        }

    }

}