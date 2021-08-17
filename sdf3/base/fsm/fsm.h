/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   maxplus.h
 *
 *  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
 *
 *  Date            :   March 23, 2009
 *
 *  Function        :   generic FSM
 *
 *  History         :
 *      23-03-09    :   Initial version.
 *
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * In other words, you are welcome to use, share and improve this program.
 * You are forbidden to forbid anyone else to use, share and improve
 * what you give them.   Happy coding!
 */

#ifndef FSM_H_INCLUDED
#define FSM_H_INCLUDED

#include "../string/cstring.h"
#include "../exception/exception.h"
#include <set>
#include <list>

namespace FSM
{

    using namespace std;

    // the abstract ancestor of FSM types
    namespace Abstract
    {

        // forward declaration of FSM state class
        class State;

        // Edge of an FSM
        class Edge
        {
            public:
                // constructor
                Edge(State *src, State *dst)
                {
                    this->source = src;
                    this->destination = dst;
                }
                virtual ~Edge() {};
                // virtual access methods
                virtual State *getSource()
                {
                    return this->source;
                }
                virtual State *getDestination()
                {
                    return this->destination;
                }

            private:
                // source and destination state of the edge
                State *source;
                State *destination;
        };

        // A set of edges based on STL set
        class SetOfEdges : public set<Edge *>
        {
            public:
                typedef SetOfEdges::const_iterator CIter;
        };

        // A list of edges based on STL list
        class ListOfEdges : public list<Edge *>
        {
            public:
                typedef ListOfEdges::const_iterator CIter;
        };

        // Ancestor of all FSM states
        class State
        {
            public:
                // constructor
                State()
                {
                    this->outgoingEdges = new SetOfEdges();
                }

                //destructor
                virtual ~State()
                {
                    delete this->outgoingEdges;
                }

                // add an outgoing edge to the state
                void addOutGoingEdge(Edge *e)
                {
                    assert(e->getSource() == this);
                    this->outgoingEdges->insert(e);
                }

                // access the outgoing edges
                virtual const SetOfEdges *getOutgoingEdges()
                {
                    return this->outgoingEdges;
                }

            protected:
                SetOfEdges *outgoingEdges;
        };

        // A set of states
        class SetOfStates : public set<State *>
        {
            public:
                typedef SetOfStates::const_iterator CIter;
        };

        // forward declaration of reachable states strategy
        class ReachableStates;

        // The abstract ansestor of finite state machines
        class FiniteStateMachine
        {
            public:
                virtual State *getInitialState() = 0 ;
        };


        //
        // A generic DFS strategy on the target FSM
        // overwrite the methods onEnterState, onLeaveState, onTransition and onSimpleCycle with
        // the desired actions
        //
        class DepthFirstSearch
        {

                // supporting class for DFS stack items
                class DFSStackItem
                {
                    public:
                        // constructor
                        DFSStackItem(State *s)
                        {
                            this->state = s;
                            this->iter = s->getOutgoingEdges()->begin();
                        };

                        // access state
                        inline State *getState()
                        {
                            return this->state;
                        }
                        inline const SetOfEdges::CIter getIter()
                        {
                            return this->iter;
                        }

                        // test if all outgoing edges have been done
                        bool atEnd()
                        {
                            return this->iter == this->state->getOutgoingEdges()->end();
                        }

                        // move to the next edge
                        void advance()
                        {
                            (this->iter)++;
                        }

                    private:
                        State *state;
                        SetOfEdges::CIter iter;
                };

            private:
                list<DFSStackItem *> dfsStack;

            public:
                typedef list<DFSStackItem *>::const_iterator DFSStackCIter;
                virtual void onEnterState(const State *s) {};
                virtual void onLeaveState(const State *s) {};
                virtual void onTransition(const Edge *e) {};
                virtual void onSimpleCycle(const list<DFSStackItem *> &stack) {};

                DepthFirstSearch(FiniteStateMachine *targetFsm)
                {
                    this->fsm = targetFsm;
                };

