add_library(graphs)
target_sources(graphs PRIVATE
        bidigraph.cpp
        algos/grviz/bidigraph_dot_writer.cpp
        algos/grviz/graph_dot_writer.cpp
        _obsolete/named_bidigraph.cpp

        PUBLIC
        bidigraph.h
        boost/bgl_graph_wrappers.h
        boost/bgl_graph_adapters.h
        algos/grviz/bidigraph_dot_writer.h
        algos/grviz/gen_dot_writer.h
        algos/grviz/graph_dot_writer.h
        _obsolete/named_bidigraph.h
)
target_include_directories(graphs PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../>
)

target_link_libraries(graphs PUBLIC Boost::graph xilib)
add_library(ldopalib::graphs ALIAS graphs)