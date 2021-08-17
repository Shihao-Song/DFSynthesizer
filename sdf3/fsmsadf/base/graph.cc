/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   graph.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 22, 1009
 *
 *  Function        :   FSM-based SADF graph
 *
 *  History         :
 *      22-04-09    :   Initial version.
 *
 * $Id: graph.cc,v 1.3 2010-12-21 21:37:53 sander Exp $
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

#include "graph.h"

namespace FSMSADF
{

    /**
     * ScenarioGraph ()
     * Constructor.
     */
    ScenarioGraph::ScenarioGraph(GraphComponent c)
        :
        GraphComponent(c)
    {
    }

    /**
     * ~ScenarioGraph ()
     * Destructor.
     */
    ScenarioGraph::~ScenarioGraph()
    {
        // Actors
        for (Actors::iterator iter = actors.begin(); iter != actors.end(); iter++)
        {
            Actor *a = *iter;

            delete a;
        }

        // Channels
        for (Channels::iterator iter = channels.begin();
             iter != channels.begin(); iter++)
        {
            Channel *c = *iter;

            delete c;
        }
    }

    /**
     * create ()
     * The function returns a pointer to a newly allocated graph object.
     */
    ScenarioGraph *ScenarioGraph::create(GraphComponent c) const
    {
        return new ScenarioGraph(c);
    }

    /**
     * createCopy ()
     * The function returns a pointer to a newly allocated graph object.
     * All properties of the graph are copied.
     */
    ScenarioGraph *ScenarioGraph::createCopy(GraphComponent c) const
    {
        ScenarioGraph *g = create(c);

        // Properties
        g->setName(getName());
        g->setType(getType());

        return g;
    }

    /**
     * clone ()
     * The function returns a pointer to a newly allocated graph object.
     * All properties, actors and channels of the graph are cloned.
     */
    ScenarioGraph *ScenarioGraph::clone(GraphComponent c) const
    {
        ScenarioGraph *g = createCopy(c);

        // Actors
        for (Actors::const_iterator iter = actors.begin();
             iter != actors.end(); iter++)
        {
            GraphComponent component = GraphComponent(g, (*iter)->getId());
            Actor *a = (*iter)->clone(component);
            g->addActor(a);
        }

        // Channels
        for (Channels::const_iterator iter = channels.begin();
             iter != channels.end(); iter++)
        {
            GraphComponent component = GraphComponent(g, (*iter)->getId());
            Channel *ch = (*iter)->clone(component);
            g->addChannel(ch);
        }

        // Storage distributions
        for (map<Scenario *, StorageDistributions>::const_iterator
             i = storageDistributions.begin();
             i != storageDistributions.end(); i++)
        {
            g->setStorageDistributionsOfScenario(g->getGraph()->
                                                 getScenario(i->first->getName()), i->second);
        }

        return g;
    }

    /**
     * constructFromXML ()
     * The function initializes all actor properties based on the XML data.
     */
    void ScenarioGraph::constructFromXML(const CNodePtr scenarioGraphNode)
    {
        // Name
        if (!CHasAttribute(scenarioGraphNode, "name"))
            throw CException("Invalid graph, missing graph name.");
        setName(CGetAttribute(scenarioGraphNode, "name"));

        // Type
        if (!CHasAttribute(scenarioGraphNode, "type"))
            throw CException("Invalid graph, missing graph type.");
        setType(CGetAttribute(scenarioGraphNode, "type"));

        // Actors
        for (CNodePtr actorNode = CGetChildNode(scenarioGraphNode, "actor");
             actorNode != NULL; actorNode = CNextNode(actorNode, "actor"))
        {
            Actor *a = createActor(GraphComponent(this, getActors().size()));
            a->constructFromXML(actorNode);
        }

        // Channels
        for (CNodePtr channelNode = CGetChildNode(scenarioGraphNode, "channel");
             channelNode != NULL; channelNode = CNextNode(channelNode, "channel"))
        {
            Channel *c = createChannel(GraphComponent(this, getChannels().size()));
            c->constructFromXML(channelNode);
        }
    }

    /**
     * constructPropertiesFromXML ()
     * The function initializes all actor properties based on the XML data.
     */
    void ScenarioGraph::constructPropertiesFromXML(Scenario *s,
            const CNodePtr scenarioNode)
    {
        // Set all properties
        for (CNode *propertiesNode = CGetChildNode(scenarioNode);
             propertiesNode != NULL; propertiesNode = CNextNode(propertiesNode))
        {
            if (CIsNode(propertiesNode, "actorProperties"))
            {
                if (!CHasAttribute(propertiesNode, "actor"))
                    throw CException("Missing 'actor' in 'actorProperties'");

                if (hasActor(CGetAttribute(propertiesNode, "actor")))
                {
                    Actor *a = getActor(CGetAttribute(propertiesNode, "actor"));
                    a->constructPropertiesFromXML(s, propertiesNode);
                }
            }
            else if (CIsNode(propertiesNode, "channelProperties"))
            {
                if (!CHasAttribute(propertiesNode, "channel"))
                    throw CException("Missing 'channel' in 'channelProperties'");

                if (hasChannel(CGetAttribute(propertiesNode, "channel")))
                {
                    Channel *c = getChannel(CGetAttribute(propertiesNode,
                                                          "channel"));
                    c->constructPropertiesFromXML(s, propertiesNode);
                }
            }
            else if (CIsNode(propertiesNode, "storageDistributions"))
            {
                StorageDistributions d;
                d.constructFromXML(propertiesNode);
                setStorageDistributionsOfScenario(s, d);
            }
        }
    }

