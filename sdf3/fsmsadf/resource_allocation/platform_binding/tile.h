/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   tile.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 12, 2009
 *
 *  Function        :   Tile.
 *
 *  History         :
 *      12-05-09    :   Initial version.
 *
 * $Id: tile.h,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_PLATFORM_BINDING_TILE_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_PLATFORM_BINDING_TILE_H_INCLUDED

#include "processor.h"
#include "memory.h"
#include "networkinterface.h"

namespace FSMSADF
{

    // Forward class definition
    class TileBinding;
    class PlatformBinding;

    // List of tile  binding pointers
    typedef list<TileBinding *> TileBindings;

    /**
     * TileBinding
     * Container for tile bindings.
     */
    class TileBinding : public GraphComponent
    {
        public:
            // Constructor
            TileBinding(GraphComponent c, Tile *t, bool create = true);

            // Destructor
            ~TileBinding();

            // Construct
            TileBinding *clone(GraphComponent c);

            // Construct
            void constructFromXML(Scenario *s, const CNodePtr tileNode);
            void constructResourceUsageFromXML(const CNodePtr tileNode);

            // Convert
            void convertToXML(Scenario *s, const CNodePtr tileNode);
            void convertResourceUsageToXML(const CNodePtr tileNode);

            // Tile
            Tile *getTile() const
            {
                return tile;
            };

            // Processor bindings
            ProcessorBindings &getProcessorBindings()
            {
                return processorBindings;
            };
            ProcessorBinding *getProcessorBinding(const Processor *p);

            // Memory bindings
            MemoryBinding *getMemoryBinding();

            // Network interface bindings
            NetworkInterfaceBindings &getNetworkInterfaceBindings()
            {
                return networkInterfaceBindings;
            };
            NetworkInterfaceBinding *getNetworkInterfaceBinding(
                const NetworkInterface *n);

            // Platform binding
            PlatformBinding *getPlatformBinding() const
            {
                return (PlatformBinding *)(getParent());
            };

        private:
            // Processor binding
            ProcessorBinding *createProcessorBinding(Processor *p);

            // Memory binding
            MemoryBinding *createMemoryBinding(Memories *m);

            // Network interface binding
            NetworkInterfaceBinding *createNetworkInterfaceBinding(NetworkInterface *n);

        private:
            // Tile
            Tile *tile;

            // Processor bindings
            ProcessorBindings processorBindings;

            // Memory bindings
            MemoryBinding *memoryBinding;

            // Network interface bindings
            NetworkInterfaceBindings networkInterfaceBindings;
    };

} // End namespace FSMSADF

#endif
