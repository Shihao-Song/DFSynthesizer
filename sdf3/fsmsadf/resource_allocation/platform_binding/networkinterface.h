/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   networkinterface.h
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
 * $Id: networkinterface.h,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_PLATFORM_BINDING_NETWORKINTERFACE_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_PLATFORM_BINDING_NETWORKINTERFACE_H_INCLUDED

#include "../platform_graph/graph.h"
#include "../../base/graph.h"
#include "constraint.h"

namespace FSMSADF
{

    // Forward class definition
    class NetworkInterfaceBinding;
    class TileBinding;

    // List of network interface binding pointers
    typedef list<NetworkInterfaceBinding *> NetworkInterfaceBindings;

    /**
     * NetworkInterfaceBinding
     * Container for tile network interface binding.
     */
    class NetworkInterfaceBinding : public GraphComponent
    {
        public:
            // Constructor
            NetworkInterfaceBinding(GraphComponent c, NetworkInterface *n);

            // Destructor
            ~NetworkInterfaceBinding();

            // Construct
            NetworkInterfaceBinding *clone(GraphComponent c);

            // Construct
            void constructFromXML(Scenario *s, const CNodePtr networkInterfaceNode);
            void constructResourceUsageFromXML(const CNodePtr networkInterfaceNode);

            // Convert
            void convertToXML(Scenario *s, const CNodePtr networkInterfaceNode);
            void convertResourceUsageToXML(const CNodePtr networkInterfaceNode);

            // Network interface
            NetworkInterface *getNetworkInterface() const
            {
                return networkInterface;
            };

            // Graph binding constraints
            GraphBindingConstraints *getGraphBindingConstraints() const;

            // Channel bindings (input)
            map<Scenario *, Channels> &getInChannelBindings()
            {
                return inChannelBindings;
            };
            void setInChannelBindings(map<Scenario *, Channels> &ch)
            {
                inChannelBindings = ch;
            };
            bool addInChannelBinding(Scenario *s, Channel *c);
            void removeInChannelBinding(Scenario *s, const Channel *c);
            bool hasInChannelBinding(Scenario *s, const Channel *c) const;

            // Resource occupancy (input)
            Size getAvailableNrInConnections(Scenario *s) const;
            Size getAllocatedNrInConnections(Scenario *s) const;
            Bandwidth getAvailableBandwidthInConnections(Scenario *s) const;
            Bandwidth getAllocatedBandwidthInConnections(Scenario *s) const;
            bool hasResourcesInChannelBinding(Scenario *s, const Channel *c) const;

            // Initial resource occupancy (input)
            Size getNrInConnectionsUsedForOtherGraphs() const
            {
                return nrInConnectionsUsedForOtherGraphs;
            };
            void setNrInConnectionsUsedForOtherGraphs(const Size n)
            {
                nrInConnectionsUsedForOtherGraphs = n;
            };
            Bandwidth getInBandwidthUsedForOtherGraphs() const
            {
                return inBandwidthUsedForOtherGraphs;
            };
            void setInBandwidthUsedForOtherGraphs(const Bandwidth b)
            {
                inBandwidthUsedForOtherGraphs = b;
            };

            // Channel bindings (output)
            map<Scenario *, Channels> &getOutChannelBindings()
            {
                return outChannelBindings;
            };
            void setOutChannelBindings(map<Scenario *, Channels> &ch)
            {
                outChannelBindings = ch;
            };
            bool addOutChannelBinding(Scenario *s, Channel *c);
            void removeOutChannelBinding(Scenario *s, const Channel *c);
            bool hasOutChannelBinding(Scenario *s, const Channel *c) const;

            // Resource occupancy (output)
            Size getAvailableNrOutConnections(Scenario *s) const;
            Size getAllocatedNrOutConnections(Scenario *s) const;
            Bandwidth getAvailableBandwidthOutConnections(Scenario *s) const;
            Bandwidth getAllocatedBandwidthOutConnections(Scenario *s) const;
            bool hasResourcesOutChannelBinding(Scenario *s, const Channel *c) const;

            // Resource occupancy of application graph
            Size getNrInConnectionsUsedForGraph() const;
            Bandwidth getInBandwidthUsedForGraph() const;
            Size getNrOutConnectionsUsedForGraph() const;
            Bandwidth getOutBandwidthUsedForGraph() const;

            // Initial resource occupancy (output)
            Size getNrOutConnectionsUsedForOtherGraphs() const
            {
                return nrOutConnectionsUsedForOtherGraphs;
            };
            void setNrOutConnectionsUsedForOtherGraphs(const Size n)
            {
                nrOutConnectionsUsedForOtherGraphs = n;
            };
            Bandwidth getOutBandwidthUsedForOtherGraphs() const
            {
                return outBandwidthUsedForOtherGraphs;
            };
            void setOutBandwidthUsedForOtherGraphs(const Bandwidth b)
            {
                outBandwidthUsedForOtherGraphs = b;
            };

            // Tile binding
            TileBinding *getTileBinding() const
            {
                return (TileBinding *)(getParent());
            };

        private:
            // Network interface
            NetworkInterface *networkInterface;

            // Channels
            map<Scenario *, Channels> inChannelBindings;
            map<Scenario *, Channels> outChannelBindings;

            // Initial resource occupancy
            Size nrInConnectionsUsedForOtherGraphs;
            Size nrOutConnectionsUsedForOtherGraphs;
            Bandwidth inBandwidthUsedForOtherGraphs;
            Bandwidth outBandwidthUsedForOtherGraphs;
    };

} // End namespace FSMSADF

#endif
