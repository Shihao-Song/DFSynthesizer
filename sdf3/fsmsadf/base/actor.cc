/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   actor.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 22, 1009
 *
 *  Function        :   FSM-based SADF actor
 *
 *  History         :
 *      22-04-09    :   Initial version.
 *
 * $Id: actor.cc,v 1.1.2.7 2010-04-22 07:07:10 mgeilen Exp $
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

#include "actor.h"
#include "graph.h"

namespace FSMSADF
{
    /**
     * Actor ()
     * Constructor.
     */
    Actor::Actor(GraphComponent c)
        :
        GraphComponent(c)
    {
        // Set the default repetitions
        setRepetitionsOfScenario(getDefaultScenario(), 1);
    }

    /**
     * ~Actor ()
     * Destructor.
     */
    Actor::~Actor()
    {
        // Ports
        for (Ports::iterator iter = ports.begin(); iter != ports.end(); iter++)
        {
            Port *p = *iter;
            delete p;
        }
        /*
        for (list<ProcessorType>::iterator i = processorTypes.begin(); i != processorTypes.end(); i++)
        {
            for (map<Scenario*, Memories>::iterator j = i->memories.begin(); j != i->memories.end(); j++)
            {
                for (Memories::iterator iter = j->second.begin(); iter != j->second.end(); iter++)
                {
                    //Memory *m = *iter;
                    /// @todo fix memory allocation.
                    //delete m;
                }
            }
        }
         */
    }
    /**
     * create ()
     * The function returns a pointer to a newly allocated actor object.
     */
    Actor *Actor::create(GraphComponent c) const
    {
        return new Actor(c);
    }
    /**
     * createCopy ()
     * The function returns a pointer to a newly allocated actor object.
     * All properties of the actor are copied.
     */
    Actor *Actor::createCopy(GraphComponent c) const
    {
        Actor *a = create(c);

        // Properties
        a->setName(getName());
        a->setType(getType());
        a->setProcessorTypes(getProcessorTypes());
        a->setDefaultProcessorType(getDefaultProcessorType());
        a->setRepetitions(getRepetitions());

        return a;
    }
    /**
     * clone ()
     * The function returns a pointer to a newly allocated actor object.
     * All properties and ports of the actor are cloned.
     */
    Actor *Actor::clone(GraphComponent c) const
    {
        Actor *a = createCopy(c);

        // Ports
        for (Ports::const_iterator iter = ports.begin();
             iter != ports.end(); iter++)
        {
            Port *p = *iter;
            Port *pA = p->clone(GraphComponent(a, p->getId()));
            a->addPort(pA);
        }

        return a;
    }
    /**
     * constructFromXML ()
     * The function initializes all basic actor properties based on the XML data.
     */
    void Actor::constructFromXML(const CNodePtr actorNode)
    {
        // Name
        if (!CHasAttribute(actorNode, "name"))
            throw CException("Invalid graph, missing actor name.");
        setName(CGetAttribute(actorNode, "name"));

        // Type
        if (!CHasAttribute(actorNode, "type"))
            throw CException("Invalid graph, missing actor type.");
        setType(CGetAttribute(actorNode, "type"));

        // Repetitions
        if (CHasAttribute(actorNode, "repetitions"))
            setRepetitionsOfScenario(getDefaultScenario(), CGetAttribute(actorNode, "repetitions"));

        // Ports
        for (CNodePtr portNode = CGetChildNode(actorNode, "port");
             portNode != NULL; portNode = CNextNode(portNode, "port"))
        {
            // Construct port
            GraphComponent component = GraphComponent(this, getPorts().size());
            Port *p = new Port(component);
            p->constructFromXML(portNode);

            // Add port to actor
            addPort(p);
        }
    }
    /**
     * constructPropertiesFromXML ()
     * The function initializes all actor properties based on the XML data.
     */
    void Actor::constructPropertiesFromXML(Scenario *s,
                                           const CNodePtr actorPropertiesNode)
    {
        CString processorType;

        // Repetitions
        if (CHasAttribute(actorPropertiesNode, "repetitions"))
        {
            this->setRepetitionsOfScenario(s, CGetAttribute(actorPropertiesNode, "repetitions"));
        }

        // Processor definitions
        for (CNode *procNode = CGetChildNode(actorPropertiesNode, "processor");
             procNode != NULL; procNode = CNextNode(procNode, "processor"))
        {
            if (!CHasAttribute(procNode, "type"))
                throw CException("Processor must have a type");

            processorType = CGetAttribute(procNode, "type");

            // Execution time
            if (CHasChildNode(procNode, "executionTime"))
            {
                CNode *execTimeNode = CGetChildNode(procNode, "executionTime");

                if (!CHasAttribute(execTimeNode, "time"))
                    throw CException("Execution time not specified");

                setExecutionTimeOfScenario(s, processorType,
                                           CGetAttribute(execTimeNode, "time"));
            }

            // Memory
            CNode *memoryNode;
            for (memoryNode = CGetChildNode(procNode, "memory"); memoryNode != NULL;
                 memoryNode = CNextNode(memoryNode, "memory"))
            {
                // Our new memory elements
                if (CHasAttribute(memoryNode, "type"))
                {
                    CString name = CGetAttribute(memoryNode, "name");
                    CString size = CGetAttribute(memoryNode, "size");
                    CString type = CGetAttribute(memoryNode, "type");

                    Memory *m = new Memory(type, name, uint(size));
                    addMemoryForScenario(s, processorType, m);
                }

            }

            // Default processor?
            if (CHasAttribute(procNode, "default")
                && CGetAttribute(procNode, "default") == "true")
            {
                setDefaultProcessorType(processorType);
            }
        }

        // Ports
        for (CNode *portNode = CGetChildNode(actorPropertiesNode, "port");
             portNode != NULL; portNode = CNextNode(portNode, "port"))
        {
            if (!CHasAttribute(portNode, "name"))
                throw CException("No port name specified.");

            if (!CHasAttribute(portNode, "rate"))
                throw CException("No port rate specified.");

            Port *p = getPort(CGetAttribute(portNode, "name"));
            p->setRateOfScenario(s, CGetAttribute(portNode, "rate"));
        }
    }
    /**
     * convertToXML ()
     * The function converts all port properties to XML data.
     */
    void Actor::convertToXML(const CNodePtr actorNode)
    {
        // Name
        CAddAttribute(actorNode, "name", getName());

        // Type
        CAddAttribute(actorNode, "type", getType());

        // Ports
        for (Ports::const_iterator iter = ports.begin();
             iter != ports.end(); iter++)
        {
            CNode *portNode = CAddNode(actorNode, "port");
            (*iter)->convertToXML(portNode);
        }
    }
    /**
     * convertPropertiesToXML ()
     * The function converts all actor properties based to XML data.
     */
    void Actor::convertPropertiesToXML(Scenario *s,
                                       const CNodePtr actorPropertiesNode)
    {
        // Name
        CAddAttribute(actorPropertiesNode, "actor", getName());

        // Repetitions
        CAddAttribute(actorPropertiesNode, "repetitions", getRepetitionsOfScenario(s));

        // Processor definitions
        for (ProcessorTypes::iterator i = processorTypes.begin();
             i != processorTypes.end(); i++)
        {
            const ProcessorType &p = *i;

            // No properties specified for this scenario?
            if (p.executionTime.find(s) == p.executionTime.end()
                && p.memories.find(s) == p.memories.end())
            {
                continue;
            }

            // Processor node
            CNode *procNode = CAddNode(actorPropertiesNode, "processor");

            // Type
            CAddAttribute(procNode, "type", p.type);

            // Default processor?
            if (p.type == getDefaultProcessorType())
                CAddAttribute(procNode, "default", "true");

            // Execution Time
            if (p.executionTime.find(s) != p.executionTime.end())
            {
                CNode *execTimeNode = CAddNode(procNode, "executionTime");
                CAddAttribute(execTimeNode, "time",
                              getExecutionTimeOfScenario(s, p.type));
            }

            // Memory
            if (p.memories.find(s) != p.memories.end())
            {
                // Get list of memories
                const Memories *mems = &((p.memories.find(s))->second);
                for (Memories::const_iterator i = mems->begin(); i != mems->end(); i++)
                {
                    Memory *m = (*i);
                    CNode *memoryNode = CAddNode(procNode, "memory");
                    CAddAttribute(memoryNode, "name", m->getName());
                    CAddAttribute(memoryNode, "size", m->getSize());
                    CAddAttribute(memoryNode, "type", m->getType());
                }
            }
        }

        // Ports
        for (Ports::iterator i = ports.begin(); i != ports.end(); i++)
        {
            Port *p = *i;

            if (p->getRate().find(s) != p->getRate().end())
            {
                CNode *portNode = CAddNode(actorPropertiesNode, "port");
                CAddAttribute(portNode, "name", p->getName());
                CAddAttribute(portNode, "rate", p->getRateOfScenario(s));
            }
        }
    }
    /**
     * isolateScenario()
     * The function removes all scenario except scenario s from the actor.
     */
    void Actor::isolateScenario(Scenario *s)
    {
        // Iterate over all ports
        for (Ports::iterator i = ports.begin(); i != ports.end(); i++)
        {
            Port *p = *i;

            p->isolateScenario(s);
        }

        // Iterate over processor types
        for (ProcessorTypes::iterator i = processorTypes.begin();
             i != processorTypes.end(); i++)
        {
            ProcessorType &procType = *i;

            // Execution time
            Time t = getExecutionTimeOfScenario(s, procType.type);
            procType.executionTime.clear();
            setExecutionTimeOfScenario(s, procType.type, t);

            // Memories
            Memories mems = getMemoriesOfScenario(s, procType.type);
            procType.memories.clear();
            setMemoriesOfScenario(s, procType.type, mems);
        }
    }
    /**
     * createPort ()
     * Create a new port on the actor.
     */
    Port *Actor::createPort(GraphComponent &c)
    {
        Port *p = new Port(c);
        addPort(p);

        return p;
    }
    /**
     * createPort ()
     * Create a new port on the actor.
     */
    Port *Actor::createPort(const Port::PortType type)
    {
        GraphComponent c = GraphComponent(this, getPorts().size());
        Port *p = createPort(c);

        p->setType(type);

        return p;
    }
    /**
     * isConnected ()
     * The function returns true if all ports are connected to a channel, else
     * it returns false.
     */
    bool Actor::isConnected() const
    {
        for (Ports::const_iterator iter = ports.begin();
             iter != ports.end(); iter++)
        {
            const Port *p = *iter;

            if (!p->isConnected())
                return false;
        }

        return true;
    }
    /**
     * getPort ()
     * The function returns a reference to a port with the given id.
     */
    Port *Actor::getPort(const CId id)
    {
        for (Ports::iterator iter = ports.begin(); iter != ports.end(); iter++)
        {
            Port *p = *iter;

            if (p->getId() == id)
                return p;
        }

        throw CException("Actor '" + getName()
                         + "' has no port with id '" + CString(id) + "'.");
    }
    /**
     * getPort ()
     * The function returns a reference to a port with the given name.
     */
    Port *Actor::getPort(const CString &name)
    {
        for (Ports::iterator iter = ports.begin(); iter != ports.end(); iter++)
        {
            Port *p = *iter;

            if (p->getName() == name)
                return p;
        }

        throw CException("Actor '" + getName() + "' has no port '" + name + "'.");
    }
    /**
     * addPort ()
     * Add a port to an actor.
     */
    void Actor::addPort(Port *p)
    {
        ports.push_back(p);
    }
    /**
     * removePort ()
     * Remove a port from an actor and destory its memory space.
     */
    void Actor::removePort(const CString &name)
    {
        for (Ports::iterator iter = ports.begin(); iter != ports.end(); iter++)
        {
            Port *p = *iter;

            if (p->getName() == name)
            {
                delete p;
                ports.erase(iter);
                return;
            }
        }

        throw CException("Actor '" + getName() + "' has no port '" + name + "'.");
    }
    /**
     * getExecutionTimeOfScenario ()
     * The function returns the execution time of the actor in scenario s on the
     * specified processor type.
     */
    Time Actor::getExecutionTimeOfScenario(Scenario *s,
                                           const CString &processorType) const
    {
        map<Scenario *, Time>::const_iterator scenarioIter;

        // Find the properties of the specified processor
        const ProcessorType &p = getProcessorType(processorType);

        // Find the properties of the specified scenario
        scenarioIter = p.executionTime.find(s);

        // Scenario has no specific properties, use default scenario
        if (scenarioIter == p.executionTime.end())
        {
            scenarioIter = p.executionTime.find(getDefaultScenario());
        }

        // Default scenario does not exist?
        if (scenarioIter == p.executionTime.end())
        {
            throw CException("Actor '" + getName() + "' has no execution time.");
        }

        return scenarioIter->second;
    }
    /**
     * setExecutionTimeOfScenario ()
     * The function sets the execution time of the actor in scenario s on the
     * specified processor type.
     */
    void Actor::setExecutionTimeOfScenario(Scenario *s,
                                           const CString &processorType, const Time t)
    {
        processorTypes[processorType].executionTime[s] = t;
    }

