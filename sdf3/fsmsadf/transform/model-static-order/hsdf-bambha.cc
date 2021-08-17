/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   hsdf-bambha.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 28, 2011
 *
 *  Function        :   Model static-order schedule in graph using technique
 *                      from Bambha.
 *
 *  History         :
 *      28-07-11    :   Initial version.
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

#include "hsdf-bambha.h"
#include "../../analysis/base/repetition_vector.h"

namespace FSMSADF
{

    /**
     * The function converts a scenario graph, which is in fact a synchronous
     * dataflow graph, to its homogeneous variant.
     * @param pointer to a scenario graph sg
     * @param pointer to a scenario s
     * @param mapping of actors in the original graph to actors in the
     *        homogeneous graph
     * @return pointer to a newly created scenario graph
     */
    ScenarioGraph *convertScenarioGraphToHomogeneousGraph(ScenarioGraph *sg,
            Scenario *s, map< Actor * , list< Actor * > > &sgActorMap)
    {
        ScenarioGraph *h;

        // Construct a new scenario graph
        h = sg->createCopy(GraphComponent(NULL, sg->getId(), sg->getName()));

        // Calculate repetition vector for the graph
        RepetitionVector repetitionVector = computeRepetitionVector(sg, s);

        // Add actors to graph (number of actors depends on repetition vector)
        for (Actors::iterator i = sg->getActors().begin();
             i != sg->getActors().end(); i++)
        {
            Actor *sgA = *i;

            for (int i = 0; i < repetitionVector[sgA->getId()]; i++)
            {
                // Create new actor
                Actor *a = sgA->createCopy(GraphComponent(h, sgA->getId(),
                                           sgA->getName() + CString("_") + CString(i)));

                // Add actor to graph
                h->addActor(a);

                // Add actor relation to mapping
                sgActorMap[sgA].push_back(a);
            }
        }

        // Channels and ports
        for (Channels::iterator i = sg->getChannels().begin();
             i != sg->getChannels().end(); i++)
        {
            Channel *gC = *i;

            // Source and destination port and actor in scenario graph
            Port *gSrcP = gC->getSrcPort();
            Port *gDstP = gC->getDstPort();
            Actor *gSrcA = gSrcP->getActor();
            Actor *gDstA = gDstP->getActor();

            uint nA = gSrcP->getRateOfScenario(s);
            uint nB = gDstP->getRateOfScenario(s);
            uint qA = repetitionVector[gSrcA->getId()];
            uint qB = repetitionVector[gDstA->getId()];
            uint d = gC->getInitialTokens();
            for (uint i = 1; i <= qA; i++)
            {
                // Get pointer to source actor
                Actor *hSrcA = h->getActor(gSrcA->getName()
                                           + CString("_") + CString(i - 1));

                for (uint k = 1; k <= nA; k++)
                {
                    uint l = 1 + (d + (i - 1) * nA + k - 1) % (nB * qB);
                    uint j = 1 + (uint) floor((double)((d + (i - 1) * nA + k - 1) %
                                                       (nB * qB)) / (double)(nB));

                    // Create port on source node
                    Port *hSrcP = gSrcP->createCopy(GraphComponent(hSrcA,
                                                    hSrcA->getPorts().size()));
                    hSrcP->setName(gSrcP->getName()
                                   + CString("_") + CString(k - 1));
                    hSrcP->setRateOfScenario(s, 1);
                    hSrcA->addPort(hSrcP);

                    // Create port on destination node
                    Actor *hDstA = h->getActor(gDstA->getName()
                                               + CString("_") + CString(j - 1));
                    Port *hDstP = gDstP->createCopy(GraphComponent(hDstA,
                                                    hDstA->getPorts().size()));
                    hDstP->setName(gDstP->getName()
                                   + CString("_") + CString(l - 1));
                    hDstP->setRateOfScenario(s, 1);
                    hDstA->addPort(hDstP);

                    // Create channel between actors
                    Channel *c = gC->createCopy(GraphComponent(h,
                                                h->getChannels().size()));
                    c->setName(gC->getName() + CString("_")
                               + CString((i - 1) * nA + (k - 1)));
                    c->connectSrc(hSrcP);
                    c->connectDst(hDstP);

                    // Initial tokens
                    uint t = (uint) floor((double)(d + (i - 1) * nA + k - 1) /
                                          (double)(nB * qB));
                    c->setInitialTokens(t);
                    // @TODO [SANDER] Token should be marked as persistent - preserve original relation

                    // Add channel to graph
                    h->addChannel(c);
                }
            }
        }

        return h;
    }

