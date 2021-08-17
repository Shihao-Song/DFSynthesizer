/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   networkinterface.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 12, 2009
 *
 *  Function        :   Tile network interface.
 *
 *  History         :
 *      12-05-09    :   Initial version.
 *
 * $Id: networkinterface.cc,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#include "networkinterface.h"
#include "graph.h"

namespace FSMSADF
{

    /**
     * NetworkInterfaceBinding()
     * Constructor.
     */
    NetworkInterfaceBinding::NetworkInterfaceBinding(GraphComponent c,
            NetworkInterface *n)
        :
        GraphComponent(c),
        networkInterface(n),
        nrInConnectionsUsedForOtherGraphs(0),
        nrOutConnectionsUsedForOtherGraphs(0),
        inBandwidthUsedForOtherGraphs(0),
        outBandwidthUsedForOtherGraphs(0)
    {
    }

    /**
     * ~NetworkInterfaceBinding()
     * Destructor.
     */
    NetworkInterfaceBinding::~NetworkInterfaceBinding()
    {
    }

    /**
     * clone()
     * Create a cloned copy of this network interface binding.
     */
    NetworkInterfaceBinding *NetworkInterfaceBinding::clone(GraphComponent c)
    {
        NetworkInterfaceBinding *nb = new NetworkInterfaceBinding(c,
                getNetworkInterface());

        // Channels
        nb->setInChannelBindings(getInChannelBindings());
        nb->setOutChannelBindings(getOutChannelBindings());

        // Initial resource occupancy
        nb->setNrInConnectionsUsedForOtherGraphs(
            getNrInConnectionsUsedForOtherGraphs());
        nb->setNrOutConnectionsUsedForOtherGraphs(
            getNrOutConnectionsUsedForOtherGraphs());
        nb->setInBandwidthUsedForOtherGraphs(getInBandwidthUsedForOtherGraphs());
        nb->setOutBandwidthUsedForOtherGraphs(getOutBandwidthUsedForOtherGraphs());

        return nb;
    }

    /**
     * constructFromXML()
     * This function creates the network interface binding as specified by the
     * connectionNode.
     */
    void NetworkInterfaceBinding::constructFromXML(Scenario *s,
            const CNodePtr networkInterfaceNode)
    {
        // Application graph
        ScenarioGraph *sg = s->getScenarioGraph();

        // Iterate over the list of channels bound to this network interface
        for (CNode *n = CGetChildNode(networkInterfaceNode, "channel");
             n != NULL; n = CNextNode(networkInterfaceNode, "channel"))
        {
            // Name specified for the channel?
            if (!CHasAttribute(n, "name"))
                throw CException("Mapped channel has no name identifier.");

            // Find channel in scenario graph
            Channel *c = sg->getChannel(CGetAttribute(n, "name"));

            // Is this channel bound to a network interface input?
            if (CHasAttribute(n, "in") && CGetAttribute(n, "in") == "true")
            {
                // Create binding of channel to network interface
                addInChannelBinding(s, c);
            }

            // Is this channel bound to a network interface output?
            if (CHasAttribute(n, "out") && CGetAttribute(n, "out") == "true")
            {
                // Create binding of channel to network interface
                addOutChannelBinding(s, c);
            }
        }
    }

    /**
     * constructFromXML()
     * This function sets the resource usgae of the network interface.
     */
    void NetworkInterfaceBinding::constructResourceUsageFromXML(
        const CNodePtr networkInterfaceNode)
    {
        // Input connections
        if (!CHasAttribute(networkInterfaceNode, "nrInConnections"))
            throw CException("Network interface usage has no nrInConnections.");
        setNrInConnectionsUsedForOtherGraphs(CGetAttribute(networkInterfaceNode,
                                             "nrInConnections"));

        // Input bandwidth
        if (!CHasAttribute(networkInterfaceNode, "inBandwidth"))
            throw CException("Network interface usage has no inBandwidth.");
        setInBandwidthUsedForOtherGraphs(CGetAttribute(networkInterfaceNode,
                                         "inBandwidth"));

        // Output connections
        if (!CHasAttribute(networkInterfaceNode, "nrOutConnections"))
            throw CException("Network interface usage has no nrOutConnections.");
        setNrOutConnectionsUsedForOtherGraphs(CGetAttribute(networkInterfaceNode,
                                              "nrOutConnections"));

        // Output bandwidth
        if (!CHasAttribute(networkInterfaceNode, "outBandwidth"))
            throw CException("Network interface usage has no outBandwidth.");
        setOutBandwidthUsedForOtherGraphs(CGetAttribute(networkInterfaceNode,
                                          "outBandwidth"));
    }

