file(GLOB_RECURSE XILIB_SOURCES src/xi/*.cpp)

add_library(xilib ${XILIB_SOURCES})
include_directories(src)
target_include_directories(xilib PUBLIC src)