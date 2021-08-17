/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   binding_aware_sdfg.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 14, 2009
 *
 *  Function        :   Binding-aware SDFG
 *
 *  History         :
 *      14-05-09    :   Initial version.
 *      19-04-11    :   Add accessors (M. Koedam)
 *                      Virtual functions, rename to SDF3FlowBase
 *      04-08-11    :   Complete rewrite to separate analysis from mapping
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

#include "binding_aware_graph.h"
#include "../../../../transform/model-static-order/hsdf-bambha.h"

namespace FSMSADF
{

    /**
     * actorMappedToProcessor()
     * The function returns true when the supplied actor is mapped to a
     * processor. Otherwise the function returns false.
     */
    bool BindingAwareGraphBase::BindingAwareProperties::actorMappedToProcessor(
        Actor *a) const
    {
        if (actorToProcessor.find(a) != actorToProcessor.end())
            return true;
        return false;
    }

    /**
     * channelMappedToConnection()
     * The function returns true when the supplied channel is mapped to a
     * connection. Otherwise the function returns false.
     */
    bool BindingAwareGraphBase::BindingAwareProperties::channelMappedToConnection(
        Channel *c) const
    {
        if (channelToConnection.find(c) != channelToConnection.end())
            return true;
        return false;
    }

    /**
     * Construct a binding aware graph based on the supplied platform binding.
     * @param platform binding b
     */
    BindingAwareGraphBase::BindingAwareGraphBase(PlatformBinding *b)
        : Graph(b->getApplicationGraph())
    {
        Graph *gApp = b->getApplicationGraph();

        // Binding-aware graph is initially equal to the application graph.
        // Hence, actors in the binding-aware graph have actor with the same
        // name in the application graph.
        for (ScenarioGraphs::iterator i = gApp->getScenarioGraphs().begin();
             i != gApp->getScenarioGraphs().end(); i++)
        {
            ScenarioGraph *sgApp = *i;
            ScenarioGraph *sg = this->getScenarioGraph(sgApp->getName());

            for (Actors::iterator j = sgApp->getActors().begin();
                 j != sgApp->getActors().end(); j++)
            {
                Actor *aApp = *j;
                Actor *a = sg->getActor(aApp->getName());

                mapBindingAwareActorToApplicationGraphActor[a] = aApp;
            }
        }

        // Platform binding based on which the binding-aware graph is
        // constructed
        setPlatformBinding(b);
    }

    /**
     * Destructor.
     */
    BindingAwareGraphBase::~BindingAwareGraphBase()
    {
    }

    /**
     * getScenarioOfScenarioGraph()
     * The function returns the scenario which uses the supplied scenario graph.
     * It is guaranteed that there is only one scenario which uses a scenario
     * graph, since the binding-aware graph uses a graph with isolated scenario.
     */
    Scenario *BindingAwareGraphBase::getScenarioOfScenarioGraph(
        const ScenarioGraph *sg) const
    {
        Graph *g = sg->getGraph();

        // Iterate over all scenarios
        for (Scenarios::iterator i = g->getScenarios().begin();
             i != g->getScenarios().end(); i++)
        {
            Scenario *s = *i;

            if (s->getScenarioGraph() == sg)
                return s;
        }

        throw CException("Scenario graph is not used in any scenario.");
    }

