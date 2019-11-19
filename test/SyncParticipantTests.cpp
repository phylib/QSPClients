//
// Created by phmoll on 7/23/19.
//
#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "../src/SyncParticipant.h"

using namespace quadtree;

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
        REQUIRE(comparePoints(participant.getSyncTrees().front().getBounds().second, Point(SyncParticipant::QUADTREE_SIZE, SyncParticipant::QUADTREE_SIZE)));

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

TEST_CASE("Apply storedChanges to the sync area of the sync participant and check if hash trees are changed correctly") {

    SECTION("Check if a chunk storedChanges are applied correctly") {
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

    SECTION("Check if hash trees and ChangeResponses are build correctly") {

        std::vector<Rectangle> syncAreas;
        syncAreas.insert(syncAreas.begin(), Rectangle(Point(0, 0), Point(64, 64)));
        SyncParticipant participant;
        participant.setSyncAreas(syncAreas);

        ChangeResponse root0 = participant.getChanges("/0,0/", 0);
        REQUIRE(!root0.delta);
        REQUIRE(root0.changeVector.size() == 32*32);
        ChangeResponse root1 = participant.getChanges("/1,0/", 0);
        REQUIRE(!root1.delta);
        REQUIRE(root1.changeVector.size() == 32*32);
        ChangeResponse firstChild = participant.getChanges("/0,0/0,1/", 0);
        REQUIRE(!firstChild.delta);
        REQUIRE(firstChild.changeVector.size() == 16*16);
        ChangeResponse childOfFirstChild = participant.getChanges("/0,0/0,1/1,2/", 0);
        REQUIRE(!childOfFirstChild.delta);
        REQUIRE(childOfFirstChild.changeVector.size() == 8*8);
        ChangeResponse secondChild = participant.getChanges("/0,0/1,1/", 0);
        REQUIRE(!secondChild.delta);
        REQUIRE(secondChild.changeVector.size() == 16*16);


        Point p(10, 20);
        participant.applyChange(p);
        participant.reHash();

        ChangeResponse root0v2 = participant.getChanges("/0,0/", root0.currentHash);
        REQUIRE(root0v2.delta);
        REQUIRE(root0v2.changeVector.size() == 1);
        REQUIRE(root0.currentHash != root0v2.currentHash);

        ChangeResponse firstChildv2 = participant.getChanges("/0,0/0,1/", firstChild.currentHash);
        REQUIRE(firstChildv2.delta);
        REQUIRE(firstChildv2.changeVector.size() == 1);

        ChangeResponse childOfFirstChildv2 = participant.getChanges("/0,0/0,1/1,2/", childOfFirstChild.currentHash);
        REQUIRE(childOfFirstChildv2.delta);
        REQUIRE(!childOfFirstChildv2.isEmpty());
        REQUIRE(childOfFirstChildv2.changeVector.size() == 1);

        ChangeResponse secondChildv2 = participant.getChanges("/0,0/1,1/", secondChild.currentHash);
        REQUIRE(secondChildv2.isEmpty());

        ChangeResponse root1v2 = participant.getChanges("/1,0/", root1.currentHash);
        REQUIRE(root1v2.isEmpty());

        // Apply a second change
        participant.applyChange(p);
        participant.reHash();

        // The inital root hash should not be available any more
        ChangeResponse root0v3 = participant.getChanges("/0,0/", root0.currentHash);
        REQUIRE(!root0v3.delta);
        // When requesting the change since the last change, only a delta should be returned
        root0v3 = participant.getChanges("/0,0/", root0v2.currentHash);
        REQUIRE(root0v3.delta);
        REQUIRE(root0v3.changeVector.size() == 1);
        REQUIRE((root0v3.changeVector.begin())->data == 2);

    }

    SECTION("Get RequestPaths for remote Regions") {

        std::vector<Rectangle> syncAreas;
        syncAreas.insert(syncAreas.begin(), Rectangle(Point(0, 0), Point(64, 64)));
        SyncParticipant participant;
        participant.setSyncAreas(syncAreas);

        std::vector<RemoteSyncArea> remoteAreas;

        // s1
        std::vector<Rectangle> remoteArea;
        remoteArea.insert(remoteArea.begin(), Rectangle(Point(0, 64), Point(64, 128)));
        remoteAreas.insert(remoteAreas.begin(), std::pair("s1", remoteArea));
        // s2
        remoteArea.clear();
        remoteArea.insert(remoteArea.begin(), Rectangle(Point(64, 0), Point(128, 64)));
        remoteAreas.insert(remoteAreas.begin(), std::pair("s2", remoteArea));
        // s3
        remoteArea.clear();
        remoteArea.insert(remoteArea.begin(), Rectangle(Point(64, 64), Point(128, 128)));
        remoteAreas.insert(remoteAreas.begin(), std::pair("s3", remoteArea));
        participant.setRemoteSyncAreas(remoteAreas);

    }

}