    /**
     * setRepetitions ()
     * The function sets the repetition counts of all scenarios.
     */
    void Actor::setRepetitions(const map<Scenario *, uint> &r)
    {
        // Clear existing map
        repetitions.clear();

        // Iterate over the new map
        for (map<Scenario *, uint>::const_iterator i = r.begin(); i != r.end(); i++)
        {
            // Find corresponding scenario in this graph
            Scenario *s = this->getScenarioGraph()->getGraph()->getScenario(i->first->getName());
            setRepetitionsOfScenario(s, i->second);
        }
    }

    /**
     * getRepetitionsOfScenario()
     * The function returns the repetitions count of the actor in the given scenario. When no
     * specific count is specified for the scenario, the default scenario is used.
     */
    uint Actor::getRepetitionsOfScenario(Scenario *s) const
    {
        if (repetitions.find(s) != repetitions.end())
            return repetitions.find(s)->second;

        if (repetitions.find(getDefaultScenario()) == repetitions.end())
            throw CException("Actor '" + getName() + "' has no repetition count.");

        return repetitions.find(getDefaultScenario())->second;
    }

    /**
     * setRepetitionsOfScenario()
     * The function sets the repetitions count of the actor in the given scenario.
     */
    void Actor::setRepetitionsOfScenario(Scenario *s, const uint r)
    {
        repetitions[s] = r;
    }


