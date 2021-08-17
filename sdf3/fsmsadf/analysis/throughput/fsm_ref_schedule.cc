/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   fsm_ref_schedule.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 4, 2009
 *
 *  Function        :   FSM-based SADF throughput analysis algorithm using
 *                      scenario graph with reference schedule.
 *
 *  History         :
 *      04-05-09    :   Initial version.
 *
 * $Id: fsm_ref_schedule.cc,v 1.1.2.2 2010-04-22 07:07:09 mgeilen Exp $
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

#include "sdf/analysis/mcm/mcmgraph.h"
#include "scenariograph.h"
#include "fsm_ref_schedule.h"
#include "../../../sdf/analysis/mcm/mcmgraph.h"
#include "../../../sdf/analysis/mcm/mcmyto.h"
#include "../maxplus/mpexplore.h"

using namespace FSMSADF::MPExplore;

namespace FSMSADF
{

    Throughput computeMCMfsm(FSM *fsm, map<Scenario *, MPTime> &delay,
                             map<Scenario *, MPTime> &period)
    {
        // Create a new MCM graph
        MCMgraph *mcmGraph = new MCMgraph();

        // Create the nodes of the MCM graph
        for (FSMstates::iterator i = fsm->getStates().begin();
             i != fsm->getStates().end(); i++)
        {
            FSMstate *s = *i;

            // Create an MCM node for this state
            MCMnode *n = new MCMnode(s->getId(), true);

            // Add the node to the MCM graph
            mcmGraph->addNode(n);

        }

        // Add edges to the MCM graph
        for (FSMstates::iterator i = fsm->getStates().begin();
             i != fsm->getStates().end(); i++)
        {
            FSMstate *s = *i;

            // Iterate over the transitions of this state
            for (FSMtransitionsCIter j = s->getTransitions().begin();
                 j != s->getTransitions().end(); j++)
            {
                // Create an edge in the MCM graph for this transition
                mcmGraph->addEdge(mcmGraph->getEdges().size(),
                                  mcmGraph->getNode((*j)->getSrcState()->getId()),
                                  mcmGraph->getNode((*j)->getDstState()->getId()),
                                  delay[s->getScenario()] + period[s->getScenario()],
                                  1.0);
            }
        }

        CDouble result = mcmGraph->calculateMaximumCycleMeanKarp();

        // Cleanup
        delete mcmGraph;

        return result;
    }

    /**
     * Throughput analysis using reference schedule
     * Compute the throughput of an FSM-based SDAF graph considering the FSM and a
     * reference schedule.
     *
     * Algorithm:
     * 1. Determine for each scenario s the eigenvector v_s and period T_s.
     * 2. Compute a suitable 'reference schedule'. This can be done by for example,
     *    taking the max over all eigenvectors. An alternative could be to use the
     *    ALAP eigenvector. Currently, the first option is used.
     * 3. Compute the delay for each scenario s using the reference schedule and the
     *    period of scenario s.
     * 4. The throughput is equal to the MCM of the FSM where the weight of a
     *    node which corresponds to scenario s is equal to (d_s + T_s).
     */
    Throughput throughputAnalysisUsingRefSchedule(Graph *g)
    {
        map<Scenario *, MaxPlus::Vector *> eigenvector;
        map<Scenario *, MPTime> period, delay;
        map<CString, MaxPlus::Vector> v_ref;
        map<Scenario *, MPExplore::Exploration *> mpe;
        Throughput thr;

        // The set of scenarios...
        Scenarios scenarios = g->getScenarios();

        // Create Max-Plus exploration object for each scenario
        for (Scenarios::const_iterator i = scenarios.begin();
             i != scenarios.end(); i++)
        {
            Scenario *s = *i;
            mpe[s] = new MPExplore::Exploration;
            mpe[s]->G = new SGraph(s->getScenarioGraph(), s);
        }

        // Compute eigenvector and period for each scenario
        for (Scenarios::const_iterator i = scenarios.begin();
             i != scenarios.end(); i++)
        {
            Scenario *s = *i;
            MaxPlus::Vector *v_s = NULL;
            MPTime T_s;

            // Compute eigenvector and period of this scenario
            mpe[s]->exploreEigen(&v_s, &T_s);

            // Store results
            eigenvector[s] = v_s;
            period[s] = T_s;
        }

        // Determine reference schedule
        v_ref = computeReferenceSchedule(eigenvector);

        // Compute delay for each scenario
        for (Scenarios::const_iterator i = scenarios.begin();
             i != scenarios.end(); i++)
        {
            Scenario *s = *i;
            MaxPlus::Vector *v_s;

            // Compute reference schedule for scenario s
            v_s = getReferenceScheduleForScenario(s, v_ref);

            // Compute delay of this scenario
            delay[s] = mpe[s]->exploreDelayForSchedule(v_s, v_s, period[s]);

            // Cleanup
            delete v_s;
        }

        // Compute MCM of the FSM using delay and period computed with reference
        // schedule
        thr = computeMCMfsm(g->getFSM(), delay, period);
        thr = 1.0 / thr;

        // Cleanup
        for (map<Scenario *, MaxPlus::Vector *>::iterator i = eigenvector.begin();
             i != eigenvector.end(); i++)
        {
            delete i->second;
        }
        for (map<Scenario *, MPExplore::Exploration *>::iterator i = mpe.begin();
             i != mpe.end(); i++)
        {
            delete i->second->G;
            delete i->second;
        }

        return thr;
    }

}  // End namespace FSMSADF