    /**
     * extractActorMapping ()
     * The function extracts the actor binding and scheduling from the platform
     * graph.
     */
    void BindingAwareGraphBase::extractActorMapping()
    {
        PlatformBinding *b = getPlatformBinding();

        // Iterate over the tile bindings
        for (TileBindings::iterator i = b->getTileBindings().begin();
             i != b->getTileBindings().end(); i++)
        {
            TileBinding *tb = *i;

            // Iterate over the processor bindings
            for (ProcessorBindings::iterator j = tb->getProcessorBindings().begin();
                 j != tb->getProcessorBindings().end(); j++)
            {
                ProcessorBinding *pb = *j;

                // Iterate over the actor bindings
                for (map<Scenario *, Actors>::iterator k =
                         pb->getActorBindings().begin();
                     k != pb->getActorBindings().end(); k++)
                {
                    Scenario *s = k->first;
                    Actors &actors = k->second;

                    // Find scenario in the binding-aware graph which
                    // corresponds to scenario s.
                    Scenario *sn = this->getScenario(s->getName());

                    // Scenario graph in the binding-aware graph
                    ScenarioGraph *sg = sn->getScenarioGraph();

                    // Iterate over all actors
                    for (Actors::iterator l = actors.begin();
                         l != actors.end(); l++)
                    {
                        Actor *a = *l;

                        // Find corresponding actor in sg
                        Actor *an = sg->getActor(a->getName());

                        // Store binding of actor an to processor p in the scenario
                        // graph sg
                        bindingAwareProperties[sg].mapActorToProcessor(an,
                                pb->getProcessor());
                    }
                }

                // Iterate over the TDMA resource allocations
                for (map<Scenario *, Time>::iterator k =
                         pb->getWheelsizeAllocations().begin();
                     k != pb->getWheelsizeAllocations().end(); k++)
                {
                    Scenario *s = k->first;
                    Time t = k->second;

                    // Find scenario in the binding-aware graph which
                    // corresponds to scenario s.
                    Scenario *sn = this->getScenario(s->getName());

                    // Scenario graph in the binding-aware graph
                    ScenarioGraph *sg = sn->getScenarioGraph();

                    // Store binding of time slice t to processor p
                    // in the scenario graph sg
                    bindingAwareProperties[sg].mapTimesliceToProcessor(
                        pb->getProcessor(), t);
                }
            }
        }
    }

    /**
     * extractChannelMapping ()
     * The function extracts the channel binding and scheduling from the platform
     * graph.
     */
    void BindingAwareGraphBase::extractChannelMapping()
    {
        PlatformBinding *b = getPlatformBinding();

        // Iterate over the connection bindings
        for (ConnectionBindings::iterator i = b->getConnectionBindings().begin();
             i != b->getConnectionBindings().end(); i++)
        {
            ConnectionBinding *cb = *i;

            // Iterate over all channel bindings
            for (map<Scenario *, Channels>::iterator j =
                     cb->getChannelBindings().begin();
                 j != cb->getChannelBindings().end(); j++)
            {
                Scenario *s = j->first;
                Channels &channels = j->second;

                // Find scenario in the binding-aware graph which
                // corresponds to scenario s.
                Scenario *sn = this->getScenario(s->getName());

                // Scenario graph in the binding-aware graph
                ScenarioGraph *sg = sn->getScenarioGraph();

                // Iterate over all channels
                for (Channels::iterator k = channels.begin();
                     k != channels.end(); k++)
                {
                    Channel *c = *k;

                    // Find corresponding channel in sg
                    Channel *cn = sg->getChannel(c->getName());

                    // Store binding of channel cn to connection c in the scenario
                    // graph sg
                    bindingAwareProperties[sg].mapChannelToConnection(cn,
                            cb->getConnection());
                }
            }
        }
    }

    /**
     * Construct a binding aware graph based on the supplied platform binding.
     */
    void BindingAwareGraphBase::constructBindingAwareGraph()
    {
        // Are the scenarios in the graph isolated?
        if (!this->hasIsolatedScenarios())
            this->isolateScenarios();

        // Extract actor related binding and scheduling properties
        extractActorMapping();

        // Extract channel related binding and scheduling properties
        extractChannelMapping();

        // Construct the graph
        modelBindingAndScheduling();
    }

    /**
     * The function returns a pointer to the actor in the application graph that
     * corresponds to the actor #a.
     * @param pointer to actor #a
     * @return pointer to corresponding actor in application graph
     */
    Actor *BindingAwareGraphBase::getCorrespondingActorInApplicationGraph(
        Actor *a)
    {
        if (mapBindingAwareActorToApplicationGraphActor.find(a)
            != mapBindingAwareActorToApplicationGraphActor.end())
        {
            return mapBindingAwareActorToApplicationGraphActor[a];
        }

        return NULL;
    }

