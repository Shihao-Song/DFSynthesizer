/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   tile.cc
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
 * $Id: tile.cc,v 1.1 2009-12-23 13:37:23 sander Exp $
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

#include "tile.h"

namespace FSMSADF
{

    /**
     * Tile ()
     * Constructor.
     */
    Tile::Tile(GraphComponent c) : GraphComponent(c)
    {
    }

    /**
     * ~Tile ()
     * Destructor.
     */
    Tile::~Tile()
    {
        // Processors
        for (Processors::iterator i = processors.begin(); i != processors.end(); i++)
            delete *i;

        // Memories
        for (Memories::iterator i = memories.begin(); i != memories.end(); i++)
            delete *i;

        // Network interfaces
        for (NetworkInterfaces::iterator i = networkInterfaces.begin();
             i != networkInterfaces.end(); i++)
        {
            delete *i;
        }
    }

    /**
     * create()
     * The function creates a new tile object
     */
    Tile *Tile::create(GraphComponent c) const
    {
        return new Tile(c);
    }

    /**
     * createCopy()
     * The function creates a copy of this tile.
     */
    Tile *Tile::createCopy(GraphComponent c) const
    {
        Tile *t = create(c);

        // Processors
        for (Processors::const_iterator i = processors.begin();
             i != processors.end(); i++)
        {
            t->addProcessor((*i)->createCopy(GraphComponent(t, getId(), getName())));
        }

        // Memories
        for (Memories::const_iterator i = memories.begin();
             i != memories.end(); i++)
        {
            t->addMemory((*i)->createCopy(GraphComponent(t, getId(), getName())));
        }

        // Network interfaces
        for (NetworkInterfaces::const_iterator i = networkInterfaces.begin();
             i != networkInterfaces.end(); i++)
        {
            t->addNetworkInterface((*i)->createCopy(GraphComponent(t,
                                                    getId(), getName())));
        }

        return t;
    }

    /**
     * clone()
     * The function creates a clone of this tile.
     */
    Tile *Tile::clone(GraphComponent c) const
    {
        Tile *t = create(c);

        // Processors
        for (Processors::const_iterator i = processors.begin();
             i != processors.end(); i++)
        {
            t->addProcessor((*i)->clone(GraphComponent(t, getId(), getName())));
        }

        // Memories
        for (Memories::const_iterator i = memories.begin();
             i != memories.end(); i++)
        {
            t->addMemory((*i)->clone(GraphComponent(t, getId(), getName())));
        }

        // Network interfaces
        for (NetworkInterfaces::const_iterator i = networkInterfaces.begin();
             i != networkInterfaces.end(); i++)
        {
            t->addNetworkInterface((*i)->clone(GraphComponent(t,
                                               getId(), getName())));
        }

        return t;
    }

    /**
     * constructFromXML()
     * The function sets all elements of this tile using the supplied tile node.
     */
    void Tile::constructFromXML(const CNodePtr tileNode)
    {
        // Name
        setName(CGetAttribute(tileNode, "name"));

        // Iterate over all child nodes of the tile
        for (CNode *n = CGetChildNode(tileNode); n != NULL; n = CNextNode(n))
        {
            // Processor
            if (CIsNode(n, "processor"))
            {
                Processor *p = createProcessor("(no name)");
                p->constructFromXML(n);
            }

            // Memory
            if (CIsNode(n, "memory"))
            {
                Memory *m = createMemory("(no name)");
                m->constructFromXML(n);
            }

            // Network interface
            if (CIsNode(n, "networkInterface"))
            {
                NetworkInterface *ni = createNetworkInterface("(no name)");
                ni->constructFromXML(n);
            }
        }
    }

    /**
     * convertToXML()
     * The function adds the elements of this tile to the supplied tile node.
     */
    void Tile::convertToXML(const CNodePtr tileNode)
    {
        // Name
        CAddAttribute(tileNode, "name", getName());

        // Processors
        for (Processors::iterator i = processors.begin(); i != processors.end(); i++)
        {
            (*i)->convertToXML(CAddNode(tileNode, "processor"));
        }

        // Memories
        for (Memories::iterator i = memories.begin(); i != memories.end(); i++)
        {
            (*i)->convertToXML(CAddNode(tileNode, "memory"));
        }

        // Network interfaces
        for (NetworkInterfaces::iterator i = networkInterfaces.begin();
             i != networkInterfaces.end(); i++)
        {
            (*i)->convertToXML(CAddNode(tileNode, "networkInterface"));
        }
    }

