#ifndef LDOPALIB_ALPHA_PLUS_ALGORITHM_H
#define LDOPALIB_ALPHA_PLUS_ALGORITHM_H

#include "xi/ldopa/pn/algos/grviz/base_ptnet_dotwriter.h"
#include "xi/ldopa/eventlog/obsolete1/csvlog.h"
#include "xi/ldopa/pn/models/base_ptnet.h"
#include "xi/ldopa/pn/models/gen_petrinet.h"

namespace xi { namespace ldopa { namespace pn {

    class AlphaPlusMiner {
    public:

        static void GenPetriNetFromCSVLog(eventlog::obsolete1::CSVLogTraces* TracesList) {
            std::vector<std::vector<std::string>> TracesVec;
            size_t iter = 0;
            for (auto i = TracesList->enumerateTraces(); i->hasNext();) {
                TracesVec.emplace_back();
                eventlog::obsolete1::ITrace *trace = i->getNext();
                for (auto j = 0; j < trace->getEventsNum(); ++j) {
                    eventlog::obsolete1::IEvent *event = trace->operator[](j);
                    TracesVec[iter].push_back(event->getActivityName());
                }
                ++iter;
            }

            MapLabeledPetriNet<> PetriNet;
            AlphaPlus(TracesVec, PetriNet);

            MapLabeledPetriNetDotWriter sd;
            sd.write("C:/Users/maxga/CLionProjects/190506143700_ldopa-0.1.2/tests/gtest/work_files/logs/TEST_1.gv",PetriNet);
        }


        static void AlphaPlus(std::vector<std::vector<std::string>>& TracesList,
                                                MapLabeledPetriNet<>& MLPN_nL1L) {

            /// step 1 and 2
            std::set<std::string> T_log;
            std::set<std::string> L1L;
            std::map<std::string, std::set<std::string>> DirectionalMatrix;
            for (auto i = 0; i < TracesList.size(); ++i) {
                std::multiset<std::string> T_repeated_temp;
                for (auto j = 0; j < TracesList[i].size(); ++j) {
                    T_log.insert(TracesList[i][j]);
                    T_repeated_temp.insert(TracesList[i][j]);
                    if (j + 1 < TracesList[i].size()) {
                        if (TracesList[i][j] == TracesList[i][j + 1]) {
                                L1L.insert(TracesList[i][j]);
                        } else {
                            DirectionalMatrix[TracesList[i][j]].insert(TracesList[i][j + 1]);
                        }
                    }
                }
            }

            /// step 3
            std::set<std::string> T_pruned;
            for (auto i : T_log) {
                if (!L1L.count(i)) {
                    T_pruned.insert(i);
                }
            }

            /// step 4
            std::set<std::pair<std::string, std::pair<std::string,std::string>>> F_L1L;

            /// step 5
            for (auto repeated_event : L1L) {
                std::set<std::string> A;
                std::set<std::string> B;
                for (auto i : T_pruned) {
                    if (DirectionalMatrix[i].count(repeated_event)) {
                        A.insert(i);
                    }
                    if (DirectionalMatrix[repeated_event].count(i)) {
                        B.insert(i);
                    }
                }
                for (auto i : A) {
                    if (!B.count(i)) {
                        for (auto j: B) {
                            if (!A.count(j)) {
                                F_L1L.insert(std::make_pair(repeated_event, std::make_pair(i, j)));
                            }
                        }
                    }
                }
            }

            /// step 6
            std::vector<std::vector<std::string>> W_nL1L;

            /// step 7
            for (auto temp_traces = 0; temp_traces < TracesList.size(); ++temp_traces) {
                std::vector<std::string> trace_eliminated;
                for (auto temp_events = 0; temp_events < TracesList[temp_traces].size(); ++temp_events) {
                    trace_eliminated.push_back(TracesList[temp_traces][temp_events]);
                }
                for (auto repeated_event : L1L) {
                    EliminateTask(trace_eliminated, repeated_event);
                }
                W_nL1L.emplace_back(trace_eliminated);
            }


            /// step 8
            std::map<std::pair<std::string, std::string>,  ::xi::ldopa::pn::GenPetriNet<>::Position> AlphaPositionMap;
            std::map<std::string,  ::xi::ldopa::pn::GenPetriNet<>::Transition> TransitionMap;
            AlphaFixed(W_nL1L, MLPN_nL1L, AlphaPositionMap, TransitionMap);

            ///step 9 - 12
            for (auto i : L1L) {
                TransitionMap[i] = MLPN_nL1L.addTransition(i);
            }

            for (const auto& i : F_L1L) {
                ::xi::ldopa::pn::GenPetriNet<>::Position pos;
                pos = AlphaPositionMap[std::make_pair(i.second.first, i.second.second)];
                MLPN_nL1L.addArcW(TransitionMap[i.first], pos);
                MLPN_nL1L.addArcW(pos, TransitionMap[i.first]);
            }

        }

