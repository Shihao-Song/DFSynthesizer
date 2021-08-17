/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   processor.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 12, 2009
 *
 *  Function        :   Tile processor.
 *
 *  History         :
 *      12-05-09    :   Initial version.
 *
 * $Id: processor.cc,v 1.1 2009-12-23 13:37:23 sander Exp $
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

#include "processor.h"

namespace FSMSADF
{

    /**
     * Processor()
     * Constructor.
     */
    Processor::Processor(GraphComponent c)
        :
        GraphComponent(c),
        wheelsize(0),
        contextSwitchOverhead(0),
        slotLength(0),
        supportsPreemption(true)
    {
    }

    /**
     * ~Processor()
     * Destructor.
     */
    Processor::~Processor()
    {
    }

    /**
     * create()
     * The function returns a new processor object.
     */
    Processor *Processor::create(GraphComponent c) const
    {
        return new Processor(c);
    }

    /**
     * createCopy()
     * The function returns a new processor object and copies all properties.
     */
    Processor *Processor::createCopy(GraphComponent c) const
    {
        Processor *p = create(c);

        // Properties
        p->setName(getName());
        p->setType(getType());
        p->setArbitrationType(getArbitrationType());
        p->setWheelsize(getWheelsize());
        p->setSlotLength(getSlotLength());
        p->setContextSwitchOverhead(getContextSwitchOverhead());
        p->setSupportForPreemption(hasSupportForPreemption());

        return p;
    }

    /**
     * clone()
     * The function returns a new processor object and copies all properties, but
     * when the parent of the graph component c and this object are different, all
     * pointers are relocated.
     */
    Processor *Processor::clone(GraphComponent c) const
    {
        Processor *p = createCopy(c);

        return p;
    }

    /**
     * constructFromXML()
     * Load all properties of this processor from the supplied XML node.
     */
    void Processor::constructFromXML(const CNodePtr processorNode)
    {
        // Name
        if (!CHasAttribute(processorNode, "name"))
            throw CException("Invalid graph, missing processor name.");
        setName(CGetAttribute(processorNode, "name"));

        // Type
        if (!CHasAttribute(processorNode, "type"))
            throw CException("Invalid graph, missing processor type.");
        setType(CGetAttribute(processorNode, "type"));

        // Arbitration
        if (CHasChildNode(processorNode, "arbitration"))
        {
            CNode *arbitrationNode = CGetChildNode(processorNode, "arbitration");

            // Type
            if (!CHasAttribute(arbitrationNode, "type"))
                throw CException("Invalid graph, missing arbitration type.");
            setArbitrationType(CGetAttribute(arbitrationNode, "type"));

            // Only supported type is TDMA
            if (getArbitrationType() != "TDMA")
                throw CException("Invalid graph, arbitration type not supported.");

            // Wheelsize
            if (!CHasAttribute(arbitrationNode, "wheelsize"))
                throw CException("Invalid graph, missing TDMA wheelsize.");
            setWheelsize(CGetAttribute(arbitrationNode, "wheelsize"));

            // Context switch overhead
            if (!CHasAttribute(arbitrationNode, "contextSwitchOverhead"))
                throw CException("Invalid graph, missing contextSwitchOverhead.");
            setContextSwitchOverhead(CGetAttribute(arbitrationNode,
                                                   "contextSwitchOverhead"));
            // Context switch overhead
            if (CHasAttribute(arbitrationNode, "slotLength"))
                setSlotLength(CGetAttribute(arbitrationNode,
                                            "slotLength"));

            // TDMA support for pre-emption
            if (!CHasAttribute(arbitrationNode, "preemptive"))
                throw CException("Invalid graph, missing preemptive.");
            if (CGetAttribute(arbitrationNode, "preemptive") == "true")
                setSupportForPreemption(true);
            else
                setSupportForPreemption(false);
        }
    }

    /**
     * convertToXML()
     * Add all properties of this processor to the supplied XML node.
     */
    void Processor::convertToXML(const CNodePtr processorNode)
    {
        // Name
        CAddAttribute(processorNode, "name", getName());

        // Type
        CAddAttribute(processorNode, "type", getType());

        // Arbitration
        CNode *arbitrationNode = CAddNode(processorNode, "arbitration");

        // Arbitration type
        CAddAttribute(arbitrationNode, "type", getArbitrationType());

        // Wheelsize
        CAddAttribute(arbitrationNode, "wheelsize", getWheelsize());

        // slotLength
        CAddAttribute(arbitrationNode, "slotLength", getSlotLength());

        // Context switch overhead
        CAddAttribute(arbitrationNode, "contextSwitchOverhead",
                      getContextSwitchOverhead());

        // TDMA support for pre-emption
        if (hasSupportForPreemption())
            CAddAttribute(arbitrationNode, "preemptive", "true");
        else
            CAddAttribute(arbitrationNode, "preemptive", "false");
    }

} // End namespace FSMSADF


