#ifndef LDOPALIB_ALPHA_TRIPLUS_ALPGORITHM_H
#define LDOPALIB_ALPHA_TRIPLUS_ALPGORITHM_H

#include "xi/ldopa/pn/algos/grviz/base_ptnet_dotwriter.h"
#include "xi/ldopa/eventlog/obsolete1/csvlog.h"
#include "xi/ldopa/pn/models/base_ptnet.h"
#include "xi/ldopa/pn/models/gen_petrinet.h"

namespace xi { namespace ldopa { namespace pn {

    class AlphaTriPlusMiner {
    public:
        static void GenPetriNetFromCSVLog(eventlog::obsolete1::CSVLogTraces* TracesList) {
            }

    protected:
        static void AlphaTriPlus(std::vector<std::vector<std::string>>& TracesList,
                                  MapLabeledPetriNet<>& MLPN, double ArtificialActivityTh, double BalanceTh,
                                  double LocalFitnessTh, double ReplayFitnessTh) {
            /// construct DFG
            /// repair log loop
            /// repair log skip
            /// clean dfg
            /// setDFG (from utils)
            /// build candidates
            /// prune candidates Balance
            /// prune candidates Fitness
            /// prune candidates Maximal
            /// build Petri net
            /// replay with all traces
            /// get problematic places from replay
            /// remove problematic places

        }
    };

}}} // namespace xi { namespace ldopa { namespace pn {

#endif //LDOPALIB_ALPHA_TRIPLUS_ALPGORITHM_H
