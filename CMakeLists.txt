cmake_minimum_required(VERSION 3.16)
project(ldopalib)

set(CMAKE_CXX_STANDARD 14)

add_library(utils)
target_sources(utils PRIVATE
        src/xi/ldopa/dllmain.cpp
        src/xi/ldopa/utils.cpp

        PUBLIC
        src/xi/ldopa/utils.h
        src/xi/ldopa/ldopa_dll.h
        src/xi/ldopa/utils/elapsed_time.h
        src/xi/ldopa/utils/progress_callback.h
)
target_include_directories(utils PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/>
)

add_library(ldopalib)
target_link_libraries(ldopalib PUBLIC utils)

add_definitions(-DSQLITE_ENABLE_COLUMN_METADATA)
add_compile_options(/Zm2000)

add_subdirectory(xilib-0.31)
target_link_libraries(ldopalib PUBLIC xilib)

add_subdirectory(boost-1.87.0)
target_link_libraries(ldopalib PUBLIC Boost::graph)

add_subdirectory(sqlite-amalgamation)
target_link_libraries(ldopalib PUBLIC sqlite3)

add_subdirectory(src/xi/ldopa/eventlog)
target_link_libraries(ldopalib PUBLIC eventlog)

add_subdirectory(src/xi/ldopa/graphs)
target_link_libraries(ldopalib PUBLIC graphs)

add_subdirectory(src/xi/ldopa/ts)
target_link_libraries(ldopalib PUBLIC ts)

add_subdirectory(src/xi/ldopa/pn)
target_link_libraries(ldopalib PUBLIC pn)
