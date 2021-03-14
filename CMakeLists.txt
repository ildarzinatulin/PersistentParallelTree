cmake_minimum_required(VERSION 3.17)
project(PersistentParallelTree)

set(CMAKE_CXX_STANDARD 14)
add_subdirectory(lib/googletest)

include_directories(lib/googletest/googletest/include)
include_directories(cmdline/include)
include_directories(pctl/include)
include_directories(chunkedseq/include)
include_directories(Trees)

set(SOURCE_FILES main.cpp Trees/BasicPersistentTree.h Trees/PersistentParallelTreap.h Trees_tests/BasicTreeTest.cpp Trees_tests/TreapTest.cpp)
add_executable(PersistentParallelTree ${SOURCE_FILES})
target_link_libraries(PersistentParallelTree gtest gtest_main)