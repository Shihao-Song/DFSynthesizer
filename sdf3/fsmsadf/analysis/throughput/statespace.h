/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   statespace.h
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
 * $Id: statespace.h,v 1.1.2.9 2010-05-07 19:21:20 mgeilen Exp $
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

#ifndef FSMSADF_ANALYSIS_THROUGHPUT_STATESPACE_H_INCLUDED
#define FSMSADF_ANALYSIS_THROUGHPUT_STATESPACE_H_INCLUDED

#include "../../base/graph.h"
#include "../../base/fsm.h"
#include "thrutils.h"

#ifdef _MSC_VER
#include <hash_set>
#else
// todo: change to unordered_set
#undef __DEPRECATED
#include <ext/hash_set>
using namespace __gnu_cxx;
#endif

namespace FSMSADF
{

    /**
     * throughputAnalysisGraphUsingStateSpace()
     * Compute the throughput of an FSM-based SDAF graph through a state-space
     * traversal.
     */
    Throughput throughputAnalysisGraphUsingStateSpace(Graph *g);

    /**
     * namespace MPExploreStateSpace
     * This namespace contains the implementation of the throughput analysis
     * function.
     */
    namespace MPExploreStateSpace
    {

        /**
         * class State
         * State represents a MaxPlus vector and a state in the FSM of the graph.
         */
        class State
        {
            public:
                // Constructor
                State(State *previousState, ::FSM::Abstract::State *newFsmState, Matrix *m);

                // Destructor
                ~State();

                // Compute hash value of the state
                size_t hashValue() const;

                // Compare the states for equality on all their token timestamps and
                // equality of the FSM state.
                bool equals(const State *s) const;

                // Compare the states
                int compare(const State *s) const;

                // Execute one iteration of matrix associated with this
                // state
                void executeIteration();

                // Normalize the vector. This sets
                // the delay of the state
                void normalize();

                // Add the state to the set of reachable states
                // returns true if the state was really added, false if it was redundant.
                bool addReachableState(State *state, MPDelay d);

                // Print the state
                void print(ostream &out) const;

            public:
                // MaxPlus vector
                MaxPlus::Vector *mpvector;

                // FSM state
                ::FSM::Abstract::State *fsmState;

                // Scenario Matrix
                Matrix *mat;

                // delay !!not part of the state!!
                // temporary to be annotated to the edge from previousState
                MPDelay delay;

                // Reachable states
                map<State *, MPDelay> reachableStates;

                // Previous state
                State *previousState;
        };

#ifdef _MSC_VER

        /**
         * class StateHasherAndComparator
         * Compute hash value of state and compares.
         */
        class StateHasherAndComparator: public stdext::hash_compare<State *, less<State *>>
        {
            public:
                size_t operator()(const State *s) const
                {
                    return s->hashValue();
                }

                bool operator()(const State *s1, const State *s2) const
                {
                    return s1->compare(s2) < 0;
                }
        };

#else
        /**
         * class StateHasher
         * Compute hash value of state.
         */
        class StateHasher
        {
            public:
                size_t operator()(const State *s) const
                {
                    return s->hashValue();
                }
        };

        /**
         * class StateComparator
         * Compare two states for equality.
         */
        class StateComparator
        {
            public:
                bool operator()(const State *s1, const State *s2) const
                {
                    return s1->equals(s2);
                }
        };

#endif

        /**
         * class States
         * Hash of states.
         */
#ifdef _MSC_VER
        class States : public stdext::hash_set<State *, StateHasherAndComparator>
#else
        class States : public hash_set<State *, StateHasher, StateComparator>
#endif
        {
            public:
                bool includes(State *x, State **y)
                {
                    States::const_iterator i = find(x);
                    if (i != end())
                    {
                        (*y) = (*i);
                        return true;
                    }
                    return false;
                }
                bool includes(State *x)
                {
                    return find(x) != end();
                }
        };

        /**
         * class StateSpaceExploration
         * The state-space analysis algorithms.
         */
        class StateSpaceExploration
        {
            public:
                // Compute the throughput of the FSM-based SADF graph using a
                // state-space traversal
                Throughput exploreThroughput(Graph *g);
                // Compute the throughput of the FSM-based SADF graph using a
                // state-space traversal while combining multiple scenarios leading
                // the same net state of the fsm.
                Throughput exploreThroughputMax(Graph *g);

            private:
                // Compute the throughput of the FSM-based SADF graph by computing the
                // MCM of the state-space
                Throughput computeThroughput(States *visitedStates,
                                             State *initialState);
                // Compute MCM using Karps theorem
                double computeMCM(States *visitedStates, State *initialState);

                // Output the state-space in DOT format
                void printStateSpace(States *states, ostream &out);
        };

        class StateQueue: public list<State *>
        {
        };

    } // End namespace MPExploreStateSpace

} // End namespace FSMSADF

#endif

