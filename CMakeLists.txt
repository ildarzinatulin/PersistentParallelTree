cmake_minimum_required(VERSION 3.17)
project(PersistentParallelTree)

set(CMAKE_CXX_STANDARD 14)
add_subdirectory(lib/googletest)

include_directories(lib/googletest/googletest/include)
include_directories(cmdline/include)
include_directories(pctl/include)
include_directories(chunkedseq/include)
include_directories(sptl/include)

add_executable(SumMeasuring main.cpp)

add_executable(TreapTest Trees_tests/TreapTest.cpp Trees/PersistentParallelTreap.h)
target_link_libraries(TreapTest gtest gtest_main)

add_executable(WorkTimeMeasuring WorkTimeMeasuring.cpp Trees/PersistentParallelTreap.h)