    /**
     * Model all binding decisions in a graph.
     */
    void BindingAwareGraphBase::modelBindingAndScheduling()
    {
        // Iterate over all scenario graphs in the binding aware graph
        for (ScenarioGraphs::iterator i = getScenarioGraphs().begin();
             i != getScenarioGraphs().end(); i++)
        {
            ScenarioGraph *sg = *i;

            // Get scenario which corresponds to this graph (this relation is
            // unique since a graph with isolated scenario is used).
            Scenario *s = getScenarioOfScenarioGraph(sg);

            // Keep list with original channels in the scenario graph. Only
            // these channels need to be modeled. Channels added by this
            // function should be ignored.
            Channels channels = sg->getChannels();

            // Iterate over all actors in the graph
            for (Actors::iterator j = sg->getActors().begin();
                 j != sg->getActors().end(); j++)
            {
                Processor *processor = NULL;
                Actor *a = *j;

                // No binding of actor to processor exists?
                if (!bindingAwareProperties[sg].actorMappedToProcessor(a))
                    throw CException("All actors must be bound to a tile.");

                processor = bindingAwareProperties[sg].getProcessorMappedToActor(a);
                // Model binding of actor a to processor p
                modelActorToTileBinding(a, processor, s);
            }

            // Iterate over all channels in the graph
            for (Channels::iterator j = channels.begin(); j != channels.end(); j++)
            {
                ChannelBindingConstraints *bc;
                Channel *c = *j;

                // @TODO [SANDER] Channel c and scenario s in platform binding are different pointers (but same element) in the binding-aware graph. All lookups should be done based on name, not on pointer!
                // Get binding constraints of channel c
                GraphBindingConstraints *gbc;
                Scenario *sApp = getPlatformBinding()->getApplicationGraph()->getScenario(s->getName());
                Channel *cApp = getPlatformBinding()->getApplicationGraph()->getScenario(s->getName())->getScenarioGraph()->getChannel(c->getName());
                gbc = getPlatformBinding()->getGraphBindingConstraints();
                bc = gbc->getConstraintsOfScenario(sApp)
                     ->getConstraintsOfChannel(cApp);

                // No binding of channel to connection exists?
                if (!bindingAwareProperties[sg].channelMappedToConnection(c))
                {
                    // Model binding of channel c to a tile
                    modelChannelToTileBinding(c, bc, s);
                }
                else
                {
                    Connection *con = bindingAwareProperties[sg].getConnectionMappedToChannel(c);
                    // Model binding of channel c to connection co
                    modelChannelToConnectionBinding(c, bc, con, s);
                }
            }
        }

        // Model static-order schedules
        //modelStaticOrderScheduleOnProcessor();
        // @TODO [SANDER] Model static-order schedules in graph
    }

    /**
     * Model the binding of an actor to a tile inside the binding-aware graph.
     * @param scenario s
     * @param actor a
     * @param processo p
     */
    void BindingAwareGraphBase::modelActorToTileBinding(Actor *a, Processor *p,
            Scenario *s)
    {
        Channel *c;

        // Scenario graph
        ScenarioGraph *sg = a->getScenarioGraph();

        // Compute WCRT when using TDMA
        Time execTime = a->getExecutionTimeOfScenario(s, p->getType());
        Time contextSwitchTime = p->getContextSwitchOverhead();
        Time tdmaTimeSlice = bindingAwareProperties[sg].getTimesliceMappedToProcessors(p);
        Time waitingTime = p->getWheelsize() - tdmaTimeSlice;
        Time wcrt = 0;

        while (execTime > 0)
        {
            wcrt = wcrt + waitingTime;

            if (execTime < tdmaTimeSlice - contextSwitchTime)
            {
                wcrt = wcrt + execTime + contextSwitchTime;
                execTime = 0;
            }
            else
            {
                wcrt = wcrt + tdmaTimeSlice;
                execTime = execTime - tdmaTimeSlice + contextSwitchTime;
            }
        }

        // Set the execution time of actor a on processor p as its worst-case
        // response time
        a->setExecutionTimeOfScenario(s, "wcrt", wcrt);
        a->setDefaultProcessorType("wcrt");

        // Add self-loop with one token
        c = a->getScenarioGraph()->createChannel(a, a);
        c->setName(a->getName() + "-selfedge");
        c->getSrcPort()->setRateOfScenario(s, 1);
        c->getDstPort()->setRateOfScenario(s, 1);
        c->setInitialTokens(1);
        c->setPersistentTokenNames(CString("PT-") + c->getName());
    }