    /**
     * convertToXML ()
     * The function converts the graph to XML data.
     */
    void ScenarioGraph::convertToXML(const CNodePtr scenarioGraphNode)
    {
        // Name
        CAddAttribute(scenarioGraphNode, "name", getName());

        // Type
        CAddAttribute(scenarioGraphNode, "type", getType());

        // Actors
        for (Actors::iterator i = actors.begin(); i != actors.end(); i++)
        {
            (*i)->convertToXML(CAddNode(scenarioGraphNode, "actor"));
        }

        // Channels
        for (Channels::iterator i = channels.begin(); i != channels.end(); i++)
        {
            (*i)->convertToXML(CAddNode(scenarioGraphNode, "channel"));
        }
    }

    /**
     * convertPropertiesToXML ()
     * The function converts the graph properties to XML data.
     */
    void ScenarioGraph::convertPropertiesToXML(Scenario *s,
            const CNodePtr scenarioNode)
    {
        // Actors
        for (Actors::iterator i = actors.begin(); i != actors.end(); i++)
        {
            CNode *actorPropertiesNode = CAddNode(scenarioNode, "actorProperties");
            (*i)->convertPropertiesToXML(s, actorPropertiesNode);
        }

        // Channels
        for (Channels::iterator i = channels.begin(); i != channels.end(); i++)
        {
            CNode *channelPropertiesNode = CAddNode(scenarioNode, "channelProperties");
            (*i)->convertPropertiesToXML(s, channelPropertiesNode);
        }

        // Storage distributions
        if (hasStorageDistributionsInScenario(s))
        {
            StorageDistributions &d = getStorageDistributionsOfScenario(s);
            d.convertToXML(CAddNode(scenarioNode, "storageDistributions"));
        }
    }

    /**
     * getActor ()
     * The function returns a reference to an actor with the given id.
     */
    Actor *ScenarioGraph::getActor(const CId id)
    {
        for (Actors::iterator iter = actors.begin(); iter != actors.end(); iter++)
        {
            Actor *a = *iter;

            if (a->getId() == id)
                return a;
        }

        throw CException("Graph '" + getName() + "' has no actor with id '"
                         + CString(id) + "'.");
    }

    /**
     * getActor ()
     * The function returns a reference to an actor with the given name.
     */
    Actor *ScenarioGraph::getActor(const CString &name)
    {
        for (Actors::iterator iter = actors.begin(); iter != actors.end(); iter++)
        {
            Actor *a = *iter;

            if (a->getName() == name)
                return a;
        }

        throw CException("Graph '" + getName() + "' has no actor with name '"
                         + name + "'.");
    }

    /**
     * hasActor ()
     * The function returns a true when an actor with the given name exists.
     * Otherwise the function returns false.
     */
    bool ScenarioGraph::hasActor(const CString &name)
    {
        for (Actors::iterator iter = actors.begin(); iter != actors.end(); iter++)
        {
            Actor *a = *iter;

            if (a->getName() == name)
                return true;
        }

        return false;
    }

    /**
     * addActor ()
     * Add an actor to a graph.
     */
    void ScenarioGraph::addActor(Actor *a)
    {
        actors.push_back(a);
    }

    /**
     * removeActor ()
     * Remove an actor from a graph.
     */
    void ScenarioGraph::removeActor(const CString &name)
    {
        for (Actors::iterator iter = actors.begin(); iter != actors.end(); iter++)
        {
            Actor *a = *iter;

            if (a->getName() == name)
            {
                actors.erase(iter);
                return;
            }
        }

        throw CException("Graph '" + getName() + "' has no actor '" + name + "'.");
    }

    /**
     * createActor ()
     * Create a new actor on the graph.
     */
    Actor *ScenarioGraph::createActor()
    {
        GraphComponent c = GraphComponent(this, getActors().size());
        Actor *a = new Actor(c);
        a->setName(CString("_a") + CString(a->getId()));
        addActor(a);

        return a;
    }

    /**
     * createActor ()
     * Create a new actor on the graph.
     */
    Actor *ScenarioGraph::createActor(GraphComponent c)
    {
        Actor *a = new Actor(c);
        addActor(a);

        return a;
    }

    /**
     * getChannel ()
     * The function returns a reference to a channel with the given id.
     */
    Channel *ScenarioGraph::getChannel(const CId id)
    {
        for (Channels::iterator iter = channels.begin();
             iter != channels.end(); iter++)
        {
            Channel *c = *iter;

            if (c->getId() == id)
                return c;
        }

        throw CException("Graph '" + getName()
                         + "' has no channel with id '" + CString(id) + "'.");
    }

    /**
     * getChannel ()
     * The function returns a reference to a channel with the given name.
     */
    Channel *ScenarioGraph::getChannel(const CString &name)
    {
        for (Channels::iterator iter = channels.begin();
             iter != channels.end(); iter++)
        {
            Channel *c = *iter;

            if (c->getName() == name)
                return c;
        }

        throw CException("Graph '" + getName()
                         + "' has no channel with name '" + name + "'.");
    }

