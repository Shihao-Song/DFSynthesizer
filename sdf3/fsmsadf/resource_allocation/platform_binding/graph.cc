/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   graph.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 30, 2006
 *
 *  Function        :   Platform graph
 *
 *  History         :
 *      30-03-06    :   Initial version.
 *
 * $Id: graph.cc,v 1.1 2009-12-23 13:37:22 sander Exp $
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
     * PlatformBinding ()
     * Constructor.
     */
    PlatformBinding::PlatformBinding(GraphComponent c, PlatformGraph *pg,
                                     Graph *ag, bool create)
        :
        GraphComponent(c),
        applicationGraph(ag),
        platformGraph(pg)
    {
        // Create tile and connection binding object for all corresponding elements
        // in the platform graph? (i.e. constructor not called through clone method)
        if (create)
        {
            // Application graph binding constraints
            setGraphBindingConstraints(new GraphBindingConstraints);

            // Create binding object for all tiles
            for (Tiles::iterator i = pg->getTiles().begin();
                 i != pg->getTiles().end(); i++)
            {
                createTileBinding(*i);
            }

            // Create binding object for all connections
            for (Connections::iterator i = pg->getConnections().begin();
                 i != pg->getConnections().end(); i++)
            {
                createConnectionBinding(*i);
            }
        }
    }

    /**
     * ~PlatformBinding ()
     * Destructor.
     */
    PlatformBinding::~PlatformBinding()
    {
        // Tile bindings
        for (TileBindings::iterator i = tileBindings.begin();
             i != tileBindings.end(); i++)
        {
            delete *i;
        }

        // Connection bindings
        for (ConnectionBindings::iterator i = connectionBindings.begin();
             i != connectionBindings.end(); i++)
        {
            delete *i;
        }

        // Application graph binding constraints
        delete graphBindingConstraints;
    }

    /**
     * clone()
     * Create a clone of this platform binding.
     */
    PlatformBinding *PlatformBinding::clone(GraphComponent c)
    {
        PlatformBinding *pb = new PlatformBinding(c, getPlatformGraph(),
                getApplicationGraph(), false);

        // Tile bindings
        for (TileBindings::iterator i = tileBindings.begin();
             i != tileBindings.end(); i++)
        {
            TileBinding *tb = *i;
            TileBinding *tbNew = tb->clone(GraphComponent(pb, tb->getId(),
                                           tb->getName()));
            pb->getTileBindings().push_back(tbNew);
        }

        // Connection bindings
        for (ConnectionBindings::iterator i = connectionBindings.begin();
             i != connectionBindings.end(); i++)
        {
            ConnectionBinding *cb = *i;
            ConnectionBinding *cbNew = cb->clone(GraphComponent(pb, cb->getId(),
                                                 cb->getName()));
            pb->getConnectionBindings().push_back(cbNew);
        }

        // Application graph binding constraints
        pb->setGraphBindingConstraints(getGraphBindingConstraints()->clone(
                                           pb->getApplicationGraph()));

        return pb;
    }

    /**
     * constructFromXML()
     * This function creates the binding as specified by the mappingNode.
     */
    void PlatformBinding::constructFromXML(const CNodePtr mappingNode)
    {
        // Name
        if (CHasAttribute(mappingNode, "name"))
            setName(CGetAttribute(mappingNode, "name"));

        // Iterate over all scenario nodes in the node
        for (CNode *sn = CGetChildNode(mappingNode, "scenario");
             sn != NULL; sn = CNextNode(mappingNode, "scenario"))
        {
            // Find scenario in the application graph
            if (!CHasAttribute(sn, "name"))
                throw CException("Scenario has no name identifier.");
            Scenario *s = getApplicationGraph()->getScenario(CGetAttribute(sn,
                          "name"));

            // Iterate over all child nodes in the scenario
            for (CNode *n = CGetChildNode(sn); n != NULL; n = CNextNode(n))
            {
                // Tile?
                if (CIsNode(n, "tile"))
                {
                    // Name?
                    if (!CHasAttribute(n, "name"))
                        throw CException("Tile has no name identifier.");

                    // Locate tile
                    Tile *t = getPlatformGraph()->getTile(CGetAttribute(n, "name"));

                    // Set resource usage
                    getTileBinding(t)->constructFromXML(s, n);
                }

                // Connection?
                if (CIsNode(n, "connection"))
                {
                    // Name?
                    if (!CHasAttribute(n, "name"))
                        throw CException("Connection has no name identifier.");

                    // Locate connection
                    Connection *c = getPlatformGraph()->getConnection(CGetAttribute(
                                        n, "name"));

                    // Set resource usage
                    getConnectionBinding(c)->constructFromXML(s, n);
                }

                // Constraint
                if (CIsNode(n, "constraints"))
                {
                    getGraphBindingConstraints()->constructFromXML(s, n);
                }
            }
        }

        // Resource usage
        if (CHasChildNode(mappingNode, "resourceUsage"))
        {
            constructResourceUsageFromXML(CGetChildNode(mappingNode,
                                          "resourceUsage"));
        }
    }

    /**
     * constructFromXML()
     * This function sets the resource usage of the platform binding.
     */
    void PlatformBinding::constructResourceUsageFromXML(
        const CNodePtr resourceUsageNode)
    {
        // Iterate over all child nodes in the node
        for (CNode *n = CGetChildNode(resourceUsageNode);
             n != NULL; n = CNextNode(n))
        {
            // Tile?
            if (CIsNode(n, "tile"))
            {
                // Name?
                if (!CHasAttribute(n, "name"))
                    throw CException("Tile has no name identifier.");

                // Locate tile
                Tile *t = getPlatformGraph()->getTile(CGetAttribute(n, "name"));

                // Set resource usage
                getTileBinding(t)->constructResourceUsageFromXML(n);
            }

            // Connection?
            if (CIsNode(n, "connection"))
            {
                // Name?
                if (!CHasAttribute(n, "name"))
                    throw CException("Connection has no name identifier.");

                // Locate connection
                Connection *c = getPlatformGraph()->getConnection(CGetAttribute(n,
                                "name"));

                // Set resource usage
                getConnectionBinding(c)->constructResourceUsageFromXML(n);
            }
        }
    }

    /**
     * convertToXML()
     * This function converts the platform binding to an XML object.
     */
    void PlatformBinding::convertToXML(const CNodePtr mappingNode)
    {
        // Name
        CAddAttribute(mappingNode, "name", getName());

        // Application graph
        CAddAttribute(mappingNode, "appGraph", applicationGraph->getName());

        // Platform graph
        CAddAttribute(mappingNode, "archGraph", platformGraph->getName());

        // Iterate over the scenarios
        for (Scenarios::iterator i = getApplicationGraph()->getScenarios().begin();
             i != getApplicationGraph()->getScenarios().end(); i++)
        {
            Scenario *s = *i;

            // Create node for this scenario
            CNode *scenarioNode = CAddNode(mappingNode, "scenario");
            CAddAttribute(scenarioNode, "name", s->getName());

            // Iterate over all tile bindings
            for (TileBindings::iterator j = tileBindings.begin();
                 j != tileBindings.end(); j++)
            {
                (*j)->convertToXML(s, CAddNode(scenarioNode, "tile"));
            }

            // Iterate over all connection bindings
            for (ConnectionBindings::iterator j = connectionBindings.begin();
                 j != connectionBindings.end(); j++)
            {
                (*j)->convertToXML(s, CAddNode(scenarioNode, "connection"));
            }

            // Constraints
            getGraphBindingConstraints()->convertToXML(s, CAddNode(scenarioNode,
                    "constraints"));
        }

        // Resource usage
        convertResourceUsageToXML(CAddNode(mappingNode, "resourceUsage"));
    }

    /**
     * convertResourceUsageToXML()
     * This function converts the resource usage of this platform binding
     * to an XML object.
     */
    void PlatformBinding::convertResourceUsageToXML(
        const CNodePtr resourceUsageNode)
    {
        // Iterate over all tile bindings
        for (TileBindings::iterator i = tileBindings.begin();
             i != tileBindings.end(); i++)
        {
            (*i)->convertResourceUsageToXML(CAddNode(resourceUsageNode, "tile"));
        }

        // Iterate over all connection bindings
        for (ConnectionBindings::iterator i = connectionBindings.begin();
             i != connectionBindings.end(); i++)
        {
            (*i)->convertResourceUsageToXML(CAddNode(resourceUsageNode,
                                            "connection"));
        }
    }

    /**
     * isInitialBinding()
     * The function returns true if this platform binding is the initial
     * binding. The name of a default binding has the value "initial".
     */
    bool PlatformBinding::isInitialBinding() const
    {
        if (getName() == "initial")
            return true;

        return false;
    }

    /**
     * getTileBinding()
     * The function returns a pointer to the tile binding object of tile t.
     */
    TileBinding *PlatformBinding::getTileBinding(const Tile *t)
    {
        for (TileBindings::iterator i = tileBindings.begin();
             i != tileBindings.end(); i++)
        {
            if ((*i)->getTile() == t)
                return *i;
        }

        throw CException("Tile '" + t->getName() + "' has no binding.");
    }

    /**
     * getConnectionBinding()
     * The function returns a pointer to the connection binding object of
     * connection c.
     */
    ConnectionBinding *PlatformBinding::getConnectionBinding(const Connection *c)
    {
        for (ConnectionBindings::iterator i = connectionBindings.begin();
             i != connectionBindings.end(); i++)
        {
            if ((*i)->getConnection() == c)
                return *i;
        }

        throw CException("Connection '" + c->getName() + "' has no binding.");
    }

    /**
     * The function returns a pointer to the processor binding object associated
     * with the actor a in scenario s. A NULL pointer is returned when the actor is
     * not bound to any processor in the given scenario.
     * @param scenario s
     * @param actor a
     * @return pointer to processor binding or NULL in case actor is not bound.
     */
    ProcessorBinding *PlatformBinding::getProcessorBindingOfActorInScenario(
        Scenario *s, Actor *a)
    {
        // Iterate over all tiles
        for (TileBindings::iterator i = getTileBindings().begin();
             i != getTileBindings().end(); i++)
        {
            TileBinding *tb = *i;

            // Iterate over all processors
            for (ProcessorBindings::iterator j = tb->getProcessorBindings().begin();
                 j != tb->getProcessorBindings().end(); j++)
            {
                ProcessorBinding *pb = *j;

                // Actor bound in scenario s to this processor
                if (pb->hasActorBinding(s, a))
                    return pb;
            }
        }

        return NULL;
    }

    /**
     * createTileBinding()
     * The function creates a tile binding object for tile t and adds it to the
     * list of tile bindings associated with this platform binding.
     */
    TileBinding *PlatformBinding::createTileBinding(Tile *t)
    {
        TileBinding *tb;

        tb = new TileBinding(GraphComponent(this, t->getId(), t->getName()), t);
        tileBindings.push_back(tb);

        return tb;
    }

    /**
     * createConnectionBinding()
     * The function creates a connection binding object for tile t and adds it to
     * the list of connection bindings associated with this platform binding.
     */
    ConnectionBinding *PlatformBinding::createConnectionBinding(Connection *c)
    {
        ConnectionBinding *cb;

        cb = new ConnectionBinding(GraphComponent(this, c->getId(), c->getName()), c);
        connectionBindings.push_back(cb);

        return cb;
    }

} // End namespace FSMSADF
