cmake_minimum_required(VERSION 3.14)
project(QuadTreeSyncEvaluation)

set(CMAKE_CXX_STANDARD 17)

find_package(Catch2 REQUIRED)

FIND_PACKAGE( Boost 1.65 COMPONENTS REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )


add_executable(QuadTreeSyncEvaluation src/main.cpp
        src/QuadTree.cpp src/QuadTree.h
        src/datastore/HashStorage.cpp src/datastore/HashStorage.h
        src/QuadTreeStructs.h)

add_executable(tests test/tests.cpp
        src/QuadTree.cpp src/QuadTree.h
        src/datastore/lru-cache.h
        src/datastore/HashStorage.cpp src/datastore/HashStorage.h
        src/QuadTreeStructs.h)
target_link_libraries(tests Catch2::Catch2)