                // Execute the depth first search
                void DoDepthFirstSearch(bool fullDFS = false)
                {
                    // store visited states
                    SetOfStates *visitedStates = new SetOfStates();

                    // put initial state on the stack
                    dfsStack.push_back(new DFSStackItem(this->fsm->getInitialState()));

                    while (!(dfsStack.empty()))
                    {
                        DFSStackItem *si = dfsStack.back();

                        // current item complete?
                        if (si->atEnd())
                        {
                            // pop it from stack
                            this->onLeaveState(si->getState());
                            dfsStack.pop_back();
                            if (fullDFS)
                            {
                                SetOfStates::iterator i = visitedStates->find((*si).getState());
                                visitedStates->erase(i);
                            }
                            delete si;
                        }
                        else
                        {
                            // goto next edge
                            Edge *e = *(si->getIter());
                            si->advance();
                            bool revisit;
                            revisit = visitedStates->find(e->getDestination()) == visitedStates->end();
                            if (revisit)
                            {
                                // if target state not visited before
                                dfsStack.push_back(new DFSStackItem(e->getDestination()));
                                this->onTransition(e);
                                this->onEnterState(e->getDestination());
                                visitedStates->insert(e->getDestination());
                            }
                            else
                            {
                                // cycle found
                                this->onSimpleCycle(dfsStack);
                            }
                        }
                    }
                }
            private:
                FiniteStateMachine *fsm;
        };

        // Reachable states strategy based on DFS
        class ReachableStates: public DepthFirstSearch
        {
            public:
                SetOfStates *result;
                ReachableStates(FiniteStateMachine *targetFsm) : DepthFirstSearch(targetFsm)
                {
                    this->result = new SetOfStates();
                };
                virtual ~ReachableStates() {};
                virtual void onEnterState(State *s)
                {
                    this->result->insert(s);
                };
        };

    };  // namespace Abstract


    namespace Labeled
    {

        // forward declarations
        template<typename StateLabelType, typename EdgeLabelType>
        class State;

        template<typename StateLabelType, typename EdgeLabelType>
        class Edge: public Abstract::Edge
        {
            public:
                Edge(State<StateLabelType, EdgeLabelType> *src, EdgeLabelType &lbl, State<StateLabelType, EdgeLabelType> *dst)
                    : Abstract::Edge(src, dst)
                {
                    this->label = lbl;
                }

                EdgeLabelType label;
        };

        template<typename StateLabelType, typename EdgeLabelType>
        class SetOfEdges : public Abstract::SetOfEdges
        {
            public:
                typedef typename SetOfEdges<StateLabelType, EdgeLabelType>::const_iterator CIter;
        };

        template<typename StateLabelType, typename EdgeLabelType>
        class ListOfEdges : public Abstract::ListOfEdges
        {
            public:
                typedef typename ListOfEdges<StateLabelType, EdgeLabelType>::const_iterator CIter;
        };

        template<typename StateLabelType, typename EdgeLabelType>
        class SetOfStates : public Abstract::SetOfStates
        {
            public:
                typedef typename SetOfStates::const_iterator CIter;
                map<StateLabelType, State<StateLabelType, EdgeLabelType>*> stateIndex;
        };

