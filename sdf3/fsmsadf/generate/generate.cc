/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   generate.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 14, 2007
 *
 *  Function        :   Generate FSM-based SADF graphs
 *
 *  History         :
 *      14-07-09    :   Initial version.
 *
 * $Id: generate.cc,v 1.1.2.3 2010-04-25 01:21:17 mgeilen Exp $
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

#include "generate.h"
#include "../analysis/analysis.h"
#include "../transform/transform.h"

namespace FSMSADF
{

    /**
     * RandomGraph()
     * Constructor.
     */
    RandomGraph::RandomGraph()
    {
        // Structure of the graph
        stronglyConnected = false;
        nrScenarioGraphs = 0;
        nrActors = 0;

        // Degree of the actors
        avgDegree = 0;
        varDegree = 0;
        minDegree = 0;
        maxDegree = 0;

        // Actor iteration counts
        avgIterCnt = 0;
        varIterCnt = 0;
        minIterCnt = 0;
        maxIterCnt = 0;

        // Initial tokens
        initialTokenProp = 0;

        // Scenarios
        nrScenarios = 0;

        // Processor types
        nrProcTypes = 0;
        mapChance = 0;
        unifiedActorProperties = false;

        // Execution time
        execTime = false;
        avgExecTime = 0;
        varExecTime = 0;
        minExecTime = 0;
        maxExecTime = 0;

        // State size
        stateSize = false;
        avgStateSize = 0;
        varStateSize = 0;
        minStateSize = 0;
        maxStateSize = 0;

        // Token size
        tokenSize = false;
        avgTokenSize = 0;
        varTokenSize = 0;
        minTokenSize = 0;
        maxTokenSize = 0;

        // Throughput constraint
        throughputConstraint = false;
        autoConcurrencyDegree = 0;
        throughputScaleFactor = 0;
    }

    /**
     * ~RandomGraph()
     * Desctructor.
     */
    RandomGraph::~RandomGraph()
    {
    }

    /**
     * createPort()
     * Create a port on the actor.
     */
    Port *RandomGraph::createPort(Actor *a, const Port::PortType type)
    {
        Port *p;

        p = a->createPort(type);
        p->setName("p" + CString(p->getId()));

        return p;
    }

    /**
     * createActor ()
     * Create an actor with unconnected ports in the scenario graph. The newly
     * created ports are added to inPorts and outPorts.
     */
    Actor *RandomGraph::createActor(ScenarioGraph *sg, Ports &inPorts,
                                    Ports &outPorts)
    {
        uint id, od;
        Port *p;

        // Create actor a on the scenario graph
        Actor *a = sg->createActor();
        a->setName("a" + CString(a->getId()));
        a->setType("t");

        // Create input port
        do
        {
            id = (uint)mtRand.randNorm(avgDegree, varDegree);
        }
        while (id < minDegree || id > maxDegree);

        while (a->getPorts().size() < id)
        {
            p = createPort(a, Port::In);
            inPorts.push_back(p);
        }

        // Create output ports
        do
        {
            od = (uint)mtRand.randNorm(avgDegree, varDegree);
        }
        while (od < minDegree || od > maxDegree);

        while (a->getPorts().size() < id + od)
        {
            p = createPort(a, Port::Out);
            outPorts.push_back(p);
        }

        return a;
    }

    /**
     * createChannel()
     * Create a channel between a randomly selected input port and a randomly
     * selected output port.
     */
    Channel *RandomGraph::createChannel(ScenarioGraph *sg, Ports &inPorts,
                                        Ports &outPorts)
    {
        Port *srcPort, *dstPort;
        Ports::iterator pi, po;
        Channel *c;

        // Select random input (dst) port
        pi = randomPort(inPorts);
        dstPort = *pi;

        // Select random output (src) port
        po = randomPort(outPorts);
        srcPort = *po;

        // Create a channel between the selected source and destination port
        c = sg->createChannel(GraphComponent(sg, sg->getChannels().size()));
        c->setName("ch" + CString(c->getId()));
        c->connectSrc(srcPort);
        c->connectDst(dstPort);

        // Remove input and output port from their lists
        inPorts.erase(pi);
        outPorts.erase(po);

        return c;
    }