    /**
     * hasChannel ()
     * The function returns a true when a channel with the given name exists.
     * Otherwise the function returns false.
     */
    bool ScenarioGraph::hasChannel(const CString &name)
    {
        for (Channels::iterator iter = channels.begin();
             iter != channels.end(); iter++)
        {
            Channel *c = *iter;

            if (c->getName() == name)
                return true;
        }

        return false;
    }

    /**
     * addChannel ()
     * Add a channel to a graph.
     */
    void ScenarioGraph::addChannel(Channel *c)
    {
        channels.push_back(c);
    }

    /**
     * removeChannel ()
     * Remove a channel from a graph.
     */
    void ScenarioGraph::removeChannel(const CString &name)
    {
        for (Channels::iterator iter = channels.begin();
             iter != channels.end(); iter++)
        {
            Channel *c = *iter;

            if (c->getName() == name)
            {
                delete c;
                channels.erase(iter);
                return;
            }
        }

        throw CException("Graph '" + getName()
                         + "' has no channel '" + name + "'.");
    }

    /**
     * createChannel ()
     * Create a new channel on the graph.
     */
    Channel *ScenarioGraph::createChannel(GraphComponent c)
    {
        Channel *ch = new Channel(c);
        addChannel(ch);

        return ch;
    }

    /**
     * createChannel ()
     * The function creates a channel between the source and destination actor.
     * Ports with the supplied rates are added to these actors.
     */
    Channel *ScenarioGraph::createChannel(Actor *src, Actor *dst)
    {
        // Create new channel
        GraphComponent c = GraphComponent(this, getChannels().size());
        Channel *ch = createChannel(c);
        ch->setName(CString("_ch") + CString(ch->getId()));

        // Create ports on the actors
        Port *srcP = src->createPort(Port::Out);
        srcP->setName(CString("_p") + CString(srcP->getId()));
        Port *dstP = dst->createPort(Port::In);
        dstP->setName(CString("_p") + CString(dstP->getId()));

        // Connect channel
        ch->connectSrc(srcP);
        ch->connectDst(dstP);

        return ch;
    }

    /**
     * extractSDFgraph()
     * The function converts the scenario graph of scenario s to a timed SDF graph.
     */
    TimedSDFgraph *ScenarioGraph::extractSDFgraph(Scenario *s) const
    {
        TimedSDFgraph *g = new TimedSDFgraph();
        SDFcomponent component;

        // Set properties
        g->setName(getName() + s->getName());
        g->setType(getType());

        // Actors
        for (Actors::const_iterator i = actors.begin(); i != actors.end(); i++)
        {
            Actor *a = *i;

            // Create a new actor
            TimedSDFactor *newA = g->createActor();

            // Properties
            newA->setName(a->getName());
            newA->setType(a->getType());

            // Default processor
            TimedSDFactor::Processor *newProc = newA->addProcessor(
                                                    a->getDefaultProcessorType());
            newProc->type = a->getDefaultProcessorType();
            newProc->execTime = a->getExecutionTimeOfScenario(s,
                                a->getDefaultProcessorType());
            newA->setDefaultProcessor(newProc->type);
        }

        // Channels
        for (Channels::const_iterator i = channels.begin(); i != channels.end(); i++)
        {
            Channel *c = *i;

            // Create a new channel
            TimedSDFchannel *newC = g->createChannel(
                                        g->getActor(c->getSrcActor()->getName()),
                                        (SDFrate)c->getSrcPort()->getRateOfScenario(s),
                                        g->getActor(c->getDstActor()->getName()),
                                        (SDFrate)c->getDstPort()->getRateOfScenario(s),
                                        c->getInitialTokens());

            // Properties
            newC->setName(c->getName());

            // Port names
            newC->getSrcPort()->setName(c->getSrcPort()->getName());
            newC->getDstPort()->setName(c->getDstPort()->getName());
        }

        return g;
    }

    /**
     * getPartialRepetitionVector()
     * The function get partial repetition vector from spec of weakly consistent fsmsadf
     */
    RepetitionVector *ScenarioGraph::getPartialRepetitionVector(Scenario *s) const
    {
        RepetitionVector *repvec = new RepetitionVector();
        // Actors
        for (Actors::const_iterator i = actors.begin(); i != actors.end(); i++)
        {
            Actor *a = *i;
            repvec->push_back(a->getRepetitionsOfScenario(s));
        }
        return repvec;
    }

    /**
     * setPartialRepetitionVector()
     * The function sets the partial repetition vector for a scenario
     */
    void ScenarioGraph::setPartialRepetitionVector(Scenario *s, const RepetitionVector &v)
    {
        // Actors
        for (Actors::const_iterator i = actors.begin(); i != actors.end(); i++)
        {
            Actor *a = *i;
            a->setRepetitionsOfScenario(s, v[a->getId()]);
        }
    }

    /**
     * The function removes all scenario except scenario s from the graph.
     */
    void ScenarioGraph::isolateScenario(Scenario *s)
    {
        // Actors
        for (Actors::iterator iter = actors.begin(); iter != actors.end(); iter++)
        {
            Actor *a = *iter;

            a->isolateScenario(s);
        }

        // Channels
        for (Channels::iterator iter = channels.begin();
             iter != channels.end(); iter++)
        {
            Channel *c = *iter;

            c->isolateScenario(s);
        }

        // Storage distributions
        if (hasStorageDistributionsInScenario(s))
        {
            StorageDistributions d = getStorageDistributionsOfScenario(s);
            storageDistributions.clear();
            setStorageDistributionsOfScenario(s, d);
        }
    }

