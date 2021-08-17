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
 * $Id: tile.h,v 1.1 2009-12-23 13:37:23 sander Exp $
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_PLATFORM_GRAPH_TILE_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_PLATFORM_GRAPH_TILE_H_INCLUDED

#include "processor.h"
#include "memory.h"
#include "networkinterface.h"

namespace FSMSADF
{

    // Forward class definition
    class Tile;

    // List of tile pointers
    typedef list<Tile *> Tiles;

    /**
     * Tile
     * Container for tile.
     */
    class Tile : public GraphComponent
    {
        public:
            // Constructor
            Tile(GraphComponent c);

            // Destructor
            virtual ~Tile();

            // Construct
            Tile *create(GraphComponent c) const;
            Tile *createCopy(GraphComponent c) const;
            Tile *clone(GraphComponent c) const;

            // Construct
            void constructFromXML(const CNodePtr tileNode);

            // Convert
            void convertToXML(const CNodePtr tileNode);

            // Processors
            Processors &getProcessors()
            {
                return processors;
            };
            Processor *getProcessor(const CString &name) const;
            Processor *createProcessor(const CString &name);
            void addProcessor(Processor *p);
            void removeProcessor(Processor *p);

            // Memories
            Memories &getMemories()
            {
                return memories;
            };
            Memory *getMemory(const CString &name) const;
            Memory *createMemory(const CString &name);
            void addMemory(Memory *m);
            void removeMemory(Memory *m);

            // Network interfaces
            NetworkInterfaces &getNetworkInterfaces()
            {
                return networkInterfaces;
            };
            NetworkInterface *getNetworkInterface(const CString &name) const;
            NetworkInterface *createNetworkInterface(const CString &name);
            void addNetworkInterface(NetworkInterface *n);
            void removeNetworkInterface(NetworkInterface *n);

        private:
            // Processors
            Processors processors;

            // Memories
            Memories memories;

            // Network interfaces
            NetworkInterfaces networkInterfaces;
    };

} // End namespace FSMSADF

#endif
