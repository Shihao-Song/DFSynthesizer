/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   actor.h
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
 * $Id: actor.h,v 1.1.2.7 2010-04-22 07:07:10 mgeilen Exp $
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

#ifndef FSMSADF_BASE_ACTOR_H_INCLUDED
#define FSMSADF_BASE_ACTOR_H_INCLUDED

#include "port.h"
#include "type.h"

namespace FSMSADF
{

    // Forward class definition
    class Actor;

    // List of actor pointers
    typedef list<Actor *> Actors;

    /**
     * Actor
     * Actor in FSM-based SADF graph
     */
    class Actor : public GraphComponent
    {
        public:

            class Memory
            {
                public:
                    Memory(const MemoryType &type, const CString name, const Size size) :
                        type(type),
                        name(name),
                        size(size)
                    {
                    };
                    // type of memory
                    MemoryType getType()
                    {
                        return type;
                    };
                    // Get the name
                    CString getName()
                    {
                        return name;
                    };
                    // Max size
                    Size getSize()
                    {
                        return size;
                    };
                private:
                    // type of memory
                    MemoryType type;
                    // Name of memory
                    CString name;
                    // Max req. size
                    Size size;
            };
            typedef list<Actor::Memory *> Memories;

            class ProcessorType
            {
                public:
                    // Constructor
                    ProcessorType(const CString &t) : type(t)
                    {
                    };

                    // Type
                    CString type;

                    // Execution time
                    map<Scenario *, Time> executionTime;

                    // Memory spaces required by the actor.
                    map<Scenario *, Memories> memories;
            };

            class ProcessorTypes : public list<ProcessorType>
            {
                public:
                    ProcessorType &operator[](const CString &type)
                    {
                        for (list<ProcessorType>::iterator i = begin(); i != end(); i++)
                            if (i->type == type)
                                return *i;
                        push_back(ProcessorType(type));
                        return back();
                    };
            };

        public:

            // Constructor
            Actor(GraphComponent c);

            // Desctructor
            ~Actor();

            // Construct
            Actor *create(GraphComponent c) const;
            Actor *createCopy(GraphComponent c) const;
            Actor *clone(GraphComponent c) const;

            // Construct
            void constructFromXML(const CNodePtr actorNode);
            void constructPropertiesFromXML(Scenario *s,
                                            const CNodePtr actorPropertiesNode);

            // Convert
            void convertToXML(const CNodePtr actorNode);
            void convertPropertiesToXML(Scenario *s,
                                        const CNodePtr actorPropertiesNode);

            // Isolate scenario
            void isolateScenario(Scenario *s);

            // Type
            CString getType() const
            {
                return type;
            };
            void setType(const CString &t)
            {
                type = t;
            };

            // Ports
            Ports &getPorts()
            {
                return ports;
            };
            Port *getPort(const CId id);
            Port *getPort(const CString &name);
            void addPort(Port *p);
            void removePort(const CString &name);
            Port *createPort(GraphComponent &c);
            Port *createPort(const Port::PortType type);

            // All ports connected to a channel?
            bool isConnected() const;

            // Execution time
            Time getExecutionTimeOfScenario(Scenario *s, const CString &processorType) const;
            void setExecutionTimeOfScenario(Scenario *s, const CString &processorType, const Time t);

            // Get memories
            Memories getMemoriesOfScenario(Scenario *s, const CString &processorType) const;
            void setMemoriesOfScenario(Scenario *s, const CString &processsortype, Memories m)
            {
                processorTypes[processsortype].memories[s] = m;
            }

            void addMemoryForScenario(Scenario *s, const CString &processorType, Memory *m);

            // Processor type
            const ProcessorTypes &getProcessorTypes() const
            {
                return processorTypes;
            };
            void setProcessorTypes(const ProcessorTypes &p);
            const ProcessorType &getProcessorType(const CString &type) const;

            // get/set repetitions for weakly consistent scenarios
            const map<Scenario *, uint> &getRepetitions() const
            {
                return repetitions;
            };
            void setRepetitions(const map<Scenario *, uint> &r);
            uint getRepetitionsOfScenario(Scenario *s) const;
            void setRepetitionsOfScenario(Scenario *s, const uint r);

            // Default processor type
            const CString &getDefaultProcessorType() const
            {
                return defaultProcessorType;
            };
            void setDefaultProcessorType(const CString &t)
            {
                defaultProcessorType = t;
            };

            // Graph
            ScenarioGraph *getScenarioGraph() const
            {
                return (ScenarioGraph *) getParent();
            };

            // Print
            ostream &print(ostream &out);
            friend ostream &operator<<(ostream &out, Actor &a)
            {
                return a.print(out);
            };

        private:
            // Default scenario
            Scenario *getDefaultScenario() const;

        private:
            // Type
            CString type;

            // Ports
            Ports ports;

            // Processor types
            ProcessorTypes processorTypes;

            // Default processor type
            CString defaultProcessorType;

            // Repetitions
            map<Scenario *, uint> repetitions;
    };

} // End namespace FSMSADF

#endif