    /**
     * dfsStronglyConnected()
     * Perform DFS from this actor. All actors visisted in the DFS are marked in the
     * visit variable with true.
     */
    void RandomGraph::dfsStronglyConnected(Actor *a, vector<bool> &visit,
                                           bool reverseChannels)
    {
        // Visisted actor a before?
        if (visit[a->getId()])
            return;

        // Mark actor a as visisted
        visit[a->getId()] = true;

        // Reverse direction of all channels?
        if (!reverseChannels)
        {
            // Continue on all outgoing channels
            for (Ports::iterator i = a->getPorts().begin();
                 i != a->getPorts().end(); i++)
            {
                Port *p = *i;

                // Is outgoing port?
                if (p->getType() == Port::Out)
                {
                    dfsStronglyConnected(p->getChannel()->getDstActor(), visit,
                                         reverseChannels);
                }
            }
        }
        else
        {
            // Continue on all incoming channels
            for (Ports::iterator i = a->getPorts().begin();
                 i != a->getPorts().end(); i++)
            {
                Port *p = *i;

                // Is incoming port?
                if (p->getType() == Port::In)
                {
                    dfsStronglyConnected(p->getChannel()->getSrcActor(), visit,
                                         reverseChannels);
                }
            }
        }
    }

    /**
     * isStronglyConnected()
     * The function returns true if the scenario graph is stronly connected,
     * otherwise it returns false.
     */
    bool RandomGraph::isStronglyConnected(ScenarioGraph *sg)
    {
        // No graph supplied
        if (sg == NULL)
            return true;

        // Initialize the visit vector
        vector<bool> visit(sg->getActors().size(), false);

        // Perform DFS from first actor
        dfsStronglyConnected(sg->getActors().front(), visit, false);

        // Visisted not all actors?
        for (vector<bool>::iterator i = visit.begin(); i != visit.end(); i++)
        {
            if ((*i) == false)
                return false;
        }

        // Initialize the reverse visit vector
        vector<bool> visitReverse(sg->getActors().size(), false);

        // Perform DFS from first actor (reversing all edges)
        dfsStronglyConnected(sg->getActors().front(), visitReverse, true);

        // Visisted not all actors?
        for (vector<bool>::iterator i = visitReverse.begin();
             i != visitReverse.end(); i++)
        {
            if ((*i) == false)
                return false;
        }

        return true;
    }

    /**
     * createScenarioGraph()
     * Create a random scenario graph in graph g.
     */
    ScenarioGraph *RandomGraph::createScenarioGraph(Graph *g)
    {
        ScenarioGraph *sg = NULL;

        do
        {
            Ports inPorts, outPorts;

            // Delete existing scenario graph (if any)
            delete sg;

            // Create a new scenario graph
            sg = new ScenarioGraph(GraphComponent(g, g->getScenarioGraphs().size()));
            sg->setName("sg" + CString(sg->getId()));
            sg->setType("t");

            // Create graph with nrActors actors
            while (sg->getActors().size() < nrActors)
            {
                Ports inPortsActor, outPortsActor;

                // Create an actor with input and output ports
                createActor(sg, inPortsActor, outPortsActor);

                // Not first actor in the graph?
                if (sg->getActors().size() > 1)
                {
                    // Create a channel between the new actor and an existing actor.
                    // This guarantees connectedness of the scenario graph.
                    if (mtRand.rand() < 0.5)
                    {
                        createChannel(sg, inPortsActor, outPorts);
                    }
                    else
                    {
                        createChannel(sg, inPorts, outPortsActor);
                    }
                }

                // Add unconnected ports to the inPorts and outPorts
                inPorts.insert(inPorts.end(), inPortsActor.begin(),
                               inPortsActor.end());
                outPorts.insert(outPorts.end(), outPortsActor.begin(),
                                outPortsActor.end());
            }

            // Create channels between the input and output ports of the actors
            while (!inPorts.empty() && !outPorts.empty())
            {
                // Create a channel
                createChannel(sg, inPorts, outPorts);
            }

            // Remove all unconnected input ports
            for (Ports::iterator i = inPorts.begin(); i != inPorts.end(); i++)
            {
                Port *p = *i;

                p->getActor()->removePort(p->getName());
            }

            // Remove all unconnected output ports
            for (Ports::iterator i = outPorts.begin(); i != outPorts.end(); i++)
            {
                Port *p = *i;

                p->getActor()->removePort(p->getName());
            }
        }
        while (stronglyConnected && !isStronglyConnected(sg));

        // Add the scenario graph to the graph
        g->addScenarioGraph(sg);

        return sg;
    }

