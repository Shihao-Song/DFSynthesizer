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

#ifndef FSMSADF_RESOURCE_ALLOCATION_PLATFORM_GRAPH_CONNECTION_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_PLATFORM_GRAPH_CONNECTION_H_INCLUDED

#include "../../base/type.h"

namespace FSMSADF
{

    // Forward class definition
    class Connection;
    class NetworkInterface;
    class PlatformGraph;

    // List of connection pointers
    typedef list<Connection *> Connections;

    /**
     * Connection
     * Container for connection.
     */
    class Connection : public GraphComponent
    {
        public:
            // Constructor
            Connection(GraphComponent c);

            // Destructor
            ~Connection();

            // Construct
            Connection *create(GraphComponent c) const;
            Connection *createCopy(GraphComponent c);
            Connection *clone(GraphComponent c);

            // Construct
            void constructFromXML(const CNodePtr connectionNode);

            // Convert
            void convertToXML(const CNodePtr connectionNode);

            // Latency
            Time getLatency() const
            {
                return latency;
            };
            void setLatency(const Time t)
            {
                latency = t;
            };

            // Network interfaces
            NetworkInterface *getSrcNetworkInterface()
            {
                return srcNetworkInterface;
            };
            NetworkInterface *getDstNetworkInterface()
            {
                return dstNetworkInterface;
            };
            void connectSrc(NetworkInterface *ni);
            void connectDst(NetworkInterface *ni);

            // Platform graph
            PlatformGraph *getGraph() const
            {
                return (PlatformGraph *)(getParent());
            };

        private:
            // Latency
            Time latency;

            // Tiles
            NetworkInterface *srcNetworkInterface;
            NetworkInterface *dstNetworkInterface;
    };

} // End namespace FSMSADF

#endif

