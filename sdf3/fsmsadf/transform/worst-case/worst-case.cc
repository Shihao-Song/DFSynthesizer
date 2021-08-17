/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   worst-case.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   June 29, 2009
 *
 *  Function        :   Transform FSM-based SADF to worst-case FSM-based SADF
 *
 *  History         :
 *      29-06-09    :   Initial version.
 *
 * $Id: worst-case.cc,v 1.1.2.2 2010-04-22 07:07:19 mgeilen Exp $
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

#include "worst-case.h"
#include "../../analysis/analysis.h"

namespace FSMSADF
{

    /**
     * transformToWorstCaseGraph ()
     * The function converts the graph to a graph with one scenario which
     * represents the worst-case behavior over all scenarios in the original
     * graph.
     */
    Graph *transformToWorstCaseGraph(Graph *g)
    {
        map<Scenario *, RepetitionVector> repetitionVectors;
        map<CString, int> repetitionVectorWC;
        map<CString, uint> channelNamesToInitialTokens;
        map<CString, Channels> channelNamesToChannels;
        map<Channel *, Scenarios> channelToScenarios;
        map<CString, Actors> actorNamesToActors;
        map<Actor *, Scenarios> actorToScenarios;

        // Mapping of actor names to actors and channel names to channels
        for (Scenarios::iterator i = g->getScenarios().begin();
             i != g->getScenarios().end(); i++)
        {
            Scenario *s = *i;
            ScenarioGraph *sg = s->getScenarioGraph();

            // Actors
            for (Actors::iterator j = sg->getActors().begin();
                 j != sg->getActors().end(); j++)
            {
                Actor *a = *j;
                actorNamesToActors[a->getName()].push_back(a);
                actorToScenarios[a].insert(s);
            }

            // Channels
            for (Channels::iterator j = sg->getChannels().begin();
                 j != sg->getChannels().end(); j++)
            {
                Channel *c = *j;
                channelNamesToChannels[c->getName()].push_back(c);
                channelToScenarios[c].insert(s);
            }
        }

        // Initial tokens in the channels
        for (map<CString, Channels>::iterator i = channelNamesToChannels.begin();
             i != channelNamesToChannels.end(); i++)
        {
            // Iterate over the channels
            for (Channels::iterator j = i->second.begin();
                 j != i->second.end(); j++)
            {
                Channel *c = *j;

                // First channel with this name?
                if (j == i->second.begin())
                {
                    channelNamesToInitialTokens[i->first] = c->getInitialTokens();
                }
                else
                {
                    // Same number of initial tokens on all channels with same name?
                    if (channelNamesToInitialTokens[i->first]
                        != c->getInitialTokens())
                    {
                        throw CException("Channel must have equal number of initial"
                                         " tokens in all scenario graphs.");
                    }

                    // Same source and destination actor for all channels with the
                    // same name?
                    if (i->second.front()->getSrcActor()->getName()
                        != c->getSrcActor()->getName()
                        || i->second.front()->getDstActor()->getName() !=
                        c->getDstActor()->getName())
                    {
                        cerr << i->second.front()->getName() << endl;
                        cerr << c->getName() << endl;
                        cerr << i->second.front()->getSrcActor()->getName() << endl;
                        cerr << c->getSrcActor()->getName() << endl;
                        cerr << i->second.front()->getDstActor()->getName() << endl;
                        cerr << c->getDstActor()->getName() << endl;
                        throw CException("Channel must have same source and "
                                         "destination actor in all scenario graphs.");
                    }
                }

                // Channel which contains initial tokens must have equal rate in all
                // scenarios.
                if (c->getInitialTokens() != 0)
                {
                    // Iterate over all scenarios
                    for (Scenarios::iterator k = channelToScenarios[c].begin();
                         k != channelToScenarios[c].end(); k++)
                    {
                        Scenario *s = *k;
                        Channel *cFirst = *(i->second.begin());
                        Scenario *sFirst = *(channelToScenarios[cFirst].begin());

                        // Are rate on channel c in scenario s constant?
                        if (cFirst->getSrcPort()->getRateOfScenario(sFirst)
                            != c->getSrcPort()->getRateOfScenario(s)
                            || cFirst->getDstPort()->getRateOfScenario(sFirst)
                            != c->getDstPort()->getRateOfScenario(s))
                        {
                            throw CException("Source and destination port of a "
                                             "channel with initial tokens must have"
                                             " constant rate.");
                        }
                    }
                }
            }
        }

        // Compute repetition vectors for all scenarios
        for (Scenarios::iterator i = g->getScenarios().begin();
             i != g->getScenarios().end(); i++)
        {
            Scenario *s = *i;
            ScenarioGraph *sg = s->getScenarioGraph();

            repetitionVectors[s] = computeRepetitionVector(sg, s);
        }

        // Worst-case repetition vector
        for (map<CString, Actors>::iterator i = actorNamesToActors.begin();
             i != actorNamesToActors.end(); i++)
        {
            int maxRepVecActor = 0;

            // Iterate over all actors with this name
            for (Actors::iterator j = i->second.begin();
                 j != i->second.end(); j++)
            {
                Actor *a = *j;

                // Iterate over all scenarios in which this actor appears
                for (Scenarios::iterator k = actorToScenarios[a].begin();
                     k != actorToScenarios[a].end(); k++)
                {
                    Scenario *s = *k;

                    // Repetition vector of actor a in scenario s larger then any
                    // entry seen before?
                    if (maxRepVecActor < repetitionVectors[s][a->getId()])
                        maxRepVecActor = repetitionVectors[s][a->getId()];
                }
            }

            // Set worst-case repetition vector entry of actor with this name
            repetitionVectorWC[i->first] = maxRepVecActor;
        }

        // Create a new graph
        Graph *gWC = new Graph;

        // Create a new scenario graph in the graph
        ScenarioGraph *sgWC = new ScenarioGraph(GraphComponent(gWC, 0, "wc"));
        gWC->addScenarioGraph(sgWC);

        // Create a new worst-case scenario
        Scenario *sWC = new Scenario(GraphComponent(gWC, 0, "wc"));
        sWC->setScenarioGraph(sgWC);
        gWC->addScenario(sWC);

        // Create FSM
        FSM *fsm = new FSM(GraphComponent(gWC, 0, "fsm"));
        gWC->setFSM(fsm);

        // Create only state of the FSM
        FSMstate *fsmState = new FSMstate(GraphComponent(fsm, 0, "s"));
        fsmState->setScenario(sWC);

        // Add state to the FSM
        fsm->addState(fsmState);
        fsm->setInitialState(fsmState);

        // Create transition from state to itself
        FSMtransition *fsmTransition = new FSMtransition(GraphComponent(fsmState,
                0, "t"));
        fsmTransition->connect(fsmState, fsmState);

        // Add actors to the scenario graph
        for (map<CString, Actors>::iterator i = actorNamesToActors.begin();
             i != actorNamesToActors.end(); i++)
        {
            Actor *aWC = sgWC->createActor();
            aWC->setName(i->first);
            aWC->setType("wc");
        }

        // Add channels to the scenario graph
        for (map<CString, Channels>::iterator i = channelNamesToChannels.begin();
             i != channelNamesToChannels.end(); i++)
        {
            Channel *c = i->second.front();

            // Locate source and destination actor in the worst-case graph
            Actor *srcActor = sgWC->getActor(c->getSrcActor()->getName());
            Actor *dstActor = sgWC->getActor(c->getDstActor()->getName());

            // Create a new channel in the worst-case graph
            Channel *cWC = sgWC->createChannel(srcActor, dstActor);
            cWC->setName(c->getName());

            if (channelNamesToInitialTokens[c->getName()] == 0)
            {
                // Set rate of src/dst port based on worst-case repetition vector
                cWC->getSrcPort()->setRateOfScenario(sWC,
                                                     repetitionVectorWC[dstActor->getName()]);
                cWC->getDstPort()->setRateOfScenario(sWC,
                                                     repetitionVectorWC[srcActor->getName()]);
            }
            else
            {
                // Set rate of src/dst port based on rate of random channel with this
                // name in the original graph
                cWC->getSrcPort()->setRateOfScenario(sWC, c->getSrcPort()
                                                     ->getRateOfScenario(*channelToScenarios[c].begin()));
                cWC->getDstPort()->setRateOfScenario(sWC, c->getDstPort()
                                                     ->getRateOfScenario(*channelToScenarios[c].begin()));

                // Add initial tokens to the channel
                cWC->setInitialTokens(channelNamesToInitialTokens[c->getName()]);
            }
        }

        // Channel properties
        for (map<CString, Channels>::iterator i = channelNamesToChannels.begin();
             i != channelNamesToChannels.end(); i++)
        {
            uint maxTokenSize = 0;

            // Iterate over all channels with this name
            for (Channels::iterator j = i->second.begin();
                 j != i->second.end(); j++)
            {
                Channel *c = *j;

                // Iterate over all scenarios in which this channel appears
                for (Scenarios::iterator k = channelToScenarios[c].begin();
                     k != channelToScenarios[c].end(); k++)
                {
                    Scenario *s = *k;

                    // Tokens size of this channel larger in scenario s?
                    if (maxTokenSize < c->getTokenSizeOfScenario(s))
                        maxTokenSize = c->getTokenSizeOfScenario(s);
                }
            }

            // Set worst-case token size of channel with this name
            sgWC->getChannel(i->first)->setTokenSizeOfScenario(sWC, maxTokenSize);
        }

        // Actor properties
        for (map<CString, Actors>::iterator i = actorNamesToActors.begin();
             i != actorNamesToActors.end(); i++)
        {
            map<CString, Time> procTypeToExecTime;
            map<CString, Size> procTypeToStateSize;

            // Get actor in worst-case graph
            Actor *aWC = sgWC->getActor(i->first);

            // Iterate over all actors with this name
            for (Actors::iterator j = i->second.begin(); j != i->second.end(); j++)
            {
                Actor *a = *j;

                // Iterate over all processor types supported by actor a
                for (Actor::ProcessorTypes::const_iterator
                     k = a->getProcessorTypes().begin();
                     k != a->getProcessorTypes().end(); k++)
                {
                    // Execution time
                    for (map<Scenario *, Time>::const_iterator
                         l = k->executionTime.begin();
                         l != k->executionTime.end(); l++)
                    {
                        if (procTypeToExecTime[k->type] < l->second)
                            procTypeToExecTime[k->type] = l->second;
                    }
                    /// @Todo fix this to new multiple memories instead of single statesize.
                    // State size
                    /*                for (map<Scenario*,Size>::const_iterator
                                            l = k->stateSize.begin();
                                            l != k->stateSize.end(); l++)
                                    {
                                        if (procTypeToStateSize[k->type] < l->second)
                                            procTypeToStateSize[k->type] = l->second;
                                    }*/
                }
            }

            // Set worst-case execution time
            for (map<CString, Time>::iterator j = procTypeToExecTime.begin();
                 j != procTypeToExecTime.end(); j++)
            {
                aWC->setExecutionTimeOfScenario(sWC, j->first, j->second);

                // Set first processor type as the default type
                if (j == procTypeToExecTime.begin())
                    aWC->setDefaultProcessorType(j->first);
            }

            // Set worst-case state size
            /// @Todo fix this to new multiple memories instead of single statesize.
            /*
            for (map<CString, Size>::iterator j = procTypeToStateSize.begin();
                    j != procTypeToStateSize.end(); j++)
            {
                aWC->setStateSizeOfScenario(sWC, j->first, j->second);
            }
             */
        }


        // Add dummy actor with execution time zero to the graph
        Actor *dummyActorWC = sgWC->createActor();
        dummyActorWC->setType("wc");
        dummyActorWC->setExecutionTimeOfScenario(sWC, "wc", 0);
        /// @todo fix this.
        //dummyActorWC->setStateSizeOfScenario(sWC, "wc", 0);
        dummyActorWC->setDefaultProcessorType("wc");

        // Connect the dummy actor to a random actor in the graph
        Channel *dummyChannelWC = sgWC->createChannel(dummyActorWC,
                                  sgWC->getActors().front());

        // Set token size to zero
        dummyChannelWC->setTokenSizeOfScenario(sWC, 0);

        // Set rates to ensure that dummy actor has entry 1 in repetition vector
        dummyChannelWC->getDstPort()->setRateOfScenario(sWC, 1);
        dummyChannelWC->getSrcPort()->setRateOfScenario(sWC,
                repetitionVectorWC[sgWC->getActors().front()->getName()]);


        // Copy throughput constraint
        gWC->setThroughputConstraint(g->getThroughputConstraint());

        return gWC;
    }

} // End of namespace

