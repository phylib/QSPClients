cmake_minimum_required(VERSION 3.10)
project(QuadTreeSyncEvaluation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wno-unused-variable -Wno-unused-but-set-variable -Wno-error=unused-variable -Wno-error=unused-but-set-variable")

find_package(Catch2 REQUIRED)

# find the protobuf compiler and libraries
find_package(Protobuf REQUIRED)

# Generate the .h and .cxx files
#SET(PROTO_SRCS src/proto)
#SET(PROTO_HDRS src/proto)
#message ("PROTO_SRCS = ${PROTO_SRCS}")
#message ("PROTO_HDRS = ${PROTO_HDRS}")
#PROTOBUF_GENERATE_CPP(src/proto src/proto proto/ChunkChanges.proto)
#PROTOBUF_GENERATE_CPP(src/proto src/proto proto/LowerLevelHashes.proto)

FIND_PACKAGE(Boost 1.65 COMPONENTS system thread program_options filesystem iostreams REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})


add_executable(SyncTreeEvaluation src/main.cpp
        src/QuadTreeStructs.h
        src/SyncTree.cpp src/SyncTree.h
        src/proto/SyncResponse.pb.h src/proto/SyncResponse.pb.cc
        src/csv/CSVReader.cpp src/csv/CSVReader.h)
target_include_directories(SyncTreeEvaluation
        PUBLIC
        ${PROTOBUF_INCLUDE_DIRS}
        ${CMAKE_CURRENT_BINARY_DIR}
        )
target_link_libraries(SyncTreeEvaluation
        PUBLIC
        ${PROTOBUF_LIBRARIES}
        ${Boost_LIBRARIES}
        )


add_executable(SyncTreeNLevelEval src/SyncTreeNLevelEvaluation.cpp
        src/QuadTreeStructs.h
        src/SyncTree.cpp src/SyncTree.h
        src/csv/CSVReader.cpp src/csv/CSVReader.h
        src/proto/LowerLevelHashes.pb.h src/proto/LowerLevelHashes.pb.cc
        src/proto/ChunkChanges.pb.h src/proto/ChunkChanges.pb.cc
        src/proto/SyncResponse.pb.h src/proto/SyncResponse.pb.cc
        src/zip/GZip.h
        )
target_include_directories(SyncTreeNLevelEval
        PUBLIC
        ${PROTOBUF_INCLUDE_DIRS}
        ${CMAKE_CURRENT_BINARY_DIR}
        )
target_link_libraries(SyncTreeNLevelEval
        PUBLIC
        ${PROTOBUF_LIBRARIES}
        ${Boost_LIBRARIES}
        )

add_executable(SyncTreeTests test/SyncTreeTests.cpp
        src/QuadTreeStructs.h
        src/SyncTree.cpp src/SyncTree.h
        src/proto/SyncResponse.pb.h src/proto/SyncResponse.pb.cc)
target_include_directories(SyncTreeTests
        PUBLIC
        ${PROTOBUF_INCLUDE_DIRS}
        ${CMAKE_CURRENT_BINARY_DIR}
        )
target_link_libraries(SyncTreeTests
        PUBLIC
        Catch2::Catch2
        ${PROTOBUF_LIBRARIES}
        ${Boost_LIBRARIES})
