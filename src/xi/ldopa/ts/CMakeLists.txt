add_library(ts)
target_sources(ts PRIVATE
        algos/dual_ts_simulator.cpp
        algos/freq_condenser.cpp
        algos/ts_metrics_calc.cpp
        algos/ts_simple_builder.cpp
        algos/varws_ts_rebuilder.cpp
        algos/obsolete1/eventlogts_builder.cpp
        algos/obsolete1/eventlogts_grviz_exporter.cpp
        models/eventlog_ts.cpp
        models/eventlog_ts_stateids.cpp
        models/evlog_ts_red.cpp
        models/obsolete1/basets.cpp
        models/obsolete1/eventlogts.cpp

        PUBLIC
        algos/dual_ts_simulator.h
        algos/freq_condenser.h
        algos/ts_metrics_calc.h
        algos/ts_sas_converter.h
        algos/ts_simple_builder.h
        algos/ts_sloops_converter.h
        algos/varws_ts_rebuilder.h
        algos/grviz/evlog_ts_red_dotwriter.h
        algos/obsolete1/eventlogts_builder.h
        algos/obsolete1/eventlogts_grviz_exporter.h
        models/eventlog_ts.h
        models/eventlog_ts_stateids.h
        models/evlog_ts_red.h
        models/labeledts.h
        models/obsolete1/basets.h
        models/obsolete1/eventlogts.h
)
target_include_directories(ts PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../>
)

target_link_libraries(ts PUBLIC Boost::graph xilib eventlog graphs utils)
add_library(ldopalib::ts ALIAS ts)