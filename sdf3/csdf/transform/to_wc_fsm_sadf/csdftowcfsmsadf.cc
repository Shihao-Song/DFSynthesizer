/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdftowcfsmsadf.h
 *
 *  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
 *
 *  Date            :   August 6, 2012
 *
 *  Function        :   Convert a CSDF graph to weakly consistent FSMSADF graph
 *
 *  History         :
 *      06-08-12    :   Initial version.
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

#include "csdftowcfsmsadf.h"

using namespace FSMSADF;

namespace CSDF
{

    /**
     * portRatesConsistent(TimedCSDFactor*, State, map<CSDFport *, int>&)
     * test consistency of the port rates of actor a in state s
     */
    bool portRatesConsistent(const TimedCSDFactor *a, const CSDFstateSpaceThroughputAnalysis::TransitionSystem::State s,  map<CSDFport *, uint>& portRates)
    {
        bool isConsistent = true;
        // for each port...
        CSDFportsCIter pi = a->portsBegin();
        while (isConsistent && pi != a->portsEnd())
        {
            CSDFport *p = *pi;
            uint rate = p->getRate()[s.actSeqPos[a->getId()]];
            // is there already a rate specified?
            if (portRates.find(p) != portRates.end())
            {
                // is it consistent?
                if (portRates[p] != rate)
                {
                    isConsistent = false;
                }
            }
            pi++;
        }
        return isConsistent;
    }

    /**
     * executionTimeConsistent(TimedCSDFactor*, State, map<CSDFport *, int>&)
     * test consistency of the port rates of actor a in state s
     */
    bool executionTimeConsistent(const TimedCSDFactor *a, CSDFgraph::RepetitionVector &firingCount,
                                 map<const TimedCSDFactor *, CSDFtime>& executionTimes)
    {
        // test execution time consistency
        CSDFtimeSequence ets = a->getExecutionTime();
        uint i = a->getId();
        int ff = firingCount[i] % ets.size();
        CSDFtime et = ets[ff];
        // is the execution time already defined?
        if (executionTimes.find(a) != executionTimes.end())
        {
            // is it consistent?
            if (executionTimes[a] != et)
            {
                return  false;
            }
        }
        return  true;
    }


    /**
     * createNewScenario()
     * create a new scenario during the CSDF to WC-FSM-SADF transformation process
     */
    Scenario *createNewScenario(FSMSADF::Graph *resultGraph, TimedCSDFgraph *csdfGraph, std::vector<Scenario *>& newScenarios, CStrings &scenarioNames, CDouble reward,
                                map<Scenario *, FSMSADF::RepetitionVector *>& scenarioRepetitions, map<Scenario *, map<CSDFport *, uint> >& scenarioPortRates,
                                map<Scenario *, map<const TimedCSDFactor *, CSDFtime> >& scenarioExecutionTimes)
    {
        // create a new scenario
        Scenario *s = new Scenario(*resultGraph);
        // a fresh id can be taken from the size of the set of scenarios already created
        s->setId(newScenarios.size());
        // create a name for the scenario
        CString scn = "s" + CString(s->getId());
        scenarioNames.push_back(scn);
        s->setName(scn);
        // set the reward as specified
        s->setReward(reward);

        // add it to the collection of new scenarios
        newScenarios.push_back(s);

        // create a new partial repetition vector to be filled
        scenarioRepetitions[s] = new FSMSADF::RepetitionVector(csdfGraph->nrActors(), 0);

        map<CSDFport *, uint> pr; // an empty port rate map
        scenarioPortRates[s] = pr;

        map<const TimedCSDFactor *, CSDFtime> etm; // empty execution time map
        scenarioExecutionTimes[s] = etm;

        return s;
    }