    /**
     * getStorageDistributionsOfScenario ()
     * The function returns the storage distributions of this scenario graph in
     * scenario s, or when scenario s is not specified it returns the storage
     * distributions in the default scenario. When also no default scenario exists,
     * an exception is thrown.
     */
    StorageDistributions &ScenarioGraph::getStorageDistributionsOfScenario(
        Scenario *s)
    {
        if (storageDistributions.find(s) != storageDistributions.end())
            return storageDistributions.find(s)->second;

        if (storageDistributions.find(getDefaultScenario())
            == storageDistributions.end())
        {
            throw CException("Scenario graph '" + getName() +
                             "' has no storage distributions.");
        }

        return storageDistributions.find(getDefaultScenario())->second;
    }

    /**
     * setStorageDistributionsOfScenario ()
     * Set the storage distributions of this scenario graph in scenario s.
     */
    void ScenarioGraph::setStorageDistributionsOfScenario(Scenario *s,
            const StorageDistributions d)
    {
        storageDistributions[s] = d;
    }

    /**
     * hasStorageDistributionsInScenario ()
     * The function returns true when storage distributions have been specified for
     * scenario s or through a default scenario. Otherwise the function returns
     * false.
     */
    bool ScenarioGraph::hasStorageDistributionsInScenario(Scenario *s) const
    {
        if (storageDistributions.find(s) != storageDistributions.end()
            || storageDistributions.find(getDefaultScenario())
            != storageDistributions.end())
        {
            return true;
        }

        return false;
    }

    /**
     * getDefaultScenario ()
     * The function returns a pointer to the default scenario.
     */
    Scenario *ScenarioGraph::getDefaultScenario() const
    {
        return getGraph()->getDefaultScenario();
    }


    /**
     * The function returns a mapping of persistent token names to a pair of a
     * channel in this scenario graph and a sequence number for the initial token on
     * this channel.
     * @return mapping of persistent token names to a pair of a channel in this
     *         scenario graph and a sequence for the initial token on this channel
     */
    const PersistentTokenMap ScenarioGraph::getPersistentTokenMap()
    {
        PersistentTokenMap mp;

        for (Channels::iterator i = getChannels().begin();
             i != getChannels().end(); i++)
        {
            Channel *c = *i;
            const CStrings &pt = c->getPersistentTokenNames();
            uint ci = 0;
            for (CStrings::const_iterator j = pt.begin(); j != pt.end(); j++, ci++)
            {
                mp[*j] = pair<Channel *, uint>(c, ci);
            }

        }

        return mp;
    }

    /**
     * The function returns a mapping of persistent final token names to a pair of a
     * channel in this scenario graph and a sequence number for the initial token on
     * this channel.
     * @return mapping of persistent token names to a pair of a channel in this
     *         scenario graph and a sequence for the initial token on this channel
     */
    const PersistentTokenMap ScenarioGraph::getPersistentTokenMapFinal()
    {
        PersistentTokenMap mp;

        for (Channels::iterator i = getChannels().begin();
             i != getChannels().end(); i++)
        {
            Channel *c = *i;
            const CStrings &pt = c->getPersistentFinalTokenNames();
            uint ci = 0;
            for (CStrings::const_iterator j = pt.begin(); j != pt.end(); j++, ci++)
            {
                mp[*j] = pair<Channel *, uint>(c, ci);
            }

        }

        return mp;
    }

    /**
    * The function returns a mapping of persistent token names to a sequence
    * number of the initial tokens in this graph.
    * (This number will correspond to the index of the token in the maxplus matrix.)
    * @return mapping of persistent token names to an index number on the initial tokens
    * in the graph.
    */
    const PersistentTokenIndexMap ScenarioGraph::getPersistentTokenIndexMap()
    {
        PersistentTokenIndexMap mp;

        uint ti = 0;
        for (Channels::iterator i = getChannels().begin();
             i != getChannels().end(); i++)
        {
            Channel *c = *i;
            uint nrtokens = c->getInitialTokens();
            const CStrings &pt = c->getPersistentTokenNames();
            for (CStrings::const_iterator j = pt.begin(); j != pt.end(); j++)
            {
                mp[*j] = ti++;
            }
            ti += nrtokens - pt.size();
        }

        return mp;
    }

    /**
    * The function returns a mapping of persistent final token names to a sequence
    * number of the final tokens in this graph.
    * (This number will correspond to the index of the token in the maxplus matrix.)
    * @return mapping of persistent final token names to an index number on the final tokens
    * in the graph.
    */
    const PersistentTokenIndexMap ScenarioGraph::getPersistentTokenIndexMapFinal()
    {
        PersistentTokenIndexMap mp;

        uint ti = 0;
        for (Channels::iterator i = getChannels().begin();
             i != getChannels().end(); i++)
        {
            Channel *c = *i;
            uint nrtokens = c->getFinalTokens();
            const CStrings &pt = c->getPersistentFinalTokenNames();
            for (CStrings::const_iterator j = pt.begin(); j != pt.end(); j++)
            {
                mp[*j] = ti++;
            }
            ti += nrtokens - pt.size();
        }

        return mp;
    }

