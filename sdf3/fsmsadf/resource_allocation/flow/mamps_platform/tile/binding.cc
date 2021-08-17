/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   binding.cc
 *
 *  Author          :   Umar Waqas (u.waqas@student.tue.nl)
 *
 *  Date            :   November 16, 2011
 *
 *  Function        :   Memory dimensioning
 *
 *  History         :
 *      16-11-11    :   Initial version.
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


#include "binding.h"
#include "../binding_aware_graph/binding_aware_graph.h"
#include "../../../../analysis/base/repetition_vector.h"
#include "../../../scheduling/earliest_deadline_first/edf.h"
#include "../../../../analysis/throughput/throughput.h"

namespace FSMSADF
{

    /**
     * TileBindingAlgo()
     * Constructor.
     */
    TileBindingAlgoMampsPlatform::TileBindingAlgoMampsPlatform()
        :
        TileBindingAlgoBase()
    {
    }

    /**
     * ~TileBindingAlgo()
     * Destructor.
     */
    TileBindingAlgoMampsPlatform::~TileBindingAlgoMampsPlatform()
    {
    }


    /**
     * analyzeThroughput ()
     * The function returns the throughput of the application graph
     * mapped onto the platform graph.
     */
    Throughput TileBindingAlgoMampsPlatform::analyzeThroughput(PlatformBinding *pb)
    {
        BindingAwareGraphBase *g;
        Throughput thr;

        // Create binding-aware graph
        g = new BindingAwareGraphMampsPlatform(pb);
        g->constructBindingAwareGraph();

        // Compute throughput of mapped graph
        // (two methods available: state-space or max-plus automaton)
        thr = throughputAnalysisGraphUsingMaxPlusAutomaton(g);
        //thr = throughputAnalysisGraphUsingStateSpace(g);

        // Cleanup
        delete g;

        return thr;
    }

