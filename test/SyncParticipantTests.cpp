//
// Created by phmoll on 7/23/19.
//
#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "../src/SyncParticipant.h"

bool comparePoints(Point p1, Point p2) {
    return p1.x == p2.x && p1.y == p2.y;
}

TEST_CASE("Initialization of a SyncParticipants and Quadtrees covering it's area") {

    SECTION("Test  area not extending a quadtree") {
        std::vector<Rectangle> syncAreas;
        syncAreas.insert(syncAreas.begin(), Rectangle(Point(0, 0), Point(10, 10)));
        SyncParticipant participant;
        participant.setSyncAreas(syncAreas);

        REQUIRE(participant.getSyncTrees().size() == 1);
        REQUIRE(comparePoints(participant.getSyncTrees().front().getBounds().first, Point(0, 0)));
        REQUIRE(comparePoints(participant.getSyncTrees().front().getBounds().second, Point(SyncParticipant::QUADTREES_SIZE, SyncParticipant::QUADTREES_SIZE)));

    }

    SECTION("Test area extending across two quadtrees") {
        std::vector<Rectangle> syncAreas;
        syncAreas.insert(syncAreas.begin(), Rectangle(Point(-10, 0), Point(10, 10)));
        SyncParticipant participant;
        participant.setSyncAreas(syncAreas);

        REQUIRE(participant.getSyncTrees().size() == 2);

    }

    SECTION("Extend an initially small sync area to a larger one") {
        std::vector<Rectangle> syncAreas;
        syncAreas.insert(syncAreas.begin(), Rectangle(Point(0, 0), Point(10, 10)));
        SyncParticipant participant;
        participant.setSyncAreas(syncAreas);

        REQUIRE(participant.getSyncTrees().size() == 1);

        syncAreas.clear();
        syncAreas.insert(syncAreas.begin(), Rectangle(Point(-10, 0), Point(10, 10)));
        participant.setSyncAreas(syncAreas);
        REQUIRE(participant.getSyncTrees().size() == 2);

    }
}

TEST_CASE("Apply changes to the sync area of the sync participant and check if hash trees are changed correctly") {

    SECTION("Check if a chunk changes are applied correctly") {
        std::vector<Rectangle> syncAreas;
        syncAreas.insert(syncAreas.begin(), Rectangle(Point(-10, 0), Point(10, 10)));
        SyncParticipant participant;
        participant.setSyncAreas(syncAreas);

        const Point &p1 = Point(3, 3);
        REQUIRE(participant.getChunk(p1).data == 0);
        participant.applyChange(p1);
        REQUIRE(participant.getChunk(p1).data == 1);
        participant.applyChange(p1);
        REQUIRE(participant.getChunk(p1).data == 2);

        const Point &p2 = Point(-3,3);
        REQUIRE(participant.getChunk(p2).data == 0);
        participant.applyChange(p2);
        REQUIRE(participant.getChunk(p2).data == 1);
        participant.applyChange(p2);
        REQUIRE(participant.getChunk(p2).data == 2);

    }

    SECTION("Check if hash trees are build correctly") {

        std::vector<Rectangle> syncAreas;
        syncAreas.insert(syncAreas.begin(), Rectangle(Point(-10, 0), Point(10, 10)));
        SyncParticipant participant;
        participant.setSyncAreas(syncAreas);

        // Todo: Request old hashtrees here

        const Point &p1 = Point(3, 3);
        participant.applyChange(p1);
        participant.applyChange(p1);

        const Point &p2 = Point(-3,3);
        participant.applyChange(p2);
        participant.applyChange(p2);

        participant.reHash();

        // Todo: Request new hashtrees here and look if changes are correct

    }

}