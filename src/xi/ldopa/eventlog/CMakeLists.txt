add_library(eventlog)
target_sources(eventlog PRIVATE
        obsolete1/csvlog.cpp
        obsolete1/csvlog_events.cpp
        sqlite/sqlitehelpers.cpp
        sqlite/sqlitelog.cpp

        PUBLIC
        eventlog.h
        obsolete1/csvlog.h
        obsolete1/csvlog_events.h
        obsolete1/eventlog.h
        sqlite/sqlitehelpers.h
        sqlite/sqlitelog.h
)
target_include_directories(eventlog PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../>
)

target_link_libraries(eventlog PUBLIC Boost::graph xilib sqlite3)
add_library(ldopalib::eventlog ALIAS eventlog)