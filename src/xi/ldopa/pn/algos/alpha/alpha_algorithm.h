#ifndef LDOPALIB_ALPHA_ALGORITHM_H
#define LDOPALIB_ALPHA_ALGORITHM_H

#pragma once

#include "gtest/gtest.h"

// ldopa dll
#include "xi/ldopa/ldopa_dll.h"
#include <xi/ldopa/utils.h>

#include "xi/ldopa/eventlog/sqlite/sqlitelog.h"
#include "xi/ldopa/pn/models/base_ptnet.h"
#include "xi/ldopa/eventlog/obsolete1/csvlog.h"
#include "xi/ldopa/graphs/bidigraph.h"
#include "xi/ldopa/pn/algos/grviz/base_ptnet_dotwriter.h"
#include "xi/ldopa/pn/algos/grviz/evlog_ptnets_dotwriter.h"


namespace xi { namespace ldopa { namespace pn {

    class AlphaMiner {
    public:
        static void GenPetriNetFromCSVLog(eventlog::obsolete1::CSVLogTraces* TracesList,  MapLabeledPetriNet<>* PetriNet) {
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

            Alpha(TracesVec, *PetriNet);
        }

        static void GenPetriNetFromSQL(eventlog::SQLiteLog* Log, MapLabeledPetriNet<>* PetriNet){
            std::vector<std::vector<std::string>> TracesVec;
            auto _actAttrID = Log->getEvActAttrId();
            size_t iter = 0;
            for (auto i = 0; i < Log->getTracesNum(); ++i) {
                TracesVec.emplace_back();
                auto trace = Log->getTrace(i);
                for (auto j = 0; j < trace->getSize(); ++j) {
                    auto event = trace->getEvent(j);
                    xi::ldopa::eventlog::IEventLog::Attribute actAttr;
                    event->getAttr(_actAttrID.c_str(), actAttr);
                    TracesVec[iter].push_back(actAttr.toString());
                }
                ++iter;
            }
            Alpha(TracesVec, *PetriNet);
        };

    private:
        static void Alpha(
                const std::vector<std::vector<std::string>>& TracesList, MapLabeledPetriNet<>& BPN) {
            std::map<std::string, std::set<std::string>> DirFollowMap;
            std::map<std::string, std::set<std::string>> InDirFollowMap;
            std::set<std::string> FirstEventsSet;
            std::set<std::string> LastEventsSet;
            std::set<std::string> AllEventsSet;


            ///Directional Matrix, Fist and Last Events
            std::vector<std::string> EventsVec;

            for (auto i : TracesList) {
                for (auto j = 0; j < i.size(); ++j) {
                    if (!AllEventsSet.count(i[j])) {
                        AllEventsSet.insert(i[j]);
                        EventsVec.push_back(i[j]);
                    }
                    if (j == 0) {
                        FirstEventsSet.insert(i[j]);
                    } else if (j == i.size() - 1) {
                        LastEventsSet.insert(i[j]);
                    }
                    if (j + 1 < i.size()) {
                        DirFollowMap[i[j]].insert(i[j+1]);
                    }
                }
            }

            ///InDirectional Matrix
            for (const auto& i : EventsVec) {
                for (const auto& j : EventsVec) {
                    if (i != j && !DirFollowMap[i].count(j) && !DirFollowMap[j].count(i)) {
                        InDirFollowMap[i].insert(j);
                        InDirFollowMap[j].insert(i);
                    }
                }
            }

            ///Getting set for Bipartite Graph
            uint64_t n = EventsVec.size();
            std::set<std::pair<std::set< std::string>,std::set<std::string>>> XwSet;
            for (auto mask = 0; mask < (1 << n); ++mask) {
                bool A_valid = true;
                std::set<std::string> A;
                for (auto i = 0; i < n; ++i) {
                    if (mask & (1 << i)) {
                        std::string A_candidate = EventsVec[i];
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
                            std::string B_candidate = EventsVec[i];
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
                                if (!DirFollowMap[j].count(B_candidate) || DirFollowMap[B_candidate].count(j)) {
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

            ///Filtring Xw into Yw
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
            ///Getting Pw set
            std::set<MapLabeledPetriNet<>::Transition> TransitionSet;
            std::map<std::pair<std::string, std::string>,  ::xi::ldopa::pn::GenPetriNet<>::Position> PositionMap;
            std::map<std::string,  ::xi::ldopa::pn::GenPetriNet<>::Transition> TransitionMap;

            ///Creating Input Arcs
            std::map<std::string, MapLabeledPetriNet<>::Transition> added_transitions;
            std::set<std::pair<std::string, MapLabeledPetriNet<>::Position>> added_TP_arcs;
            std::set<std::pair<MapLabeledPetriNet<>::Position, std::string>> added_PT_arcs;

            MapLabeledPetriNet<>::Position InputPos = BPN.addPosition("Input");
            for (auto i : FirstEventsSet) {
                MapLabeledPetriNet<>::Transition t = BPN.addTransition(i);
                TransitionMap[i] = t;
                //PositionMap[std::make_pair("Input None", i)] = InputPos;
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
                    if (!added_TP_arcs.count(std::make_pair(a, pos)))
                    {
                        BPN.addArcW(t_a, pos);
                        added_TP_arcs.insert(std::make_pair(a, pos));
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
            }
        }
    };

}}} // namespace xi { namespace ldopa { namespace pn {

#endif //LDOPALIB_ALPHA_ALGORITHM_H
