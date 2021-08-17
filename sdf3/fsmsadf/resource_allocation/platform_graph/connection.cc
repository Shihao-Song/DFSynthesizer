/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   connection.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 12, 2009
 *
 *  Function        :   Connection between two network interface.
 *
 *  History         :
 *      12-05-09    :   Initial version.
 *
 * $Id: connection.cc,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#include "connection.h"
#include "graph.h"

namespace FSMSADF
{

    /**
     * Connection ()
     * Constructor.
     */
    Connection::Connection(GraphComponent c)
        :
        GraphComponent(c),
        srcNetworkInterface(NULL),
        dstNetworkInterface(NULL)
    {
    }

    /**
     * ~Connection ()
     * Destructor.
     */
    Connection::~Connection()
    {
    }

    /**
     * create()
     * The function returns a new Connection object.
     */
    Connection *Connection::create(GraphComponent c) const
    {
        return new Connection(c);
    }

    /**
     * createCopy()
     * The function returns a copy of this connection object. Note that the
     * network interfaces are not relocated. Use the clone function to enforce
     * relocation.
     */
    Connection *Connection::createCopy(GraphComponent c)
    {
        Connection *co = create(c);

        // Properties
        co->setName(getName());
        co->setLatency(getLatency());
        co->srcNetworkInterface = getSrcNetworkInterface();
        co->dstNetworkInterface = getDstNetworkInterface();

        return co;
    }

    /**
     * clone()
     * Create a cloned copy of this connection. Cloning will create a new
     * connection between the source and destination network interface.
     */
    Connection *Connection::clone(GraphComponent c)
    {
        NetworkInterface *n;
        PlatformGraph *pg;
        Tile *t;

        // Create a copy of the connection
        Connection *co = createCopy(c);

        // Get the platform graph associated with the new connection
        pg = co->getGraph();

        // Relocate source network interface
        t = pg->getTile(getSrcNetworkInterface()->getTile()->getName());
        n = t->getNetworkInterface(getSrcNetworkInterface()->getName());
        co->connectSrc(n);

        // Relocate destination network interface
        t = pg->getTile(getDstNetworkInterface()->getTile()->getName());
        n = t->getNetworkInterface(getDstNetworkInterface()->getName());
        co->connectDst(n);

        return co;
    }

    /**
     * constructFromXML()
     * The function sets the properties of this connection using the supplied
     * XML node.
     */
    void Connection::constructFromXML(const CNodePtr connectionNode)
    {
        NetworkInterface *n;
        Tile *t;

        // Name
        if (!CHasAttribute(connectionNode, "name"))
            throw CException("Invalid graph, missing connection name.");
        setName(CGetAttribute(connectionNode, "name"));

        // Source network interface
        if (!CHasAttribute(connectionNode, "srcTile"))
            throw CException("Invalid graph, missing connection src tile.");
        if (!CHasAttribute(connectionNode, "srcNetworkInterface"))
            throw CException("Invalid graph, missing connection src ni.");
        t = getGraph()->getTile(CGetAttribute(connectionNode, "srcTile"));
        n = t->getNetworkInterface(CGetAttribute(connectionNode,
                                   "srcNetworkInterface"));
        connectSrc(n);

        // Destination network interface
        if (!CHasAttribute(connectionNode, "dstTile"))
            throw CException("Invalid graph, missing connection dst tile.");
        if (!CHasAttribute(connectionNode, "dstNetworkInterface"))
            throw CException("Invalid graph, missing connection dst ni.");
        t = getGraph()->getTile(CGetAttribute(connectionNode, "dstTile"));
        n = t->getNetworkInterface(CGetAttribute(connectionNode,
                                   "dstNetworkInterface"));
        connectDst(n);

        // Latency
        if (!CHasAttribute(connectionNode, "latency"))
            throw CException("Invalid graph, missing connection latency.");
        setLatency(CGetAttribute(connectionNode, "latency"));
    }

    /**
     * convertToXML()
     * The function adds the properties of this connection object to the
     * supplied node.
     */
    void Connection::convertToXML(const CNodePtr connectionNode)
    {
        // Name
        CAddAttribute(connectionNode, "name", getName());

        // Source network interface
        CAddAttribute(connectionNode, "srcTile",
                      srcNetworkInterface->getTile()->getName());
        CAddAttribute(connectionNode, "srcNetworkInterface",
                      srcNetworkInterface->getName());

        // Destination network interface
        CAddAttribute(connectionNode, "dstTile",
                      dstNetworkInterface->getTile()->getName());
        CAddAttribute(connectionNode, "dstNetworkInterface",
                      dstNetworkInterface->getName());

        // Latency
        CAddAttribute(connectionNode, "latency", getLatency());
    }

    /**
     * connectSrc()
     * The function connects the source of this connection to the supplied
     * network interface.
     */
    void Connection::connectSrc(NetworkInterface *ni)
    {
        srcNetworkInterface = ni;
        srcNetworkInterface->addConnectionAsOutput(this);
    }

    /**
     * connectDst()
     * The function connects the destination of this connection to the supplied
     * network interface.
     */
    void Connection::connectDst(NetworkInterface *ni)
    {
        dstNetworkInterface = ni;
        dstNetworkInterface->addConnectionAsInput(this);
    }

} // End namespace FSMSADF

