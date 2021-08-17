/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   memory.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 12, 2009
 *
 *  Function        :   Tile memory.
 *
 *  History         :
 *      04-05-11    :   Add support for multiple memory types (M.Koedam)
 *      12-05-09    :   Initial version.
 *
 * $Id: memory.h,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_PLATFORM_GRAPH_MEMORY_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_PLATFORM_GRAPH_MEMORY_H_INCLUDED

#include "../../base/type.h"

namespace FSMSADF
{

    // Forward class definition
    class Memory;
    class Tile;

    // List of memory pointers
    typedef list<Memory *> Memories;


    /**
     * Memory
     * Container for tile memory.
     */
    class Memory : public GraphComponent
    {
        public:

            // Constructor
            Memory(GraphComponent c);

            // Destructor
            ~Memory();

            // Construct
            Memory *create(GraphComponent c) const;
            Memory *createCopy(GraphComponent c) const;
            Memory *clone(GraphComponent c) const;

            // Construct
            void constructFromXML(const CNodePtr memoryNode);

            // Convert
            void convertToXML(const CNodePtr memoryNode);

            // Size
            Size getSize() const
            {
                return size;
            };
            void setSize(const Size sz)
            {
                size = sz;
            };

            // Tile
            Tile *getTile() const
            {
                return (Tile *)(getParent());
            };

            // Type
            MemoryType getType() const
            {
                return type;
            };

            void setType(MemoryType::Value t)
            {
                type = t;
            };
            void setType(CString t)
            {
                type = t;
            };

        private:
            // Size
            Size size;
            MemoryType type;
    };

} // End namespace FSMSADF

#endif