    /**
     * getProcessor()
     * The function returns a pointer to a processor with the supplied name. When
     * no such processor exists, an exception is thrown.
     */
    Processor *Tile::getProcessor(const CString &name) const
    {
        for (Processors::const_iterator i = processors.begin();
             i != processors.end(); i++)
        {
            if (name == (*i)->getName())
                return *i;
        }

        throw CException("Tile does not contain a processor with name '" + name + "'.");
    }

    /**
     * createProcessor()
     * The function creates a new processor and adds it to this tile.
     */
    Processor *Tile::createProcessor(const CString &name)
    {
        Processor *p = new Processor(GraphComponent(this, processors.size(), name));
        addProcessor(p);
        return p;
    }

    /**
     * addProcessor()
     * The function adds processor p to this tile.
     */
    void Tile::addProcessor(Processor *p)
    {
        processors.push_back(p);
    }

    /**
     * removeProcessor()
     * The function removes the processor p from the tile.
     */
    void Tile::removeProcessor(Processor *p)
    {
        for (Processors::iterator i = processors.begin(); i != processors.end(); i++)
        {
            if (p->getName() == (*i)->getName())
            {
                processors.erase(i);
                break;
            }
        }
    }

    /**
     * getMemory()
     * The function returns a pointer to a memory with the supplied
     * name. When no such memory exists, an exception is thrown.
     */
    Memory *Tile::getMemory(const CString &name) const
    {
        for (Memories::const_iterator i = memories.begin();
             i != memories.end(); i++)
        {
            if (name == (*i)->getName())
                return *i;
        }

        throw CException("Tile does not contain a memory with name '" + name + "'.");
    }

    /**
     * createMemory()
     * The function creates a new memory and adds it to the tile.
     */
    Memory *Tile::createMemory(const CString &name)
    {
        Memory *m = new Memory(GraphComponent(this, memories.size(), name));
        addMemory(m);
        return m;
    }

    /**
     * addMemory()
     * The function adds memory m to the tile.
     */
    void Tile::addMemory(Memory *m)
    {
        memories.push_back(m);
    }

    /**
     * removeMemory()
     * The function removes memory m from the tile.
     */
    void Tile::removeMemory(Memory *m)
    {
        for (Memories::iterator i = memories.begin(); i != memories.end(); i++)
        {
            if (m->getName() == (*i)->getName())
            {
                memories.erase(i);
                break;
            }
        }
    }

    /**
     * getNetworkInterface()
     * The function returns a pointer to a network interface with the supplied
     * name. When no such network interface exists, an exception is thrown.
     */
    NetworkInterface *Tile::getNetworkInterface(const CString &name) const
    {
        // Iterate over the network interfaces
        for (NetworkInterfaces::const_iterator i = networkInterfaces.begin();
             i != networkInterfaces.end(); i++)
        {
            if ((*i)->getName() == name)
            {
                return (*i);
            }
        }

        throw CException("Tile does not contain a network interface withh name '"
                         + name + "'.");
    }

    /**
     * createNetworkInterface()
     * The function creates a new network interface and adds it to this tile.
     */
    NetworkInterface *Tile::createNetworkInterface(const CString &name)
    {
        // Create a new NI
        NetworkInterface *n = new NetworkInterface(GraphComponent(this,
                networkInterfaces.size(), name));

        // Add NI to tile
        addNetworkInterface(n);

        return n;
    }

    /**
     * addNetworkInterface()
     * The function adds the network interface n to the tile.
     */
    void Tile::addNetworkInterface(NetworkInterface *n)
    {
        networkInterfaces.push_back(n);
    }

    /**
     * removeNetworkInterface()
     * The function removes the network interface n from the tile.
     */
    void Tile::removeNetworkInterface(NetworkInterface *n)
    {
        // Iterate over the network interfaces
        for (NetworkInterfaces::iterator i = networkInterfaces.begin();
             i != networkInterfaces.end(); i++)
        {
            if ((*i)->getName() == n->getName())
            {
                networkInterfaces.erase(i);
                break;
            }
        }
    }

} // End namespace FSMSADF

