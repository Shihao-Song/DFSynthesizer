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
 *      13-05-11    :   Add support for multiple memories and memory types.
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
#include "graph.h"

namespace FSMSADF
{
    /**
     * MemoryBinding()
     * Constructor.
     */
    MemoryBinding::MemoryBinding(GraphComponent c, Memories *m)
        :
        GraphComponent(c),
        memories(m)
    {
        // Create an initial used size for all memory
        for (Memories::iterator i = this->memories->begin(); i != this->memories->end(); i++)
        {
            setMemorySizeUsedForOtherGraphs(*i, (Size) 0);
        }
    }
    /**
     * ~MemoryBinding()
     * Destructor.
     */
    MemoryBinding::~MemoryBinding()
    {
    }
    /**
     * clone()
     * Create a cloned copy of this memory binding.
     */
    MemoryBinding *MemoryBinding::clone(GraphComponent c)
    {
        MemoryBinding *mb = new MemoryBinding(c, getMemories());

        // Actors
        mb->setActorBindings(getActorBindings());

        // Channels
        mb->setChannelBindings(getChannelBindings());

        // Memory size used for other graphs
        for (Memories::iterator i = memories->begin(); i != memories->end(); i++)
        {
            mb->setMemorySizeUsedForOtherGraphs(*i, getMemorySizeUsedForOtherGraphs(*i));
        }
        return mb;
    }

    /**
     * constructFromXML()
     * This function creates the memory binding as specified by the
     * connectionNode.
     */
    /// @todo fixme
    void MemoryBinding::constructFromXML(Scenario *s,
                                         const CNodePtr memoryNode)
    {
        // Application graph
        ScenarioGraph *sg = s->getScenarioGraph();

        // Iterate over the list of actors bound to this memory
        for (CNode *n = CGetChildNode(memoryNode, "actor");
             n != NULL; n = CNextNode(memoryNode, "actor"))
        {
            // Name specified for the actor?
            if (!CHasAttribute(n, "name"))
                throw CException("Mapped actor has no name identifier.");

            // Processor type specified for the actor?
            if (!CHasAttribute(n, "processorType"))
                throw CException("Mapped actor has no processorType identifier.");

            // Find actor in scenario graph
            Actor *a = sg->getActor(CGetAttribute(n, "name"));

            // Find processor type in actor
            const Actor::ProcessorType &p = a->getProcessorType(CGetAttribute(n, "processorType"));

            // Create binding of actor to memory
            addActorBinding(s, a, p);
        }

        // Iterate over the list of channels bound to this memory
        for (CNode *n = CGetChildNode(memoryNode, "channel");
             n != NULL; n = CNextNode(memoryNode, "channel"))
        {
            // Name specified for the channel?
            if (!CHasAttribute(n, "name"))
                throw CException("Mapped channel has no name identifier.");

            // Buffer type specified for the actor?
            if (!CHasAttribute(n, "bufferType"))
                throw CException("Mapped actor has no bufferType identifier.");

            // Find channel in scenario graph
            Channel *c = sg->getChannel(CGetAttribute(n, "name"));

            // Buffer location
            BufferSize b;
            BufferLocation bufferLocation = CGetAttribute(n, "bufferType");

            // Create binding of channel to memory
            addChannelBinding(s, c, bufferLocation);
        }
    }

