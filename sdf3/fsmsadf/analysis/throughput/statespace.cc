/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   statespace.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   August 12, 2009
 *
 *  Function        :   FSM-based SADF throughput analysis algorithm using
 *                      state-space traversal.
 *
 *  History         :
 *      12-08-09    :   Initial version.
 *
 * $Id: statespace.cc,v 1.2.2.23 2010-08-18 09:01:00 mgeilen Exp $
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

#include "statespace.h"
#include "sdf/analysis/mcm/mcmgraph.h"

//#define _DEBUG_

namespace FSMSADF
{

    /**
     * throughputAnalysisGraphUsingStateSpace()
     * Compute the throughput of an FSM-based SDAF graph through a state-space
     * traversal.
     */
    Throughput throughputAnalysisGraphUsingStateSpace(Graph *g)
    {
        MPExploreStateSpace::StateSpaceExploration sse;
        Graph *gt;
        Throughput thr;

        gt = GraphConversion::ensureStronglyBounded(g);

        // Compute throughput
        thr = sse.exploreThroughputMax(gt);

        // Cleanup
        delete gt;

        return thr;
    }

    namespace MPExploreStateSpace
    {

        /**
         * State()
         * Constructor.
         */
        State::State(State *prevState, ::FSM::Abstract::State *newFsmState, Matrix *m)
        {
            // FSM state
            this->fsmState = newFsmState;

            // Matrix
            this->mat = m;

            // vector
            if (prevState != NULL)
            {
                this->mpvector = prevState->mpvector;
            }

            // Previous state
            this->previousState = prevState;
        }

        /**
         * ~State()
         * Destructor
         */
        State::~State()
        {
        }

        /**
         * hashValue()
         * Compute hash value of the state.
         */
        size_t State::hashValue() const
        {
            size_t h = 0;

            // Vector
            for (uint i = 0; i < this->mpvector->getSize(); i++)
            {
                int dum;
                // attempt to make hash more effective for small execution times <<1.
                // was: h = 31*h + (size_t)this->mpvector->get(i);
                h = 31 * h + (size_t) ldexp(frexp(this->mpvector->get(i), &dum), 31);
            }

            // FSM state
            h = 31 * h + (unsigned long)fsmState;

            return h;
        }

        /**
         * equals()
         * Compare the states for equality on all their token timestamps and
         * equality of the FSM state.
         */
        bool State::equals(const State *s) const
        {
            // FSM state
            if (this->fsmState != s->fsmState) return false;

            // Vector
            for (uint i = 0; i < this->mpvector->getSize(); i++)
            {
                if (fabs(this->mpvector->get(i) - s->mpvector->get(i)) > MP_EPSILON) return false;
            }

            return true;
        }

        /**
         * compare()
         * The function implements a comparison to generate a total order for MS hash_set
         * return -1 if receiver smaller than s,
         * 0 if equal, otherwise 1
         * performs a lexicographical order on fsm state and initial tokens.
         */
        int State::compare(const State *s) const
        {
            // FSM state
            if (this->fsmState != s->fsmState)
            {
                if (this->fsmState < s->fsmState)
                    return -1;
                else
                    return 1;
            }

            // Vector
            for (uint i = 0; i < this->mpvector->getSize(); i++)
            {
                MPDelay diff;
                diff = this->mpvector->get(i) - s->mpvector->get(i);
                if (-diff > MP_EPSILON) return -1;
                if (diff > MP_EPSILON) return 1;
            }

            return 0;
        }

        /**
         * executeIteration()
         * Execute one iteration of the scenario graph associated with this state.
         */
        void State::executeIteration()
        {
#ifdef _DEBUG_
            cerr << "execute iteration for state: " << endl;
            print(cerr);
            cerr << endl;
#endif

            this->mpvector = this->mat->mpmultiply(*this->mpvector);
        }

        /**
         * normalize()
         * Normalize the production time of the tokens in all FIFOs.
         */
        void State::normalize()
        {
            this->delay = this->mpvector->normalize();
        }