    /**
     * print ()
     * Print the graph to the supplied output stream.
     */
    ostream &ScenarioGraph::print(ostream &out)
    {
        out << "ScenarioGraph (" << getName() << ")" << endl;
        out << "id:        " << getId() << endl;
        out << "type:      " << getType() << endl;
        out << endl;

        for (Actors::iterator iter = actors.begin(); iter != actors.end(); iter++)
        {
            Actor *a = *iter;

            a->print(out);
        }

        for (Channels::iterator iter = channels.begin();
             iter != channels.end(); iter++)
        {
            Channel *c = *iter;

            c->print(out);
        }

        out << endl;

        return out;
    }

    /**
     * Graph ()
     * Constructor.
     */
    Graph::Graph(GraphComponent c)
        :
        GraphComponent(c),
        fsm(NULL)
    {
        createDefaultScenario();
    }

    /**
     * Constructor.
     */
    Graph::Graph(Graph *g)
        :
        GraphComponent(NULL, 0),
        fsm(NULL)
    {
        createDefaultScenario();

        // Properties
        setName(g->getName());
        setThroughputConstraint(g->getThroughputConstraint());

        // Scenarios (create copy, but do not set scenario graph as these do not
        // exist yet)
        for (Scenarios::iterator i = g->getScenarios().begin();
             i != g->getScenarios().end(); i++)
        {
            // Do not clone the default scenario
            if ((*i) != getDefaultScenario())
            {
                Scenario *s = (*i)->createCopy(GraphComponent(this, (*i)->getId()));
                addScenario(s);
            }
        }

        // Scenario graphs
        for (ScenarioGraphs::const_iterator i = g->getScenarioGraphs().begin();
             i != g->getScenarioGraphs().end(); i++)
        {
            ScenarioGraph *sg = (*i)->clone(GraphComponent(this, (*i)->getId()));
            addScenarioGraph(sg);
        }

        // Link scenarios to scenario graphs
        for (Scenarios::iterator i = g->getScenarios().begin();
             i != g->getScenarios().end(); i++)
        {
            Scenario *s = (*i);

            // Skip the default scenario as it is not related to any scenario graph
            if (s != getDefaultScenario())
            {
                getScenario(s->getName())->setScenarioGraph(getScenarioGraph(s->getScenarioGraph()->getName()));
            }
        }

        // FSM
        if (g->getFSM() != NULL)
        {
            setFSM(g->getFSM()->clone(GraphComponent(this, g->getFSM()->getId())));
        }
    }

    /**
     * Graph ()
     * Constructor.
     */
    Graph::Graph()
        :
        GraphComponent(NULL, 0),
        fsm(NULL)
    {
        createDefaultScenario();
    }

    /**
     * ~Graph ()
     * Destructor.
     */
    Graph::~Graph()
    {
        // Scenario graphs
        for (ScenarioGraphs::iterator i = scenarioGraphs.begin();
             i != scenarioGraphs.end(); i++)
        {
            delete *i;
        }

        // Default scenario
        delete defaultScenario;

        // FSM
        delete fsm;
    }

    /**
     * create ()
     * The function returns a pointer to a newly allocated graph object.
     */
    Graph *Graph::create(GraphComponent c) const
    {
        return new Graph(c);
    }

    /**
     * createCopy ()
     * The function returns a pointer to a newly allocated graph object.
     * All properties of the graph are copied.
     */
    Graph *Graph::createCopy(GraphComponent c) const
    {
        Graph *g = create(c);

        // Properties
        g->setName(getName());
        g->setThroughputConstraint(getThroughputConstraint());

        return g;
    }

    /**
     * clone ()
     * The function returns a pointer to a newly allocated graph object.
     * All properties, actors and channels of the graph are cloned.
     */
    Graph *Graph::clone(GraphComponent c) const
    {
        Graph *g = createCopy(c);

        // Scenarios (create copy, but do not set scenario graph as these do not
        // exist yet)
        for (Scenarios::iterator i = scenarios.begin(); i != scenarios.end(); i++)
        {
            // Do not clone the default scenario
            if ((*i) != getDefaultScenario())
            {
                Scenario *s = (*i)->createCopy(GraphComponent(g, (*i)->getId()));
                g->addScenario(s);
            }
        }

        // Scenario graphs
        for (ScenarioGraphs::const_iterator i = scenarioGraphs.begin();
             i != scenarioGraphs.end(); i++)
        {
            ScenarioGraph *sg = (*i)->clone(GraphComponent(g, (*i)->getId()));
            g->addScenarioGraph(sg);
        }

        // Link scenarios to scenario graphs
        for (Scenarios::iterator i = scenarios.begin(); i != scenarios.end(); i++)
        {
            Scenario *s = (*i);

            // Skip the default scenario as it is not related to any scenario graph
            if (s != getDefaultScenario())
            {
                g->getScenario(s->getName())->setScenarioGraph(g->getScenarioGraph(s->getScenarioGraph()->getName()));
            }
        }

        // FSM
        if (getFSM() != NULL)
        {
            g->setFSM(getFSM()->clone(GraphComponent(g, getFSM()->getId())));
        }

        return g;
    }

