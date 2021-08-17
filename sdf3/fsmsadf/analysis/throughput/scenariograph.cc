/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   scenariograph.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 4, 2009
 *
 *  Function        :   FSM-based SADF throughput analysis algorithm using
 *                      scenario graph.
 *
 *  History         :
 *      04-05-09    :   Initial version.
 *
 * $Id: scenariograph.cc,v 1.2.2.2 2010-04-22 07:07:09 mgeilen Exp $
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

#include "scenariograph.h"
#include "../maxplus/mpexplore.h"

using namespace FSMSADF::MPExplore;

namespace FSMSADF
{

    /**
     * computeReferenceSchedule()
     * The function computes a reference schedule based on the set of eigenvectors.
     * The reference schedule contains a map of channel names to initial token time
     * stamps.
     */
    map<CString, MaxPlus::Vector> computeReferenceSchedule(
        map<Scenario *, MaxPlus::Vector *> &eigenvector)
    {
        map<CString, MaxPlus::Vector> v_ref;

        // Iterate over all eigenvectors
        for (map<Scenario *, MaxPlus::Vector *>::iterator i = eigenvector.begin();
             i != eigenvector.end(); i++)
        {
            Scenario *s = i->first;
            ScenarioGraph *sg = s->getScenarioGraph();
            uint idx = 0;

            // Iterate over all channels in the scenario graph
            for (Channels::iterator j = sg->getChannels().begin();
                 j != sg->getChannels().end(); j++)
            {
                Channel *c = *j;
                CString cName = c->getName();

                // Is a channel with the same name already part of the reference
                // schedule?
                map<CString, MaxPlus::Vector>::iterator k = v_ref.find(cName);
                if (k != v_ref.end())
                {
                    // Channel must have same number of initial tokens in all
                    // scenario graphs.
                    if (k->second.getSize() != c->getInitialTokens())
                    {
                        throw CException("Channel '" + cName + "' should have same "
                                         "number of initial tokens in all scenario "
                                         "graphs.");
                    }

                    // Take maximum time stamp as reference time stamp
                    for (uint l = 0; l < c->getInitialTokens(); l++)
                    {
                        MPTime a = v_ref[cName].get(l);
                        MPTime b = i->second->get(idx++);
                        v_ref[cName].put(l, MP_MAX(a, b));
                    }
                }
                else
                {
                    // The channel has not been seen before, use the time stamps
                    // of this eigen vector as reference schedule.
                    for (uint l = 0; l < c->getInitialTokens(); l++)
                    {
                        v_ref[cName].put(l, i->second->get(idx++));
                    }
                }
            }

            // All entries of the eigenvector should have been placed in the
            // reference schedule
            ASSERT(idx == i->second->getSize(), "All initial tokens should be "
                   "used in the reference schedule.");
        }

        return v_ref;
    }

    /**
     * getReferenceScheduleForScenario()
     * The function returns the reference schedule for scenario s based on the
     * reference schedule in v_ref.
     */
    MaxPlus::Vector *getReferenceScheduleForScenario(Scenario *s,
            map<CString, MaxPlus::Vector> &v_ref)
    {
        ScenarioGraph *sg = s->getScenarioGraph();
        MaxPlus::Vector *v_s = new MaxPlus::Vector();

        // Iterate over all channels
        for (Channels::iterator i = sg->getChannels().begin();
             i != sg->getChannels().end(); i++)
        {
            Channel *c = *i;

            // Append entry to reference schedule v_s for each initial token on
            // the channel
            for (uint j = 0; j < c->getInitialTokens(); j++)
            {
                v_s->put(v_s->getSize(), v_ref[c->getName()].get(j));
            }
        }

        return v_s;
    }

    /**
     * Throughput analysis
     * Compute the throughput of an FSM-based SDAF graph while ignoring the FSM.
     *
     * Algorithm:
     * 1. Determine for each scenario s the eigenvector v_s and period T_s.
     * 2. Compute a suitable 'reference schedule'. This can be done by for example,
     *    taking the max over all eigenvectors. An alternative could be to use the
     *    ALAP eigenvector. Currently, the first option is used.
     * 3. Compute the delay for each scenario s using the reference schedule and the
     *    period of scenario s.
     * 4. The throughput is equal to max_s (d_s + T_s).
     */
    Throughput throughputAnalysisGraph(Graph *g)
    {
        map<Scenario *, MaxPlus::Vector *> eigenvector;
        map<Scenario *, MPTime> period, delay;
        map<CString, MaxPlus::Vector> v_ref;
        map<Scenario *, MPExplore::Exploration *> mpe;
        MPTime thr = 0;

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

        // Compute throughput
        for (Scenarios::const_iterator i = scenarios.begin();
             i != scenarios.end(); i++)
        {
            Scenario *s = *i;

            thr = MP_MAX(thr, delay[s] + period[s]);
        }

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

        return Throughput((1.0) / thr);
    }

}  // End namespace FSMSADF