    /**
     * createScenario()
     * Create a random scenario and associate it with a randomly selected scenario
     * graph.
     */
    Scenario *RandomGraph::createScenario(Graph *g)
    {
        ScenarioGraph *sg;
        Scenario *s;

        // Create a new scenario
        s = new Scenario(GraphComponent(g, g->getScenarios().size()));
        s->setName("s" + CString(s->getId()));
        g->addScenario(s);

        // Select a random scenario graph
        sg = randomScenarioGraph(g->getScenarioGraphs());
        s->setScenarioGraph(sg);

        return s;
    }

    /**
     * createFSM()
     * Create a fully-connected FSM.
     */
    FSM *RandomGraph::createFSM(Graph *g)
    {
        FSM *fsm;

        // Create a new FSM
        fsm = new FSM(GraphComponent(g, 0, "fsm"));
        g->setFSM(fsm);

        // Create a state for each scenario
        for (Scenarios::iterator i = g->getScenarios().begin();
             i != g->getScenarios().end(); i++)
        {
            Scenario *s = *i;
            FSMstate *st;

            // Create a new state for this scenario
            st = new FSMstate(GraphComponent(fsm, fsm->getStates().size()));
            st->setName("s" + CString(st->getId()));
            st->setScenario(s);
            fsm->addState(st);
        }

        // Create a transition between each pair of states
        for (FSMstates::iterator i = fsm->getStates().begin();
             i != fsm->getStates().end(); i++)
        {
            FSMstate *si = *i;

            for (FSMstates::iterator j = fsm->getStates().begin();
                 j != fsm->getStates().end(); j++)
            {
                FSMstate *sj = *j;
                FSMtransition *t;

                // Create a transition from state si to state sj
                t = new FSMtransition(GraphComponent(fsm, 0));
                t->connect(si, sj);
            }
        }

        // Select a random state as the initial state
        fsm->setInitialState(randomState(fsm->getStates()));

        return fsm;
    }

    /**
     * getChannelsBetweenActors()
     * The function adds the channels which go directly from actor a to actor b to
     * the list channels.
     */
    void RandomGraph::getChannelsBetweenActors(Actor *a, Actor *b,
            Channels &channels)
    {
        // Iterate over the ports of actor a
        for (Ports::iterator i = a->getPorts().begin();
             i != a->getPorts().end(); i++)
        {
            Port *p = *i;

            // Is this port an output port?
            if (p->getType() == Port::Out)
            {
                Channel *c = p->getChannel();

                // Does the channel connect to actor b?
                if (c->getDstActor() == b)
                {
                    channels.push_back(c);
                }
            }
        }
    }

    /**
     * findCycles()
     * The function performs a DFS on the scenario graph. This DFS is performed by
     * a recursive call to the findCycles function. The parent actor of this call
     * is the actor p. Actor a is the current actor from which the DFS is continued.
     * The maps color and parent indicate respectively the visit status and parent
     * of the actors. Channels contains a list of channels which are part of a
     * cycle. When discovering a cycle, a random channel on the cycle is added to
     * this list. From this point onwards it is assumed that this channel is no
     * longer part of the graph.
     */
    void RandomGraph::findCycles(Actor *p, Actor *a, map<Actor *, uint> &color,
                                 map<Actor *, Actor *> &parent, Channels &initialTokenchannels)
    {
        set<Actor *> actors;
        Channels channels;

        // Visited actor a before?
        if (color[a] != 0)
        {
            // Reconstruct (back-ward) the pairs of actors seen on this cycle
            Actor *v = a;
            Actor *vp = p;
            do
            {
                // Find all channels from the parent of v to v
                getChannelsBetweenActors(vp, v, channels);

                // Previous pair of actors
                v = vp;
                vp = parent[v];
            }
            while (v != a);

            // No channel on this cycle has been selected before
            if (find_first_of(initialTokenchannels.begin(),
                              initialTokenchannels.end(), channels.begin(), channels.end())
                == initialTokenchannels.end())
            {
                // Select random channel and add all channels between its source
                // and destination actor to the list of initialTokenchannels
                Channel *c = randomChannel(channels);
                getChannelsBetweenActors(c->getSrcActor(), c->getDstActor(),
                                         initialTokenchannels);
            }

            return;
        }

        // Start visiting actor a
        color[a] = 1;
        parent[a] = p;

        // Find reachable actors
        for (Ports::iterator i = a->getPorts().begin();
             i != a->getPorts().end(); i++)
        {
            Port *p = *i;

            // Output port?
            if (p->getType() == Port::Out)
            {
                Channel *c = p->getChannel();

                // Channel is not marked as initial token channel? Channels which
                // are marked as initial token channel should not be considered as
                // those cycles are already broken.
                if (find(initialTokenchannels.begin(), initialTokenchannels.end(), c)
                    == initialTokenchannels.end())
                {
                    actors.insert(p->getChannel()->getDstActor());
                }
            }
        }

        // Continue search from reachable actors
        for (set<Actor *>::iterator i = actors.begin(); i != actors.end(); i++)
        {
            findCycles(a, *i, color, parent, initialTokenchannels);
        }

        // End visiting actor a
        color[a] = 0;
    }

