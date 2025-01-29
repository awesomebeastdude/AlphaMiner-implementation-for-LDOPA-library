cmake_minimum_required(VERSION 3.16)
project(Ldopalib)

set(CMAKE_CXX_STANDARD 14)

file(GLOB_RECURSE SOURCES src/xi/ldopa/*.cpp)

set(SQLITE3_SRC_DIR sqlite-amalgamation)
set(SQLITE3_SRC ${SQLITE3_SRC_DIR}/sqlite3.c)
add_library(ldopalib STATIC ${SOURCES} ${SQLITE3_SRC})
target_include_directories(ldopalib PUBLIC src)
target_include_directories(ldopalib PUBLIC ${SQLITE3_SRC_DIR})
add_definitions(-DSQLITE_ENABLE_COLUMN_METADATA)
add_compile_options(/Zm2000)

add_subdirectory(xilib-0.31)
include_directories(xilib-0.31/src)
target_link_libraries(ldopalib PUBLIC xilib)

add_subdirectory(boost-1.87.0)
target_link_libraries(ldopalib PUBLIC Boost::graph)

add_subdirectory(googletest)
include(GoogleTest)
file(GLOB_RECURSE TEST_SOURCES
        tests/gtest/src/ts/*.cpp
        tests/gtest/src/pn/*.cpp
        tests/gtest/src/graphs/*.cpp
        #tests/gtest/src/eventlog/*.cpp
)
add_executable(ldopatests ${TEST_SOURCES})
target_link_libraries(ldopatests PUBLIC gtest gtest_main ldopalib)
target_include_directories(ldopatests PUBLIC tests/gtest/src)
gtest_discover_tests(ldopatests)
enable_testing()
