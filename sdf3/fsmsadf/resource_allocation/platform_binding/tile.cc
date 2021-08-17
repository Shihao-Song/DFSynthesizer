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
 * $Id: tile.cc,v 1.1 2009-12-23 13:37:22 sander Exp $
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
     * TileBinding ()
     * Constructor.
     */
    TileBinding::TileBinding(GraphComponent c, Tile *t, bool create)
        :
        GraphComponent(c),
        tile(t)
    {
        // Create processor, memory and network interface binding object for all
        // corresponding elements in the tile?
        if (create)
        {
            // Create binding object for all processors
            for (Processors::iterator i = t->getProcessors().begin();
                 i != t->getProcessors().end(); i++)
            {
                createProcessorBinding(*i);
            }

            /// @todo fix the ref assignment
            createMemoryBinding(&(t->getMemories()));

            // Create binding object for all network interfaces
            for (NetworkInterfaces::iterator i = t->getNetworkInterfaces().begin();
                 i != t->getNetworkInterfaces().end(); i++)
            {
                createNetworkInterfaceBinding(*i);
            }
        }
    }

    /**
     * ~TileBinding ()
     * Destructor.
     */
    TileBinding::~TileBinding()
    {
        // Processor bindings
        for (ProcessorBindings::iterator i = processorBindings.begin();
             i != processorBindings.end(); i++)
        {
            delete *i;
        }

        // Memory bindings
        delete memoryBinding;

        // Network interface bindings
        for (NetworkInterfaceBindings::iterator i = networkInterfaceBindings.begin();
             i != networkInterfaceBindings.end(); i++)
        {
            delete *i;
        }
    }

    /**
     * clone()
     * Create a cloned copy of this tile binding.
     */
    TileBinding *TileBinding::clone(GraphComponent c)
    {
        TileBinding *tb = new TileBinding(c, getTile(), false);

        // Processor bindings
        for (ProcessorBindings::iterator i = processorBindings.begin();
             i != processorBindings.end(); i++)
        {
            ProcessorBinding *pb = *i;
            ProcessorBinding *pbNew = pb->clone(GraphComponent(tb,
                                                pb->getId(), pb->getName()));
            tb->getProcessorBindings().push_back(pbNew);
        }

        // Memory bindings
        tb->memoryBinding = memoryBinding->clone(GraphComponent(tb,
                            memoryBinding->getId(),
                            memoryBinding->getName()));

        // Network interface bindings
        for (NetworkInterfaceBindings::iterator i = networkInterfaceBindings.begin();
             i != networkInterfaceBindings.end(); i++)
        {
            NetworkInterfaceBinding *nb = *i;
            NetworkInterfaceBinding *nbNew = nb->clone(GraphComponent(tb,
                                             nb->getId(), nb->getName()));
            tb->getNetworkInterfaceBindings().push_back(nbNew);
        }

        return tb;
    }

    /**
     * constructFromXML()
     * This function creates the tile binding as specified by the
     * tileNode.
     */
    void TileBinding::constructFromXML(Scenario *s, const CNodePtr tileNode)
    {
        // Iterate over all child nodes in the tile
        for (CNode *n = CGetChildNode(tileNode); n != NULL; n = CNextNode(n))
        {
            // Processor?
            if (CIsNode(n, "processor"))
            {
                // Name?
                if (!CHasAttribute(n, "name"))
                    throw CException("Processor has no name identifier.");

                // Locate processor
                Processor *p = getTile()->getProcessor(CGetAttribute(n, "name"));

                // Set resource usage
                getProcessorBinding(p)->constructFromXML(s, n);

            }

            // Memory?
            /// @todo fix this.
            if (CIsNode(n, "memory"))
            {
                // Name?
                if (!CHasAttribute(n, "name"))
                    throw CException("Memory has no name identifier.");
                // Set resource usage
                getMemoryBinding()->constructFromXML(s, n);

            }

            // Network interface?
            if (CIsNode(n, "networkInterface"))
            {
                // Name?
                if (!CHasAttribute(n, "name"))
                    throw CException("Network interface has no name identifier.");

                // Locate network interface
                NetworkInterface *ni = getTile()->getNetworkInterface(
                                           CGetAttribute(n, "name"));

                // Set resource usage
                getNetworkInterfaceBinding(ni)->constructFromXML(s, n);
            }
        }
    }

    /**
     * constructFromXML()
     * This function sets the resource usage of the tile.
     */
    void TileBinding::constructResourceUsageFromXML(const CNodePtr tileNode)
    {
        // Iterate over all child nodes in the tile
        for (CNode *n = CGetChildNode(tileNode); n != NULL; n = CNextNode(n))
        {
            // Processor?
            if (CIsNode(n, "processor"))
            {
                // Name?
                if (!CHasAttribute(n, "name"))
                    throw CException("Processor has no name identifier.");

                // Locate processor
                Processor *p = getTile()->getProcessor(CGetAttribute(n, "name"));

                // Set resource usage
                getProcessorBinding(p)->constructResourceUsageFromXML(n);

            }

            // Memory?
            if (CIsNode(n, "memory"))
            {
                // Name?
                if (!CHasAttribute(n, "name"))
                    throw CException("Memory has no name identifier.");
                /// @todo FIX THIS!
                // Locate memory
                Memory *m = getTile()->getMemory(CGetAttribute(n, "name"));

                // Set resource usage
                getMemoryBinding()->constructResourceUsageFromXML(m, n);

            }

            // Network interface?
            if (CIsNode(n, "networkInterface"))
            {
                // Name?
                if (!CHasAttribute(n, "name"))
                    throw CException("Network interface has no name identifier.");

                // Locate network interface
                NetworkInterface *ni = getTile()->getNetworkInterface(
                                           CGetAttribute(n, "name"));

                // Set resource usage
                getNetworkInterfaceBinding(ni)->constructResourceUsageFromXML(n);
            }
        }
    }

    /**
     * convertToXML()
     * This function converts the tile binding to an XML object.
     */
    void TileBinding::convertToXML(Scenario *s, const CNodePtr tileNode)
    {
        // Name
        CAddAttribute(tileNode, "name", getName());

        // Iterate over all processor bindings
        for (ProcessorBindings::iterator i = processorBindings.begin();
             i != processorBindings.end(); i++)
        {
            (*i)->convertToXML(s, CAddNode(tileNode, "processor"));
        }


        // Iterate over all memory bindings
        // This function will add subnodes memory. so pass it the tileNode.
        // Memories are only known in the binding, not the tile.
        memoryBinding->convertToXML(s, tileNode);

        // Iterate over all network interface bindings
        for (NetworkInterfaceBindings::iterator i = networkInterfaceBindings.begin();
             i != networkInterfaceBindings.end(); i++)
        {
            (*i)->convertToXML(s, CAddNode(tileNode, "networkInterface"));
        }
    }

    /**
     * convertToXML()
     * This function converts the resource usage of this tile binding
     * to an XML object.
     */
    void TileBinding::convertResourceUsageToXML(const CNodePtr tileNode)
    {
        // Name
        CAddAttribute(tileNode, "name", getName());

        // Iterate over all processor bindings
        for (ProcessorBindings::iterator i = processorBindings.begin();
             i != processorBindings.end(); i++)
        {
            (*i)->convertResourceUsageToXML(CAddNode(tileNode, "processor"));
        }

        // Iterate over all memory bindings
        // this function it will add memory subnodes, so pass it tileNode
        // This is because there can be multiple memories known by the binding
        // and not the tile.
        /// @todo make the tile know about memories?
        memoryBinding->convertResourceUsageToXML(tileNode);


        // Iterate over all network interface bindings
        for (NetworkInterfaceBindings::iterator i = networkInterfaceBindings.begin();
             i != networkInterfaceBindings.end(); i++)
        {
            (*i)->convertResourceUsageToXML(CAddNode(tileNode, "networkInterface"));
        }
    }

    /**
     * getProcessorBinding()
     * The function returns a pointer to the processor binding associated
     * with processor p.
     */
    ProcessorBinding *TileBinding::getProcessorBinding(const Processor *p)
    {
        for (ProcessorBindings::iterator i = processorBindings.begin();
             i != processorBindings.end(); i++)
        {
            if ((*i)->getProcessor() == p)
                return *i;
        }

        throw CException("Processor '" + p->getName() + "' has no binding.");
    }

    /**
     * getMemoryBinding()
     * The function returns a pointer to the memory binding
     */
    MemoryBinding *TileBinding::getMemoryBinding()
    {
        return memoryBinding;
    }

    /**
     * getNetworkInterfaceBinding()
     * The function returns a pointer to the network interface binding associated
     * with network interface n.
     */
    NetworkInterfaceBinding *TileBinding::getNetworkInterfaceBinding(
        const NetworkInterface *n)
    {
        for (NetworkInterfaceBindings::iterator i = networkInterfaceBindings.begin();
             i != networkInterfaceBindings.end(); i++)
        {
            if ((*i)->getNetworkInterface() == n)
                return *i;
        }

        throw CException("Network interface '" + n->getName() + "' has no binding.");
    }

    /**
     * createProcessorBinding()
     * The function creates a processor binding object for processor p
     * and adds it to the list of processor bindings associated with this
     * tile binding.
     */
    ProcessorBinding *TileBinding::createProcessorBinding(Processor *p)
    {
        ProcessorBinding *pb;

        pb = new ProcessorBinding(GraphComponent(this, p->getId(), p->getName()), p);
        processorBindings.push_back(pb);

        return pb;
    }

    /**
     * createMemoryBinding()
     * The function creates a memory binding object for memory m
     * and adds it to the list of memory bindings associated with this
     * tile binding.
     */
    MemoryBinding *TileBinding::createMemoryBinding(Memories *m)
    {
        MemoryBinding *mb;
        /// @todo what id,name?
        mb = new MemoryBinding(GraphComponent(this, 0, "MemBinding"), m);
        memoryBinding = mb;

        return mb;
    }

    /**
     * createNetworkInterfaceBinding()
     * The function creates a network interface binding object for network interface
     * n and adds it to the list of network interface bindings associated with this
     * tile binding.
     */
    NetworkInterfaceBinding *TileBinding::createNetworkInterfaceBinding(
        NetworkInterface *n)
    {
        NetworkInterfaceBinding *nb;

        nb = new NetworkInterfaceBinding(GraphComponent(this, n->getId(),
                                         n->getName()), n);
        networkInterfaceBindings.push_back(nb);

        return nb;
    }

} // End namespace FSMSADF

