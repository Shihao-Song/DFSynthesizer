/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   memory.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 12, 2009
 *
 *  Function        :   Tile memory.
 *
 *  History         :
 *      12-05-09    :   Initial version.
 *
 * $Id: memory.cc,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#include "memory.h"

namespace FSMSADF
{

    /**
     * Memory()
     * Constructor.
     */
    Memory::Memory(GraphComponent c)
        :
        GraphComponent(c),
        size(0),
        type(MemoryType::Shared)
    {

    }

    /**
     * ~Memory()
     * Destructor.
     */
    Memory::~Memory()
    {
    }

    /**
     * create()
     * The function returns a new memory object.
     */
    Memory *Memory::create(GraphComponent c) const
    {
        return new Memory(c);
    }

    /**
     * createCopy()
     * The function returns a new memory object and copies all properties.
     */
    Memory *Memory::createCopy(GraphComponent c) const
    {
        Memory *m = create(c);

        // Properties
        m->setName(getName());
        m->setSize(getSize());

        return m;
    }

    /**
     * clone()
     * The function returns a new memory object and copies all properties, but when
     * the parent of the graph component c and this object are different, all
     * pointers are relocated.
     */
    Memory *Memory::clone(GraphComponent c) const
    {
        Memory *m = createCopy(c);

        return m;
    }

    /**
     * constructFromXML()
     * Load all properties of this memory from the supplied XML node.
     */
    void Memory::constructFromXML(const CNodePtr memoryNode)
    {
        // Name
        if (!CHasAttribute(memoryNode, "name"))
            throw CException("Invalid graph, missing memory name.");
        setName(CGetAttribute(memoryNode, "name"));

        // Size
        if (!CHasAttribute(memoryNode, "size"))
            throw CException("Invalid graph, missing memory size.");
        setSize(CGetAttribute(memoryNode, "size"));

        /// @todo docuemnt this.
        if (CHasAttribute(memoryNode, "type"))
        {
            setType(CGetAttribute(memoryNode, "type"));
        }
    }

    /**
     * convertToXML()
     * Add all properties of this memory to the supplied XML node.
     */
    void Memory::convertToXML(const CNodePtr memoryNode)
    {
        // Name
        CAddAttribute(memoryNode, "name", getName());

        // Size
        CAddAttribute(memoryNode, "size", getSize());

        // Type
        CAddAttribute(memoryNode, "type", getType());
    }

} // End namespace