        /**
         * addReachableState()
         * Add the state to the set of reachable states.
         */
        bool State::addReachableState(State *state, MPDelay d)
        {
            // if there is some reachable state (q',v') with d' such that
            // q'=q and d+v <= d'+v' then forget
            map<State *, MPDelay>::const_iterator i;
            for (i = this->reachableStates.begin(); i != this->reachableStates.end(); i++)
            {
                if ((*i).first->fsmState == state->fsmState)
                {
                    Vector *vp = (*i).first->mpvector;
                    Vector *v = state->mpvector;
                    MPDelay dp = (*i).second;
                    bool dominates = true;
                    for (unsigned int k = 0; dominates && k < this->mpvector->getSize(); k++)
                    {
                        dominates = dominates && (d + (v->get(k)) <= dp + (vp->get(k)));
                    }
                    if (dominates) return false;
                }
            }

            // if state is already reachable update delay
            if (this->reachableStates.find(state) != this->reachableStates.end())
            {
                if (d > this->reachableStates[state])
                    this->reachableStates[state] = d;
                return false;
            }
            else
            {
                this->reachableStates[state] = d;
                return true;
            }

#ifdef _DEBUG_
            cerr << "reached state: " << endl;
            state->print(cerr);
            cerr << "from state: " << endl;
            print(cerr);
            cerr << "with delay: " << state->delay << endl;
#endif
        }

        /**
         * print()
         * Print the state.
         */
        void State::print(ostream &out) const
        {
            // Vector
            for (uint i = 0; i < this->mpvector->getSize(); i++)
            {
                out << "[" << this->mpvector->get(i) << "]: ";
                out << endl;
            }

            // FSM state
            out << "fsm state: " << ((ELSState *) fsmState)->stateLabel << endl;
        }


        /**
         * exploreThroughput()
         * Compute the throughput of the FSM-based SADF graph using a state-space
         * traversal.
         */
        Throughput StateSpaceExploration::exploreThroughput(Graph *g)
        {

            class _local
            {
                public:
                    void createNewReachableStates(State *curState, ScenarioMatrices *sm, StateQueue *sq)
                    {
                        // Create new reachable state from this state
                        ELSSetOfEdges *edges = (ELSSetOfEdges *)curState->fsmState->getOutgoingEdges();
                        ELSSetOfEdges::CIter i;
                        for (i = edges->begin(); i != edges->end(); i++)
                        {
                            State *newState;
                            ELSEdge *e = (ELSEdge *) *i;
                            // Create a new state based on this transition
                            newState = new State(curState, (ELSState *)e->getDestination(), (*sm)[e->label]);
                            // Add new state to the state queue
                            sq->push_front(newState);
                        }
                    }
            };
            _local local;

            State *curState, *initialState;
            States *visitedStates = new States;
            StateQueue *stateQueue = new StateQueue();
            Throughput thr;

            ScenarioMatrices *scenarioMatrices;
            scenarioMatrices = GraphConversion::computeMaxPlusMatricesFromScenarios(g);

            // create a minimizal edge labelled scenario automaton
            EdgeLabeledScenarioFSM *elsFSM = FSMConverter::minimalEdgeLabeledFSM(g->getFSM());

            // Create an initial state based on the initial FSM state
            initialState = new State(NULL, elsFSM->getInitialState(), NULL);
            initialState->mpvector = new Vector(scenarioMatrices->vectorSize(), 0.0);

            // Add the initial state to the set of visited states
            visitedStates->insert(initialState);

#ifdef _DEBUG_
            cerr << "insert state: " << endl;
            initialState->print(cerr);
            cerr << endl;
#endif

            //create new states to explore frm the initial state.
            local.createNewReachableStates(initialState, scenarioMatrices, stateQueue);

            // Explore state-space
            while (!stateQueue->empty())
            {
                // Take the first state from the queue
                curState = *stateQueue->begin();
                stateQueue->erase(stateQueue->begin());
                // Execute one iteration of the scenario graph
                curState->executeIteration();

                // Normalize state
                curState->normalize();

#ifdef _DEBUG_
                cerr << endl;
                cerr << "reached state: " << endl;
                curState->print(cerr);
                cerr << endl;
#endif

                // Reached this state before?
                if (visitedStates->includes(curState))
                {
                    // Get recurrent state from the set of visisted states
                    State *s = *visitedStates->find(curState);

#ifdef _DEBUG_
                    cerr << "recurrent state:" << endl;
                    s->print(cerr);
                    cerr << endl;
#endif

                    // Previous state can go to recurrent state
                    curState->previousState->addReachableState(s, curState->delay);

                    // Cleanup
                    delete curState;
                }
                else
                {
                    // Add state to the set of reachable states of the previous
                    // state
                    bool added = curState->previousState->addReachableState(curState, curState->delay);

                    // if the state was not dedundant and has really been added
                    if (added)
                    {
                        // Add state to the set of visited states
                        visitedStates->insert(curState);

#ifdef _DEBUG_
                        cerr << endl;
                        cerr << "insert state: " << endl;
                        curState->print(cerr);
                        cerr << endl;
#endif

                        // Create new reachable state from this state
                        local.createNewReachableStates(curState, scenarioMatrices, stateQueue);
                    }
                }
            }

#ifdef _DEBUG_
            ofstream dot("statespace.dot");
            printStateSpace(visitedStates, dot);
#endif

            // Compute throughput
            thr = computeThroughput(visitedStates, initialState);

            // Cleanup
            for (States::iterator i = visitedStates->begin();
                 i != visitedStates->end();)
            {
                State *s = *i;
                visitedStates->erase(i++);
                delete s;
            }
            delete visitedStates;
            delete stateQueue;

            return thr;
        }