    protected:
        static void EliminateTask(std::vector<std::string>& Trace, std::string& Event) {
            for (auto i : Trace) {
                if (i == Event) {
                    Trace.erase(std::remove(Trace.begin(), Trace.end(), Event), Trace.end());
                }
            }
        }

        static void AlphaFixed(const std::vector<std::vector<std::string>>& TracesList, MapLabeledPetriNet<>& BPN,
        std::map<std::pair<std::string, std::string>, GenPetriNet<>::Position>& PositionMap,
                std::map<std::string,  GenPetriNet<>::Transition>& TransitionMap) {

            /// step 1 - 3
            std::set<std::string> FirstEventsSet;
            std::set<std::string> LastEventsSet;
            std::set<std::string> AllEventsSet;
            std::vector<std::string> AllEventsVec;
            std::map<std::string, std::set<std::string>> DirFollowMap;
            std::map<std::string, std::set<std::string>> InDirFollowMap;
            std::map<std::string, std::set<std::string>> TriangleFollowMap;
            std::map<std::string, std::set<std::string>> CasualFollowMap;

                /// Fist, Last, Direct and Triangle follow
            for (auto i : TracesList) {
                for (auto j = 0; j < i.size(); ++j) {
                    if (!AllEventsSet.count(i[j])) {
                        AllEventsSet.insert(i[j]);
                        AllEventsVec.emplace_back(i[j]);
                    }
                    if (j == 0) {
                        FirstEventsSet.insert(i[j]);
                    } else if (j == i.size() - 1) {
                        LastEventsSet.insert(i[j]);
                    }
                    if (j + 1 < i.size()) {
                        DirFollowMap[i[j]].insert(i[j + 1]);
                        if (j + 2 < i.size() && i[j] == i[j + 2]) {
                            TriangleFollowMap[i[j]].insert(i[j + 1]);
                        }
                    }
                }
            }
                /// Indirect follow
            for (const auto& i : AllEventsSet) {
                for (const auto& j : AllEventsSet) {
                    if (i != j && !DirFollowMap[i].count(j) && !DirFollowMap[j].count(i)) {
                        InDirFollowMap[i].insert(j);
                        InDirFollowMap[j].insert(i);
                    }
                }
            }

                /// Casual follow
            for (const auto& i : AllEventsSet) {
                for (const auto& j : AllEventsSet) {
                    if (i != j && DirFollowMap[i].count(j) && (!DirFollowMap[j].count(i) ||
                    (TriangleFollowMap[i].count(j) && TriangleFollowMap[j].count(i)))) {
                        CasualFollowMap[i].insert(j);
                    }
                }
            }

            /// step 4
            uint64_t n = AllEventsVec.size();
            std::set<std::pair<std::set< std::string>,std::set<std::string>>> XwSet;
            for (auto mask = 0; mask < (1 << n); ++mask) {
                bool A_valid = true;
                std::set<std::string> A;
                for (auto i = 0; i < n; ++i) {
                    if (mask & (1 << i)) {
                        std::string A_candidate = AllEventsVec[i];
                        for (const auto& j : A) {
                            if (!InDirFollowMap[j].count(A_candidate)) {
                                A_valid = false;
                                break;
                            }
                        }
                        if (!A_valid) {
                            break;
                        }
                        A.insert(A_candidate);
                    }
                }
                if (!A_valid || A.empty()) {
                    continue;
                }

                for (auto mask_b = 0; mask_b < (1 << n); ++mask_b) {
                    std::set<std::string> B;
                    bool B_valid = true;
                    for (auto i = 0; i < n; ++i) {
                        if (mask_b & (1 << i)) {
                            std::string B_candidate = AllEventsVec[i];
                            for (auto j : B) {
                                if (!InDirFollowMap[j].count(B_candidate)) {
                                    B_valid = false;
                                    break;
                                }
                            }
                            if (!B_valid) {
                                break;
                            }
                            for (auto j : A) {
                                if (!CasualFollowMap[j].count(B_candidate)) {
                                    B_valid = false;
                                    break;
                                }
                            }
                            if (!B_valid) {
                                break;
                            }
                            B.insert(B_candidate);
                        }
                    }
                    if (!B_valid || B.empty()) {
                        continue;
                    }
                    XwSet.insert(std::make_pair(A, B));
                }
            }

            /// step 5
            std::set<std::pair<std::set< std::string>,std::set<std::string>>> YwSet;
            for (const auto& i : XwSet) {
                bool ismax = true;
                for (const auto& j : XwSet) {
                    if ((i.first != j.first || i.second != j.second) &&
                        (std::includes(j.first.begin(), j.first.end(),i.first.begin(), i.first.end())) &&
                        std::includes(j.second.begin(), j.second.end(),i.second.begin(), i.second.end())) {
                        ismax = false;
                        break;
                    }
                }
                if (ismax && !i.first.empty() && !i.second.empty()) {
                    YwSet.insert(i);
                }
            }

            /// step 6-8

            ///Getting Pw set
            std::set<MapLabeledPetriNet<>::Transition> TransitionSet;

            ///Creating Input Arcs
            std::map<std::string, MapLabeledPetriNet<>::Transition> added_transitions;
            std::set<std::pair<std::string, MapLabeledPetriNet<>::Position>> added_TP_arcs;
            std::set<std::pair<MapLabeledPetriNet<>::Position, std::string>> added_PT_arcs;

            MapLabeledPetriNet<>::Position InputPos = BPN.addPosition("Input");
            for (auto i : FirstEventsSet) {
                MapLabeledPetriNet<>::Transition t = BPN.addTransition(i);
                TransitionMap[i] = t;
                PositionMap[std::make_pair("Input None", i)] = InputPos;
                added_transitions[i] = t;
                BPN.addArcW(InputPos, t);
            }

            ///Creating Middle Arcs
            for (auto i : YwSet) {
                MapLabeledPetriNet<>::Position pos = BPN.addPosition();
                for (auto a : i.first) {
                    MapLabeledPetriNet<>::Transition t_a;
                    if (!added_transitions.count(a)) {
                        t_a = BPN.addTransition(a);
                        added_transitions[a] = t_a;
                        TransitionMap[a] = t_a;
                    } else {
                        t_a = added_transitions[a];
                    }
                    bool added_tp_arc = false;
                    if (!added_TP_arcs.count(std::make_pair(a, pos)))
                    {
                        BPN.addArcW(t_a, pos);
                        added_TP_arcs.insert(std::make_pair(a, pos));
                        added_tp_arc = true;
                    }
                    for (auto b : i.second) {
                        MapLabeledPetriNet<>::Transition t_b;
                        if (!added_transitions.count(b)) {
                            t_b = BPN.addTransition(b);
                            TransitionMap[b] = t_b;
                            added_transitions[b] = t_b;
                        } else {
                            t_b = added_transitions[b];
                        }

                        if (!added_PT_arcs.count(std::make_pair(pos, b)))
                        {
                            BPN.addArcW(pos, t_b);
                            added_PT_arcs.insert(std::make_pair(pos, b));
                            PositionMap[std::make_pair(a, b)] = pos;
                            // PositionMap[std::make_pair(b, a)] = pos;
                        } else if (added_tp_arc) {
                            PositionMap[std::make_pair(a, b)] = pos;
                        }
                    }
                }
            }

            ///Creating Output Arcs
            MapLabeledPetriNet<>::Position OutputPos = BPN.addPosition("Output");
            for (auto i : LastEventsSet) {
                MapLabeledPetriNet<>::Transition t;
                if (!added_transitions.count(i)) {
                    t = BPN.addTransition(i);
                    TransitionMap[i] = t;
                } else {
                    t = added_transitions[i];
                }
                BPN.addArcW(t, OutputPos);
                PositionMap[std::make_pair(i, "Output None")] = InputPos;
            }
        }
    };

}}} // namespace xi { namespace ldopa { namespace pn {

#endif //LDOPALIB_ALPHA_PLUS_ALGORITHM_H