    /**
     *
     * @param s
     * @param processorType
     * @param m
     */
    void Actor::addMemoryForScenario(Scenario *s, const CString &processorType, Memory *m)
    {
        processorTypes[processorType].memories[s].push_back(m);
    };
    /**
     * setProcessorTypes ()
     * The function sets the processor type objects of all scenarios.
     */
    void Actor::setProcessorTypes(const ProcessorTypes &p)
    {
        // Clear existing map
        processorTypes.clear();

        // Iterate over the processor types
        for (ProcessorTypes::const_iterator i = p.begin(); i != p.end(); i++)
        {
            ProcessorType t = *i;

            // Iterate over the execution times
            for (map<Scenario *, Time>::iterator j = t.executionTime.begin();
                 j != t.executionTime.end(); j++)
            {
                // Find corresponding scenario in this graph
                Scenario *s = getScenarioGraph()->getGraph()->getScenario(
                                  j->first->getName());
                processorTypes[t.type].executionTime[s] = j->second;
            }

            // Iterate over the memories
            for (map<Scenario *, Memories>::iterator j = t.memories.begin();
                 j != t.memories.end(); j++)
            {
                // Find corresponding scenario in this graph
                Scenario *s = getScenarioGraph()->getGraph()->getScenario(
                                  j->first->getName());
                processorTypes[t.type].memories[s] = j->second;
            }
        }
    }
    /**
     * getProcessorType()
     * The function returns a pointer to the processor type object of the specified
     * type.
     */
    const Actor::ProcessorType &Actor::getProcessorType(const CString &type) const
    {
        for (ProcessorTypes::const_iterator i = processorTypes.begin();
             i != processorTypes.end(); i++)
        {
            if (i->type == type)
                return *i;
        }

        throw CException("Actor '" + getName() + "' does not support processor"
                         " type '" + type + "'.");
    }
    /**
     * getDefaultScenario ()
     * The function returns a pointer to the default scenario.
     */
    Scenario *Actor::getDefaultScenario() const
    {
        return getScenarioGraph()->getGraph()->getDefaultScenario();
    }
    /**
     * print ()
     * Print the actor to the supplied output stream.
     */
    ostream &Actor::print(ostream &out)
    {
        out << "Actor (" << getName() << ")" << endl;
        out << "id:        " << getId() << endl;
        out << "type:      " << getType() << endl;

        for (Ports::iterator iter = ports.begin(); iter != ports.end(); iter++)
        {
            Port *p = *iter;

            p->print(out);
        }

        out << "default processor type: " << defaultProcessorType << endl;

        for (ProcessorTypes::iterator i = processorTypes.begin();
             i != processorTypes.end(); i++)
        {
            out << "processor type: " << i->type << endl;

            for (map<Scenario *, Time>::iterator j = i->executionTime.begin();
                 j != i->executionTime.end(); j++)
            {
                out << "exec time: " << j->second << " (scenario: ";
                out << j->first->getName() << ")" << endl;
            }

            for (map<Scenario *, Memories>::iterator j = i->memories.begin();
                 j != i->memories.end(); j++)
            {
                for (Memories::iterator m = j->second.begin(); m != j->second.end(); m++)
                {
                    out << "Memory: " << (*m)->getName() << "size: " << (*m)->getSize() << " (scenario: ";
                    out << j->first->getName() << ")" << endl;
                }
            }
        }

        out << endl;

        return out;
    }
    FSMSADF::Actor::Memories Actor::getMemoriesOfScenario(Scenario *s,
            const CString &processorType) const
    {
        map<Scenario *, Memories>::const_iterator scenarioIter;

        // Find the properties of the specified processor
        const ProcessorType &p = getProcessorType(processorType);

        // Find the properties of the specified scenario
        scenarioIter = p.memories.find(s);

        // Scenario has no specific properties, use default scenario
        if (scenarioIter == p.memories.end())
        {
            scenarioIter = p.memories.find(getDefaultScenario());
        }

        // Default scenario does not exist?
        if (scenarioIter == p.memories.end())
        {
            throw CException("Default Scenario has no memories: " + s->getName());
        }

        return (scenarioIter->second);
    }

} // End namespace FSMSADF