        template<typename StateLabelType, typename EdgeLabelType>
        class State: public Abstract::State
        {
            public:
                StateLabelType stateLabel;
                State(const StateLabelType &withLabel) : Abstract::State()
                {
                    this->stateLabel = withLabel;
                }
                const StateLabelType &getLabel(void) const
                {
                    return this->stateLabel;
                }
                // return all next states reachable via an edge labelled l
                SetOfStates<StateLabelType, EdgeLabelType> *nextStatesOfEdgeLabel(const EdgeLabelType l)
                {
                    SetOfStates<StateLabelType, EdgeLabelType> *result = new SetOfStates<StateLabelType, EdgeLabelType>();
                    SetOfEdges<StateLabelType, EdgeLabelType> *es =
                        (SetOfEdges<StateLabelType, EdgeLabelType> *) this->getOutgoingEdges();
                    typename SetOfEdges<StateLabelType, EdgeLabelType>::CIter i;
                    for (i = es->begin(); i != es->end(); i++)
                    {
                        Edge<StateLabelType, EdgeLabelType> *e = (Edge<StateLabelType, EdgeLabelType> *) *i;
                        if (e->label == l) result->insert(e->getDestination());
                    }
                    return result;
                }
                // return an arbitrary next state reachable via an edge labelled l
                // or null if no such state exists
                State<StateLabelType, EdgeLabelType> *nextStateOfEdgeLabel(const EdgeLabelType l)
                {
                    SetOfEdges<StateLabelType, EdgeLabelType> *es =
                        (SetOfEdges<StateLabelType, EdgeLabelType> *) this->getOutgoingEdges();
                    typename SetOfEdges<StateLabelType, EdgeLabelType>::CIter i;
                    for (i = es->begin(); i != es->end(); i++)
                    {
                        Edge<StateLabelType, EdgeLabelType> *e = *i;
                        if (e->label == l) return e->getDestination();
                    }
                    return 0L;
                }
        };

        template<typename StateLabelType, typename EdgeLabelType>
        class FiniteStateMachine: public Abstract::FiniteStateMachine
        {
            public:
                FiniteStateMachine(): Abstract::FiniteStateMachine()
                {
                    states = new SetOfStates<StateLabelType, EdgeLabelType>();
                    edges = new SetOfEdges<StateLabelType, EdgeLabelType>();
                };

                ~FiniteStateMachine()
                {
                    typename SetOfEdges<StateLabelType, EdgeLabelType>::iterator ei;
                    for (ei = this->edges->begin(); ei != this->edges->end(); ei++)
                    {
                        delete *ei;
                    }
                    delete edges;

                    typename SetOfStates<StateLabelType, EdgeLabelType>::iterator si;
                    for (si = this->states->begin(); si != this->states->end(); si++)
                    {
                        delete *si;
                    }
                    delete states;
                }

                void addState(State<StateLabelType, EdgeLabelType> *s)
                {
                    states->insert(s);
                };

                Edge<StateLabelType, EdgeLabelType> *addEdge(State<StateLabelType, EdgeLabelType> *src, EdgeLabelType lbl, State<StateLabelType, EdgeLabelType> *dst)
                {
                    Edge<StateLabelType, EdgeLabelType> *e = new Edge<StateLabelType, EdgeLabelType>(src, lbl, dst);
                    edges->insert(e);
                    src->addOutGoingEdge(e);
                    return e;
                };

                void setInitialState(State<StateLabelType, EdgeLabelType> *s)
                {
                    this->initialState = s;
                };

                State<StateLabelType, EdgeLabelType> *getInitialState()
                {
                    return this->initialState;
                }

                State<StateLabelType, EdgeLabelType> *getStateLabeled(const StateLabelType &s)
                {
                    // try the index first
                    if (this->states->stateIndex.find(s) != this->states->stateIndex.end())
                    {
                        return this->states->stateIndex[s];
                    }
                    // for now just a linear search
                    typename SetOfStates<StateLabelType, EdgeLabelType>::CIter i = this->states->begin();
                    while (i != this->states->end())
                    {
                        State<StateLabelType, EdgeLabelType> *t = dynamic_cast<State<StateLabelType, EdgeLabelType>*>(*i);
                        if ((t->stateLabel) == s)
                        {
                            this->states->stateIndex[s] = t;
                            return t;
                        }
                        i++;
                    }
                    throw CException("error - state not found in FiniteStateMachine::getStateLabeled");
                }

                Abstract::SetOfStates *reachableStates(void)
                {
                    // use generic DFS function to add states found
                    Abstract::ReachableStates *rs = new Abstract::ReachableStates(this);
                    rs->DoDepthFirstSearch();
                    Abstract::SetOfStates *result = rs->result;
                    delete rs;
                    return result;
                };