    /**
     * convertSDFGtoCSDFG ()
     * The function converts an CSDFG to an equivalent CSDFG.
     */
    FSMSADF::Graph *convertCSDFGtoWCFSMSADFG(TimedCSDFgraph *csdfGraph)
    {
        FSMSADF::Graph *resultGraph;

        // Create the new graphh
        FSMSADF::GraphComponent component = FSMSADF::GraphComponent(NULL, 0, csdfGraph->getName());
        resultGraph = new FSMSADF::Graph(component);

        // compute the repetitionvector of the CSDF
        CSDFgraph::RepetitionVector v = csdfGraph->getRepetitionVector();
        // maintain another vector to count the number of firings the actors hae performed
        CSDFgraph::RepetitionVector firingCount(v.size());

        // determine smallest set of scenarios to model the CSDF iteration
        // within a single scenario, all port rates must remain constant
        // all execution times must remain constant
        // and there cannot be dependencies on future scenarios.

        // set currentState to the initial state
        CSDFstateSpaceThroughputAnalysis::TransitionSystem::State currentState;
        currentState.init(csdfGraph->nrActors(), csdfGraph->nrChannels());
        currentState.clear();
        currentState.setInitialTokens(*csdfGraph);

        // maintain a list of scenarios we have created and their names,
        // store their partial repetition vectors, port rates and actor execution times
        std::vector<Scenario *> newScenarios;
        CStrings scenarioNames;
        map<Scenario *, FSMSADF::RepetitionVector *> scenarioRepetitions;
        map<Scenario *, map<CSDFport *, uint> > scenarioPortRates;
        map<Scenario *, map<const TimedCSDFactor *, CSDFtime> > scenarioExecutionTimes;



        // create the first, starting scenario
        Scenario *s;
        s = createNewScenario(resultGraph, csdfGraph, newScenarios, scenarioNames, 1.0, scenarioRepetitions,
                              scenarioPortRates, scenarioExecutionTimes);

        // remember the starting state of the scenario.
        CSDFstateSpaceThroughputAnalysis::TransitionSystem::State startingState;
        startingState = currentState;

        // continue until we have completed the iteration
        while (! isEmpty(v))
        {
            // portRates refers to the current scenarios portrates similarly for executionTimes
            map<CSDFport *, uint>& portRates = scenarioPortRates[s];
            map<const TimedCSDFactor *, CSDFtime>& executionTimes = scenarioExecutionTimes[s];

            // look for an enabled actor consistent with the current scenario
            bool found = false;
            // i counts the CSDF actors
            uint i = 0;
            // loop through the actors until the end or until we found one
            while (!found && i < csdfGraph->nrActors())
            {
                // a is CSDF actor number i
                TimedCSDFactor *a = (TimedCSDFactor *) csdfGraph->getActor(i);

                // test if a has remaining firings in the iteration and it is enabled in the current state
                if (v[i] > 0 && CSDFstateSpaceThroughputAnalysis::TransitionSystem::actorReadyToFire(currentState, a))
                {
                    // test if firing a is consistent with current scenario in terms of port rates and execution time
                    if (portRatesConsistent(a, currentState, portRates) &&
                        executionTimeConsistent(a, firingCount, executionTimes))
                    {
                        // if it is, add it

                        // update port rates in scenario
                        CSDFportsCIter pi = a->portsBegin();
                        while (pi != a->portsEnd())
                        {
                            CSDFport *p = *pi;
                            uint rate = p->getRate()[currentState.actSeqPos[a->getId()]];
                            portRates[p] = rate;
                            pi++;
                        }

                        // update actor execution time
                        CSDFtimeSequence ets = a->getExecutionTime();
                        int ff = firingCount[i] % ets.size();
                        executionTimes[a] = ets[ff];

                        // add firing of a to scenario
                        (*(scenarioRepetitions[s]))[a->getId()]++;

                        // update state according to firing
                        CSDFstateSpaceThroughputAnalysis::TransitionSystem::fireActor(&currentState, a);

                        // update iteration count
                        v[i]--;
                        firingCount[i]++;

                        found = true;
                    }
                    else // not consistent
                    {
                        // try the next actor
                        i++;
                    }

                }
                else // actor i cannot fire or has completed its iteration
                {
                    // try the next actor
                    i++;
                }
            }
            if (!found || isEmpty(v))
            {
                // no more actor, add new scenario

                // create a corresponding scenario graph for the scenario, details
                ScenarioGraph *sg = new ScenarioGraph(GraphComponent(resultGraph, newScenarios.size()));
                sg->setName(s->getName());
                sg->setType(s->getName());

                // add scenario and scenario graph to sadf graph
                resultGraph->addScenarioGraph(sg);
                s->setScenarioGraph(sg);
                resultGraph->addScenario(s);

                sg->setPartialRepetitionVector(s, *(scenarioRepetitions[s]));

                // OK, next let's build the scenario graph

                // determine which actors and channels are to be included in the scenario graph
                std::set<CSDFchannel *> relevantChannels;
                std::set<TimedCSDFactor *> relevantActors;

                // determine channels on which tokens are produced or consumed, or on which initial tokens quietly sit.
                CSDFchannelsCIter ci = csdfGraph->channelsBegin();
                while (ci != csdfGraph->channelsEnd())
                {
                    TimedCSDFchannel *c = (TimedCSDFchannel *)(*ci);
                    CSDFport *sp = c->getSrcPort();
                    CSDFport *dp = c->getDstPort();
                    // if they have a port rate in portRates, then tokens on this channel have moved
                    // any channel with initial tokens (in this scenario) has to be included
                    if (
                        (portRates.find(sp) != portRates.end()) || (portRates.find(dp) != portRates.end()) ||
                        startingState.ch[c->getId()] > 0
                    )
                    {
                        // add the channel and both connected actors
                        relevantChannels.insert(c);
                        relevantActors.insert((TimedCSDFactor *) c->getSrcActor());
                        relevantActors.insert((TimedCSDFactor *) c->getDstActor());
                    }
                    ci++;
                }

                // add the relevant actors

                // remember the link between old CSDF ports and new FSMSADF ports
                map<CSDFport *, FSMSADF::Port *> portMap;

                // for all actors in relevantActors...
                std::set<TimedCSDFactor *>::iterator ai = relevantActors.begin();
                while (ai != relevantActors.end())
                {
                    const TimedCSDFactor *a = *ai;
                    // create the actor
                    FSMSADF::Actor *na = sg->createActor();
                    na->setName(a->getName());
                    na->setType(a->getType());
                    // number of repetitions
                    uint rep = (*(scenarioRepetitions[s]))[a->getId()];
                    na->setRepetitionsOfScenario(s, rep);
                    // set the execution time
                    na->setExecutionTimeOfScenario(s, "model", (FSMSADF::Time) executionTimes[a]);
                    na->setDefaultProcessorType("model");
                    // create the ports identical to the CSDF actor
                    CSDFportsCIter pi = a->portsBegin();
                    while (pi != a->portsEnd())
                    {
                        CSDFport *p = *pi;

                        // determine type in/out
                        FSMSADF::Port::PortType pt = FSMSADF::Port::In;
                        if (p->getType() == CSDFport::Out)
                        {
                            pt = FSMSADF::Port::Out;
                        }
                        // create the port
                        FSMSADF::Port *np = na->createPort(pt);
                        np->setName(p->getName());
                        np->setRateOfScenario(s, portRates[p]);
                        portMap[p] = np;
                        pi++;
                    }
                    ai++;
                }

                // add relevant channels
                std::set<CSDFchannel *>::const_iterator rci = relevantChannels.begin();
                while (rci != relevantChannels.end())
                {
                    const CSDFchannel *c = *rci;
                    // create channel
                    FSMSADF::Channel *nc = sg->createChannel(FSMSADF::GraphComponent(sg, sg->getChannels().size()));
                    nc->setName(c->getName());

                    // link it up to the right ports
                    nc->connectSrc(portMap[c->getSrcPort()]);
                    nc->connectDst(portMap[c->getDstPort()]);

                    // add initial tokens from state at the start of the firing
                    int nt = startingState.ch[c->getId()];
                    nc->setInitialTokens(nt);
                    // determine the token names
                    CStrings tn;
                    for (int i = 0; i < nt; i++)
                    {
                        CString tname = c->getName() + CString(i);
                        tn.push_back(tname);
                    }
                    nc->setPersistentTokenNames(tn);
                    // add final tokens from state at the end of the firing
                    nt = currentState.ch[c->getId()];
                    nc->setFinalTokens(nt);
                    CStrings ftn;
                    for (int i = 0; i < nt; i++)
                    {
                        CString tname = c->getName() + CString(i);
                        ftn.push_back(tname);
                    }
                    nc->setPersistentFinalTokenNames(ftn);

                    rci++;
                }


                // continue with next scenario if iteration is not complete yet
                if (! isEmpty(v))
                {
                    s = createNewScenario(resultGraph, csdfGraph, newScenarios, scenarioNames, 0.0, scenarioRepetitions,
                                          scenarioPortRates, scenarioExecutionTimes);
                    startingState = currentState;
                }

            }
        }

        // we have created all scenarios, now create the scenario fsm
        FSMSADF::FSM *fsm = new FSMSADF::FSM(GraphComponent(resultGraph, 0, "fsm"));

        // add a state for each scenario
        CStringsIter j = scenarioNames.begin();
        for (uint i = 0; i < resultGraph->getScenarios().size(); i++, j++)
        {
            CString name = *j;
            FSMstate *state = new FSMstate(GraphComponent(resultGraph, fsm->getStates().size(), name));
            state->setScenario(newScenarios[i]);
            fsm->addState(state);
        }

        // add circular transitions
        CStringsIter k = scenarioNames.begin();
        j = k++;
        for (uint i = 0; i < resultGraph->getScenarios().size() - 1; i++, j++, k++)
        {
            CString srcName = *j;
            CString dstName = *k;
            FSMtransition *transition = new FSMtransition(GraphComponent(resultGraph, 0));
            transition->connect(fsm->getState(srcName), fsm->getState(dstName));
        }

        // final transition closing the cycle
        FSMtransition *transition = new FSMtransition(GraphComponent(resultGraph, 0));
        CString &firstState = scenarioNames.front();
        CString &lastState = scenarioNames.back();
        transition->connect(fsm->getState(lastState), fsm->getState(firstState));

        // set the initial state
        fsm->setInitialState(fsm->getState("s0"));

        resultGraph->setFSM(fsm);

        return resultGraph;
    }
} //namespace CSDF