    /**
     * constructFromXML ()
     * The function initializes all actor properties based on the XML data.
     */
    void Graph::constructFromXML(const CNodePtr applicationGraphNode)
    {
        ScenarioGraph *sg;
        CString name;
        Scenario *s;

        // Name
        if (!CHasAttribute(applicationGraphNode, "name"))
            throw CException("Application graph has no name.");
        setName(CGetAttribute(applicationGraphNode, "name"));

        // Graph (node: fsmsadf)
        if (!CHasChildNode(applicationGraphNode, "fsmsadf"))
        {
            throw CException("Graph does not contain an FSM-SADF graph.");
        }
        else
        {
            CNode *fsmsadfNode = CGetChildNode(applicationGraphNode, "fsmsadf");
            for (CNode *sgNode = CGetChildNode(fsmsadfNode, "scenariograph");
                 sgNode != NULL; sgNode = CNextNode(sgNode, "scenariograph"))
            {
                sg = new ScenarioGraph(GraphComponent(this, getScenarioGraphs().size()));
                sg->constructFromXML(sgNode);
                addScenarioGraph(sg);
            }
        }

        // Graph properties (node: fsmsadfProperties)
        if (CHasChildNode(applicationGraphNode, "fsmsadfProperties"))
        {
            CNode *fsmsadfPropertiesNode
                = CGetChildNode(applicationGraphNode, "fsmsadfProperties");
            for (CNode *propertiesNode = CGetChildNode(fsmsadfPropertiesNode);
                 propertiesNode != NULL;
                 propertiesNode = CNextNode(propertiesNode))
            {
                if (CIsNode(propertiesNode, "defaultProperties"))
                {
                    // Iterate over all scenario graphs
                    for (ScenarioGraphs::iterator i = scenarioGraphs.begin();
                         i != scenarioGraphs.end(); i++)
                    {
                        (*i)->constructPropertiesFromXML(getDefaultScenario(),
                                                         propertiesNode);
                    }
                }

                if (CIsNode(propertiesNode, "scenarios"))
                {
                    for (CNode *scenarioNode = CGetChildNode(propertiesNode, "scenario");
                         scenarioNode != NULL;
                         scenarioNode = CNextNode(scenarioNode, "scenario"))
                    {
                        // Name
                        if (!CHasAttribute(scenarioNode, "name"))
                            throw CException("Scenario has no name.");
                        name = CGetAttribute(scenarioNode, "name");

                        // Scenario graph
                        if (!CHasAttribute(scenarioNode, "graph"))
                            throw CException("Scenario has no graph.");
                        sg = getScenarioGraph(CGetAttribute(scenarioNode, "graph"));

                        // reward
                        CDouble reward = 1.0;
                        if (CHasAttribute(scenarioNode, "reward"))
                            reward = CGetAttribute(scenarioNode, "reward");

                        // Create a new scenario
                        s = new Scenario(GraphComponent(this, getScenarios().size()));
                        s->setName(name);
                        s->setScenarioGraph(sg);
                        s->setReward(reward);
                        addScenario(s);

                        // Set the properties of the actors and channels for this
                        // scenario
                        sg->constructPropertiesFromXML(s, scenarioNode);
                    }
                }

                if (CIsNode(propertiesNode, "graphProperties"))
                {
                    if (CHasChildNode(propertiesNode, "timeConstraints"))
                    {
                        CNode *timeConstraintsNode
                            = CGetChildNode(propertiesNode, "timeConstraints");
                        if (CHasChildNode(timeConstraintsNode, "throughput"))
                        {
                            CString throughputConstraint = CGetNodeContent(
                                                               CGetChildNode(timeConstraintsNode, "throughput"));
                            setThroughputConstraint((double)(throughputConstraint));
                        }
                    }
                }
            }
        }

        // FSM (node: fsm)
        if (!CHasChildNode(applicationGraphNode, "fsm"))
        {
            throw CException("Graph does not contain an FSM.");
        }
        else
        {
            fsm = new FSM(GraphComponent(this, 0));
            fsm->constructFromXML(CGetChildNode(applicationGraphNode, "fsm"));
        }
    }