    /**
     * The function models a channel mapped to a connection.
     *
     * The transformation performed by this function on a channel c with n
     * initial tokens and a buffer size distribution b looks as follows
     *
     *          p    c           n               q
     *        s -----------------*---------------> d
     *
     *                           |
     *                           V
     *
     *               c           n
     *        +------------------*--------------------------+
     *        |                                             |
     *      p | p   b[src]-n       b[dst]                 q v q
     *  +---- s <-----*------+ +------*-------------------- d <------+
     *  |   p | p  cs        | |1         cd              q ^ q      |
     *  |     |              | |                            |        |
     *  |     |           1 1| v 1      1      1 1    1     |        |
     *  |     +--------*---> acor ------> acol --> at ------+        |
     *  |         csr  n   1 | ^ 1  crl        clt      ctd          |
     *  |                    | |                                     |
     *  |                  1 * |                                     |
     *  |                    +-+                                     |
     *  |                    ca                                      |
     *  |      n    csd                1    1     cdd                |
     *  +------*-----------------------> ad -------------------------+
     *
     * Actor ad enforces the minimal delay of channel c to be considered in TDMA
     * allocation. This minimal delay creates scheduling freedom for the
     * interconnect scheduling problem.
     *
     * Actors acor and acol models the latency and rrate of sending a token
     * through the channel c. This is a latency-rate model of the connection.
     * The assumption is that buffer space is released in the source channel as
     * soon as the data is sent. At this moment, space is claimed at the
     * destination side (i.e. a transmission on the connection may only start
     * when space is available at the destination).
     *
     * Actor at models the worst-case TDMA timewheel synchronization which may
     * occur when tokens are sent from the source processor to the destination
     * processor.
     *
     * Channels cs and cd model the source and destination buffer of the
     * channel.
     */
    void BindingAwareGraphBase::modelChannelToConnectionBinding(Channel *c,
            ChannelBindingConstraints *bc, Connection *co, Scenario *s)
    {
        CString persistentTokenNames;

        // Scenario graph
        ScenarioGraph *sg = c->getScenarioGraph();

        // Buffer size of channel c
        BufferSize b = bc->getBufferSize();

        // Processor to which destination actor of channel c is bound
        Processor *pd = bindingAwareProperties[sg].getProcessorMappedToActor(c->getDstActor());

        // Execution times of the actors acol, acor, ad, and al
        Time latency = co->getLatency();
        Time rate = (Time)(ceil(c->getTokenSizeOfScenario(s) / bc->getBandwidth()));
        Time tdma = pd->getWheelsize()
                    - bindingAwareProperties[sg].getTimesliceMappedToProcessors(pd);
        Time delay = bc->getDelay();

        // Create an actor acol which models the connection latency
        Actor *acol = sg->createActor();
        acol->setName(c->getName() + "_latency");
        acol->setExecutionTimeOfScenario(s, "latency", latency);
        acol->setDefaultProcessorType("latency");

        // Create an actor acor which models the connection rate
        Actor *acor = sg->createActor();
        acor->setName(c->getName() + "_rate");
        acor->setExecutionTimeOfScenario(s, "rate", rate);
        acor->setDefaultProcessorType("rate");

        // Create actor ad which models the timewheel synchronization
        Actor *at = sg->createActor();
        at->setName(c->getName() + "_tdma");
        at->setExecutionTimeOfScenario(s, "tdma", tdma);
        at->setDefaultProcessorType("tdma");

        // Create actor ad which models the minimal delay of channel c
        Actor *ad = sg->createActor();
        ad->setName(c->getName() + "_delay");
        ad->setExecutionTimeOfScenario(s, "delay", delay);
        ad->setDefaultProcessorType("delay");

        // Create channel to model source buffer
        Channel *cs = sg->createChannel(acor, c->getSrcActor());
        cs->setName(c->getName() + "-src-buffer");
        cs->getSrcPort()->setRateOfScenario(s, 1);
        cs->getDstPort()->setRateOfScenario(s, c->getSrcPort()->getRateOfScenario(s));
        if (b[BufferLocation::Src] < c->getInitialTokens())
            throw CException("Insufficient buffer space for initial tokens.");
        cs->setInitialTokens(b[BufferLocation::Src] - c->getInitialTokens());
        persistentTokenNames = "";
        for (uint i = 0; i < cs->getInitialTokens(); i++)
        {
            if (!persistentTokenNames.empty())
                persistentTokenNames += ",";

            persistentTokenNames = persistentTokenNames + CString("PT-") + CString(i);
            persistentTokenNames = persistentTokenNames + CString("-");
            persistentTokenNames = persistentTokenNames + cs->getName();
        }
        cs->setPersistentTokenNames(persistentTokenNames);

        // Create channel to model destination buffer
        Channel *cd = sg->createChannel(c->getDstActor(), acor);
        cd->setName(c->getName() + "-dst-buffer");
        cd->getSrcPort()->setRateOfScenario(s, c->getDstPort()->getRateOfScenario(s));
        cd->getDstPort()->setRateOfScenario(s, 1);
        cd->setInitialTokens(b[BufferLocation::Dst]);
        persistentTokenNames = "";
        for (uint i = 0; i < cd->getInitialTokens(); i++)
        {
            if (!persistentTokenNames.empty())
                persistentTokenNames += ",";

            persistentTokenNames = persistentTokenNames + CString("PT-") + CString(i);
            persistentTokenNames = persistentTokenNames + CString("-");
            persistentTokenNames = persistentTokenNames + cd->getName();
        }
        cd->setPersistentTokenNames(persistentTokenNames);

        // Create channel from source actor to actor which models connection rate
        Channel *csr = sg->createChannel(c->getSrcActor(), acor);
        csr->setName(c->getName() + "-connection-rate");
        csr->getSrcPort()->setRateOfScenario(s, c->getSrcPort()->getRateOfScenario(s));
        csr->getDstPort()->setRateOfScenario(s, 1);
        csr->setInitialTokens(c->getInitialTokens());
        persistentTokenNames = "";
        for (uint i = 0; i < csr->getInitialTokens(); i++)
        {
            if (!persistentTokenNames.empty())
                persistentTokenNames += ",";

            persistentTokenNames = persistentTokenNames + CString("PT-") + CString(i);
            persistentTokenNames = persistentTokenNames + CString("-");
            persistentTokenNames = persistentTokenNames + csr->getName();
        }
        csr->setPersistentTokenNames(persistentTokenNames);

        // Add self-loop to actor acor (only 1 token sent at the same time)
        Channel *ca = sg->createChannel(acor, acor);
        ca->setName(c->getName() + "-self-edge");
        ca->getSrcPort()->setRateOfScenario(s, 1);
        ca->getDstPort()->setRateOfScenario(s, 1);
        ca->setInitialTokens(1);
        ca->setPersistentTokenNames(CString("PT-") + ca->getName());

        // Create channel from actor which models connection rate to actor which
        // models connection latency
        Channel *crl = sg->createChannel(acor, acol);
        crl->setName(c->getName() + "-latency-1");
        crl->getSrcPort()->setRateOfScenario(s, 1);
        crl->getDstPort()->setRateOfScenario(s, 1);
        crl->setInitialTokens(0);

        // Create channel from actor which models connection latency to actor
        // which models TDMA synchronization
        Channel *clt = sg->createChannel(acol, at);
        clt->setName(c->getName() + "-tdma-1");
        clt->getSrcPort()->setRateOfScenario(s, 1);
        clt->getDstPort()->setRateOfScenario(s, 1);
        clt->setInitialTokens(0);

        // Create channel from actor which models TDMA synchronization to
        // destination actor of the channel
        Channel *ctd = sg->createChannel(at, c->getDstActor());
        ctd->setName(c->getName() + "-tdma-2");
        ctd->getSrcPort()->setRateOfScenario(s, 1);
        ctd->getDstPort()->setRateOfScenario(s, c->getDstPort()->getRateOfScenario(s));
        ctd->setInitialTokens(0);

        // Create channel from source actor to actor which models channel delay
        Channel *csd = sg->createChannel(c->getSrcActor(), ad);
        csd->setName(c->getName() + "-latency-2");
        csd->getSrcPort()->setRateOfScenario(s, c->getSrcPort()->getRateOfScenario(s));
        csd->getDstPort()->setRateOfScenario(s, 1);
        csd->setInitialTokens(c->getInitialTokens());
        persistentTokenNames = "";
        for (uint i = 0; i < csd->getInitialTokens(); i++)
        {
            if (!persistentTokenNames.empty())
                persistentTokenNames += ",";

            persistentTokenNames = persistentTokenNames + CString("PT-") + CString(i);
            persistentTokenNames = persistentTokenNames + CString("-");
            persistentTokenNames = persistentTokenNames + csd->getName();
        }
        csd->setPersistentTokenNames(persistentTokenNames);

        // Create channel actor which models channel delay to destination actor
        Channel *cdd = sg->createChannel(ad, c->getDstActor());
        cdd->setName(c->getName() + "-latency-3");
        cdd->getSrcPort()->setRateOfScenario(s, 1);
        cdd->getDstPort()->setRateOfScenario(s, c->getDstPort()->getRateOfScenario(s));
        cdd->setInitialTokens(0);
    }