    /**
     * selectInitialTokenChannels()
     * The function selects a random set of channels on which initial tokens can
     * be stored. It is guaranteed that at least one channel is selected from each
     * cycle in the graph.
     */
    Channels RandomGraph::selectInitialTokenChannels(ScenarioGraph *sg)
    {
        Channels initialTokenchannels;
        map<Actor *, Actor *> parent;
        map<Actor *, uint> color;

        // Find channels to break cycles - select one channel from each cycle
        for (Actors::iterator i = sg->getActors().begin();
             i != sg->getActors().end(); i++)
        {
            Actor *a = *i;

            findCycles(NULL, a, color, parent, initialTokenchannels);
        }

        // Randomly add some more channels
        while (mtRand.rand() < initialTokenChannelProp)
        {
            Channel *c = randomChannel(sg->getChannels());

            getChannelsBetweenActors(c->getSrcActor(), c->getDstActor(),
                                     initialTokenchannels);
        }

        return initialTokenchannels;
    }

    /**
     * assignRatesToPorts()
     * The function assigns rate to the ports of the actors of the scenario graph
     * under scenario s. The rates are computed from the repetition vector entries
     * of the actors.
     */
    void RandomGraph::assignRatesToPorts(ScenarioGraph *sg, Scenario *s,
                                         RepetitionVector &repVec)
    {
        // Iterate over all channels in the scenario graph
        for (Channels::iterator i = sg->getChannels().begin();
             i != sg->getChannels().end(); i++)
        {
            Channel *c = *i;

            // Assign rates to the ports
            c->getSrcPort()->setRateOfScenario(s, repVec[c->getDstActor()->getId()]);
            c->getDstPort()->setRateOfScenario(s, repVec[c->getSrcActor()->getId()]);
        }
    }

    /**
     * assignInitialTokensToChannels()
     * Assign initial tokens to the initialTokenChannels. The minimal number of
     * tokens placed in a channel is the number of tokens needed to complete one
     * iteration of the destination actor of each channel. The number of firings
     * of the destination actor in one iteration is taken from the repetition vector
     * repVec.
     */
    void RandomGraph::assignInitialTokensToChannels(Channels &initialTokenchannels,
            RepetitionVector &repVec)
    {
        for (Channels::iterator i = initialTokenchannels.begin();
             i != initialTokenchannels.end(); i++)
        {
            Channel *c = *i;

            // Insert minimal number of tokens
            c->setInitialTokens(repVec[c->getSrcActor()->getId()]
                                * repVec[c->getDstActor()->getId()]);

            // Insert additional tokens
            while (mtRand.rand() < initialTokenProp)
            {
                c->setInitialTokens(c->getInitialTokens() + 1);
            }
        }
    }

