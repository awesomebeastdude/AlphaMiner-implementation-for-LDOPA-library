add_library(pn INTERFACE)
target_sources(pn INTERFACE
        algos/alpha/alpha_algorithm.h
        algos/alpha/alpha_plus_algorithm.h
        algos/alpha/alpha_plusplus_algorithm.h
        algos/alpha/alpha_triplus_alpgorithm.h
        algos/grviz/base_ptnet_dotwriter.h
        algos/grviz/evlog_ptnets_dotwriter.h
        algos/regions/pn_synthesis.h
        models/base_ptnet.h
        models/evlog_ptnets.h
        models/gen_petrinet.h
)
target_include_directories(pn INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../>
)
target_link_libraries(pn INTERFACE Boost::graph xilib graphs eventlog ts utils)
add_library(ldopalib::pn ALIAS pn)
