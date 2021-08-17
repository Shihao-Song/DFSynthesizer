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
     * PlatformGraph ()
     * Constructor.
     */
    PlatformGraph::PlatformGraph(GraphComponent c)
        :
        GraphComponent(c)
    {
    }

    /**
     * PlatformGraph ()
     * Constructor.
     */
    PlatformGraph::PlatformGraph()
        :
        GraphComponent(NULL, 0)
    {
    }

    /**
     * ~PlatformGraph ()
     * Destructor.
     */
    PlatformGraph::~PlatformGraph()
    {
        // Tiles
        for (Tiles::iterator i = tiles.begin(); i != tiles.end(); i++)
        {
            delete(*i);
        }

        // Connections
        for (Connections::iterator i = connections.begin();
             i != connections.end(); i++)
        {
            delete(*i);
        }
    }

    /**
     * create()
     * The function creates a new platform graph object.
     */
    PlatformGraph *PlatformGraph::create(GraphComponent c) const
    {
        return new PlatformGraph(c);
    }

    /**
     * createCopy()
     * The function creates a copy of the platform graph. Note that copying does
     * not relocate connections. The clone function should be used for this purpose.
     */
    PlatformGraph *PlatformGraph::createCopy(GraphComponent c)
    {
        PlatformGraph *pg = create(c);

        // Name
        pg->setName(getName());

        // Tiles
        for (Tiles::iterator i = tiles.begin(); i != tiles.end(); i++)
        {
            pg->addTile((*i)->createCopy(GraphComponent(pg, getId(), getName())));
        }

        // Connections
        for (Connections::iterator i = connections.begin();
             i != connections.end(); i++)
        {
            pg->addConnection((*i)->createCopy(GraphComponent(pg, getId(),
                                               getName())));
        }

        return pg;
    }

    /**
     * clone()
     * The function creates a clone of this platfomr graph.
     */
    PlatformGraph *PlatformGraph::clone(GraphComponent c)
    {
        PlatformGraph *pg = create(c);

        // Name
        pg->setName(getName());

        // Tiles
        for (Tiles::iterator i = tiles.begin(); i != tiles.end(); i++)
        {
            pg->addTile((*i)->clone(GraphComponent(pg, getId(), getName())));
        }

        // Connections
        for (Connections::iterator i = connections.begin();
             i != connections.end(); i++)
        {
            pg->addConnection((*i)->clone(GraphComponent(pg, getId(), getName())));
        }

        return pg;
    }

    void PlatformGraph::constructFromXML(const CNodePtr platformGraphNode)
    {
        // Name
        if (!CHasAttribute(platformGraphNode, "name"))
            throw CException("Invalid graph, missing platform graph name.");
        setName(CGetAttribute(platformGraphNode, "name"));

        // Iterate over all child nodes of the tile
        for (CNode *n = CGetChildNode(platformGraphNode);
             n != NULL; n = CNextNode(n))
        {
            // Tile
            if (CIsNode(n, "tile"))
            {
                Tile *t = createTile("(no name)");
                t->constructFromXML(n);
            }

            // Connection
            if (CIsNode(n, "connection"))
            {
                Connection *c = createConnection("(no name)");
                c->constructFromXML(n);
            }
        }
    }

    /**
     * convertToXML()
     * The function adds all elements of this graph to the supplied node.
     */
    void PlatformGraph::convertToXML(const CNodePtr platformGraphNode)
    {
        // Name
        CAddAttribute(platformGraphNode, "name", getName());

        // Tiles
        for (Tiles::iterator i = tiles.begin(); i != tiles.end(); i++)
        {
            (*i)->convertToXML(CAddNode(platformGraphNode, "tile"));
        }

        // Connections
        for (Connections::iterator i = connections.begin();
             i != connections.end(); i++)
        {
            (*i)->convertToXML(CAddNode(platformGraphNode, "connection"));
        }
    }

    /**
     * getTile()
     * The function returns a pointer to a tile with the supplied id. When
     * no such tile exists, an exception is thrown.
     */
    Tile *PlatformGraph::getTile(const CId id)
    {
        for (Tiles::iterator i = tiles.begin(); i != tiles.end(); i++)
        {
            if ((*i)->getId() == id)
                return *i;
        }

        throw CException("Platfrom graph does not contain a connection with name '"
                         + CString(id) + "'.");
    }

    /**
     * getTile()
     * The function returns a pointer to a tile with the supplied name. When
     * no such tile exists, an exception is thrown.
     */
    Tile *PlatformGraph::getTile(const CString &name)
    {
        for (Tiles::iterator i = tiles.begin(); i != tiles.end(); i++)
        {
            if ((*i)->getName() == name)
                return *i;
        }

        throw CException("Platfrom graph does not contain a connection with name '"
                         + name + "'.");
    }

    /**
     * createTile()
     * The function creates a new tile and adds it to the graph.
     */
    Tile *PlatformGraph::createTile(const CString &name)
    {
        Tile *t = new Tile(GraphComponent(this, tiles.size(), name));
        addTile(t);
        return t;
    }

    /**
     * addTile()
     * The function adds tile t to the graph.
     */
    void PlatformGraph::addTile(Tile *t)
    {
        tiles.push_back(t);
    }

    /**
     * removeTile()
     * The function removes tile t from the graph.
     */
    void PlatformGraph::removeTile(Tile *t)
    {
        for (Tiles::iterator i = tiles.begin(); i != tiles.end(); i++)
        {
            if ((*i)->getName() == t->getName())
            {
                tiles.erase(i);
                break;
            }
        }
    }

    /**
     * getConnection()
     * The function returns a pointer to a connection with the supplied id. When
     * no such connection exists, an exception is thrown.
     */
    Connection *PlatformGraph::getConnection(const CId id)
    {
        for (Connections::iterator i = connections.begin();
             i != connections.end(); i++)
        {
            if ((*i)->getId() == id)
                return *i;
        }

        throw CException("Platfrom graph does not contain a connection with id '"
                         + CString(id) + "'.");
    }

    /**
     * getConnection()
     * The function returns a pointer to a connection with the supplied name. When
     * no such connection exists, an exception is thrown.
     */
    Connection *PlatformGraph::getConnection(const CString &name)
    {
        for (Connections::iterator i = connections.begin();
             i != connections.end(); i++)
        {
            if ((*i)->getName() == name)
                return *i;
        }

        throw CException("Platfrom graph does not contain a connection with name '"
                         + name + "'.");
    }

    /**
     * createConnection()
     * The function creates a new connection and adds it to the graph.
     */
    Connection *PlatformGraph::createConnection(const CString &name)
    {
        Connection *c = new Connection(GraphComponent(this, connections.size(),
                                       name));
        addConnection(c);

        return c;
    }

    /**
     * addConnection()
     * The function adds the connection c to the graph.
     */
    void PlatformGraph::addConnection(Connection *c)
    {
        connections.push_back(c);
    }

    /**
     * removeConnection()
     * The function removes connection c from the graph.
     */
    void PlatformGraph::removeConnection(Connection *c)
    {
        for (Connections::iterator i = connections.begin();
             i != connections.end(); i++)
        {
            if ((*i)->getName() == c->getName())
            {
                connections.erase(i);
                break;
            }
        }
    }

} // End namespace FSMSADF
