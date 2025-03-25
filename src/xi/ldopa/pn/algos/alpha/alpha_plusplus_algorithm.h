#ifndef LDOPALIB_ALPHA_PLUSPLUS_ALGORITHM_H
#define LDOPALIB_ALPHA_PLUSPLUS_ALGORITHM_H

#include "xi/ldopa/pn/algos/grviz/base_ptnet_dotwriter.h"
#include "xi/ldopa/pn/algos/alpha/alpha_plus_algorithm.h"
#include "xi/ldopa/eventlog/obsolete1/csvlog.h"
#include "xi/ldopa/pn/models/base_ptnet.h"
#include "xi/ldopa/pn/models/gen_petrinet.h"
#include "queue"

namespace xi { namespace ldopa { namespace pn {

class AlphaPlusPlusMiner : public AlphaPlusMiner {
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
        AlphaPlusPlus(TracesVec, PetriNet);

        MapLabeledPetriNetDotWriter sd;
        sd.write("C:/Users/maxga/CLionProjects/190506143700_ldopa-0.1.2/tests/gtest/work_files/logs/TEST_1.gv", PetriNet);
    }

private:
    static void AlphaPlusPlus(std::vector<std::vector<std::string>>& TracesList,
                                          MapLabeledPetriNet<>& MLPN) {
        /// step 1 and 2
        std::set<std::string> T_log;
        std::set<std::string> L1L;
        std::vector<std::string> L1L_Vec;
        std::map<std::string, std::set<std::string>> DirFollowMap;
        std::map<std::string, std::set<std::string>> TriangleFollowMap;
        std::map<std::string, std::set<std::string>> InDirFollowMap;
        std::map<std::string, std::set<std::string>> EventualFollowMap;
        std::map<std::string, std::set<std::string>> OrSplitMap;
        std::map<std::string, std::set<std::string>> OrJoinMap;
        std::map<std::string, std::set<std::string>> CasualFollowMap;

            /// T_log, L1L; Direct follow and Triangle follow
        for (auto & i : TracesList) {
            for (auto j = 0; j < i.size(); ++j) {
                T_log.insert(i[j]);
                if (j + 1 < i.size()) {
                    if (i[j] == i[j + 1]) {
                        L1L.insert(i[j]);
                        L1L_Vec.push_back(i[j]);
                    } else {
                        DirFollowMap[i[j]].insert(i[j + 1]);
                        if (j + 2 < i.size() && i[j] == i[j + 2]) {
                            TriangleFollowMap[i[j]].insert(i[j + 1]);
                        }
                    }
                }
            }
        }

            /// InDirect and Casual follow
        for (const auto& i : T_log) {
            for (const auto &j: T_log) {
                if (i != j) {
                    if (!DirFollowMap[i].count(j) && !DirFollowMap[j].count(i)) {
                        InDirFollowMap[i].insert(j);
                        InDirFollowMap[j].insert(i);
                    }
                    if (DirFollowMap[i].count(j) && !DirFollowMap[j].count(i) ||
                        TriangleFollowMap[i].count(j) || TriangleFollowMap[j].count(i)) {
                        CasualFollowMap[i].insert(j);
                    }
                }
            }
        }

            /// OrSplit and OrJoin
        for (const auto& i : T_log) {
            for (const auto& j: T_log) {
                if (InDirFollowMap[i].count(j)) {
                    for (const auto& c : T_log) {
                        if (CasualFollowMap[c].count(i) && CasualFollowMap[c].count(j)) {
                            OrSplitMap[i].insert(j);
                        }
                        if (CasualFollowMap[i].count(c) && CasualFollowMap[j].count(c)) {
                            OrJoinMap[i].insert(j);
                        }
                    }
                }
            }
        }


        /// step 3
        std::set<std::string> T_pruned_Set;
        std::vector<std::string> T_pruned_Vec;
        for (const auto& i : T_log) {
            if (!L1L.count(i)) {
                T_pruned_Set.insert(i);
                T_pruned_Vec.push_back(i);
            }
        }

        ///step 4
        std::set<std::tuple<std::set<std::string>, std::set<std::string>, std::set<std::string>>> Xw4Set;
        GenXwSetForStep4(Xw4Set, T_pruned_Vec, L1L_Vec, DirFollowMap, TriangleFollowMap, InDirFollowMap);


        /// step 5
        std::set<std::tuple<std::set<std::string>, std::set<std::string>, std::set<std::string>>> LwSet;
        for (const auto& i : Xw4Set) {
            bool ismax = true;
            for (const auto& j : Xw4Set) {
                if ((std::get<0>(i) != std::get<0>(j) || std::get<1>(i) != std::get<1>(j) ||
                        std::get<2>(i) != std::get<2>(j)) &&
                        (std::includes(std::get<0>(j).begin(), std::get<0>(j).end(),
                                   std::get<0>(i).begin(), std::get<0>(i).end()) &&
                                   std::includes(std::get<1>(j).begin(), std::get<1>(j).end(),
                                      std::get<1>(i).begin(), std::get<1>(i).end()) &&
                                      std::includes(std::get<2>(j).begin(), std::get<2>(j).end(),
                                      std::get<2>(i).begin(), std::get<2>(i).end()))) {
                    ismax = false;
                    break;
                }
            }
            if (ismax && !std::get<0>(i).empty() && !std::get<1>(i).empty() && !std::get<2>(i).empty()) {
                LwSet.insert(i);
            }
        }

        /// step 6
        std::vector<std::vector<std::string>> W_nL1L;

        /// step 7
        for (auto & temp_traces : TracesList) {
            std::vector<std::string> trace_eliminated;
            trace_eliminated.reserve(temp_traces.size());
            for (const auto & temp_trace : temp_traces) {
                trace_eliminated.push_back(temp_trace);
            }
            for (auto repeated_event : L1L) {
                EliminateTask(trace_eliminated, repeated_event);
            }
            W_nL1L.emplace_back(trace_eliminated);
        }

            /// Eventual follow
        for (const auto& i : T_pruned_Set) {
            for (const auto& j: T_pruned_Set) {
                if (!DirFollowMap[i].count(j) && i != j) {
                    bool out_valid = false;
                    for (auto trace : W_nL1L) {
                        int64_t p1 = -1;
                        int64_t p2 = -1;
                        for (size_t t = 0; t < trace.size(); ++t) {
                            bool valid = true;
                            if (trace[t] == i) {
                                p1 = t;
                            } else if (trace[t] == j) {
                                p2 = t;
                            }
                            if (p1 > -1 && p2 > -1 && p1 < p2) {
                                for (size_t iter = p1 + 1; iter < p2; ++iter) {
                                    if (trace[iter] == i || trace[iter] == j ||
                                            OrJoinMap[trace[iter]].count(i) ||
                                            OrSplitMap[trace[iter]].count(i)) {
                                        valid = false;
                                        break;
                                    }
                                }
                                if (valid && p2 - p1 > 1) {
                                    EventualFollowMap[i].insert(j);
                                    out_valid = true;
                                    break;
                                }
                            }
                        }
                        if (out_valid) {
                            break;
                        }
                    }
                }
            }
        }



        /// step 8
        std::map<std::string, std::set<std::string>> IDw1;

            /// Based on "Detecting Implicit Dependencies Between Tasks from Event Log" paper
        GetImplicitDependenciesByTheorem1(IDw1, DirFollowMap, TriangleFollowMap, InDirFollowMap, EventualFollowMap, CasualFollowMap, T_pruned_Vec);

        /// step 9
        std::set<std::pair<std::set<std::string>, std::set<std::string>>> AlphaPositionMap;
        std::map<std::string,  ::xi::ldopa::pn::GenPetriNet<>::Transition> TransitionMap;
        AlphaFixed(W_nL1L, AlphaPositionMap, T_pruned_Set, T_pruned_Vec, DirFollowMap, InDirFollowMap, TriangleFollowMap, CasualFollowMap);

        /// step 10
        for (const auto& i : IDw1) {
            for (const auto& j : i.second) {
                CasualFollowMap[i.first].insert(j);
            }
        }

        std::map<std::string, std::set<std::string>> IDw2;

            /// Based on "Detecting Implicit Dependencies Between Tasks from Event Log" paper
        GetImplicitDependenciesByTheorem2Case1(IDw2, DirFollowMap, TriangleFollowMap, InDirFollowMap, EventualFollowMap, CasualFollowMap, OrSplitMap,T_pruned_Vec);
        GetImplicitDependenciesByTheorem2Case2(IDw2, DirFollowMap, TriangleFollowMap, InDirFollowMap, EventualFollowMap, CasualFollowMap, OrJoinMap, T_pruned_Vec);


        /// step 11

        for (const auto& a : T_pruned_Set) {
            for (const auto& b : T_pruned_Set) {
                for (const auto& c : T_pruned_Set) {
                    if ((IDw2[a].count(b) && IDw2[a].count(c) &&
                    (CasualFollowMap[b].count(c) || EventualFollowMap[b].count(c))) ||
                    (IDw2[a].count(c) && IDw2[b].count(c) &&
                    (CasualFollowMap[a].count(b) || EventualFollowMap[a].count(b)))) {
                            IDw2[a].erase(c);
                    }
                }
            }
        }


        /// step 12

        std::vector<std::pair<std::set<std::string>, std::set<std::string>>> AlphaPositionVec;
        AlphaPositionVec.reserve(AlphaPositionMap.size());
        for (const auto& i : AlphaPositionMap) {
            if (!i.first.count("Input None") && !i.second.count("Output None")) {
                AlphaPositionVec.push_back(i);
            }
        }

        std::set<std::pair<std::set<std::string>, std::set<std::string>>> Xw12Set;

        for (const auto& i : IDw2) {
            for (const auto& j : i.second) {
                CasualFollowMap[i.first].insert(j);
            }
        }

            /// OrSplit and OrJoin after IDw2
        for (const auto& i : T_pruned_Set) {
            for (const auto& j: T_pruned_Set) {
                if (InDirFollowMap[i].count(j)) {
                    for (const auto& c : T_pruned_Set) {
                        if (CasualFollowMap[c].count(i) && CasualFollowMap[c].count(j)) {
                            OrSplitMap[i].insert(j);
                        }
                        if (CasualFollowMap[i].count(c) && CasualFollowMap[j].count(c)) {
                            OrJoinMap[i].insert(j);
                        }
                    }
                }
            }
        }

        GenXwSetForStep12(Xw12Set, AlphaPositionVec, InDirFollowMap, EventualFollowMap, CasualFollowMap);

        /// step 13

        std::set<std::pair<std::set<std::string>,std::set<std::string>>> YwSet_temp = Xw12Set;
        for (const auto& i : AlphaPositionMap) {
            YwSet_temp.insert(i);
        }

        std::set<std::pair<std::set<std::string>,std::set<std::string>>> YwSet;
        for (const auto& i : Xw12Set) {
            bool ismax = true;
            for (const auto& j : YwSet_temp) {
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

        for (const auto& i : AlphaPositionMap) {
            bool ismax = true;
            for (const auto& j : YwSet_temp) {
                if ((i.first != j.first || i.second != j.second) &&
                    (std::includes(j.first.begin(), j.first.end(),i.first.begin(), i.first.end())) &&
                    std::includes(j.second.begin(), j.second.end(),i.second.begin(), i.second.end())) {
                    ismax = false;
                    break;
                }
            }
            if (ismax) {
                YwSet.insert(std::make_pair(i.first, i.second));
            }
        }

        /// step 14

            /// Based on "Detecting Implicit Dependencies Between Tasks from Event Log" paper
        std::map<std::string, std::set<std::string>> IDw3;
        GetImplicitDependenciesByTheorem3(IDw3, DirFollowMap, TriangleFollowMap, InDirFollowMap, EventualFollowMap,
                                          CasualFollowMap, OrSplitMap, OrJoinMap, T_pruned_Vec);

        EliminateRDRByRule2(IDw3);


        std::set<std::pair<std::set<std::string>, std::set<std::string>>> Xw16Set;
        GenXwSetForStep16(Xw16Set, InDirFollowMap, T_pruned_Vec, IDw3);

        std::set<std::pair<std::set<std::string>, std::set<std::string>>> ZwSet;

        for (const auto& i : Xw16Set) {
            bool ismax = true;
            for (const auto& j : Xw16Set) {
                if ((i.first != j.first || i.second != j.second) &&
                    (std::includes(j.first.begin(), j.first.end(),i.first.begin(), i.first.end())) &&
                    std::includes(j.second.begin(), j.second.end(),i.second.begin(), i.second.end())) {
                    ismax = false;
                    break;
                }
            }
            if (ismax && !i.first.empty() && !i.second.empty()) {
                ZwSet.insert(i);
            }
        }

        bool Input_pos_used = false;
        xi::ldopa::pn::GenPetriNet<>::Position Input_pos;
        bool Output_pos_used = false;
        xi::ldopa::pn::GenPetriNet<>::Position Output_pos;

        std::map<std::pair<std::string, std::string>,  std::set<GenPetriNet<>::Position>> PlacesMap;
        AddPlacesWithExclusion(PlacesMap, YwSet, LwSet, MLPN, Input_pos_used, Input_pos, Output_pos_used, Output_pos);
        AddPlacesWithExclusion(PlacesMap, ZwSet, LwSet, MLPN, Input_pos_used, Input_pos, Output_pos_used, Output_pos);
        AddPlacesFrom_LwSet(PlacesMap, LwSet, MLPN, Input_pos_used, Input_pos, Output_pos_used, Output_pos);

        for (const auto& i : T_pruned_Set) {
            TransitionMap[i] = MLPN.addTransition(i);
        }

        for (const auto& i : L1L) {
            TransitionMap[i] = MLPN.addTransition(i);
        }

        std::set<std::pair<std::string, GenPetriNet<>::Position>> TP_added_set;
        std::set<std::pair<GenPetriNet<>::Position, std::string>> PT_added_set;

        for (const auto& P : PlacesMap) {
            if (P.first.first != "Input None" && P.first.second != "Output None") {
                for (auto i : P.second) {
                    if (!TP_added_set.count(std::make_pair(P.first.first, i))) {
                        MLPN.addArcW(TransitionMap[P.first.first], i);
                        TP_added_set.insert(std::make_pair(P.first.first, i));
                    }
                    if (!PT_added_set.count(std::make_pair(i, P.first.second))) {
                        MLPN.addArcW(i, TransitionMap[P.first.second]);
                        PT_added_set.insert(std::make_pair(i, P.first.second));
                    }
                }
            } else if (P.first.first == "Input None") {
                for (auto i: P.second) {
                    MLPN.addArcW(i, TransitionMap[P.first.second]);
                }
            } else {
                for (auto i : P.second) {
                    MLPN.addArcW(TransitionMap[P.first.first], i);
                }
            }
        }

    }

    static void GenXwSetForStep4(std::set<std::tuple<std::set<std::string>, std::set<std::string>, std::set<std::string>>>& XwSet,
                         std::vector<std::string>& T_pruned_Vec, std::vector<std::string>& L1L_Vec,
                         std::map<std::string, std::set<std::string>>& DirFollowMap,
                         std::map<std::string, std::set<std::string>>& TriangleFollowMap,
                         std::map<std::string, std::set<std::string>>& InDirFollowMap) {
        uint64_t n_t = T_pruned_Vec.size();
        uint64_t n_l = L1L_Vec.size();
        for (auto mask = 0; mask < (1 << n_t); ++mask) {
            bool A_valid = true;
            std::set<std::string> A;
            for (auto i = 0; i < n_t; ++i) {
                if (mask & (1 << i)) {
                    std::string A_candidate = T_pruned_Vec[i];
                    for (const auto &j: A) {
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
            if (!A_valid) {
                continue;
            }
            for (auto mask_b = 0; mask_b < (1 << n_t); ++mask_b) {
                std::set<std::string> B;
                bool B_valid = true;
                for (auto i = 0; i < n_t; ++i) {
                    if (mask_b & (1 << i)) {
                        std::string B_candidate = T_pruned_Vec[i];
                        for (const auto& j : B) {
                            if (!InDirFollowMap[j].count(B_candidate)) {
                                B_valid = false;
                                break;
                            }
                        }
                        if (!B_valid) {
                            break;
                        }
                        for (const auto& j : A) {
                            if (DirFollowMap[j].count(B_candidate) && DirFollowMap[B_candidate].count(j) &&
                                !(TriangleFollowMap[j].count(B_candidate) || TriangleFollowMap[B_candidate].count(j))) {
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
                if (!B_valid) {
                    continue;
                }

                for (auto mask_c = 0; mask_c < (1 << n_l); ++mask_c) {
                    std::set<std::string> C;
                    bool C_valid = true;
                    for (auto i = 0; i < n_l; ++i) {
                        if (mask_c & (1 << i)) {
                            std::string C_candidate = L1L_Vec[i];
                            for (const auto& j : A) {
                                if (!(DirFollowMap[j].count(C_candidate) &&
                                      !(TriangleFollowMap[C_candidate].count(j)))) {
                                    C_valid = false;
                                    break;
                                }
                            }
                            if (!C_valid) {
                                break;
                            }
                            for (const auto& j : B) {
                                if (!(DirFollowMap[C_candidate].count(j) &&
                                      !(TriangleFollowMap[C_candidate].count(j)))) {
                                    C_valid = false;
                                    break;
                                }
                            }
                            if (!C_valid) {
                                break;
                            }
                            C.insert(C_candidate);
                        }
                    }
                    if (!C_valid) {
                        continue;
                    }
                    XwSet.insert(std::make_tuple(A, B, C));
                }
            }
        }
    }

    static void GenXwSetForStep12(std::set<std::pair<std::set<std::string>, std::set<std::string>>>& XwSet,
                                  std::vector<std::pair<std::set<std::string>, std::set<std::string>>>& AlphaPositionVec,
                                  std::map<std::string, std::set<std::string>>& InDirFollowMap,
                                  std::map<std::string, std::set<std::string>>& EventualFollowMap,
                                  std::map<std::string, std::set<std::string>>& CasualFollow_IDw12Map) {

        std::set<std::string> IDwTransSet_forA;
        std::set<std::string> IDwTransSet_forB;
        for (const auto& i : CasualFollow_IDw12Map) {
            IDwTransSet_forA.insert(i.first);
            for (const auto& j : i.second) {
                IDwTransSet_forB.insert(j);
            }
        }

        std::vector<std::string> IDwTransVec_forA;
        std::vector<std::string> IDwTransVec_forB;
        IDwTransVec_forA.reserve(IDwTransSet_forA.size());
        for (const auto& i : IDwTransSet_forA) {
            IDwTransVec_forA.push_back(i);
        }
        IDwTransVec_forB.reserve(IDwTransSet_forB.size());
        for (const auto& i : IDwTransSet_forB) {
            IDwTransVec_forB.push_back(i);
        }

        size_t n_a = IDwTransVec_forA.size();
        size_t n_b = IDwTransVec_forB.size();

        for (auto P : AlphaPositionVec) {
            for (auto mask_1 = 0; mask_1 < (1 << n_a); ++mask_1) {
                std::set<std::string> A2;
                bool A2_valid = true;
                for (auto i = 0; i < n_a; ++i) {
                    if (mask_1 & (1 << i)) {
                        std::string a2_candidate = IDwTransVec_forA[i];
                        if (P.first.count(a2_candidate)) {
                            A2_valid = false;
                            break;
                        }
                        for (const auto& a : P.first) {
                            if (!InDirFollowMap[a2_candidate].count(a)) {
                                A2_valid = false;
                                break;
                            }
                        }
                        if (!A2_valid) {
                            break;
                        }
                        for (const auto& b : P.second) {
                            if (!CasualFollow_IDw12Map[a2_candidate].count(b)) {
                                A2_valid = false;
                                break;
                            }
                        }
                        if (!A2_valid) {
                            break;
                        }
                        A2.insert(a2_candidate);
                    }
                }
                if (!A2_valid) {
                    continue;
                }
                for (auto mask_2 = 0; mask_2 < (1 << n_b); ++mask_2) {
                    std::set<std::string> B2;
                    bool B2_valid = true;
                    for (auto i = 0; i < n_b; ++i) {
                        if (mask_2 & (1 << i)) {
                            std::string b2_candidate = IDwTransVec_forB[i];
                            if (P.second.count(b2_candidate)) {
                                B2_valid = false;
                                break;
                            }
                            for (const auto& b : P.second) {
                                if (!InDirFollowMap[b].count(b2_candidate)) {
                                    B2_valid = false;
                                    break;
                                }
                            }

                            if (!B2_valid) {
                                break;
                            }

                            for (const auto& a : P.first) {
                                if (!CasualFollow_IDw12Map[a].count(b2_candidate)) {
                                    B2_valid = false;
                                    break;
                                }
                            }

                            if (!B2_valid) {
                                break;
                            }

                            for (const auto& a : A2) {
                                if (!CasualFollow_IDw12Map[a].count(b2_candidate)) {
                                    B2_valid = false;
                                    break;
                                }
                            }
                            if (!B2_valid) {
                                break;
                            }

                            B2.insert(b2_candidate);
                        }
                    }
                    if (!B2_valid) {
                        continue;
                    }
                    if (!B2.empty() || !A2.empty()) {
                        A2.insert(P.first.begin(), P.first.end());
                        B2.insert(P.second.begin(), P.second.end());
                        XwSet.insert(std::make_pair(A2, B2));
                    }
                }
            }
        }
    }

    static void GenXwSetForStep16(std::set<std::pair<std::set<std::string>, std::set<std::string>>>& XwSet,
                                  std::map<std::string, std::set<std::string>>& InDirFollowMap,
                                  std::vector<std::string>& T_pruned_Vec,
                                  std::map<std::string, std::set<std::string>>& IDw3) {
        size_t n = T_pruned_Vec.size();
        for (auto mask = 0; mask < (1 << n); ++mask) {
            bool A_valid = true;
            std::set<std::string> A;
            for (auto i = 0; i < n; ++i) {
                if (mask & (1 << i)) {
                    std::string A_candidate = T_pruned_Vec[i];
                    for (const auto &j: A) {
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
                        std::string B_candidate = T_pruned_Vec[i];
                        for (const auto& j: B) {
                            if (!InDirFollowMap[j].count(B_candidate)) {
                                B_valid = false;
                                break;
                            }
                        }
                        if (!B_valid) {
                            break;
                        }
                        for (const auto& j: A) {
                            if (!IDw3[j].count(B_candidate)) {
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
    }

    static void GetImplicitDependenciesByTheorem1(std::map<std::string, std::set<std::string>>& IDw1,
                                                   std::map<std::string, std::set<std::string>>& DirFollowMap,
                                                   std::map<std::string, std::set<std::string>>& TriangleFollowMap,
                                                   std::map<std::string, std::set<std::string>>& InDirFollowMap,
                                                   std::map<std::string, std::set<std::string>>& EventualFollowMap,
                                                   std::map<std::string, std::set<std::string>>& CasualFollowMap,
                                                   std::vector<std::string>& T_pruned_Vec) {

        std::set<std::pair<std::set<std::string>, std::set<std::string>>> XwSet;
        for (const auto& t : T_pruned_Vec) {
            bool t_1_2_found = false;
            for (const auto& t_1 : T_pruned_Vec) {
                for (const auto& t_2 : T_pruned_Vec) {
                    if (t_1 != t_2 && CasualFollowMap[t_1].count(t) && CasualFollowMap[t_2].count(t) &&
                        InDirFollowMap[t_1].count(t_2) && CasualFollowMap[t_1] != CasualFollowMap[t_2]) {
                        t_1_2_found = true;
                        break;
                    }
                }
                if (t_1_2_found) {
                    break;
                }
            }
            if (!t_1_2_found) {
                continue;
            }
            uint64_t n = T_pruned_Vec.size();
            for (auto mask = 0; mask < (1 << n); ++mask) {
                bool A_valid = true;
                std::set<std::string> A;
                for (auto i = 0; i < n; ++i) {
                    if (mask & (1 << i)) {
                        std::string A_candidate = T_pruned_Vec[i];
                        for (const auto &j: A) {
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
                            std::string B_candidate = T_pruned_Vec[i];
                            for (const auto& j: B) {
                                if (!InDirFollowMap[j].count(B_candidate)) {
                                    B_valid = false;
                                    break;
                                }
                            }
                            if (!B_valid) {
                                break;
                            }
                            for (const auto& j: A) {
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
                    if (!B_valid || !B.count(t)) {
                        continue;
                    }
                    XwSet.insert(std::make_pair(A, B));
                }
            }

            std::set<std::pair<std::set<std::string>, std::set<std::string>>> YwSet;
            for (const auto &i: XwSet) {
                bool ismax = true;
                for (const auto &j: XwSet) {
                    if ((i.first != j.first || i.second != j.second) &&
                        std::includes(j.first.begin(), j.first.end(), i.first.begin(), i.first.end()) &&
                        std::includes(j.second.begin(), j.second.end(), i.second.begin(), i.second.end())) {
                        ismax = false;
                        break;
                    }
                }
                if (ismax) {
                    YwSet.insert(i);
                }
            }

            for (const auto& pair1: YwSet) {
                for (const auto& pair2: YwSet) {
                    for (const auto& a1: pair1.first) {
                        if (!pair2.first.count(a1)) {
                            bool valid = true;
                            for (const auto& a2: pair2.first) {
                                if ((DirFollowMap[a1].count(a2) && DirFollowMap[a2].count(a1) &&
                                    !(TriangleFollowMap[a1].count(a2) || TriangleFollowMap[a2].count(a1))) ||
                                    CasualFollowMap[a2].count(a1) || EventualFollowMap[a2].count(a1)) {
                                    valid = false;
                                    break;
                                }
                            }
                            if (valid) {
                                for (const auto& b_candidate: pair2.second) {
                                    if (!CasualFollowMap[a1].count(b_candidate)) {
                                        IDw1[a1].insert(b_candidate);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    static void  GetImplicitDependenciesByTheorem2Case1(std::map<std::string, std::set<std::string>>& IDw2,
                                                   std::map<std::string, std::set<std::string>>& DirFollowMap,
                                                   std::map<std::string, std::set<std::string>>& TriangleFollowMap,
                                                   std::map<std::string, std::set<std::string>>& InDirFollowMap,
                                                   std::map<std::string, std::set<std::string>>& EventualFollowMap,
                                                   std::map<std::string, std::set<std::string>>& CasualFollowMap,
                                                   std::map<std::string, std::set<std::string>>& OrSplitMap,
                                                   std::vector<std::string>& T_pruned_Vec) {
        size_t n = T_pruned_Vec.size();
        for (const auto& t : T_pruned_Vec) {
            std::set<std::set<std::string>> XwSet;
            bool t_1_2_found = false;
            for (const auto& t_1 : T_pruned_Vec) {
                for (const auto& t_2 : T_pruned_Vec) {
                    if (t_1 != t_2 && CasualFollowMap[t].count(t_1) && CasualFollowMap[t].count(t_2) &&
                    DirFollowMap[t_1].count(t_2) && DirFollowMap[t_2].count(t_1) &&
                    !TriangleFollowMap[t_1].count(t_2) && !TriangleFollowMap[t_2].count(t_1)) {
                        t_1_2_found = true;
                        break;
                    }
                }
                if (t_1_2_found) {
                    break;
                }
            }
            if (!t_1_2_found) {
                continue;
            }
            for (auto mask = 0; mask < (1 << n); ++mask) {
                bool X_valid = true;
                std::set<std::string> X;
                for (auto i = 0; i < n; ++i) {
                    if (mask & (1 << i)) {
                        std::string X_candidate = T_pruned_Vec[i];
                        if (!CasualFollowMap[t].count(X_candidate)) {
                            X_valid = false;
                            break;

                        }
                        for (const auto& x_t : X) {
                            if (!InDirFollowMap[x_t].count(X_candidate)) {
                                X_valid = false;
                                break;
                            }
                        }
                        if (!X_valid) {
                            break;
                        }
                        X.insert(X_candidate);
                    }
                }
                if (X_valid && !X.empty()) {
                    XwSet.insert(X);
                }
            }


            std::set<std::set<std::string>> YwSet;
            for (const auto& i : XwSet) {
                bool ismax = true;
                for (const auto& j : XwSet) {
                    if ((i != j) &&
                        (std::includes(j.begin(), j.end(),i.begin(), i.end()))) {
                        ismax = false;
                        break;
                    }
                }
                if (ismax && !i.empty() && !i.empty()) {
                    YwSet.insert(i);
                }
            }

            for (const auto& i : YwSet) {
                for (const auto& a : T_pruned_Vec) {
                    bool y_2_valid = true;
                    for (const auto& y: i) {
                        if ((DirFollowMap[y].count(a) && DirFollowMap[a].count(y) &&
                            !TriangleFollowMap[a].count(y) && !TriangleFollowMap[y].count(a)) ||
                            CasualFollowMap[y].count(a) || EventualFollowMap[y].count(a)) {
                            y_2_valid = false;
                            break;
                        }
                    }
                    if (!y_2_valid) {
                        continue;
                    }

                    for (const auto& b : T_pruned_Vec) {
                        if (OrSplitMap[a].count(b)) {
                            bool y_1_valid = false;
                            for (const auto& y: i) {
                                if ((DirFollowMap[y].count(b) && DirFollowMap[b].count(y) &&
                                    !(TriangleFollowMap[y].count(b) || TriangleFollowMap[b].count(y))) ||
                                    CasualFollowMap[y].count(b) || EventualFollowMap[y].count(b)) {
                                    y_1_valid = true;
                                    break;
                                }
                            }
                            if (!y_1_valid) {
                                continue;
                            }

                            if (EventualFollowMap[t].count(a)) {
                                IDw2[t].insert(a);
                            }
                        }
                    }
                }
            }
        }

    }

    static void  GetImplicitDependenciesByTheorem2Case2(std::map<std::string, std::set<std::string>>& IDw2,
                                                        std::map<std::string, std::set<std::string>>& DirFollowMap,
                                                        std::map<std::string, std::set<std::string>>& TriangleFollowMap,
                                                        std::map<std::string, std::set<std::string>>& InDirFollowMap,
                                                        std::map<std::string, std::set<std::string>>& EventualFollowMap,
                                                        std::map<std::string, std::set<std::string>>& CasualFollowMap,
                                                        std::map<std::string, std::set<std::string>>& OrJoinMap,
                                                        std::vector<std::string>& T_pruned_Vec) {
        size_t n = T_pruned_Vec.size();
        for (const auto& t : T_pruned_Vec) {
            std::set<std::set<std::string>> XwSet;
           bool t_1_2_found = false;
            for (const auto& t_1 : T_pruned_Vec) {
                for (const auto& t_2 : T_pruned_Vec) {
                    if (t_1 != t_2 && CasualFollowMap[t_1].count(t) && CasualFollowMap[t_2].count(t) &&
                        DirFollowMap[t_1].count(t_2) && DirFollowMap[t_2].count(t_1) &&
                        !TriangleFollowMap[t_1].count(t_2) && !TriangleFollowMap[t_2].count(t_1)) {
                        t_1_2_found = true;
                        break;
                    }
                }
                if (t_1_2_found) {
                    break;
                }
            }
            if (!t_1_2_found) {
                continue;
            }
            for (auto mask = 0; mask < (1 << n); ++mask) {
                bool X_valid = true;
                std::set<std::string> X;
                for (auto i = 0; i < n; ++i) {
                    if (mask & (1 << i)) {
                        std::string X_candidate = T_pruned_Vec[i];
                        if (!CasualFollowMap[X_candidate].count(t)) {
                            X_valid = false;
                            break;

                        }
                        for (const auto& x_t : X) {
                            if (!InDirFollowMap[x_t].count(X_candidate)) {
                                X_valid = false;
                                break;
                            }
                        }
                        if (!X_valid) {
                            break;
                        }
                        X.insert(X_candidate);
                    }
                }
                if (X_valid && !X.empty()) {
                    XwSet.insert(X);
                }
            }

            std::set<std::set<std::string>> YwSet;
            for (const auto& i : XwSet) {
                bool ismax = true;
                for (const auto& j : XwSet) {
                    if ((i != j) &&
                        (std::includes(j.begin(), j.end(),i.begin(), i.end()))) {
                        ismax = false;
                        break;
                    }
                }
                if (ismax) {
                    YwSet.insert(i);
                }
            }
            for (const auto& i : YwSet) {
                for (const auto& a : T_pruned_Vec) {

                    bool y_2_valid = true;
                    for (const auto& y: i) {
                        if (DirFollowMap[y].count(a) && DirFollowMap[a].count(y) &&
                            !(TriangleFollowMap[a].count(y) || TriangleFollowMap[y].count(a)) ||
                            CasualFollowMap[a].count(y) || EventualFollowMap[a].count(y)) {
                            y_2_valid = false;
                            break;
                        }
                    }
                    if (!y_2_valid) {
                        continue;
                    }

                    for (const auto& b : T_pruned_Vec) {
                        if (OrJoinMap[a].count(b)) {
                            bool y_1_valid = false;
                            for (const auto& y: i) {
                                if (DirFollowMap[y].count(b) && DirFollowMap[b].count(y) &&
                                    !(TriangleFollowMap[y].count(b) || TriangleFollowMap[b].count(y)) ||
                                    CasualFollowMap[b].count(y) || EventualFollowMap[b].count(y)) {
                                    y_1_valid = true;
                                    break;
                                }
                            }
                            if (!y_1_valid) {
                                continue;
                            }
                            if (EventualFollowMap[a].count(t)) {
                                IDw2[a].insert(t);
                            }
                        }
                    }
                }
            }
        }


    }

    static void  GetImplicitDependenciesByTheorem3(std::map<std::string, std::set<std::string>>& IDw3,
                                                        std::map<std::string, std::set<std::string>>& DirFollowMap,
                                                        std::map<std::string, std::set<std::string>>& TriangleFollowMap,
                                                        std::map<std::string, std::set<std::string>>& InDirFollowMap,
                                                        std::map<std::string, std::set<std::string>>& EventualFollowMap,
                                                        std::map<std::string, std::set<std::string>>& CasualFollowMap,
                                                        std::map<std::string, std::set<std::string>>& OrSplitMap,
                                                        std::map<std::string, std::set<std::string>>& OrJoinMap,
                                                        std::vector<std::string>& T_pruned_Vec) {
        size_t n = T_pruned_Vec.size();
        for (const auto& a: T_pruned_Vec) {
            for (const auto& b: T_pruned_Vec) {
                if (OrJoinMap[a].count(b)) {
                    std::set<std::pair<std::set<std::string>, std::set<std::string>>> XwSet;
                    for (auto mask_a = 0; mask_a < (1 << n); ++mask_a) {
                        bool A_valid = true;
                        std::set<std::string> A;
                        for (auto i = 0; i < n; ++i) {
                            if (mask_a & (1 << i)) {
                                std::string a_candidate = T_pruned_Vec[i];
                                if (!EventualFollowMap[a].count(a_candidate) || EventualFollowMap[b].count(a_candidate)) {
                                    A_valid = false;
                                    break;
                                }
                                for (const auto& a_i : A) {
                                    if (!(DirFollowMap[a_candidate].count(a_i) && DirFollowMap[a_i].count(a_candidate) &&
                                        !(TriangleFollowMap[a_candidate].count(a_i) || TriangleFollowMap[a_i].count(a_candidate)))) {
                                        A_valid = false;
                                        break;
                                    }
                                }
                                if (!A_valid) {
                                    break;
                                }
                                A.insert(a_candidate);
                            }
                        }
                        if (!A_valid) {
                            continue;
                        }
                        for (auto mask_b = 0; mask_b < (1 << n); ++mask_b) {
                            bool B_valid = true;
                            std::set<std::string> B;
                            for (auto i = 0; i < n; ++i) {
                                if (mask_b & (1 << i)) {
                                    std::string b_candidate = T_pruned_Vec[i];
                                    if (EventualFollowMap[a].count(b_candidate) || !EventualFollowMap[b].count(b_candidate)) {
                                        B_valid = false;
                                        break;
                                    }
                                    for (const auto& b_i : B) {
                                        if (!(DirFollowMap[b_candidate].count(b_i) && DirFollowMap[b_i].count(b_candidate) &&
                                              !(TriangleFollowMap[b_candidate].count(b_i) || TriangleFollowMap[b_i].count(b_candidate)))) {
                                            B_valid = false;
                                            break;
                                        }
                                    }
                                    if (!B_valid) {
                                        break;
                                    }
                                    B.insert(b_candidate);
                                }
                            }
                            if (!B_valid) {
                                continue;
                            }
                            for (const auto& a_i : A) {
                                B_valid = false;
                                for (const auto& b_i : B) {
                                    if (OrSplitMap[b_i].count(a_i)) {
                                        B_valid = true;
                                        break;
                                    }
                                }
                                if (!B_valid) {
                                    break;
                                }
                            }
                            if (!B_valid) {
                                continue;
                            }
                            for (const auto& b_i : B) {
                                B_valid = false;
                                for (const auto& a_i : A) {
                                    if (OrSplitMap[a_i].count(b_i)) {
                                        B_valid = true;
                                        break;
                                    }
                                }
                                if (!B_valid) {
                                    break;
                                }
                            }
                            if (!B_valid) {
                                continue;
                            }
                            XwSet.insert(std::make_pair(A, B));
                        }
                    }

                    std::set<std::pair<std::set<std::string>, std::set<std::string>>> YwSet;
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

                    for (const auto& y :YwSet) {
                        std::set<std::string> A_sub;
                        std::set<std::string> B_sub;
                        for (const auto& t: T_pruned_Vec) {
                            if (!y.first.count(t)) {
                                bool t_valid = false;
                                for (const auto& b_i : y.second) {
                                    if (OrSplitMap[b_i].count(t)) {
                                        t_valid = true;
                                        break;
                                    }
                                }
                                if (t_valid) {
                                    t_valid = false;
                                    for (const auto& a_i: y.first) {
                                        if (CasualFollowMap[a_i].count(t) || EventualFollowMap[a_i].count(t)) {
                                            t_valid = true;
                                            break;
                                        }
                                    }
                                    if (t_valid) {
                                        A_sub.insert(t);
                                    }
                                }
                            }
                            if (!y.second.count(t)) {
                                bool t_valid = false;
                                for (const auto& a_i : y.first) {
                                    if (OrSplitMap[a_i].count(t)) {
                                        t_valid = true;
                                        break;
                                    }
                                }
                                if (t_valid) {
                                    t_valid = false;
                                    for (const auto& b_i: y.second) {
                                        if (CasualFollowMap[b_i].count(t) || EventualFollowMap[b_i].count(t)) {
                                            t_valid = true;
                                            break;
                                        }
                                    }
                                    if (t_valid) {
                                        B_sub.insert(t);
                                    }
                                }
                            }
                        }

                        std::set<std::string> sub_inc;
                        for (const auto& B : y.second) {
                            for (const auto& i : CasualFollowMap) {
                                if (i.second.count(B)) {
                                    sub_inc.insert(i.first);
                                }
                            }
                        }
                        for (const auto& B : B_sub) {
                            for (const auto& i : CasualFollowMap) {
                                if (i.second.count(B)) {
                                    sub_inc.insert(i.first);
                                }
                            }
                        }

                        for (const auto& a_i : y.first) {
                            bool a_i_valid = true;
                            for (const auto& i : CasualFollowMap) {
                                if (i.second.count(a_i)) {
                                    if (!sub_inc.count(i.first)) {
                                        a_i_valid = false;
                                        break;
                                    }
                                }
                            }
                        }

                        sub_inc.clear();
                        for (const auto& A : y.first) {
                            for (const auto& i : CasualFollowMap) {
                                if (i.second.count(A)) {
                                    sub_inc.insert(i.first);
                                }
                            }
                        }
                        for (const auto& A : A_sub) {
                            for (const auto& i : CasualFollowMap) {
                                if (i.second.count(A)) {
                                    sub_inc.insert(i.first);
                                }
                            }
                        }

                        for (const auto& b_i : y.second) {
                            bool b_i_valid = true;
                            for (const auto& i : CasualFollowMap) {
                                if (i.second.count(b_i)) {
                                    if (!sub_inc.count(i.first)) {
                                        b_i_valid = false;
                                        break;
                                    }
                                }
                            }
                            if (b_i_valid) {
                                IDw3[b].insert(b_i);
                            }
                        }
                    }
                }
            }
        }
    }

    static bool isReachable(std::map<std::string, std::set<std::string>>& IDw3, const std::string& a, const std::string& b, std::set<std::string>& visited) {
        if (a == b) {
            return true;
        }
        visited.insert(a);

        for (const std::string& neighbor : IDw3[a]) {
            if (!visited.count(neighbor) && isReachable(IDw3, neighbor, b, visited)) {
                return true;
            }
        }
        return false;
    }

    static bool HasPathL2(std::map<std::string, std::set<std::string>>& IDw3, const std::string& a, const std::string& b) {
        for (const std::string& neighbor : IDw3[a]) {
            if (neighbor == b) {
                continue;
            }
            std::set<std::string> visited;
            if (isReachable(IDw3, neighbor, b, visited)) {
                return true;
            }
        }
        return false;
    }

    static void EliminateRDRByRule2(std::map<std::string, std::set<std::string>>& IDw3) {
        std::set<std::pair<std::string, std::string>> RemoveSet;
        for (const auto& i : IDw3) {
            std::string a = i.first;
            for (const auto& b : i.second) {
                if (HasPathL2(IDw3, a, b)) {
                    RemoveSet.insert(std::make_pair(a, b));
                }
            }
        }

        for (const auto& i : RemoveSet) {
            IDw3[i.first].erase(i.second);
        }
    }

    static void AddPlacesWithExclusion(std::map<std::pair<std::string, std::string>, std::set<GenPetriNet<>::Position>>& PlacesMap,
                                 std::set<std::pair<std::set<std::string>, std::set<std::string>>>& FromSet,
                                 std::set<std::tuple<std::set<std::string>, std::set<std::string>, std::set<std::string>>>& ExcludeSet,
                                 MapLabeledPetriNet<>& MLPN, bool& Input_pos_used, xi::ldopa::pn::GenPetriNet<>::Position& Input_pos,
                                 bool& Output_pos_used, xi::ldopa::pn::GenPetriNet<>::Position& Output_pos) {
        for (const auto& i : FromSet) {
            std::set<std::string> A = i.first;
            std::set<std::string> B = i.second;
            if (!A.count("Input None") && !B.count("Output None")) {
                bool A_B_valid = true;
                for (auto L : ExcludeSet) {
                    if (std::get<0>(L) == A && std::get<1>(L) == B) {
                        A_B_valid = false;
                        break;
                    }
                }
                if (!A_B_valid) {
                    continue;
                }
                GenPetriNet<>::Position pos = MLPN.addPosition();
                for (const auto& a : A) {
                    for (const auto& b : B) {
                        PlacesMap[std::make_pair(a, b)].insert(pos);
                    }
                }
            }
            if (A.count("Input None")) {
                if (!Input_pos_used) {
                    Input_pos = MLPN.addPosition("Input");
                    Input_pos_used = true;
                }
                for (const auto& b : B) {
                    if (!PlacesMap.count(std::make_pair("Input None", b))) {
                        PlacesMap[std::make_pair("Input None",b)].insert(Input_pos);
                    }
                }
            }
            if (B.count("Output None")) {
                if (!Output_pos_used) {
                    Output_pos = MLPN.addPosition("Output");
                    Output_pos_used = true;
                }
                for (const auto& a : A) {
                    if (!PlacesMap.count(std::make_pair(a, "Output None"))) {
                        PlacesMap[std::make_pair(a, "Output None")].insert(Output_pos);
                    }
                }
            }
        }
    }

    static void AddPlacesFrom_LwSet(std::map<std::pair<std::string, std::string>, std::set<GenPetriNet<>::Position>>& PlacesMap,
                                    std::set<std::tuple<std::set<std::string>, std::set<std::string>, std::set<std::string>>>& LwSet,
                                    MapLabeledPetriNet<>& MLPN, bool& Input_pos_used, xi::ldopa::pn::GenPetriNet<>::Position& Input_pos,
                                    bool& Output_pos_used, xi::ldopa::pn::GenPetriNet<>::Position& Output_pos) {
        for (auto i : LwSet) {
            std::set<std::string> A_sub = std::get<0>(i);
            A_sub.insert(std::get<2>(i).begin(), std::get<2>(i).end());
            std::set<std::string> B_sub = std::get<1>(i);
            B_sub.insert(std::get<2>(i).begin(), std::get<2>(i).end());
            if (!A_sub.count("Input None") && !B_sub.count("Output None")) {
                GenPetriNet<>::Position pos = MLPN.addPosition();
                for (const auto& a : A_sub) {
                    for (const auto& b : B_sub) {
                        PlacesMap[std::make_pair(a, b)].insert(pos);
                    }
                }
            }
            if (A_sub.count("Input None")) {
                if (!Input_pos_used) {
                    Input_pos = MLPN.addPosition("Input");
                    Input_pos_used = true;
                }
                for (const auto& b : B_sub) {
                    if (!PlacesMap.count(std::make_pair("Input None", b))) {
                        PlacesMap[std::make_pair("Input None",b)].insert(Input_pos);
                    }
                }
            }
            if (B_sub.count("Output None")) {
                if (!Output_pos_used) {
                    Output_pos = MLPN.addPosition("Output");
                    Output_pos_used = true;
                }
                for (const auto& a : A_sub) {
                    if (!PlacesMap.count(std::make_pair(a, "Output None"))) {
                        PlacesMap[std::make_pair(a, "Output None")].insert(Output_pos);
                    }
                }
            }
        }
    }


    static void AlphaFixed(const std::vector<std::vector<std::string>>& TracesList,
                            std::set<std::pair<std::set<std::string>, std::set<std::string>>>& PositionMap,
                            std::set<std::string>& AllEventsSet,
                            std::vector<std::string>& AllEventsVec,
                            std::map<std::string, std::set<std::string>>& DirFollowMap,
                            std::map<std::string, std::set<std::string>>& InDirFollowMap,
                            std::map<std::string, std::set<std::string>>& TriangleFollowMap,
                            std::map<std::string, std::set<std::string>>& CasualFollowMap) {

        /// step 1 - 3
        std::set<std::string> FirstEventsSet;
        std::set<std::string> LastEventsSet;

        /// Fist and Last events
        for (auto i: TracesList) {
            for (auto j = 0; j < i.size(); ++j) {
                if (j == 0) {
                    FirstEventsSet.insert(i[j]);
                }
                if (j == i.size() - 1) {
                    LastEventsSet.insert(i[j]);
                }
            }
        }

        /// step 4
        uint64_t n = AllEventsVec.size();
        std::set<std::pair<std::set<std::string>, std::set<std::string>>> XwSet;
        for (auto mask = 0; mask < (1 << n); ++mask) {
            bool A_valid = true;
            std::set<std::string> A;
            for (auto i = 0; i < n; ++i) {
                if (mask & (1 << i)) {
                    std::string A_candidate = AllEventsVec[i];
                    for (const auto &j: A) {
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
                        for (const auto &j: B) {
                            if (!InDirFollowMap[j].count(B_candidate)) {
                                B_valid = false;
                                break;
                            }
                        }
                        if (!B_valid) {
                            break;
                        }
                        for (const auto &j: A) {
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
        std::set<std::pair<std::set<std::string>, std::set<std::string>>> YwSet;
        for (const auto &i: XwSet) {
            bool ismax = true;
            for (const auto &j: XwSet) {
                if ((i.first != j.first || i.second != j.second) &&
                    std::includes(j.first.begin(), j.first.end(), i.first.begin(), i.first.end()) &&
                    std::includes(j.second.begin(), j.second.end(), i.second.begin(), i.second.end())) {
                    ismax = false;
                    break;
                }
            }
            if (ismax && !i.first.empty() && !i.second.empty()) {
                YwSet.insert(i);
            }
        }

        /// step 6-8

        std::set<std::string> a_input = {"Input None"};
        std::set<std::string> b_input;
        for (const auto &i: FirstEventsSet) {
            b_input.insert(i);
        }
        PositionMap.insert(std::make_pair(a_input, b_input));

        for (auto i : YwSet) {
            std::set<std::string> a_t;
            std::set<std::string> b_t;
            for (auto a : i.first) {
                a_t.insert(a);
                for (auto b : i.second) {
                    b_t.insert(b);
                }
            }
            PositionMap.insert(std::make_pair(a_t, b_t));
        }

        std::set<std::string> a_output;
        std::set<std::string> b_output = {"Output None"};
        for (const auto &i: LastEventsSet) {
            a_output.insert(i);
        }
        PositionMap.insert(std::make_pair(a_output, b_output));
    }
};

}}} // namespace xi { namespace ldopa { namespace pn {

#endif //LDOPALIB_ALPHA_PLUSPLUS_ALGORITHM_H