    /**
     * convertToXML()
     * This function converts the network interface binding to an XML object.
     */
    void NetworkInterfaceBinding::convertToXML(Scenario *s,
            const CNodePtr networkInterfaceNode)
    {
        map<Scenario *, Channels>::iterator scenInChannelIter, scenOutChannelIter;

        // Name
        CAddAttribute(networkInterfaceNode, "name", getName());

        // Input channel binding exists for this scenario?
        scenInChannelIter = inChannelBindings.find(s);
        if (scenInChannelIter != inChannelBindings.end())
        {
            // List of all channels bound to this network interface
            for (Channels::iterator i = scenInChannelIter->second.begin();
                 i != scenInChannelIter->second.end(); i++)
            {
                CNode *n = CAddNode(networkInterfaceNode, "channel");
                CAddAttribute(n, "name", (*i)->getName());
                CAddAttribute(n, "in", "true");
            }
        }

        // Output channel binding exists for this scenario?
        scenOutChannelIter = outChannelBindings.find(s);
        if (scenOutChannelIter != outChannelBindings.end())
        {
            // List of all channels bound to this network interface
            for (Channels::iterator i = scenOutChannelIter->second.begin();
                 i != scenOutChannelIter->second.end(); i++)
            {
                CNode *n = CAddNode(networkInterfaceNode, "channel");
                CAddAttribute(n, "name", (*i)->getName());
                CAddAttribute(n, "out", "true");
            }
        }
    }

    /**
     * convertToXML()
     * This function converts the resource usage of this network interface binding
     * to an XML object.
     */
    void NetworkInterfaceBinding::convertResourceUsageToXML(
        const CNodePtr networkInterfaceNode)
    {
        // Name
        CAddAttribute(networkInterfaceNode, "name", getName());

        // Input connections
        CAddAttribute(networkInterfaceNode, "nrInConnections",
                      getNrInConnectionsUsedForGraph()
                      + getNrInConnectionsUsedForOtherGraphs());

        // Input bandwidth
        CAddAttribute(networkInterfaceNode, "inBandwidth",
                      getInBandwidthUsedForGraph()
                      + getInBandwidthUsedForOtherGraphs());

        // Output connections
        CAddAttribute(networkInterfaceNode, "nrOutConnections",
                      getNrOutConnectionsUsedForGraph()
                      + getNrOutConnectionsUsedForOtherGraphs());

        // Output bandwidth
        CAddAttribute(networkInterfaceNode, "outBandwidth",
                      getOutBandwidthUsedForGraph()
                      + getOutBandwidthUsedForOtherGraphs());
    }

    /**
     * getGraphBindingConstraints()
     * Get the graph binding constraints associated with this binding.
     */
    GraphBindingConstraints *NetworkInterfaceBinding::getGraphBindingConstraints()
    const
    {
        return getTileBinding()->getPlatformBinding()->getGraphBindingConstraints();
    }

    /**
     * addInChannelBinding()
     * The function binds channel c in scenario s to this interface. The function
     * returns true on success. Otherwise it returns false.
     */
    bool NetworkInterfaceBinding::addInChannelBinding(Scenario *s, Channel *c)
    {
        // Binding already exists?
        if (hasInChannelBinding(s, c))
            return true;

        // Sufficient resources available?
        if (hasResourcesInChannelBinding(s, c))
        {
            inChannelBindings[s].push_back(c);
            return true;
        }

        return false;
    }

    /**
     * removeInChannelBinding()
     * The function removes the binding of channel c in scenario s to this
     * network interface.
     */
    void NetworkInterfaceBinding::removeInChannelBinding(Scenario *s,
            const Channel *c)
    {
        for (Channels::iterator i = inChannelBindings[s].begin();
             i != inChannelBindings[s].end(); i++)
        {
            if (*i == c)
            {
                inChannelBindings[s].erase(i);
                break;
            }
        }
    }

    /**
     * hasInChannelBinding()
     * The function returns true when a binding of channel c in scenario s to this
     * network interface exists. Otherwise it returns false.
     */
    bool NetworkInterfaceBinding::hasInChannelBinding(Scenario *s,
            const Channel *c) const
    {
        map<Scenario *, Channels>::const_iterator scenIter = inChannelBindings.find(s);

        if (scenIter == inChannelBindings.end())
            return false;

        for (Channels::const_iterator i = scenIter->second.begin();
             i != scenIter->second.end(); i++)
        {
            if (*i == c)
                return true;
        }

        return false;
    }