    /**
     * This function is used by #TileBindingAlgoBase to create GraphBinding object.
     * So if you want another GraphBinding object, override this function.
     *
     * @param pb a #PlatformBinging
     * @return a new #GraphBindingBase algorithm
     */
    GraphBindingBase *TileBindingAlgoMampsPlatform::createGraphBinding(PlatformBinding *pb)
    {
        return new GraphBindingMampsPlatform(pb);
    }
    /**
     * constructTileSchedules()
     * Construct static-order and TDMA schedules for the application on all platform
     * tiles.
     */
    bool TileBindingAlgoMampsPlatform::constructTileSchedules(const uint maxNrAppBindings)
    {
        EarliestDeadLineFirstScheduling edfScheduling;
        GraphBindings applicationBindings;
        Processors processors;
        CId mappingId = 0;

        // Create a list with all processors in the platform
        for (Tiles::iterator i = platformGraph->getTiles().begin();
             i != platformGraph->getTiles().end(); i++)
        {
            Tile *t = *i;

            for (Processors::iterator j = t->getProcessors().begin();
                 j != t->getProcessors().end(); j++)
            {
                Processor *p = *j;

                processors.push_back(p);
            }
        }

        // Create an application binding for each platform binding (except initial
        // binding)
        for (PlatformBindings::iterator i = platformBindings->begin();
             i != platformBindings->end(); i++)
        {
            PlatformBinding *pb = *i;

            if (!pb->isInitialBinding())
            {
                // Create an application binding given the initial platform binding
                applicationBindings.push_back(createGraphBinding(pb));
            }
        }

        // Create an empty set of new application bindings
        GraphBindings newAppBindings;

        // For each application binding b in applicationBindings
        for (GraphBindings::iterator i = applicationBindings.begin();
             i != applicationBindings.end(); i++)
        {
            set<Processor *> usedProcessors;
            GraphBindingBase *b = *i;

            // Create schedule for binding b
            logInfo("Creating schedule for binding '" +
                    b->getPlatformBinding()->getName() + "'");

            // Create an empty set of new platform bindings bbNew
            GraphBindings bbNew;

            // Iterate over all scenarios
            for (map<Scenario *, map<Actor *, ActorBinding> >::iterator
                 j = b->getActorBindings().begin();
                 j != b->getActorBindings().end(); j++)
            {
                Scenario *s = j->first;

                // Iterate over all actor bindings
                for (map<Actor *, ActorBinding>::iterator k = j->second.begin();
                     k != j->second.end(); k++)
                {
                    Actor *a = k->first;
                    ActorBinding &ab = k->second;
                    Processor *p = ab.processor;

                    // Compute minimum TDMA time slice allocation on p given actor
                    // binding and scheduling type (i.e. preemptive, non-preemptive)
                    if (p->hasSupportForPreemption())
                    {
                        b->setMinTDMAslice(s, p,
                                           max(p->getContextSwitchOverhead() + 1,
                                               b->getMinTDMAslice(s, p)));
                    }
                    else
                    {
                        b->setMinTDMAslice(s, p, max(b->getMinTDMAslice(s, p),
                                                     min(p->getWheelsize(), p->getContextSwitchOverhead()
                                                         + a->getExecutionTimeOfScenario(s, p->getType()))));
                    }

                    // Compute maximum TDMA time slice allocation on p given
                    // existing resource usage
                    b->setMaxTDMAslice(s, p, b->getMaxTDMAslice(s, p));

                    // Add processor to the set of used processors
                    usedProcessors.insert(p);
                }
            }

            // Unified mapping requires that all scenarios have the same TDMA slice
            // allocation
            for (Processors::iterator i = processors.begin();
                 i != processors.end(); i++)
            {
                Processor *p = *i;
                Time maxMinTDMAslice = 0, minMaxTDMAslice = TIME_MAX;

                // Find maximum minTDMAslice and minimum maxTDMAslice
                for (Scenarios::iterator
                     j = applicationGraph->getScenarios().begin();
                     j != applicationGraph->getScenarios().end(); j++)
                {
                    Scenario *s = *j;

                    maxMinTDMAslice = max(maxMinTDMAslice, b->getMinTDMAslice(s, p));
                    minMaxTDMAslice = min(minMaxTDMAslice, b->getMaxTDMAslice(s, p));
                }

                // Set maximum minTDMAslice and minimum maxTDMAslice
                for (Scenarios::iterator
                     j = applicationGraph->getScenarios().begin();
                     j != applicationGraph->getScenarios().end(); j++)
                {
                    Scenario *s = *j;

                    // Is the processor used in at least one scenario?
                    if (usedProcessors.find(p) != usedProcessors.end())
                    {
                        b->setMinTDMAslice(s, p, maxMinTDMAslice);
                        b->setMaxTDMAslice(s, p, minMaxTDMAslice);
                    }
                    else
                    {
                        // No actor is mapped to the processor in any scenario. So,
                        // there is no need to allocate a TDMA slice on it.
                        b->setMinTDMAslice(s, p, 0);
                        b->setMaxTDMAslice(s, p, 0);
                    }
                }
            }

            // Add b to the set of application bindings ab
            GraphBindings ab;
            ab.push_back(b);



            // While application bindings left in ab
            while (!ab.empty())
            {
                bool flag = true;

                // Take first application binding bs from set ab
                GraphBindingBase *bs = ab.front();
                ab.pop_front();

                // Assign TDMA time slices for each scenario associated with the
                // application graph
                for (Scenarios::iterator
                     j = applicationGraph->getScenarios().begin();
                     j != applicationGraph->getScenarios().end() && flag; j++)
                {
                    Scenario *s = *j;

                    // For each processor p in the application binding bs associated
                    // with as
                    for (Processors::iterator k = processors.begin();
                         k != processors.end() && flag; k++)
                    {
                        Processor *p = *k;
                        Time slice;

                        // Compute slice
                        slice = bs->getMinTDMAslice(s, p) +
                                (bs->getMaxTDMAslice(s, p)
                                 - bs->getMinTDMAslice(s, p)) / 2;

                        // Allocate time slice
                        if (!bs->allocateTDMAslice(s, p, slice))
                        {
                            /* The slice allocation was not successful */
                            int maxSlice = bs->getMaxTDMAslice(s, p);

                            if (maxSlice < slice)
                                slice = maxSlice;

                            flag = bs->allocateTDMAslice(s, p, slice);
                        }

                        if (flag)
                        {
                            // Create schedule for binding b
                            logMsg(
                                "Set time slice in scenario '" + s->getName() +
                                "' on processor '"  + p->getTile()->getName() +
                                "." + p->getName() + "' to " +
                                CString(bs->getAllocatedTDMAslice(s, p)) +
                                " time-units"
                            );
                        }
                        else
                        {
                            // Create schedule for binding b
                            logMsg(
                                "Set time slice in scenario '" + s->getName() +
                                "' on processor '"  + p->getTile()->getName() +
                                "." + p->getName() + "' Failed"
                            );
                        }
                    }
                }

                if (flag)
                {
                    // Create EDF schedule for the platform binding
                    BindingAwareGraphMampsPlatform *bg;
                    bg = new BindingAwareGraphMampsPlatform(bs->getPlatformBinding());
                    edfScheduling.schedule(bs->getPlatformBinding(), bg);
                    delete bg;

                    // Throughput of bs meets throughput constraint?
                    if (isThroughputConstraintSatisfied(bs->getPlatformBinding()))
                    {
                        bool validNewBinding = true;

                        // Compute cost of bs
                        bs->computeParetoQuantities(repetitionVectors, true);

                        // Add bs to bbNew
                        bs->getPlatformBinding()->setName(mappingId++);
                        bbNew.push_back(bs);

                        // Clone bs
                        GraphBindingBase *bsNew = bs->clone();

                        // Decrease maximum TDMA time slice allocation for all
                        // processors
                        for (map<Processor *, map<Scenario *, Time> >::iterator
                             i = bsNew->getMaxTDMAslices().begin();
                             i != bsNew->getMaxTDMAslices().end(); i++)
                        {
                            Processor *p = i->first;

                            for (map<Scenario *, Time>::iterator j = i->second.begin();
                                 j != i->second.end(); j++)
                            {
                                Scenario *s = j->first;

                                bsNew->setMaxTDMAslice(s, p,
                                                       bs->getAllocatedTDMAslice(s, p));

                                if (bsNew->getMaxTDMAslice(s, p) != 0
                                    && bsNew->getMaxTDMAslice(s, p) -
                                    bsNew->getMinTDMAslice(s, p) <= 1)
                                {
                                    validNewBinding = false;
                                }
                            }
                        }

                        // Is this new binding possible within available timewheels?
                        if (validNewBinding)
                            ab.push_back(bsNew);
                    }
                    else
                    {
                        bool validNewBinding = true;

                        // Clone bs
                        GraphBindingBase *bsNew = bs->clone();

                        // Increase minimum TDMA time slice allocation for all
                        // processor
                        for (map<Processor *, map<Scenario *, Time> >::iterator
                             i = bsNew->getMinTDMAslices().begin();
                             i != bsNew->getMinTDMAslices().end(); i++)
                        {
                            Processor *p = i->first;

                            // Increase minimum time slice of all scenarios
                            for (map<Scenario *, Time>::iterator j = i->second.begin();
                                 j != i->second.end(); j++)
                            {
                                Scenario *s = j->first;

                                bsNew->setMinTDMAslice(s, p,
                                                       bs->getAllocatedTDMAslice(s, p));

                                if (bsNew->getMaxTDMAslice(s, p) != 0
                                    && bsNew->getMaxTDMAslice(s, p)
                                    - bsNew->getMinTDMAslice(s, p) <= 1)
                                {
                                    validNewBinding = false;
                                }
                            }
                        }

                        // Is this new binding possible within available timewheels?
                        if (validNewBinding)
                            ab.push_back(bsNew);
                    }
                }
            }

            // Perform pareto minimization on bbNew
            paretoMinimizationOfApplicationBindings(bbNew);

            // Add bbNew to newAppBindings
            for (GraphBindings::iterator j = bbNew.begin(); j != bbNew.end(); j++)
                newAppBindings.push_back(*j);
        }

        // Select new platform bindings from newAppBindings using heuristic
        reduceApplicationBindings(newAppBindings, maxNrAppBindings);

        // Replace the existing platform bindings (except the initial binding)
        replacePlatformBindings(newAppBindings);

        // Found no valid platform binding?
        if (newAppBindings.empty())
            return false;

        // Output the pareto dimensions of all mappings
        for (GraphBindings::iterator i = newAppBindings.begin();
             i != newAppBindings.end(); i++)
        {
            GraphBindingBase *gb = *i;

            cerr << "Pareto quantities of mapping '";
            cerr << gb->getPlatformBinding()->getName() << "'" << endl;
            gb->getParetoQuantities().print(cerr);
        }

        return true;
    }

} // End namespace FSMSADF

