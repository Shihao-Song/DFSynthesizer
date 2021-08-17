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
     * ConnectionBinding ()
     * Constructor.
     */
    ConnectionBinding::ConnectionBinding(GraphComponent c, Connection *co)
        :
        GraphComponent(c),
        connection(co)
    {
    }

    /**
     * ~ConnectionBinding ()
     * Destructor.
     */
    ConnectionBinding::~ConnectionBinding()
    {
    }

    /**
     * clone()
     * Create a cloned copy of this connection binding.
     */
    ConnectionBinding *ConnectionBinding::clone(GraphComponent c)
    {
        ConnectionBinding *cb = new ConnectionBinding(c, getConnection());

        // Channel bindings
        cb->setChannelBindings(getChannelBindings());

        return cb;
    }

    /**
     * constructFromXML()
     * This function creates the connection binding as specified by the
     * connectionNode.
     */
    void ConnectionBinding::constructFromXML(Scenario *s,
            const CNodePtr connectionNode)
    {
        ScenarioGraph *sg = s->getScenarioGraph();

        // Iterate over the list of channels bound to this connection
        for (CNode *n = CGetChildNode(connectionNode, "channel");
             n != NULL; n = CNextNode(connectionNode, "channel"))
        {
            // Name specified for the channel
            if (!CHasAttribute(n, "name"))
                throw CException("Mapped channel has no name identifier.");

            // Find channel in scenario graph
            Channel *c = sg->getChannel(CGetAttribute(n, "name"));

            // Create binding of channel to connection
            addChannelBinding(s, c);
        }
    }

    /**
     * constructResourceUsageFromXML()
     * This function sets the resource usgae of the connection.
     */
    void ConnectionBinding::constructResourceUsageFromXML(
        const CNodePtr connectionNode)
    {
    }

    /**
     * convertToXML()
     * This function converts the connection binding to an XML object.
     */
    void ConnectionBinding::convertToXML(Scenario *s, const CNodePtr connectionNode)
    {
        map<Scenario *, Channels>::iterator scenIter = channelBindings.find(s);

        // Name
        CAddAttribute(connectionNode, "name", getName());

        // Binding exists for this scenario?
        if (scenIter != channelBindings.end())
        {
            // List of all channels bound to this connection
            for (Channels::iterator i = scenIter->second.begin();
                 i != scenIter->second.end(); i++)
            {
                CNode *n = CAddNode(connectionNode, "channel");
                CAddAttribute(n, "name", (*i)->getName());
            }
        }
    }

    /**
     * convertResourceUsageToXML()
     * This function converts the resource usage of this connection binding to an
     * XML object.
     */
    void ConnectionBinding::convertResourceUsageToXML(const CNodePtr connectionNode)
    {
        // Name
        CAddAttribute(connectionNode, "name", getName());
    }

    /**
     * addChannelBinding()
     * The function binds channel c in scenario s to this connection. The function
     * returns true on success. Otherwise it returns false.
     */
    bool ConnectionBinding::addChannelBinding(Scenario *s, Channel *c)
    {
        // Binding already exists?
        if (hasChannelBinding(s, c))
            return true;

        // Sufficient resource available?
        if (hasResourcesChannelBinding(s, c))
        {
            channelBindings[s].push_back(c);
            return true;
        }

        return false;
    }

    /**
     * removeChannelBinding()
     * The function removes the binding of channel c in scenario s to this
     * connection.
     */
    void ConnectionBinding::removeChannelBinding(Scenario *s, const Channel *c)
    {
        for (Channels::iterator i = channelBindings[s].begin();
             i != channelBindings[s].end(); i++)
        {
            if (*i == c)
            {
                channelBindings[s].erase(i);
                break;
            }
        }
    }

    /**
     * hasChannelBinding()
     * The function returns true when a binding of channel c in scenario s to this
     * connection exists. Otherwise it returns false.
     */
    bool ConnectionBinding::hasChannelBinding(Scenario *s, const Channel *c) const
    {
        map<Scenario *, Channels>::const_iterator scenIter = channelBindings.find(s);

        if (scenIter == channelBindings.end())
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
     * hasResourcesChannelBinding()
     * The function checks wether there are sufficient resource available to bind
     * the channel c to the connection in scenario s.
     */
    bool ConnectionBinding::hasResourcesChannelBinding(Scenario *s,
            const Channel *c) const
    {
        return true;
    }

} // End namespace FSMSADF