    /**
     * getAvailableNrInConnections()
     * The function returns the number of available input connections on this
     * network interface in scenario s.
     */
    Size NetworkInterfaceBinding::getAvailableNrInConnections(Scenario *s) const
    {
        return (networkInterface->getMaxNrInConnections()
                - nrInConnectionsUsedForOtherGraphs
                - getAllocatedNrInConnections(s));
    }

    /**
     * getAllocatedNrInConnections()
     * The function returns the number of input connections allocated to the
     * application graph on this network interface in scenario s.
     */
    Size NetworkInterfaceBinding::getAllocatedNrInConnections(Scenario *s) const
    {
        map<Scenario *, Channels>::const_iterator scenIter = inChannelBindings.find(s);

        if (scenIter != inChannelBindings.end())
            return scenIter->second.size();

        return 0;
    }

    /**
     * getAvailableBandwidthInConnections()
     * The function returns the amount of input bandwidth which is available on
     * this network interface in scenario s.
     */
    Bandwidth NetworkInterfaceBinding::getAvailableBandwidthInConnections(
        Scenario *s) const
    {
        return (networkInterface->getInBandwidth()
                - inBandwidthUsedForOtherGraphs
                - getAllocatedBandwidthInConnections(s));
    }

    /**
     * getAllocatedBandwidthInConnections()
     * The function returns the amount of input bandwidth which is allocated to the
     * application graph on this network interface in scenario s.
     */
    Bandwidth NetworkInterfaceBinding::getAllocatedBandwidthInConnections(
        Scenario *s) const
    {
        ChannelBindingConstraints *cb;
        map<Scenario *, Channels>::const_iterator scenIter = inChannelBindings.find(s);
        Bandwidth b = 0;

        if (scenIter == inChannelBindings.end())
            return 0;

        for (Channels::const_iterator i =  scenIter->second.begin();
             i != scenIter->second.end(); i++)
        {
            cb = getGraphBindingConstraints()->getConstraintsOfScenario(s)
                 ->getConstraintsOfChannel(*i);
            b += cb->getBandwidth();
        }

        return b;
    }

    /**
     * hasResourcesInChannelBinding()
     * The function checks whether there are sufficient resources available
     * to bind channel c to this network interface.
     */
    bool NetworkInterfaceBinding::hasResourcesInChannelBinding(Scenario *s,
            const Channel *c) const
    {
        ChannelBindingConstraints *cb;
        cb = getGraphBindingConstraints()->getConstraintsOfScenario(s)
             ->getConstraintsOfChannel(c);

        if (getAvailableBandwidthInConnections(s) > cb->getBandwidth()
            && getAvailableNrInConnections(s) > 1)
        {
            return true;
        }

        return false;
    }

    /**
     * addOutChannelBinding()
     * The function binds channel c in scenario s to this interface. The function
     * returns true on success. Otherwise it returns false.
     */
    bool NetworkInterfaceBinding::addOutChannelBinding(Scenario *s, Channel *c)
    {
        // Binding already exists?
        if (hasOutChannelBinding(s, c))
            return true;

        // Sufficient resources available?
        if (hasResourcesOutChannelBinding(s, c))
        {
            outChannelBindings[s].push_back(c);
            return true;
        }

        return false;
    }

    /**
     * removeOutChannelBinding()
     * The function removes the binding of channel c in scenario s to this
     * network interface.
     */
    void NetworkInterfaceBinding::removeOutChannelBinding(Scenario *s,
            const Channel *c)
    {
        for (Channels::iterator i = outChannelBindings[s].begin();
             i != outChannelBindings[s].end(); i++)
        {
            if (*i == c)
            {
                outChannelBindings[s].erase(i);
                break;
            }
        }
    }

    /**
     * hasOutChannelBinding()
     * The function returns true when a binding of channel c in scenario s to this
     * network interface exists. Otherwise it returns false.
     */
    bool NetworkInterfaceBinding::hasOutChannelBinding(Scenario *s,
            const Channel *c) const
    {
        map<Scenario *, Channels>::const_iterator scenIter = outChannelBindings.find(s);

        if (scenIter == outChannelBindings.end())
            return false;

        for (Channels::const_iterator i = scenIter->second.begin();
             i != scenIter->second.end(); i++)
        {
            if (*i == c)
                return true;
        }

        return false;
    }

    /**
     * getAvailableNrOutConnections()
     * The function returns the number of available output connections on this
     * network interface in scenario s.
     */
    Size NetworkInterfaceBinding::getAvailableNrOutConnections(Scenario *s) const
    {
        return (networkInterface->getMaxNrOutConnections()
                - nrOutConnectionsUsedForOtherGraphs
                - getAllocatedNrOutConnections(s));
    }

