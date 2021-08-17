/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   maxplusautomaton.h
 *
 *  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
 *
 *  Date            :   April 26, 2010
 *
 *  Function        :   FSM-based SADF throughput analysis algorithm using
 *                      max plus automaton.
 *
 *  History         :
 *      26-04-10    :   Initial version.
 *
 * $Id: maxplusautomaton.h,v 1.1.2.4 2010-05-07 06:14:52 mgeilen Exp $
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

#ifndef FSMSADF_ANALYSIS_THROUGHPUT_MAXPLUSAUTOMATON_H_INCLUDED
#define FSMSADF_ANALYSIS_THROUGHPUT_MAXPLUSAUTOMATON_H_INCLUDED

#include "../../base/graph.h"
#include "base/fsm/fsm.h"
#include "thrutils.h"

using namespace ::FSM::Labeled;

namespace FSMSADF
{

    /**
     * throughputAnalysisGraphUsingMaxPlusAutomaton()
     * Compute the throughput of an FSM-based SDAF graph through a max-plus automaton.
     */
    Throughput throughputAnalysisGraphUsingMaxPlusAutomaton(Graph *g);

    /**
     * throughputAnalysisGraphUsingMaxPlusAutomatonWeaklyConsistent()
     * Compute the throughput of a weakly consistent FSM-based SDAF graph through a max-plus automaton.
     */
    Throughput throughputAnalysisGraphUsingMaxPlusAutomatonWeaklyConsistent(Graph *g);

    /**
     * criticalScenarioSequenceWeaklyConsistent()
     * Compute a critical scenario sequence of a weakly consistent FSM-based SDAF graph through a max-plus automaton.
     */
    ScenarioList *criticalScenarioSequenceWeaklyConsistent(Graph *g);

    /**
     * namespace MPExploreStateSpace
     * This namespace contains the implementation of the throughput analysis
     * function.
     */
    namespace MPExploreMaxPlusAutomaton
    {
        /**
         * states of the MPA are labelled with the ID of the original fsm and the token number
         */
        typedef struct
        {
            CId id;
            unsigned int tokenNr;
        } MPAStateLabel;

        /**
         * to support easy construction of state labels
         */
        inline MPAStateLabel make_mpastatelabel(CId i, unsigned int n)
        {
            MPAStateLabel sl;
            sl.id = i;
            sl.tokenNr = n;
            return sl;
        }

        /**
         * comparing state labels
         */
        inline bool operator==(MPAStateLabel s, MPAStateLabel t)
        {
            return s.id == t.id && s.tokenNr == t.tokenNr;
        }

        /**
         * comparing state labels, lexicographic order
         */
        inline bool operator<(MPAStateLabel s, MPAStateLabel t)
        {
            if (s.id < t.id) return true;
            if (s.id > t.id) return false;
            return s.tokenNr < t.tokenNr;
        }

        /**
         * an MPA edge is labelled with a delay and a scenario name
         */
        typedef struct
        {
            MPDelay delay;
            CString *scenario;
        } MPAEdgeLabel;

        /**
         * to support easy construction od an edge label
         */
        inline MPAEdgeLabel make_mpaedgelabel(MPDelay d, CString *sc)
        {
            MPAEdgeLabel el;
            el.delay = d;
            el.scenario = sc;
            return el;
        }

        // types for edges and states and sets
        typedef ::FSM::Labeled::State<MPAStateLabel, MPAEdgeLabel> MPAState;
        typedef ::FSM::Labeled::Edge<MPAStateLabel, MPAEdgeLabel> MPAEdge;
        typedef ::FSM::Labeled::SetOfStates<MPAStateLabel, MPAEdgeLabel> MPASetOfStates;
        typedef ::FSM::Labeled::SetOfEdges<MPAStateLabel, MPAEdgeLabel> MPASetOfEdges;

        /**
         * A max-plus automaton
         */
        class MaxPlusAutomaton: public
            ::FSM::Labeled::FiniteStateMachine<MPAStateLabel, MPAEdgeLabel>
        {
            public:
                virtual ~MaxPlusAutomaton() {};
                
                // calculate the MCM of the maxplus automaton graph delay
                MPDelay calculateMCM(void);
        };

        /**
         * An edge label type for a max-plus automaton with rewards
         */
        typedef struct
        {
            MPDelay delay;
            CString *scenario;
            CDouble reward;
        } MPAREdgeLabel;

        /**
         * support for easy construction of a edge label with rewards
         */
        inline MPAREdgeLabel make_rewedgelabel(MPDelay d, CString *sc, CDouble r)
        {
            MPAREdgeLabel el;
            el.delay = d;
            el.scenario = sc;
            el.reward = r;
            return el;
        }

        // types of states, edges, sets and cycle of an MPAR
        typedef ::FSM::Labeled::State<MPAStateLabel, MPAREdgeLabel > MPARState;
        typedef ::FSM::Labeled::Edge<MPAStateLabel, MPAREdgeLabel > MPAREdge;
        typedef ::FSM::Labeled::SetOfStates<MPAStateLabel, MPAREdgeLabel > MPARSetOfStates;
        typedef ::FSM::Labeled::SetOfEdges<MPAStateLabel, MPAREdgeLabel > MPARSetOfEdges;
        typedef ::FSM::Labeled::ListOfEdges<MPAStateLabel, MPAREdgeLabel > MPARCycle;

        /**
         * A max-plus automaton with rewards. In addition to the usual max-plus
         * automaton, its edges are labelled with rewards, a quantified amount of
         * 'progress'.
         */
        class MaxPlusAutomatonWithRewards: public
            ::FSM::Labeled::FiniteStateMachine<MPAStateLabel, MPAREdgeLabel >
        {
            public:
                virtual ~MaxPlusAutomatonWithRewards() {};
                
                // compute the maximum cycle ratio of delay over progress
                CDouble calculateMCR(void);
                // compute the maximum cycle ratio of delay over progress and also return a critical cycle
                CDouble calculateMCRAndCycle(MPARCycle **cycle);
        };

        /**
         *  A class to encapsulate the analysis algorithms on FSMSADFs using MPAs
         */
        class ThroughputCalculator
        {
            public:
                // strongly consistent FSMSADF
                Throughput calculateThroughput(Graph *g);

                // weakly consistent FSMSADF
                bool testScenarioConsistency(Graph *g);
                bool testScenarioConsistencyOfScenarioGraph(Scenario *s);
                Throughput calculateThroughputWeaklyConsistent(Graph *g);
                ScenarioList *criticalScenarioSequenceWeaklyConsistent(Graph *g);
                ScenarioList *throughputAndCriticalScenarioSequenceWeaklyConsistent(Graph *g, Throughput &th);
        };

    } // End namespace MPExploreMaxPlusAutomaton

} // End namespace FSMSADF

#endif