                SetOfStates<StateLabelType, EdgeLabelType> *getStates(void)
                {
                    return this->states;
                }


                SetOfEdges<StateLabelType, EdgeLabelType> *getEdges(void)
                {
                    return this->edges;
                }

                // determinize the automaton based on edge labels only.
                // Using the subset construction
                FiniteStateMachine<StateLabelType, EdgeLabelType> *determinizeEdgeLabels(void)
                {
                    FiniteStateMachine<StateLabelType, EdgeLabelType> *result;
                    result = new FiniteStateMachine<StateLabelType, EdgeLabelType>();

                    // maintain map of sets of states to the corresponding new states.
                    map < const SetOfStates<StateLabelType, EdgeLabelType>,
                        State<StateLabelType, EdgeLabelType> * > newStatesMap;

                    // queue of states that need to be further explored.
                    list<SetOfStates<StateLabelType, EdgeLabelType>*> *unprocessed;
                    unprocessed = new list<SetOfStates<StateLabelType, EdgeLabelType>*>();

                    // create initial state
                    SetOfStates<StateLabelType, EdgeLabelType> *initialStateSet;
                    initialStateSet = new SetOfStates<StateLabelType, EdgeLabelType>();
                    initialStateSet->insert(this->getInitialState());

                    CId newStateId = 0;
                    State<StateLabelType, EdgeLabelType> *si = new State<StateLabelType, EdgeLabelType>(newStateId++);
                    newStatesMap[*initialStateSet] = si;
                    result->addState(si);
                    result->setInitialState(si);

                    // add initial state to list of unprocessed state sets
                    unprocessed->push_back(initialStateSet);


                    while (!unprocessed->empty())
                    {
                        SetOfStates<StateLabelType, EdgeLabelType> *Q;
                        Q = *unprocessed->begin();
                        unprocessed->erase(unprocessed->begin());

                        //get all outgoing labels
                        set<EdgeLabelType> labels;
                        typename SetOfStates<StateLabelType, EdgeLabelType>::CIter i;
                        for (i = Q->begin(); i != Q->end(); i++)
                        {
                            State<StateLabelType, EdgeLabelType> *s = (State<StateLabelType, EdgeLabelType> *) *i;
                            this->insertOutgoingLabels(s, labels);
                        }

                        // for each label in lables get the image states into a set Qnext
                        typename set<EdgeLabelType>::const_iterator j;
                        for (j = labels.begin(); j != labels.end(); j++)
                        {
                            EdgeLabelType l = *j;

                            // collect image state in Qnext
                            SetOfStates<StateLabelType, EdgeLabelType> *Qnext;
                            Qnext = new SetOfStates<StateLabelType, EdgeLabelType>();

                            // for every state s in Q
                            typename SetOfStates<StateLabelType, EdgeLabelType>::CIter i;
                            for (i = Q->begin(); i != Q->end(); i++)
                            {
                                State<StateLabelType, EdgeLabelType> *s = (State<StateLabelType, EdgeLabelType> *) *i;
                                SetOfStates<StateLabelType, EdgeLabelType> *limg;
                                limg = s->nextStatesOfEdgeLabel(l);

                                // add all l-images from s to Qnext
                                typename SetOfStates<StateLabelType, EdgeLabelType>::CIter k;
                                for (k = limg->begin(); k != limg->end(); k++)
                                {
                                    State<StateLabelType, EdgeLabelType> *simg = (State<StateLabelType, EdgeLabelType> *) *k;
                                    Qnext->insert(simg);
                                }
                                delete limg;
                            }
                            // add new state in fsm if necessary
                            State<StateLabelType, EdgeLabelType> *ns;
                            if (newStatesMap.find(*Qnext) == newStatesMap.end())
                            {
                                // state does not yet exist, make new state
                                ns = new State<StateLabelType, EdgeLabelType>(newStateId++);
                                newStatesMap[*Qnext] = ns;
                                result->addState(ns);
                                unprocessed->push_back(Qnext);
                            }
                            else
                            {
                                // state already exists, fetch from newStatesMap
                                ns = newStatesMap[*Qnext];
                            }

                            // add an edge in the new fsm
                            result->addEdge(newStatesMap[*Q], l, ns);

                        }

                    }
                    delete initialStateSet;
                    delete unprocessed;

                    // set initial state
                    return result;
                }


