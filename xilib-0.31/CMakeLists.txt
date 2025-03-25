file(GLOB_RECURSE XILIB_SOURCES src/xi/*.cpp src/xi/*.h)

add_library(xilib ${XILIB_SOURCES})
target_include_directories(xilib PUBLIC src)