    /**
     * The function models a channel mapped to a tile.
     */
    void BindingAwareGraphBase::modelChannelToTileBinding(Channel *c,
            ChannelBindingConstraints *bc, Scenario *s)
    {
        CString persistentTokenNames;

        BufferSize b = bc->getBufferSize();
        Channel *m;

        // Add channel from dst to src actor to model memory space
        m = c->getScenarioGraph()->createChannel(c->getDstActor(), c->getSrcActor());
        m->setName(c->getName() + "-buffer");
        m->getSrcPort()->setRateOfScenario(s, c->getDstPort()->getRateOfScenario(s));
        m->getDstPort()->setRateOfScenario(s, c->getSrcPort()->getRateOfScenario(s));

        // Initial tokens reflect storage space
        if (b[BufferLocation::Mem] < c->getInitialTokens())
            throw CException("Insufficient buffer space to store initial tokens.");
        m->setInitialTokens(b[BufferLocation::Mem] - c->getInitialTokens());
        persistentTokenNames = "";
        for (uint i = 0; i < m->getInitialTokens(); i++)
        {
            if (!persistentTokenNames.empty())
                persistentTokenNames += ",";

            persistentTokenNames = persistentTokenNames + CString("PT-") + CString(i);
            persistentTokenNames = persistentTokenNames + CString("-");
            persistentTokenNames = persistentTokenNames + m->getName();
        }
        m->setPersistentTokenNames(persistentTokenNames);
    }

    /**
     * Model a static-order schedule inside the binding-aware graph.
     */
    void BindingAwareGraphBase::modelStaticOrderScheduleOnProcessor()
    {
        modelStaticOrderScheduleInGraphUsingBambha(getPlatformBinding(), this,
                mapBindingAwareActorToApplicationGraphActor);
    }

} // End namespace FSMSADF