    /**
     * assignRatesAndInitialTokens()
     * The function assigns randomly selected rates to all ports of all scenario
     * graphs. The function also assigns initial tokens to the channels in the
     * scenario graphs. Sufficiently many tokens are added to the graphs to ensure
     * a deadlock-free execution. Furthermore, it holds that the rates on a channel
     * with initial tokens is constant within all scenarios in which this channel
     * appears. When a rate zero is assigned to a channel c in a scenario, then a
     * new scenario graph is created for this scenario. The channel c will be
     * removed from this new scenario graph. Note that the number of scenario graphs
     * may have increased after this function. The number of scenarios does not
     * change.
     */
    void RandomGraph::assignRatesAndInitialTokens(Graph *g)
    {
        // Iterate over all scenario graphs
        for (ScenarioGraphs::iterator i = g->getScenarioGraphs().begin();
             i != g->getScenarioGraphs().end(); i++)
        {
            ScenarioGraph *sg = *i;
            Channels initialTokenchannels;
            set<Actor *> actors;

            // Rate zero is not supported
            if (minIterCnt == 0)
            {
                throw CException("Iteration count must be at least 1 "
                                 "(minIterCnt > 0).");
            }

            // Initialize the repetition vector
            RepetitionVector repVec(sg->getActors().size(), -1);

            // Select channels used for initial tokens
            initialTokenchannels = selectInitialTokenChannels(sg);

            // Determine actors with fixed repetition vector entry
            for (Channels::iterator j = initialTokenchannels.begin();
                 j != initialTokenchannels.end(); j++)
            {
                Channel *c = *j;

                // Source and destination actor of this channel must have a
                // constant repetition vector entry
                actors.insert(c->getSrcActor());
                actors.insert(c->getDstActor());
            }

            // Iterate over all scenarios
            for (Scenarios::iterator j = g->getScenarios().begin();
                 j != g->getScenarios().end(); j++)
            {
                Scenario *s = *j;

                // Scenario s does not use scenario graph sg?
                if (s->getScenarioGraph() != sg)
                    continue;

                // Iterate over all actors in the graph
                for (Actors::iterator k = sg->getActors().begin();
                     k != sg->getActors().end(); k++)
                {
                    Actor *a = *k;

                    // Actor a has no fixed repetition vector entry (first
                    // condition) or its repetition vector entry has not been fixed
                    // yet (second condition)?
                    if (actors.find(a) == actors.end() || repVec[a->getId()] == -1)
                    {
                        do
                        {
                            repVec[a->getId()] = (uint)mtRand.randNorm(avgIterCnt,
                                                 varIterCnt);
                        }
                        while (repVec[a->getId()] < minIterCnt
                               || repVec[a->getId()] > maxIterCnt);
                    }
                }

                // Assign rates based on repetition vector
                assignRatesToPorts(sg, s, repVec);
            }

            // Assign initial tokens to the channels
            assignInitialTokensToChannels(initialTokenchannels, repVec);
        }
    }

    /**
     * createGraph()
     * Create a random graph.
     */
    Graph *RandomGraph::createGraph()
    {
        Graph *g;

        // Create a new graph
        g = new Graph();
        g->setName("g");

        // Create scenario graphs
        while (g->getScenarioGraphs().size() < nrScenarioGraphs)
        {
            createScenarioGraph(g);
        }

        // Create scenarios
        while (g->getScenarios().size() < nrScenarios)
        {
            createScenario(g);
        }

        // Assign rates and initial tokens to the ports and channels in each
        // scenario (graph)
        assignRatesAndInitialTokens(g);

        // Create FSM
        createFSM(g);

        return g;
    }

    /**
     * randomProcessorTypes()
     * The function returns a list of randomly selected processor types.
     */
    CStrings RandomGraph::randomProcessorTypes()
    {
        CStrings processorTypes;

        // Assign actor properties till actor maps to at least one processor type
        while (processorTypes.empty())
        {
            for (uint procType = 0; procType < nrProcTypes; procType++)
            {
                // Does actor map on this processor type?
                if (mtRand.rand() < mapChance)
                {
                    CString type = CString("p") + CString(procType);
                    processorTypes.push_back(type);
                }
            }
        }

        return processorTypes;
    }

