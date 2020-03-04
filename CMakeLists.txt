cmake_minimum_required(VERSION 3.10)
project(QuadTreeSyncEvaluation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-pedantic -Wall -Werror -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable -Wno-error=unused-variable -Wno-error=unused-but-set-variable -fdiagnostics-color")

find_package(PkgConfig REQUIRED)

# Check if Catch2 (Lib for tests) is installed
find_package(Catch2 REQUIRED)

# Check if protobuf is installed
find_package(Protobuf REQUIRED)

# Check if NDN-CXX is installed
pkg_check_modules(NDN_CXX REQUIRED libndn-cxx)
#include_directories(${NDN_CXX_INCLUDE_DIRS})

# Check if Boost and all components are installed
FIND_PACKAGE(Boost 1.65 COMPONENTS system thread program_options filesystem iostreams REQUIRED)

find_package(spdlog REQUIRED)


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
        ${NDN_CXX_INCLUDE_DIRS}
        )
target_link_libraries(SyncTreeNLevelEval
        PUBLIC
        ${PROTOBUF_LIBRARIES}
        ${Boost_LIBRARIES}
        ${NDN_CXX_LIBRARIES}
        spdlog::spdlog
        )

add_executable(SyncTreeTests test/SyncTreeTests.cpp
        src/QuadTreeStructs.h
        src/SyncTree.cpp src/SyncTree.h
        src/proto/SyncResponse.pb.h src/proto/SyncResponse.pb.cc)
target_include_directories(SyncTreeTests
        PUBLIC
        ${PROTOBUF_INCLUDE_DIRS}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${NDN_CXX_INCLUDE_DIRS}
        ${CATCH2_INCLUDE_DIRS}
        )
target_link_libraries(SyncTreeTests
        PUBLIC
        Catch2::Catch2
        ${PROTOBUF_LIBRARIES}
        ${Boost_LIBRARIES}
        ${NDN_CXX_LIBRARIES}
        spdlog::spdlog
        )

add_executable(EvaluationSyncClient src/EvaluationSyncClient.cpp
        src/QuadTreeStructs.h
        src/SyncTree.cpp src/SyncTree.h
        src/ServerModeSyncClient.h src/ServerModeSyncClient.cpp
        src/csv/CSVReader.cpp src/csv/CSVReader.h
        src/logging/ChunkLogger.h src/logging/ChunkLogger.cpp
        src/proto/SyncResponse.pb.h src/proto/SyncResponse.pb.cc)
target_link_libraries(EvaluationSyncClient
        PUBLIC
        ${PROTOBUF_LIBRARIES}
        ${Boost_LIBRARIES}
        ${NDN_CXX_LIBRARIES}
        spdlog::spdlog
        )
target_include_directories(EvaluationSyncClient
        PUBLIC
        ${PROTOBUF_INCLUDE_DIRS}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${NDN_CXX_INCLUDE_DIRS}
        )

add_executable(Testclient src/Testclient.cpp)
target_link_libraries(Testclient
        PUBLIC
        ${NDN_CXX_LIBRARIES}
        spdlog::spdlog
        )
target_include_directories(Testclient
        PUBLIC
        ${NDN_CXX_INCLUDE_DIRS}
        )

add_executable(SVSClient src/svs/client_main.cpp
        src/svs/svs.hpp src/svs/svs.cpp
        src/svs/svs_common.hpp src/svs/svs_helper.hpp)
target_link_libraries(SVSClient
        PUBLIC
        ${NDN_CXX_LIBRARIES}
        )
target_include_directories(SVSClient
        PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
        ${NDN_CXX_INCLUDE_DIRS}
        )
