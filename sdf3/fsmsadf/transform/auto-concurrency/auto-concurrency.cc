/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   auto-concurrency.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 20, 2005
 *
 *  Function        :   Model maximum amount of auto-concurrency explicitly.
 *
 *  History         :
 *      20-07-05    :   Initial version.
 *
 * $Id: auto-concurrency.cc,v 1.1.2.2 2010-04-22 07:07:19 mgeilen Exp $
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

#include "auto-concurrency.h"

namespace FSMSADF
{

    /**
     * modelAutoConcurrencyInGraph ()
     * Create a graph in which the maximum amount of auto-concurrency on all actors
     * in all scenario graphs is made explicit through self-edges.
     */
    Graph *modelAutoConcurrencyInGraph(const Graph *g, const uint maxDegree)
    {
        Graph *gNew;

        // Create a copy of the original graph
        gNew = g->clone(GraphComponent(g->getParent(), g->getId()));

        // Iterate over all scenario graphs
        for (ScenarioGraphs::iterator i = gNew->getScenarioGraphs().begin();
             i != gNew->getScenarioGraphs().end(); i++)
        {
            ScenarioGraph *sg = *i;

            // Iterate over all actors in the graph
            for (Actors::iterator j = sg->getActors().begin();
                 j != sg->getActors().end(); j++)
            {
                Actor *a = *j;
                Channel *c;

                // Create self-edge on the actor
                c = sg->createChannel(a, a);

                // Add initial tokens to the channel
                c->setInitialTokens(maxDegree);

                // Set rates on the ports in all scenarios (i.e. set rate only in
                // default scenario)
                c->getSrcPort()->setRateOfScenario(gNew->getDefaultScenario(), 1);
                c->getDstPort()->setRateOfScenario(gNew->getDefaultScenario(), 1);
            }
        }

        return gNew;
    }

} // End namespace FSMSADF

