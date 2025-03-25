#[[
cmake_minimum_required(VERSION 3.16) # Or your desired minimum CMake version

project(SQLite3)

# Source files
set(SOURCES shell.c sqlite3.c)
set(HEADERS sqlite3.h sqlite3ext.h)

# Add executable target (if you need a command-line tool)
add_library(SQLite3 STATIC ${SOURCES})

target_include_directories(SQLite3 PUBLIC .)

# Install headers and library (optional)
install(TARGETS SQLite3 DESTINATION bin)
install(FILES ${HEADERS} DESTINATION include)]]

# sqlite3/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

# Create a static library from the sqlite3.c file
add_library(sqlite3 STATIC
        sqlite3.c
        shell.c # optional, only if you want to use the sqlite3 shell functionality
        sqlite3.h
        sqlite3ext.h
)

# Define include directories for the library
target_include_directories(sqlite3 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Optionally, you can define preprocessor flags here if needed
# target_compile_definitions(sqlite3 PRIVATE SQLITE_ENABLE_COLUMN_METADATA)

# Make sure you are compiling with C++ support if required
set_target_properties(sqlite3 PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON
)