                typedef map <
                State<StateLabelType, EdgeLabelType> *,
                      SetOfStates<StateLabelType, EdgeLabelType> * >
                      EquivalenceMap;

                // minimize the automaton based on edge labels only.
                FiniteStateMachine<StateLabelType, EdgeLabelType> *minimizeEdgeLabels(void)
                {
                    // partition refinement alorithm

                    // generate a vector of equivalence classes
                    list<SetOfStates<StateLabelType, EdgeLabelType>*> *eqClasses =
                        new list<SetOfStates<StateLabelType, EdgeLabelType>*>();

                    // populate it with s set of all states
                    SetOfStates<StateLabelType, EdgeLabelType> *initialClass =
                        new SetOfStates<StateLabelType, EdgeLabelType>(*(this->getStates()));
                    eqClasses->push_back(initialClass);

                    // initially map all state to the initial class
                    EquivalenceMap eqMap;
                    typename SetOfStates<StateLabelType, EdgeLabelType>::CIter si;
                    for (si = initialClass->begin(); si != initialClass->end(); si++)
                    {
                        eqMap[(State<StateLabelType, EdgeLabelType> *)*si] = initialClass;
                    }

                    // partition refinement
                    bool changed;
                    do
                    {
                        changed = false;

                        list<SetOfStates<StateLabelType, EdgeLabelType>*> *newEqClasses =
                            new list<SetOfStates<StateLabelType, EdgeLabelType>*>();

                        // for every potential equivalence class
                        typename list<SetOfStates<StateLabelType, EdgeLabelType>*>::const_iterator i;
                        for (i = eqClasses->begin(); i != eqClasses->end(); i++)
                        {
                            SetOfStates<StateLabelType, EdgeLabelType> *clss = *i;

                            typename SetOfStates<StateLabelType, EdgeLabelType>::iterator i;
                            i = clss->begin();

                            //pick arbitrary state from class
                            State<StateLabelType, EdgeLabelType> *s1, *s2;
                            s1 = (State<StateLabelType, EdgeLabelType> *) *i;

                            SetOfStates<StateLabelType, EdgeLabelType> *equivSet = new SetOfStates<StateLabelType, EdgeLabelType>();
                            SetOfStates<StateLabelType, EdgeLabelType> *remainingSet = new SetOfStates<StateLabelType, EdgeLabelType>();
                            equivSet->insert(s1);

                            //check whether all other states can go with the same label to
                            //the same set of other equivalence classes.
                            while (++i != clss->end())
                            {
                                s2 = (State<StateLabelType, EdgeLabelType> *) *i;
                                if (this->edgesEquivalent(eqMap, s1, s2))
                                {
                                    equivSet->insert(s2);
                                }
                                else
                                {
                                    remainingSet->insert(s2);
                                }
                            }
                            //if not, split the class
                            if (equivSet->size() == clss->size())
                            {
                                newEqClasses->push_back(equivSet);
                                this->mapStates(eqMap, equivSet);
                                delete remainingSet;
                            }
                            else
                            {
                                newEqClasses->push_back(equivSet);
                                this->mapStates(eqMap, equivSet);
                                newEqClasses->push_back(remainingSet);
                                this->mapStates(eqMap, remainingSet);
                                changed = true;
                            }
                            delete clss;
                        }
                        list<SetOfStates<StateLabelType, EdgeLabelType>*> *tempEqClasses;
                        tempEqClasses = eqClasses;
                        eqClasses = newEqClasses;
                        delete tempEqClasses;

                    }
                    while (changed);

                    FiniteStateMachine<StateLabelType, EdgeLabelType> *result =
                        new FiniteStateMachine<StateLabelType, EdgeLabelType>();

                    // make a state for every equivalence class
                    map < SetOfStates<StateLabelType, EdgeLabelType> *,
                        State<StateLabelType, EdgeLabelType> * > newStateMap;
                    typename list<SetOfStates<StateLabelType, EdgeLabelType>*>::const_iterator cli;
                    CId sid = 0;
                    for (cli = eqClasses->begin(); cli != eqClasses->end(); cli++)
                    {
                        State<StateLabelType, EdgeLabelType> *ns = new State<StateLabelType, EdgeLabelType>(sid);
                        result->addState(ns);
                        newStateMap[*cli] = ns;
                        sid++;
                    }

                    // make the appropriate edges
                    for (cli = eqClasses->begin(); cli != eqClasses->end(); cli++)
                    {
                        // take a representative state
                        State<StateLabelType, EdgeLabelType> *s =
                            (State<StateLabelType, EdgeLabelType> *) * ((*cli)->begin());
                        SetOfEdges<StateLabelType, EdgeLabelType> *es =
                            (SetOfEdges<StateLabelType, EdgeLabelType> *) s->getOutgoingEdges();
                        typename SetOfEdges<StateLabelType, EdgeLabelType>::CIter edi;
                        // for every outgoing edge
                        for (edi = es->begin(); edi != es->end(); edi++)
                        {
                            Edge<StateLabelType, EdgeLabelType> *ed = (Edge<StateLabelType, EdgeLabelType> *) *edi;
                            result->addEdge(newStateMap[*cli], ed->label, newStateMap[eqMap[(State<StateLabelType, EdgeLabelType> *)ed->getDestination()]]);
                        }
                    }

                    // set initial state
                    result->setInitialState(newStateMap[eqMap[this->getInitialState()]]);

                    // cleanup
                    typename list<SetOfStates<StateLabelType, EdgeLabelType>*>::iterator csi;
                    for (csi = eqClasses->begin(); csi != eqClasses->end(); csi++)
                    {
                        delete *csi;
                    }
                    delete eqClasses;

                    return result;
                }