    /**
     * convertToXML ()
     * The function converts all the graph to XML data.
     */
    void Graph::convertToXML(const CNodePtr applicationGraphNode)
    {
        set<CString> defaultActors, defaultChannels;

        // Graph (name)
        CAddAttribute(applicationGraphNode, "name", getName());

        // Graph (node: fsmsadf)
        CNode *fsmsadfNode = CAddNode(applicationGraphNode, "fsmsadf");
        for (ScenarioGraphs::iterator i = scenarioGraphs.begin();
             i != scenarioGraphs.end(); i++)
        {
            ScenarioGraph *sg = *i;

            CNode *sgNode = CAddNode(fsmsadfNode, "scenariograph");
            CAddAttribute(sgNode, "name", sg->getName());
            sg->convertToXML(sgNode);
        }

        // Graph properties (node: fsmsadfProperties)
        CNode *fsmsadfPropertiesNode = CAddNode(applicationGraphNode,
                                                "fsmsadfProperties");

        // Default properties (present on all scenario graphs)
        CNode *defaultPropertiesNode = CAddNode(fsmsadfPropertiesNode,
                                                "defaultProperties");
        for (ScenarioGraphs::iterator i = scenarioGraphs.begin();
             i != scenarioGraphs.end(); i++)
        {
            ScenarioGraph *sg = *i;

            // Actors
            for (Actors::iterator i = sg->getActors().begin();
                 i != sg->getActors().end(); i++)
            {
                // Not seen this actor before?
                if (defaultActors.find((*i)->getName()) == defaultActors.end())
                {
                    CNode *actorPropertiesNode = CAddNode(defaultPropertiesNode,
                                                          "actorProperties");
                    (*i)->convertPropertiesToXML(getDefaultScenario(),
                                                 actorPropertiesNode);
                    defaultActors.insert((*i)->getName());
                }
            }
        }
        for (ScenarioGraphs::iterator i = scenarioGraphs.begin();
             i != scenarioGraphs.end(); i++)
        {
            ScenarioGraph *sg = *i;

            // Channels
            for (Channels::iterator i = sg->getChannels().begin();
                 i != sg->getChannels().end(); i++)
            {
                // Not seen this channel before?
                if (defaultChannels.find((*i)->getName()) == defaultChannels.end())
                {
                    CNode *channelPropertiesNode = CAddNode(defaultPropertiesNode,
                                                            "channelProperties");
                    (*i)->convertPropertiesToXML(getDefaultScenario(),
                                                 channelPropertiesNode);
                    defaultChannels.insert((*i)->getName());
                }
            }
        }

        // Scenarios
        CNode *scenariosNode = CAddNode(fsmsadfPropertiesNode, "scenarios");
        for (Scenarios::iterator i = scenarios.begin(); i != scenarios.end(); i++)
        {
            Scenario *s = *i;

            CNode *scenarioNode = CAddNode(scenariosNode, "scenario");
            CAddAttribute(scenarioNode, "name", s->getName());
            CAddAttribute(scenarioNode, "graph", s->getScenarioGraph()->getName());
            CAddAttribute(scenarioNode, "reward", s->getReward());

            s->convertToXML(scenarioNode);
        }

        // Graph properties
        CNode *graphPropertiesNode = CAddNode(fsmsadfPropertiesNode,
                                              "graphProperties");
        CNode *timeConstraintsNode = CAddNode(graphPropertiesNode, "timeConstraints");
        CAddNode(timeConstraintsNode, "throughput", getThroughputConstraint());

        // FSM (node: fsm)
        if (fsm != NULL)
        {
            fsm->convertToXML(CAddNode(applicationGraphNode, "fsm"));
        }
    }

    /**
     * extractSDFgraph()
     * The function converts the scenario graph of scenario s to a timed SDF graph.
     */
    TimedSDFgraph *Graph::extractSDFgraph(Scenario *s) const
    {
        return s->getScenarioGraph()->extractSDFgraph(s);
    }

    /**
     * getRepetitionVector()
     * The function returns the partial repetition vector as specified for the scenario in a weakly consistent fsmsadf.
     */
    RepetitionVector *Graph::getPartialRepetitionVector(Scenario *s) const
    {
        return s->getScenarioGraph()->getPartialRepetitionVector(s);
    }


    /**
     * isolateScenarios()
     * The function creates a seperate scenario graph for each scenario.
     */
    void Graph::isolateScenarios()
    {
        // Iterate over the set of scenarios
        for (Scenarios::iterator i = scenarios.begin(); i != scenarios.end(); i++)
        {
            Scenario *s = *i;

            // Clone the scenario graph
            ScenarioGraph *sg = s->getScenarioGraph()->clone(*(s->getScenarioGraph()));
            addScenarioGraph(sg);

            // Update the name of the scenario graph
            sg->setName(sg->getName() + "-" + s->getName());
            sg->setId(s->getId());

            // Update the scenario graph associated with this scenario
            s->setScenarioGraph(sg);

            // Isolate the scenario s in this scenario graph
            sg->isolateScenario(s);
        }

        // Remove all scenario graphs which are not used in any scenario
        for (ScenarioGraphs::iterator i = scenarioGraphs.begin();
             i != scenarioGraphs.end();)
        {
            ScenarioGraph *sg = *i;
            bool graphUsed = false;

            // Iterate over all scenarios
            for (Scenarios::iterator j = scenarios.begin();
                 j != scenarios.end(); j++)
            {
                Scenario *s = *j;

                if (s->getScenarioGraph() == sg)
                {
                    graphUsed = true;
                    break;
                }
            }

            // Is the graph not used?
            if (!graphUsed)
                scenarioGraphs.erase(i++);
            else
                ++i;
        }
    }

    /**
     * hasIsolatedScenarios()
     * The function returns true when the scenarios in the graph are isolated (i.e.
     * each scenario graph is used in exactly one scenario). Otherwise the function
     * returns false.
     */
    bool Graph::hasIsolatedScenarios() const
    {
        set<ScenarioGraph *> usedScenarioGraphs;

        // Iterate over all scenarios
        for (Scenarios::const_iterator i = scenarios.begin();
             i != scenarios.end(); i++)
        {
            Scenario *s = *i;
            ScenarioGraph *sg = s->getScenarioGraph();

            // Has this scenario graph been used in another scenario?
            if (usedScenarioGraphs.find(sg) != usedScenarioGraphs.end())
                return false;

            // Add scenario graph to the set of used graphs
            usedScenarioGraphs.insert(sg);
        }

        return true;
    }

    /**
     * getScenario()
     * The function returns a pointer to the scenario with the supplied name.
     */
    Scenario *Graph::getScenario(const CString &name) const
    {
        // Check regular scenarios
        for (Scenarios::const_iterator i = scenarios.begin();
             i != scenarios.end(); i++)
        {
            if ((*i)->getName() == name)
                return (*i);
        }

        // Check default scenario
        if (getDefaultScenario()->getName() == name)
            return getDefaultScenario();

        throw CException("Graph does not contain a scenario with name '" +
                         name + "'.");
    }