    /**
     * constructResourceUsageFromXML()
     * This function sets the resource usgae of the memoru.
     */
    /// @todo fix me
    void MemoryBinding::constructResourceUsageFromXML(Memory *m, const CNodePtr tileNode)
    {
        // Used memory
        if (!CHasAttribute(tileNode, "size"))
            throw CException("Memoy resource usage has no size.");
        setMemorySizeUsedForOtherGraphs(m, CGetAttribute(tileNode, "size"));
    }
    /**
     * convertToXML()
     * This function converts the memory binding to an XML object.
     */
    void MemoryBinding::convertToXML(Scenario *s, const CNodePtr tileNode)
    {
        for (Memories::iterator mems = memories->begin();
             mems != memories->end(); mems++)
        {
            Memory *m = *mems;
            CNode *memoryNode = CAddNode(tileNode, "memory");
            map<Scenario *, ActorBindings>::iterator scenActorIter;
            map<Scenario *, ChannelBindings>::iterator scenChannelIter;

            // Name
            CAddAttribute(memoryNode, "name", m->getName());

            // Actor binding exists for this scenario?
            scenActorIter = actorBindings.find(s);
            if (scenActorIter != actorBindings.end())
            {
                // List of all actors bound to this memory
                for (ActorBindings::iterator i = scenActorIter->second.begin();
                     i != scenActorIter->second.end(); i++)
                {
                    /* Bound to this memory*/
                    if (i->tile_memory == m)
                    {
                        CNode *n = CAddNode(memoryNode, "actor");
                        CAddAttribute(n, "name", i->actor->getName());
                        CAddAttribute(n, "processorType", i->processorType.type);
                    }
                }
            }

            // Channel binding exists for this scenario?
            scenChannelIter = channelBindings.find(s);
            if (scenChannelIter != channelBindings.end())
            {
                // List of all channels bound to this memory
                for (ChannelBindings::iterator i = scenChannelIter->second.begin();
                     i != scenChannelIter->second.end(); i++)
                {
                    /* Bound to this memory*/
                    if (i->memory == m)
                    {
                        BufferSize b;
                        CNode *n = CAddNode(memoryNode, "channel");
                        CAddAttribute(n, "name", i->channel->getName());
                        CAddAttribute(n, "bufferType", i->bufferLocation);
                    }
                }
            }
        }
    }
    /**
     * convertResourceUsageToXML()
     * This function converts the resource usage of this memory binding to an
     * XML object.
     */
    void MemoryBinding::convertResourceUsageToXML(const CNodePtr memoryNode)
    {
        for (Memories::iterator mems = memories->begin();
             mems != memories->end(); mems++)
        {
            CNode *mem = CAddNode(memoryNode, "memory");
            Memory *m = *mems;
            // Name
            CAddAttribute(mem, "name", m->getName());
            CAddAttribute(mem, "size", getMemorySizeUsedForGraph(m)
                          + getMemorySizeUsedForOtherGraphs(m));
        }
    }
    /**
     * getGraphBindingConstraints()
     * Get the graph binding constraints associated with this binding.
     */
    GraphBindingConstraints *MemoryBinding::getGraphBindingConstraints() const
    {
        return getTileBinding()->getPlatformBinding()->getGraphBindingConstraints();
    }
    /**
     * addActorBinding()
     * The function binds actor a in scenario s to this memory. The function
     * returns true on success. Otherwise it returns false.
     */
    bool MemoryBinding::addActorBinding(Scenario *s, Actor *a,
                                        const Actor::ProcessorType &t)
    {
        //    Actor::Memory *mem;
        // Binding already exists?
        if (hasActorBinding(s, a))
            return true;
        /// @todo we need a mapping strategy. stupid greedy for now.
        Actor::Memories mem = a->getMemoriesOfScenario(s, t.type);
        for (Actor::Memories::iterator i = mem.begin();
             i != mem.end(); i++)
        {
            bool mapped = false;
            Actor::Memory *amem = *i;

            for (Memories::iterator j = memories->begin(); !mapped && j != memories->end(); j++)
            {
                Memory *memory = *j;
                if (amem->getType() == memory->getType())
                {
                    if (getAvailableMemorySize(s, memory) >= amem->getSize())
                    {
                        actorBindings[s].push_back(MemoryBinding::ActorBinding(a, t, amem, memory));
                        mapped = true;
                    }
                }
            }
            if (!mapped)
            {
                // Remove the mappings we made so far.
                removeActorBinding(s, a);
                //throw CException("Failed to create a mapping");
                return false;
            }
        }
        return true;
    }
    /**
     * removeActorBinding()
     * The function removes the binding of actor a in scenario s to this
     * memory.
     */
    void MemoryBinding::removeActorBinding(Scenario *s, const Actor *a)
    {
        for (ActorBindings::iterator i = actorBindings[s].begin();
             i != actorBindings[s].end(); i++)
        {
            if (i->actor == a)
            {
                actorBindings[s].erase(i);
                break;
            }
        }
    }
    /**
     * hasActorBinding()
     * The function returns true when a binding of actor a in scenario s to this
     * memory exists. Otherwise it returns false.
     */
    bool MemoryBinding::hasActorBinding(Scenario *s, const Actor *a) const
    {
        map<Scenario *, ActorBindings>::const_iterator scenIter;

        scenIter = actorBindings.find(s);
        if (scenIter == actorBindings.end())
            return false;

        for (ActorBindings::const_iterator i = scenIter->second.begin();
             i != scenIter->second.end(); i++)
        {
            if (i->actor == a)
                return true;
        }

        return false;
    }
    /**
     * addChannelBinding()
     * The function binds channel c in scenario s to this memory. The function
     * returns true on success. Otherwise it returns false.
     */
    bool MemoryBinding::addChannelBinding(Scenario *s, Channel *c,
                                          BufferLocation l)
    {
        ChannelBindingConstraints *cb;
        cb = getGraphBindingConstraints()->getConstraintsOfScenario(s)
             ->getConstraintsOfChannel(c);
        // Binding already exists?
        if (hasChannelBinding(s, c))
            return true;

        // Sufficient resources available?
        for (Memories::iterator j = memories->begin();
             j != memories->end(); j++)
        {
            MemoryType type(MemoryType::Shared);
            if (l == BufferLocation::Dst)
            {
                type = MemoryType::CommIn;
            }
            else if (l == BufferLocation::Src)
            {
                type = MemoryType::CommOut;
            }
            else
            {
                // The channel is placed into the memory (both sides on same tile?)
                type = MemoryType::Data;
            }
            Memory *mem = *j;
            if (mem->getType() == type || mem->getType() == MemoryType::Shared)
            {
                if (getAvailableMemorySize(s, mem) > cb->getBufferSize()[l])
                {
                    ChannelBinding cb = ChannelBinding(c, l, mem);
                    channelBindings[s].push_back(cb);
                    return true;
                }
            }
        }

        return false;
    }
    /**
     * removeChannelBinding()
     * The function removes the binding of channel c in scenario s to this
     * memory.
     */
    void MemoryBinding::removeChannelBinding(Scenario *s, const Channel *c)
    {
        for (ChannelBindings::iterator i = channelBindings[s].begin();
             i != channelBindings[s].end(); i++)
        {
            if (i->channel == c)
            {
                channelBindings[s].erase(i);
                break;
            }
        }
    }
    /**
     * hasChannelBinding()
     * The function returns true when a binding of channel c in scenario s to this
     * memory exists. Otherwise it returns false.
     */
    bool MemoryBinding::hasChannelBinding(Scenario *s, const Channel *c) const
    {
        map<Scenario *, ChannelBindings>::const_iterator scenIter;

        scenIter = channelBindings.find(s);
        if (scenIter == channelBindings.end())
            return false;

        for (ChannelBindings::const_iterator i = scenIter->second.begin();
             i != scenIter->second.end(); i++)
        {
            if (i->channel == c)
                return true;
        }

        return false;
    }
    /**
     * getAvailableMemorySize()
     * The function returns the amount of memory which is available in this memory
     * in scenario s.
     */
    Size MemoryBinding::getAvailableMemorySize(Scenario *s, Memory *m) const
    {
        return (m->getSize()
                - getAllocatedMemorySize(s, m)
                - getMemorySizeUsedForOtherGraphs(m));
    }
    /**
     * getAllocatedMemorySize()
     * The function returns the amount of memory which has been allocated to the
     * application in scenario s.
     */
    Size MemoryBinding::getAllocatedMemorySize(Scenario *s, Memory *m) const
    {
        map<Scenario *, ChannelBindings>::const_iterator channelScenIter;
        map<Scenario *, ActorBindings>::const_iterator actorScenIter;
        Size szActors = 0;
        Size szChannels = 0;

        /// @todo Actors require max memory size off all bound actors to this memory within sceanrio s
        // Actors require max state size of all bound actors within scenario s
        actorScenIter = actorBindings.find(s);
        if (actorScenIter != actorBindings.end())
        {
            MemoryBinding::ActorBindings b = actorScenIter->second;
            ActorBindings::const_iterator i = b.begin();
            while (i != b.end())
            {
                const MemoryBinding::ActorBinding *c = &(*i);
                if (m == (c->tile_memory))
                {
                    szActors = MAX(szActors, c->actor_memory->getSize());
                }
                i++;
            }
        }

        // Channels require sum of requirements per channel
        channelScenIter = channelBindings.find(s);
        if (channelScenIter != channelBindings.end())
        {
            for (ChannelBindings::const_iterator i = channelScenIter->second.begin();
                 i != channelScenIter->second.end(); i++)
            {
                ChannelBindingConstraints *cb;
                Channel *c = i->channel;
                BufferLocation l = i->bufferLocation;
                cb = getGraphBindingConstraints()->getConstraintsOfScenario(s)
                     ->getConstraintsOfChannel(c);
                if (i->memory == m)
                {
                    szChannels += cb->getBufferSize()[l];
                }
            }
        }

        return szActors + szChannels;
    }
    /**
     * getMemorySizeUsedForGraph()
     * The function returns the maximal amount of memory which has been allocated
     * to the application in any of its scenarios.
     */
    Size MemoryBinding::getMemorySizeUsedForGraph(Memory *mem) const
    {
        // Application graph
        Graph *g = getTileBinding()->getPlatformBinding()->getApplicationGraph();
        Size maxSize = 0;

        for (Scenarios::iterator i = g->getScenarios().begin();
             i != g->getScenarios().end(); i++)
        {
            Size size = getAllocatedMemorySize(*i, mem);
            maxSize = MAX(maxSize, size);
        }

        return maxSize;
    }

} // End namespace