        /**
         * exploreThroughputMax()
         * Compute the throughput of the FSM-based SADF graph using a state-space
         * traversal while combine multiple scenarios leading the same net state of
         * the fsm.
         */
        Throughput StateSpaceExploration::exploreThroughputMax(Graph *g)
        {

            class _local
            {
                public:
                    void createNewReachableStates(State *curState, StateQueue *sq)
                    {
                        // Create new reachable state from this state
                        MLSSetOfEdges *edges = (MLSSetOfEdges *)curState->fsmState->getOutgoingEdges();
                        MLSSetOfEdges::CIter i;
                        for (i = edges->begin(); i != edges->end(); i++)
                        {
                            State *newState;
                            MLSEdge *e = (MLSEdge *) *i;
                            // Create a new state based on this transition
                            newState = new State(curState, (MLSState *)e->getDestination(), e->label);
                            // Add new state to the state queue
                            sq->push_front(newState);
                        }
                    }
            };
            _local local;

            State *curState, *initialState;
            States *visitedStates = new States;
            StateQueue *stateQueue = new StateQueue();
            Throughput thr;

            // Create maxplus matrices from the individual scenario graphs
            ScenarioMatrices *scenarioMatrices;
            scenarioMatrices = GraphConversion::computeMaxPlusMatricesFromScenarios(g);

            // create a minimizal edge labelled scenario automaton
            EdgeLabeledScenarioFSM *elsFSM = FSMConverter::minimalEdgeLabeledFSM(g->getFSM());
            MatrixLabeledScenarioFSM *mlsFSM = FSMConverter::convertToMatrixLabeledScenarioFSM(elsFSM, scenarioMatrices);
            //      MatrixLabeledScenarioFSM* mlsFSM = FSMConverter::convertToFullMatrixLabeledScenarioFSM(elsFSM, scenarioMatrices);
            delete elsFSM;

            // Create an initial state based on the initial FSM state
            initialState = new State(NULL, mlsFSM->getInitialState(), NULL);
            initialState->mpvector = new Vector(scenarioMatrices->vectorSize(), 0.0);

            // Add the initial state to the set of visited states
            visitedStates->insert(initialState);

#ifdef _DEBUG_
            cerr << "insert state: " << endl;
            initialState->print(cerr);
            cerr << endl;
#endif

            //create new states to explore frm the initial state.
            local.createNewReachableStates(initialState, stateQueue);

            // Explore state-space
            while (!stateQueue->empty())
            {
                // Take the first state from the queue
                curState = *stateQueue->begin();
                stateQueue->erase(stateQueue->begin());
                // Execute one iteration of the scenario graph
                curState->executeIteration();

                // Normalize state
                curState->normalize();

                // Reached this state before?
                if (visitedStates->includes(curState))
                {
                    // Get recurrent state from the set of visisted states
                    State *s = *visitedStates->find(curState);

                    // Previous state can go to recurrent state
                    curState->previousState->addReachableState(s, curState->delay);

                    // Cleanup
                    delete curState;
                }
                else
                {
                    //cout << "adding state: " << endl;
                    //curState->print(cout);
                    //cout << endl;
                    // Add state to the set of reachable states of the previous
                    // state
                    bool added = curState->previousState->addReachableState(curState, curState->delay);

                    // if the state was not dedundant and has really been added
                    if (added)
                    {
                        // Add state to the set of visited states
                        visitedStates->insert(curState);

                        // Create new reachable state from this state
                        local.createNewReachableStates(curState, stateQueue);
                    }
                }
            }

            // Compute throughput
            //cout << "number of states: " << visitedStates->size() << endl;
            thr = computeThroughput(visitedStates, initialState);

            // Cleanup
            for (States::iterator i = visitedStates->begin();
                 i != visitedStates->end();)
            {
                State *s = *i;
                visitedStates->erase(i++);
                delete s;
            }
            delete visitedStates;
            delete stateQueue;

            return thr;
        }