    /**
     * Model a static order schedule in the scenario graph using the technique
     * from Bambha. Note that this technique can only be applied to homogeneous
     * graphs.
     * @param pointer to a scenario graph sg
     * @param pointer to scenario s
     * @param reference to a static order schedule so that schedules one
     *        iteration of the graph
     * @throw an exception is thrown when the graph is not a homogeneous graph
     * @throw an exception is thrown when the schedule is longer then one
     *        iteration
     */
    void modelStaticOrderScheduleInScenarioGraphUsingBambha(ScenarioGraph *sg,
            Scenario *s, StaticOrderSchedule &so)
    {
        Actor *a, *aN;

        // Technique only works for periodic static-order schedules
        if (!so.isPeriodic())
            throw CException("Static-order schedule is not periodic.");

        // Graph should be homogeneous
        RepetitionVector repetitionVector = computeRepetitionVector(sg, s);
        for (uint i = 0; i < repetitionVector.size(); i++)
        {
            if (repetitionVector[i] != 1)
                throw CException("Scenario graph is not homogeneous.");
        }

        // Occurance count of actors in the original graph
        RepetitionVector occuranceCount(so.getActorCount().size(), 0);

        // Iterate over the schedule
        for (uint i = so.getStartPeriodicSchedule(); !so.isLast(i);
             i = so.next(i))
        {
            /* Actor pointer in schedule may point to a different scenario
             * graph. Locate the corresponding actor in the supplied scenario
             * graph.
             */
            a = sg->getActor(so[i].actor->getName() + CString("_")
                             + CString(occuranceCount[so[i].actor->getId()]));

            // Locate next actor in the schedule
            aN = sg->getActor(so[so.next(i)].actor->getName() + CString("_")
                              + CString(occuranceCount[so[so.next(i)].actor->getId()]));

            // Create a channel from a to aN (no initial token needed)
            sg->createChannel(a, aN);

            // Increase occurance count of the actor a
            occuranceCount[so[i].actor->getId()]++;
        }

        // Create back-edge with initial token from end to start of schedule
        a = sg->getActor(so[so.last()].actor->getName() + CString("_")
                         + CString(occuranceCount[so[so.last()].actor->getId()]));

        // Locate next actor in the schedule
        aN = sg->getActor(so[so.getStartPeriodicSchedule()].actor->getName()
                          + CString("_") + CString(
                              occuranceCount[so[so.getStartPeriodicSchedule()].actor->getId()]));

        // Create a channel from a to aN (one initial token needed)
        Channel *c = sg->createChannel(a, aN);
        c->setInitialTokens(1);
        // @TODO [SANDER] Token should be marked as persistent

        // Increase occurance count of the actor a
        occuranceCount[so[so.last()].actor->getId()]++;
    }

    /**
     * The function extends the static-order schedule to include a complete
     * iteration of all actors contained in the schedule. The required
     * occurance count is determined by the sgActorMap. Each actor in the
     * static-order schedule should appear as often as the length of the
     * corresponding list inside the sgActorMap.
     * @param mapping of old actors (used in static-order schedule) to new
     *        actor references
     * @param static-order schedule
     */
    void extendStaticOrderScheduleToIteration(
        map<Actor *, list< Actor * > > &sgActorMap, StaticOrderSchedule &so)
    {
        uint unfoldingFactor = 1;
        map<Actor *, uint> actorOccuranceCount;

        // Create list with all actors that appear in the static-order schedule
        // and their occurance count
        for (StaticOrderSchedule::iterator i = so.begin(); i != so.end(); i++)
        {
            Actor *a = (*i).actor;

            if (actorOccuranceCount.find(a) == actorOccuranceCount.end())
                actorOccuranceCount[(*i).actor] = 1;
            else
                actorOccuranceCount[(*i).actor] += 1;
        }

        // Compute schedule extension based on length of the lists in the
        // sgActorMap
        for (map<Actor *, uint>::iterator i = actorOccuranceCount.begin();
             i != actorOccuranceCount.end(); i++)
        {
            // Extension of the schedule needed?
            if (i->second *unfoldingFactor != sgActorMap[i->first].size())
            {
                // Extension only allowed if schedule has not been extended
                // before
                if (unfoldingFactor != 1)
                {
                    throw CException("Different actors require different "
                                     "static-order schedule unfoldings.");
                }

                // Extension is only possible if required extensions includes
                // a complete number of schedule iterations
                if (sgActorMap[i->first].size() % i->second != 0)
                {
                    throw CException("Schedule must be extended with full "
                                     "number of iterations");
                }

                unfoldingFactor = sgActorMap[i->first].size() / i->second;
            }
        }

        // No schedule extension needed?
        if (unfoldingFactor == 1)
            return;

        // Create a copy of the original schedule
        StaticOrderSchedule soOld = so;

        // Clear the old static order schedule (but not the copy)
        so.clear();

        // Extend the schedule
        for (uint i = 0; i < unfoldingFactor; i++)
        {
            so.insert(so.end(), soOld.begin(), soOld.end());
        }
    }