            private:

                void insertOutgoingLabels(State<StateLabelType, EdgeLabelType> *s,
                                          set<EdgeLabelType> &labels)
                {
                    SetOfEdges<StateLabelType, EdgeLabelType> *e =
                        (SetOfEdges<StateLabelType, EdgeLabelType> *) s->getOutgoingEdges();

                    // collect all labels in edges of s
                    typename SetOfEdges<StateLabelType, EdgeLabelType>::CIter i;
                    for (i = e->begin(); i != e->end(); i++)
                    {
                        Edge<StateLabelType, EdgeLabelType> *ed =
                            (Edge<StateLabelType, EdgeLabelType> *) *i;
                        labels.insert(ed->label);
                    }
                }

                // function only used by minimizeEdgeLabels
                bool edgesEquivalent(
                    EquivalenceMap &m,
                    State<StateLabelType, EdgeLabelType> *s1,
                    State<StateLabelType, EdgeLabelType> *s2)
                {
                    // s1 and s2 are equivalent if for every s1-a->C, s2-a->C
                    // and vice versa
                    set<EdgeLabelType> labels;
                    this->insertOutgoingLabels(s1, labels);
                    this->insertOutgoingLabels(s2, labels);

                    // for every label, compare outgong edges
                    typename set<EdgeLabelType>::const_iterator k;
                    for (k = labels.begin(); k != labels.end(); k++)
                    {
                        EdgeLabelType l = *k;
                        SetOfStates<StateLabelType, EdgeLabelType> *ns1 = s1->nextStatesOfEdgeLabel(l);
                        SetOfStates<StateLabelType, EdgeLabelType> *ns2 = s2->nextStatesOfEdgeLabel(l);
                        // collect classes of states in ns1 and ns2
                        set<SetOfStates<StateLabelType, EdgeLabelType>*> cs1;
                        set<SetOfStates<StateLabelType, EdgeLabelType>*> cs2;
                        typename SetOfStates<StateLabelType, EdgeLabelType>::CIter j;
                        for (j = ns1->begin(); j != ns1->end(); j++)
                        {
                            State<StateLabelType, EdgeLabelType> *s = (State<StateLabelType, EdgeLabelType> *) *j;
                            cs1.insert(m[s]);
                        }
                        for (j = ns2->begin(); j != ns2->end(); j++)
                        {
                            State<StateLabelType, EdgeLabelType> *s = (State<StateLabelType, EdgeLabelType> *) *j;
                            cs2.insert(m[s]);
                        }

                        delete ns1;
                        delete ns2;

                        // compare classes
                        if (cs1 != cs2) return false;
                    }
                    return true;
                }

