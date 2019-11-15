cmake_minimum_required(VERSION 3.10)
project(QuadTreeSyncEvaluation)

set(CMAKE_CXX_STANDARD 17)

find_package(Catch2 REQUIRED)

FIND_PACKAGE( Boost 1.65 COMPONENTS REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )


add_executable(QuadTreeSyncEvaluation src/main.cpp
        src/QuadTree.cpp src/QuadTree.h
        src/datastore/HashStorage.cpp src/datastore/HashStorage.h
        src/QuadTreeStructs.h)

add_executable(QuadTreeTests test/QuadTreeTests.cpp
        src/QuadTree.cpp src/QuadTree.h
        src/datastore/HashStorage.cpp src/datastore/HashStorage.h
        src/QuadTreeStructs.h)
target_link_libraries(QuadTreeTests Catch2::Catch2)

add_executable(SyncParticipantsTests test/SyncParticipantTests.cpp
        src/SyncParticipant.cpp src/SyncParticipant.h
        src/QuadTree.cpp src/QuadTree.h
        src/datastore/HashStorage.cpp src/datastore/HashStorage.h
        src/QuadTreeStructs.h)
target_link_libraries(SyncParticipantsTests Catch2::Catch2)

add_executable(SyncTreeTests test/SyncTreeTests.cpp
        src/QuadTreeStructs.h
        src/SyncTree.cpp src/SyncTree.h)
target_link_libraries(SyncTreeTests Catch2::Catch2)
