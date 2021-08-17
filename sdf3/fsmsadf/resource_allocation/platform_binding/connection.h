/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   connection.h
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
 * $Id: connection.h,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_PLATFORM_BINDING_CONNECTION_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_PLATFORM_BINDING_CONNECTION_H_INCLUDED

#include "../platform_graph/graph.h"
#include "../../base/graph.h"

namespace FSMSADF
{

    // Forward class definition
    class ConnectionBinding;
    class PlatformBinding;

    // List of connection pointers
    typedef list<ConnectionBinding *> ConnectionBindings;

    /**
     * ConnectionBinding
     * Container for connection binding.
     */
    class ConnectionBinding : public GraphComponent
    {
        public:
            // Constructor
            ConnectionBinding(GraphComponent c, Connection *co);

            // Destructor
            ~ConnectionBinding();

            // Construct
            ConnectionBinding *clone(GraphComponent c);

            // Construct
            void constructFromXML(Scenario *s, const CNodePtr connectionNode);
            void constructResourceUsageFromXML(const CNodePtr connectionNode);

            // Convert
            void convertToXML(Scenario *s, const CNodePtr connectionNode);
            void convertResourceUsageToXML(const CNodePtr connectionNode);

            // Connection
            Connection *getConnection() const
            {
                return connection;
            };

            // Channel bindings
            map<Scenario *, Channels> &getChannelBindings()
            {
                return channelBindings;
            };
            void setChannelBindings(map<Scenario *, Channels> &ch)
            {
                channelBindings = ch;
            };
            bool addChannelBinding(Scenario *s, Channel *c);
            void removeChannelBinding(Scenario *s, const Channel *c);
            bool hasChannelBinding(Scenario *s, const Channel *c) const;
            bool hasResourcesChannelBinding(Scenario *s, const Channel *c) const;

            // Platform binding
            PlatformBinding *getPlatformBinding() const
            {
                return (PlatformBinding *)(getParent());
            };

        private:
            // Connection
            Connection *connection;

            // Channel bindings
            map<Scenario *, Channels> channelBindings;
    };

} // End namespace FSMSADF

#endif