    /**
     * assignActorProperties()
     * Assign actor properties to actor a in the scenarios listed in scenarios to
     * the listed processor types.
     */
    void RandomGraph::assignActorProperties(Actor *a, Scenarios &scenarios,
                                            CStrings &processorTypes)
    {
        Size sz;
        Time t;

        for (CStrings::iterator i = processorTypes.begin();
             i != processorTypes.end(); i++)
        {
            CString type = *i;

            // First processor type is the default
            if (i == processorTypes.begin())
                a->setDefaultProcessorType(type);

            // Assign random execution time?
            if (execTime)
            {
                for (Scenarios::iterator i = scenarios.begin();
                     i != scenarios.end(); i++)
                {
                    Scenario *s = *i;

                    do
                    {
                        t = (Time) mtRand.randNorm(avgExecTime, varExecTime);
                    }
                    while (t < minExecTime || t > maxExecTime);

                    a->setExecutionTimeOfScenario(s, type, t);
                }
            }

            // Assign random state size?
            if (stateSize)
            {
                for (Scenarios::iterator i = scenarios.begin();
                     i != scenarios.end(); i++)
                {
                    Scenario *s = *i;

                    do
                    {
                        sz = (Size) mtRand.randNorm(avgStateSize, varStateSize);
                    }
                    while (sz < minStateSize || sz > maxStateSize);
                    /// @todo Make a more advanced random memory generator.
                    /// Currently it makes one shared memory with size X.
                    /// This is 'equal' to the old statesize option.
                    Actor::Memory *m = new Actor::Memory(MemoryType::Shared, "mem", sz);
                    a->addMemoryForScenario(s, type, m);

                }
            }
        }
    }

    /**
     * assignChannelProperties()
     * Assign channel properties to channel c in the scenarios listed in scenarios.
     */
    void RandomGraph::assignChannelProperties(Channel *c, Scenarios &scenarios)
    {
        uint sz;

        // Assign random token size to channel?
        if (tokenSize)
        {
            for (Scenarios::iterator i = scenarios.begin();
                 i != scenarios.end(); i++)
            {
                Scenario *s = *i;

                do
                {
                    sz = (uint)mtRand.randNorm(avgTokenSize, varTokenSize);
                }
                while (sz < minTokenSize || sz > maxTokenSize);

                c->setTokenSizeOfScenario(s, sz);
            }
        }
    }

    void RandomGraph::assignThroughputConstraint(Graph *g)
    {
        Throughput thr;
        Graph *gNew;

        // No throughput constraint needed?
        if (!throughputConstraint)
            return;

        // Model auto-concurrency degree in the graph
        if (autoConcurrencyDegree != 0)
        {
            gNew = modelAutoConcurrencyInGraph(g, autoConcurrencyDegree);
        }
        else
        {
            gNew = g->clone(GraphComponent(g->getParent(), g->getId()));
        }

        // Compute "maximal" throughput of the graph
        thr = throughputAnalysisWithScenarioTransitions(gNew);

        // Set throughput constraint as fraction of the "maximal" throughput
        g-> setThroughputConstraint(thr * throughputScaleFactor
                                    * mtRand.rand());

        // Cleanup
        delete gNew;
    }

    /**
     * assignProperties()
     * Assign random properties to the graph and its scenario graphs.
     */
    void RandomGraph::assignProperties(Graph *g)
    {
        map<CString, CStrings> mapActorToProcTypes;

        // Iterate over the scenario graphs
        for (ScenarioGraphs::iterator i = g->getScenarioGraphs().begin();
             i != g->getScenarioGraphs().end(); i++)
        {
            ScenarioGraph *sg = *i;
            Scenarios scenarios;

            // Create list of all scenarios in which the scenario graph appears
            for (Scenarios::iterator i = g->getScenarios().begin();
                 i != g->getScenarios().end(); i++)
            {
                Scenario *s = *i;

                if (s->getScenarioGraph() == sg)
                {
                    scenarios.insert(s);
                }
            }

            // Iterate over actors in the scenario graph
            for (Actors::iterator j = sg->getActors().begin();
                 j != sg->getActors().end(); j++)
            {
                Actor *a = *j;
                CStrings processorTypes;

                // Actor properties
                if (unifiedActorProperties)
                {
                    // No set of processor types selected yet?
                    if (mapActorToProcTypes.find(a->getName())
                        == mapActorToProcTypes.end())
                    {
                        mapActorToProcTypes[a->getName()] = randomProcessorTypes();
                    }

                    // Processor types
                    processorTypes = mapActorToProcTypes[a->getName()];

                    // Assign actor properties
                    assignActorProperties(a, scenarios, processorTypes);
                }
                else
                {
                    // Select random set of processor types supported by this actor
                    processorTypes = randomProcessorTypes();

                    // Assign actor properties
                    assignActorProperties(a, scenarios, processorTypes);
                }
            }

            // Iterate over channels in the scenario graph
            for (Channels::iterator j = sg->getChannels().begin();
                 j != sg->getChannels().end(); j++)
            {
                Channel *c = *j;

                // Channel properties
                assignChannelProperties(c, scenarios);
            }
        }

        // Throughput constraint
        assignThroughputConstraint(g);
    }