                // function only used by minimizeEdgeLabels
                void mapStates(EquivalenceMap &m, SetOfStates<StateLabelType, EdgeLabelType> *sos)
                {
                    typename SetOfStates<StateLabelType, EdgeLabelType>::CIter i;
                    for (i = sos->begin(); i != sos->end(); i++)
                    {
                        State<StateLabelType, EdgeLabelType> *s = (State<StateLabelType, EdgeLabelType> *) *i;
                        m[s] = sos;
                    }
                }




                SetOfStates<StateLabelType, EdgeLabelType> *states;
                SetOfEdges<StateLabelType, EdgeLabelType> *edges;
                State<StateLabelType, EdgeLabelType> *initialState;
        };

    };

    namespace StateStringLabeled
    {

        // make an FSM class with unlabeled edges, based on the labeled one with some dummy char labels
        //

        class SetOfEdges: public Labeled::SetOfEdges<CString, char> {};
        class SetOfStates: public Labeled::SetOfStates<CString, char> {};

        class State: public Labeled::State<CString, char>
        {
            public:
                State(const CString &withLabel) : Labeled::State<CString, char>(withLabel)
                {
                }

                SetOfEdges *getOutgoingEdges(void)
                {
                    return (SetOfEdges *) Labeled::State<CString, char>::getOutgoingEdges();
                }
        };

        class FiniteStateMachine: public Labeled::FiniteStateMachine<CString, char>
        {
            public:
                State *getInitialState()
                {
                    return (State *)Labeled::FiniteStateMachine<CString, char>::getInitialState();
                };
                void setInitialStateLabeled(const CString &sl);
                void addStateLabeled(const CString &sl);
                void addEdge(State *src, State *dst);
                void addEdgeLabeled(const CString &src, const CString &dst);
                SetOfStates *reachableStates(void);
        };

        class Edge: public Labeled::Edge<CString, char> {};
    }


    namespace Product
    {

        class FiniteStateMachine;

        class State : public Abstract::State
        {
            public:
                State(Abstract::State *s1, Abstract::State *s2, FiniteStateMachine *m) : Abstract::State()
                {
                    this->sa = s1;
                    this->sb = s2;
                    this->fsm = m;
                    outgoingEdgesDone = false;
                }

                virtual Abstract::SetOfEdges *getOutgoingEdges();

            private:
                Abstract::State *sa;
                Abstract::State *sb;
                FiniteStateMachine *fsm;
                bool outgoingEdgesDone;
        };


        class FiniteStateMachine : public Abstract::FiniteStateMachine
        {
            public:

                FiniteStateMachine(Abstract::FiniteStateMachine *fsm1, Abstract::FiniteStateMachine *fsm2)
                {
                    this->fsma = fsm1;
                    this->fsmb = fsm2;
                }

                State *getInitialState();

                virtual bool matchEdges(Abstract::Edge *e1, Abstract::Edge *e2) = 0;

                virtual Abstract::Edge *ensureEdge(Abstract::Edge *e1, Abstract::Edge *s2) = 0;

            private:
                Abstract::FiniteStateMachine *fsma;
                Abstract::FiniteStateMachine *fsmb;
        };

    }

}

#endif