        /**
         * computeThroughput()
         * Comput the MCM of the state space. The
         * inverse of the MCM is the throughput of the graph.
         */
        Throughput StateSpaceExploration::computeThroughput(States *visitedStates,
                State *initialState)
        {
#ifdef _COMPUTE_MCM_USING_DFS
            map<State *, State *> parents;
            States color;
#endif
            double mcm;

#ifdef _DEBUG_
            cerr << endl;
            cerr << "Throughput computation:" << endl;
#endif

#ifdef _COMPUTE_MCM_USING_DFS
            // Compute mcm using DFS
            parents[initialState] = NULL;
            mcm = dfsStateSpace(visitedStates, initialState, parents, color);
#else
            // Compute mcm using Karp
            mcm = computeMCM(visitedStates, initialState);
#endif

            // MCM is zero? Then there is no throughput (throughout is infinite)
            if (mcm == 0.0)
                throw CException("Cannot compute throughout. MCM of the statespace is 0 (throughput is infinite).");

            return Throughput(1.0 / mcm);
        }



        /**
         * computeMCM()
         * The function returns the MCM of the statespace.
         */
        double StateSpaceExploration::computeMCM(States *visitedStates,
                State *initialState)
        {
            map<State *, MCMnode *> stateNodes;
            CId id = 0;

            // Create a new MCM graph
            MCMgraph *mcmGraph = new MCMgraph;

            // Create the nodes of the MCM graph
            for (States::iterator i = visitedStates->begin();
                 i != visitedStates->end(); i++)
            {
                State *s = *i;

                // Create an MCM node for this state
                MCMnode *n = new MCMnode(id, true);

                // Store node of this state
                stateNodes[s] = n;
                id++;

                // Add the node to the MCM graph
                mcmGraph->addNode(n);
            }

            // Counter for edge Ids
            CId eId = 0;

            // Add edges to the MCM graph
            for (States::iterator i = visitedStates->begin();
                 i != visitedStates->end(); i++)
            {
                State *s = *i;

                // Iterate over the reachable states
                for (map<State *, MPDelay>::iterator j = s->reachableStates.begin();
                     j != s->reachableStates.end(); j++)
                {
                    // Add the edge to the MCM graph and the src and dst node
                    mcmGraph->addEdge(eId++, stateNodes[s], stateNodes[(*j).first], (*j).second, 1);
                }
            }

            // calculate the MCM
            return mcmGraph->calculateMaximumCycleMeanKarp();
        }

        /**
         * printStateSpace()
         * The function outputs the state-space in DOT format.
         */
        void StateSpaceExploration::printStateSpace(States *states, ostream &out)
        {
            map<State *, uint> stateId;

            out << "digraph statespace {" << endl;
            out << "    size=\"7,10\";" << endl;

            for (States::iterator i = states->begin(); i != states->end(); i++)
            {
                State *s = *i;
                uint sId;

                // No id assigned to state s?
                if (stateId.find(s) == stateId.end())
                    stateId[s] = stateId.size();

                // Get id of state s
                sId = stateId[s];

                // Output state s as a new node
                out << "    " << sId << " [shape=box label=\"";

                // vector
                for (uint j = 0; j < s->mpvector->getSize(); j++)
                {
                    out << "[" << s->mpvector->get(j) << "]";
                    out << "\\n";
                }

                // FSM state
                out << "fsm state: " << ((ELSState *) s->fsmState)->stateLabel << "\\n";

                // Delay
                out << "delay: " << s->delay << "\\n";

                // Close node
                out << "\"];" << endl;

                // Output edges to all reachable states
                for (map<State *, MPDelay>::iterator j = s->reachableStates.begin();
                     j != s->reachableStates.end(); j++)
                {
                    State *t = (*j).first;
                    uint tId;

                    // No id assigned to state t?
                    if (stateId.find(t) == stateId.end())
                        stateId[t] = stateId.size();

                    // Get id of state t
                    tId = stateId[t];

                    // Output edge from state s to state t
                    out << "    " << sId << " -> " << tId << " [label=";
                    out << t->delay << "];" << endl;
                }
            }

            out << "}" << endl;
        }




    } // End namespace MPExploreStateSpace

} // End namespace FSMSADF