    /**
     * getAllocatedNrOutConnections()
     * The function returns the number of output connections allocated to the
     * application graph on this network interface in scenario s.
     */
    Size NetworkInterfaceBinding::getAllocatedNrOutConnections(Scenario *s) const
    {
        map<Scenario *, Channels>::const_iterator scenIter = outChannelBindings.find(s);

        if (scenIter != outChannelBindings.end())
            return scenIter->second.size();

        return 0;
    }

    /**
     * getAvailableBandwidthOutConnections()
     * The function returns the amount of output bandwidth which is available on
     * this network interface in scenario s.
     */
    Bandwidth NetworkInterfaceBinding::getAvailableBandwidthOutConnections(
        Scenario *s) const
    {
        return (networkInterface->getOutBandwidth()
                - outBandwidthUsedForOtherGraphs
                - getAllocatedBandwidthOutConnections(s));
    }

    /**
     * getAllocatedBandwidthOutConnections()
     * The function returns the amount of output bandwidth which is allocated to the
     * application graph on this network interface in scenario s.
     */
    Bandwidth NetworkInterfaceBinding::getAllocatedBandwidthOutConnections(
        Scenario *s) const
    {
        ChannelBindingConstraints *cb;
        map<Scenario *, Channels>::const_iterator scenIter = outChannelBindings.find(s);
        Bandwidth b = 0;

        if (scenIter == outChannelBindings.end())
            return 0;

        for (Channels::const_iterator i =  scenIter->second.begin();
             i != scenIter->second.end(); i++)
        {
            cb = getGraphBindingConstraints()->getConstraintsOfScenario(s)
                 ->getConstraintsOfChannel(*i);
            b += cb->getBandwidth();
        }

        return b;
    }

    /**
     * hasResourcesOutChannelBinding()
     * The function checks whether there are sufficient resources available
     * to bind channel c to this network interface.
     */
    bool NetworkInterfaceBinding::hasResourcesOutChannelBinding(Scenario *s,
            const Channel *c) const
    {
        ChannelBindingConstraints *cb;
        cb = getGraphBindingConstraints()->getConstraintsOfScenario(s)
             ->getConstraintsOfChannel(c);

        if (getAvailableBandwidthOutConnections(s) > cb->getBandwidth()
            && getAvailableNrOutConnections(s) > 1)
        {
            return true;
        }

        return false;
    }

    /**
     * getNrInConnectionsUsedForGraph()
     * The function returns the maximal amount of connections which has been
     * allocated to the application in any of its scenarios.
     */
    Size NetworkInterfaceBinding::getNrInConnectionsUsedForGraph() const
    {
        Size n = 0;
        Graph *g = getTileBinding()->getPlatformBinding()->getApplicationGraph();

        for (Scenarios::iterator i = g->getScenarios().begin();
             i != g->getScenarios().end(); i++)
        {
            n = MAX(n, getAllocatedNrInConnections(*i));
        }

        return n;
    }

    /**
     * getInBandwidthUsedForGraph()
     * The function returns the maximal amount of bandwidth which has been allocated
     * to the application in any of its scenarios.
     */
    Bandwidth NetworkInterfaceBinding::getInBandwidthUsedForGraph() const
    {
        Bandwidth n = 0;
        Graph *g = getTileBinding()->getPlatformBinding()->getApplicationGraph();

        for (Scenarios::iterator i = g->getScenarios().begin();
             i != g->getScenarios().end(); i++)
        {
            n = MAX(n, getAllocatedBandwidthInConnections(*i));
        }

        return n;
    }

    /**
     * getNrOutConnectionsUsedForGraph()
     * The function returns the maximal amount of connections which has been
     * allocated to the application in any of its scenarios.
     */
    Size NetworkInterfaceBinding::getNrOutConnectionsUsedForGraph() const
    {
        Size n = 0;
        Graph *g = getTileBinding()->getPlatformBinding()->getApplicationGraph();

        for (Scenarios::iterator i = g->getScenarios().begin();
             i != g->getScenarios().end(); i++)
        {
            n = MAX(n, getAllocatedNrOutConnections(*i));
        }

        return n;
    }

    /**
     * getOutBandwidthUsedForGraph()
     * The function returns the maximal amount of bandwidth which has been allocated
     * to the application in any of its scenarios.
     */
    Bandwidth NetworkInterfaceBinding::getOutBandwidthUsedForGraph() const
    {
        Bandwidth n = 0;
        Graph *g = getTileBinding()->getPlatformBinding()->getApplicationGraph();

        for (Scenarios::iterator i = g->getScenarios().begin();
             i != g->getScenarios().end(); i++)
        {
            n = MAX(n, getAllocatedBandwidthOutConnections(*i));
        }

        return n;
    }

} // End namespace

