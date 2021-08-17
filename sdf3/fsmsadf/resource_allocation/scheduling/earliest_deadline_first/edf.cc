/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   edf.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   June 2, 2009
 *
 *  Function        :   Earliest deadline first scheduling
 *
 *  History         :
 *      02-06-09    :   Initial version.
 *
 * $Id: edf.cc,v 1.1 2009-12-23 13:37:23 sander Exp $
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

#include "edf.h"

namespace FSMSADF
{

    /**
     * Create earliest-deadline-first schedule for each scenario graph in the
     * platform binding. These schedules are added to the processors inside the
     * platform binding.
     * @param platform binding pb
     * @param binding aware graph g
     */
    void EarliestDeadLineFirstScheduling::schedule(PlatformBinding *pb,
            BindingAwareGraphBase *g)
    {
        // Construct schedule for each scenario in the application graph
        for (Scenarios::iterator i = g->getScenarios().begin();
             i != g->getScenarios().end(); i++)
        {
            Scenario *s = *i;

            // Construct schedule for the binding-aware graph in scenario s
            scheduleScenario(pb, g, s);
        }
    }

    /**
     * scheduleScenario()
     * Create an earliest-deadline-first schedule for the scenario graph
     * corresponding to scenario s.
     */
    void EarliestDeadLineFirstScheduling::scheduleScenario(PlatformBinding *pb,
            BindingAwareGraphBase *g, Scenario *s)
    {
        // Find scenario sApp corresponding scenario s in the application graph
        Scenario *sApp = pb->getApplicationGraph()->getScenario(s->getName());

        // Create precedence graph based on binding aware graph g
        PrecedenceGraph *pg = new PrecedenceGraph(s);

        // Create empty static-order schedule on each processor
        for (TileBindings::iterator i = pb->getTileBindings().begin();
             i != pb->getTileBindings().end(); i++)
        {
            TileBinding *tb = *i;

            // Iterate over all processors
            for (ProcessorBindings::iterator
                 j = tb->getProcessorBindings().begin();
                 j != tb->getProcessorBindings().end(); j++)
            {
                ProcessorBinding *pb = *j;
                StaticOrderSchedule so;

                // Assign empty schedule in scenario s to the processor binding
                pb->setStaticOrderSchedule(sApp, so);
            }
        }

        // Iterate over all nodes in the precedence graph from highest to lowest
        // deadline
        for (PrecedenceGraph::Node *n = pg->getEarliestDeadline();
             n != NULL; n = pg->getEarliestDeadline())
        {
            Actor *a = n->actor;
            Actor *aApp = g->getCorrespondingActorInApplicationGraph(a);

            // Actor mapped to a processor
            if (aApp != NULL)
            {
                ProcessorBinding *p;

                p = pb->getProcessorBindingOfActorInScenario(sApp, aApp);
                if (p != NULL)
                {
                    // Get static order schedule of processor p in scenario s
                    StaticOrderSchedule &so = p->getStaticOrderSchedule(sApp);

                    // Add actor to end of static order schedule so
                    so.insertActor(so.end(), aApp);

                    // First static-order schedule entry on each processor
                    // becomes start of periodic schedule
                    so.setStartPeriodicSchedule(0);
                }
            }

            // Remove node from precedence graph
            pg->removeNode(n);
        }

        // Cleanup
        delete pg;
    }

} // End namespace FSMSADF