    /**
     * generateGraph()
     * Create a random graph.
     */
    Graph *RandomGraph::generateGraph()
    {
        Graph *g;

        // Create a random graph
        g = createGraph();

        // Assign properties to the graph and its scenario graphs
        assignProperties(g);

        return g;
    }

    /**
     * randomPort()
     * The function returns an iterator to a random port in the list.
     */
    Ports::iterator RandomGraph::randomPort(Ports &p)
    {
        uint n, i = 0;
        Ports::iterator iter = p.begin();

        ASSERT(p.size() != 0, "Empty list of ports.");

        if (p.size() > 1)
            n = mtRand.randInt(p.size() - 1);
        else
            n = 0;

        while (i < n)
        {
            i++;
            iter++;
        }

        return iter;
    }

    /**
     * randomChannel()
     * The function returns a random channel in the list.
     */
    Channel *RandomGraph::randomChannel(Channels &c)
    {
        Channels::iterator iter = c.begin();
        uint n, i = 0;

        ASSERT(c.size() != 0, "Empty list of channels.");

        if (c.size() > 1)
            n = mtRand.randInt(c.size() - 1);
        else
            n = 0;

        while (i < n)
        {
            i++;
            iter++;
        }

        return *iter;
    }

    /**
     * randomScenarioGraph()
     * The function returns random scenario graph in the list.
     */
    ScenarioGraph *RandomGraph::randomScenarioGraph(ScenarioGraphs &s)
    {
        ScenarioGraphs::iterator iter = s.begin();
        uint n, i = 0;

        ASSERT(s.size() != 0, "Empty list of scenario graphs.");

        if (s.size() > 1)
            n = mtRand.randInt(s.size() - 1);
        else
            n = 0;

        while (i < n)
        {
            i++;
            iter++;
        }

        return *iter;
    }

    /**
     * randomState()
     * The function returns a randomly selected FSM state.
     */
    FSMstate *RandomGraph::randomState(FSMstates &s)
    {
        FSMstates::iterator iter = s.begin();
        uint n, i = 0;

        ASSERT(s.size() != 0, "Empty list of scenario graphs.");

        if (s.size() > 1)
            n = mtRand.randInt(s.size() - 1);
        else
            n = 0;

        while (i < n)
        {
            i++;
            iter++;
        }

        return *iter;
    }