    /**
     * The function updates all static-order schedules of scenario s using the
     * relation provided in the sgActorMap parameter. It provides for each actor
     * a list of new actors. The length of the list corresponds to the number
     * of actor firings of the original (old) actor that need to occur in a
     * single iteration of the graph. If needed, the function extends
     * static-orders schedules such that a complete iteration is scheduled.
     * @param platform binding with static-order schedules
     * @param scenario s
     * @param mapping of old actors (used in schedule) to new actors
     * @throw an exception is thrown when a static-order schedule is found that
     *        is longer then one iteration
     */
    void convertStaticOrderSchedulesToSingleIterationHomogeneous(
        PlatformBinding *pb, Scenario *s, map<Actor *, list<Actor *> > &sgActorMap)
    {
        // Iterate over all tiles
        for (TileBindings::const_iterator i = pb->getTileBindings().begin();
             i != pb->getTileBindings().end(); i++)
        {
            TileBinding *tb = *i;

            // Iterate over all processors
            for (ProcessorBindings::const_iterator
                 j = tb->getProcessorBindings().begin();
                 j != tb->getProcessorBindings().end(); j++)
            {
                ProcessorBinding *prB = *j;
                StaticOrderSchedule &so = prB->getStaticOrderSchedule(s);

                // Technique only works for periodic static-order schedules
                if (!so.isPeriodic())
                    throw CException("Static-order schedule is not periodic.");

                // Extend the static-order schedule to a complete iteration
                extendStaticOrderScheduleToIteration(sgActorMap, so);

                // Iterate over the static-order schedule
                for (uint k = 0; !so.isLast(k); k++)
                {
                    ASSERT(!sgActorMap[so[k].actor].empty(),
                           "Schedule not long enough.");

                    // Remove pointer to new actor from the list
                    Actor *aNew = sgActorMap[so[k].actor].front();
                    sgActorMap[so[k].actor].pop_front();

                    // Update the static-order schedule
                    so[k].actor = aNew;
                }
            }
        }
    }

    /**
     * Models the static-order schedules of all processors in the graph g using
     * the technique from Bambha.
     * This function will create a seperate scenario graph for each scenario.
     * These scenario graphs are converted to their corresponding HSDFGs and
     * additional edges are introduced to model the static-order schedules.
     */
    void modelStaticOrderScheduleInGraphUsingBambha(PlatformBinding *pb,
            Graph *g, map<Actor *, Actor *> &mapActorToActorInputGraph)
    {
        // Clone platform binding
        PlatformBinding *pbNew = pb->clone(GraphComponent(NULL, 0));

        // Isolate scenarios
        g->isolateScenarios();

        // Construct homogeneous graph for each scenario
        for (Scenarios::iterator i = g->getScenarios().begin();
             i != g->getScenarios().end(); i++)
        {
            map< Actor *, list<Actor *> > sgActorMap;

            // Scenario and its scenario graph
            Scenario *s = *i;
            ScenarioGraph *sg = s->getScenarioGraph();

            // Create homogeneous graph
            ScenarioGraph *sgHomo = convertScenarioGraphToHomogeneousGraph(sg,
                                    s, sgActorMap);

            // Replace existing scenario graph
            g->replaceScenarioGraph(sg, sgHomo);

            // Update actor pointers in static order schedules
            convertStaticOrderSchedulesToSingleIterationHomogeneous(pbNew, s,
                    sgActorMap);

            // Cleanup existing scenario graph
            delete sg;
        }

        // Add static-order scheduling constraints to the graph
        for (TileBindings::const_iterator i = pb->getTileBindings().begin();
             i != pb->getTileBindings().end(); i++)
        {
            TileBinding *tb = *i;

            for (ProcessorBindings::const_iterator
                 j = tb->getProcessorBindings().begin();
                 j != tb->getProcessorBindings().end(); j++)
            {
                ProcessorBinding *prB = *j;

                for (map<Scenario *, StaticOrderSchedule>::iterator
                     k = prB->getStaticOrderSchedules().begin();
                     k != prB->getStaticOrderSchedules().end(); k++)
                {
                    Scenario *s = k->first;
                    StaticOrderSchedule &so = k->second;

                    // Find corresponding scenario graph
                    ScenarioGraph *sg = g->getScenarioGraph(s->getName());

                    // Model static order schedule in the scenario graph
                    modelStaticOrderScheduleInScenarioGraphUsingBambha(sg, s, so);
                }
            }
        }

        // Cleanup
        delete pbNew;
    }

    /**
     * Create a graph in which the static-order schedules of all processors are
     * modelled explicitly using the technique from Bambha.
     * This function will create a seperate scenario graph for each scenario.
     * These scenario graphs are converted to their corresponding HSDFGs and
     * additional edges are introduced to model the static-order schedules.
     */
    Graph *modelStaticOrderScheduleInGraphUsingBambha(PlatformBinding *pb)
    {
        map<Actor *, Actor *> mapActorToActorInputGraph;

        // Clone application graph
        Graph *g = pb->getApplicationGraph();
        Graph *gNew = g->clone(GraphComponent(NULL, g->getId(), g->getName()));

        // Transform the graph
        modelStaticOrderScheduleInGraphUsingBambha(pb, gNew,
                mapActorToActorInputGraph);

        return gNew;
    }

} // End namespace FSMSADF