    /**
     * getScenarioGraph()
     * The function returns a pointer to the scenario graph with the supplied name.
     */
    ScenarioGraph *Graph::getScenarioGraph(const CString &name) const
    {
        for (ScenarioGraphs::const_iterator i = scenarioGraphs.begin();
             i != scenarioGraphs.end(); i++)
        {
            if ((*i)->getName() == name)
                return (*i);
        }

        throw CException("Graph does not contain a scenario graph with name '" +
                         name + "'.");
    }



    /**
     * The function replaces all the scenario graph fromG with the scenario graph
     * toG. The function replaces also all references to this scenario graph inside
     * the FSM. The scenario graph fromG will be removed from the graph, but the
     * scenario graph will not be destroyed. The user is responsible for freeing the
     * memory claimed by this object.
     * @param scenario graph fromG
     * @param scenario graph toG
     */
    void Graph::replaceScenarioGraph(ScenarioGraph *fromG, ScenarioGraph *toG)
    {
        // Update all scenarios
        for (Scenarios::iterator i = scenarios.begin(); i != scenarios.end(); i++)
        {
            Scenario *s = *i;

            if (s->getScenarioGraph() == fromG)
                s->setScenarioGraph(toG);
        }

        // Add new scenario graph to the application graph
        this->addScenarioGraph(toG);

        // Remove old scenario graph from the application graph
        this->scenarioGraphs.remove(fromG);
    }

    /**
     * getDefaultScenario()
     * The function returns a pointer to the default scenario.
     */
    Scenario *Graph::getDefaultScenario() const
    {
        return defaultScenario;
    }

    /**
     * createDefaultScenario()
     * Create a default scenario. This scenario is not part of the set of scenarios.
     * It is just used as a NULL object to store default properties.
     */
    void Graph::createDefaultScenario()
    {
        // Create a default scenario
        GraphComponent component = GraphComponent(this, 0, "_default_");
        defaultScenario = new Scenario(component);
    }

    /**
     * The function returns a list with the names of all persistent tokens that
     * are used in the graph.
     * @return list with names of persistent tokens
     */
    const CStrings Graph::getPersistentTokens() const
    {
        CStrings pt;

        // Graph should have at least one scenario graph
        if (scenarioGraphs.size() == 0)
            throw CException("No scenario graphs found.");

        // Create list of persistent tokens from persistent token map of first graph
        PersistentTokenMap mp = scenarioGraphs.front()->getPersistentTokenMap();
        for (PersistentTokenMap::iterator i = mp.begin(); i != mp.end(); i++)
        {
            pt.push_back(i->first);
        }

        // Check that all scenario graphs define the same set of persistent tokens
        for (ScenarioGraphs::const_iterator i = scenarioGraphs.begin();
             i != scenarioGraphs.end(); i++)
        {
            ScenarioGraph *sg = *i;
            PersistentTokenMap mpSg = sg->getPersistentTokenMap();

            // Same size
            if (mpSg.size() != pt.size())
            {
                throw CException("Persistent initial tokens do not match "
                                 "in all scenarios.");
            }

            // All names exist
            for (CStrings::iterator j = pt.begin(); j != pt.end(); j++)
            {
                if (mpSg.find(*j) == mpSg.end())
                {
                    throw CException("Persistent initial tokens do not match "
                                     "in all scenarios.");
                }
            }
        }

        return pt;
    }



    /**
     * The function returns a list with the names of all persistent tokens that
     * are used in the given scenario.
     * @return list with names of persistent tokens
     */
    const CStrings Graph::getPersistentTokensOfScenario(const Scenario &s) const
    {
        ScenarioGraph *sg = s.getScenarioGraph();

        CStrings pt;

        // Create list of persistent tokens from persistent token map of first graph
        PersistentTokenMap mp = sg->getPersistentTokenMap();
        for (PersistentTokenMap::iterator i = mp.begin(); i != mp.end(); i++)
        {
            pt.push_back(i->first);
        }

        return pt;
    }

    /**
     * The function returns a list with the names of all persistent final tokens that
     * are used in the given scenario.
     * @return list with names of persistent tokens
     */
    const CStrings Graph::getPersistentTokensFinalOfScenario(const Scenario &s) const
    {
        ScenarioGraph *sg = s.getScenarioGraph();

        CStrings pt;

        // Create list of persistent tokens from persistent token map of first graph
        PersistentTokenMap mp = sg->getPersistentTokenMapFinal();
        for (PersistentTokenMap::iterator i = mp.begin(); i != mp.end(); i++)
        {
            pt.push_back(i->first);
        }

        return pt;
    }


    /**
     * print ()
     * Print the graph to the supplied output stream.
     */
    ostream &Graph::print(ostream &out)
    {
        out << "Graph (" << getName() << ")" << endl;
        out << "id:        " << getId() << endl;
        out << endl;

        for (Scenarios::iterator i = scenarios.begin(); i != scenarios.end(); i++)
        {
            (*i)->print(out);
        }

        for (ScenarioGraphs::iterator i = scenarioGraphs.begin();
             i != scenarioGraphs.end(); i++)
        {
            (*i)->print(out);
        }

        if (fsm != NULL)
        {
            fsm->print(out);
        }

        out << endl;

        return out;
    }

} // End namespace FSMSADF