    /**
     * initializeFromXML()
     * Initialize the random graph generator from the XML node.
     */
    void RandomGraph::initializeFromXML(CNode *settingsNode)
    {
        // Number of scenario graphs
        if (CHasChildNode(settingsNode, "stronglyConnected"))
            stronglyConnected = true;

        // Number of scenario graphs
        if (!CHasChildNode(settingsNode, "nrScenarioGraphs"))
            throw CException("Missing nrScenarioGraphs element.");
        nrScenarioGraphs = CGetAttribute(CGetChildNode(settingsNode,
                                         "nrScenarioGraphs"), "n");

        // Number of actors in a scenario graphs
        if (!CHasChildNode(settingsNode, "nrActors"))
            throw CException("Missing nrActors element.");
        nrActors = CGetAttribute(CGetChildNode(settingsNode, "nrActors"), "n");

        // In/out degree of actors
        if (!CHasChildNode(settingsNode, "degree"))
            throw CException("Missing degree element.");
        avgDegree = CGetAttribute(CGetChildNode(settingsNode, "degree"), "avg");
        varDegree = CGetAttribute(CGetChildNode(settingsNode, "degree"), "var");
        minDegree = CGetAttribute(CGetChildNode(settingsNode, "degree"), "min");
        maxDegree = CGetAttribute(CGetChildNode(settingsNode, "degree"), "max");

        // Repetition vector entries of actors
        if (!CHasChildNode(settingsNode, "iterCnt"))
            throw CException("Missing iterCnt element.");
        avgIterCnt = CGetAttribute(CGetChildNode(settingsNode, "iterCnt"), "avg");
        varIterCnt = CGetAttribute(CGetChildNode(settingsNode, "iterCnt"), "var");
        minIterCnt = CGetAttribute(CGetChildNode(settingsNode, "iterCnt"), "min");
        maxIterCnt = CGetAttribute(CGetChildNode(settingsNode, "iterCnt"), "max");

        // Probability that channels are selected to store initial tokens
        if (!CHasChildNode(settingsNode, "initialTokenChannelProp"))
            throw CException("Missing initialTokenChannelProp element.");
        initialTokenChannelProp = CGetAttribute(CGetChildNode(settingsNode,
                                                "initialTokenChannelProp"), "p");

        // Probability that more then the minimal number of initial tokens are
        // placed on a channel
        if (!CHasChildNode(settingsNode, "initialTokenProp"))
            throw CException("Missing initialTokenProp element.");
        initialTokenProp = CGetAttribute(CGetChildNode(settingsNode,
                                         "initialTokenProp"), "p");

        // Number of scenarios
        if (!CHasChildNode(settingsNode, "nrScenarios"))
            throw CException("Missing nrScenarios element.");
        nrScenarios = CGetAttribute(CGetChildNode(settingsNode, "nrScenarios"), "n");

        // Number of different processor type
        if (!CHasChildNode(settingsNode, "nrProcTypes"))
            throw CException("Missing nrProcTypes element.");
        nrProcTypes = CGetAttribute(CGetChildNode(settingsNode, "nrProcTypes"), "n");

        // Probability that actor supports a processor type
        if (!CHasChildNode(settingsNode, "mapChance"))
            throw CException("Missing mapChance element.");
        mapChance = CGetAttribute(CGetChildNode(settingsNode, "mapChance"), "p");

        // Create unified actor properties
        if (CHasChildNode(settingsNode, "unifiedActorProperties"))
            unifiedActorProperties = true;

        // Execution time of an actor
        if (CHasChildNode(settingsNode, "execTime"))
        {
            execTime = true;
            avgExecTime = CGetAttribute(CGetChildNode(settingsNode, "execTime"),
                                        "avg");
            varExecTime = CGetAttribute(CGetChildNode(settingsNode, "execTime"),
                                        "var");
            minExecTime = CGetAttribute(CGetChildNode(settingsNode, "execTime"),
                                        "min");
            maxExecTime = CGetAttribute(CGetChildNode(settingsNode, "execTime"),
                                        "max");
        }

        // State size of an actor
        if (CHasChildNode(settingsNode, "stateSize"))
        {
            stateSize = true;
            avgStateSize = CGetAttribute(CGetChildNode(settingsNode, "stateSize"),
                                         "avg");
            varStateSize = CGetAttribute(CGetChildNode(settingsNode, "stateSize"),
                                         "var");
            minStateSize = CGetAttribute(CGetChildNode(settingsNode, "stateSize"),
                                         "min");
            maxStateSize = CGetAttribute(CGetChildNode(settingsNode, "stateSize"),
                                         "max");
        }

        // Token size of an token
        if (CHasChildNode(settingsNode, "tokenSize"))
        {
            tokenSize = true;
            avgTokenSize = CGetAttribute(CGetChildNode(settingsNode, "tokenSize"),
                                         "avg");
            varTokenSize = CGetAttribute(CGetChildNode(settingsNode, "tokenSize"),
                                         "var");
            minTokenSize = CGetAttribute(CGetChildNode(settingsNode, "tokenSize"),
                                         "min");
            maxTokenSize = CGetAttribute(CGetChildNode(settingsNode, "tokenSize"),
                                         "max");
        }

        // Throughput constraint
        if (CHasChildNode(settingsNode, "throughputConstraint"))
        {
            throughputConstraint = true;
            autoConcurrencyDegree = CGetAttribute(CGetChildNode(settingsNode,
                                                  "throughputConstraint"),
                                                  "autoConcurrencyDegree");
            throughputScaleFactor = CGetAttribute(CGetChildNode(settingsNode,
                                                  "throughputConstraint"),
                                                  "throughputScaleFactor");
        }
    }

} // End namespace FSMSADF

