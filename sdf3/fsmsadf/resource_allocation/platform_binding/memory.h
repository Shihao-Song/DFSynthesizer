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

#ifndef FSMSADF_RESOURCE_ALLOCATION_PLATFORM_BINDING_MEMORY_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_PLATFORM_BINDING_MEMORY_H_INCLUDED

#include "../platform_graph/graph.h"
#include "../../base/graph.h"
#include "constraint.h"

namespace FSMSADF
{

    // Forward class definition
    class MemoryBinding;
    class TileBinding;

    // List of memory pointers
    typedef list<MemoryBinding *> MemoryBindings;

    /**
     * MemoryBinding
     * Container for tile memory binding.
     */
    class MemoryBinding : public GraphComponent
    {
        public:
            // Actor binding

            class ActorBinding
            {
                public:
                    ActorBinding(Actor *a, const Actor::ProcessorType &t, Actor::Memory *memory, Memory *tmem)
                        : actor(a), processorType(t), actor_memory(memory), tile_memory(tmem)
                    {
                    };
                    Actor *actor;
                    Actor::ProcessorType processorType;
                    // Want to create a link between an actor memory and a Tile memory
                    Actor::Memory *actor_memory;
                    Memory *tile_memory;
            };
            typedef list<ActorBinding> ActorBindings;

            // Channel binding

            class ChannelBinding
            {
                public:
                    ChannelBinding(Channel *c, BufferLocation l, Memory *mem)
                        : channel(c), bufferLocation(l), memory(mem)
                    {
                    };
                    Channel *channel;
                    BufferLocation bufferLocation;
                    Memory *memory;
            };
            typedef list<ChannelBinding> ChannelBindings;

        public:

            // Constructor
            MemoryBinding(GraphComponent c, Memories *m);

            // Destructor
            ~MemoryBinding();

            // Construct
            MemoryBinding *clone(GraphComponent c);

            // Construct
            void constructFromXML(Scenario *s, const CNodePtr tileNode);
            void constructResourceUsageFromXML(Memory *m, const CNodePtr tileNode);

            // Convert
            void convertToXML(Scenario *s, const CNodePtr memoryNode);
            void convertResourceUsageToXML(const CNodePtr memoryNode);

            // Memory
            Memories *getMemories() const
            {
                return memories;
            };

            // Graph binding constraints
            GraphBindingConstraints *getGraphBindingConstraints() const;

            // Actor bindings
            map<Scenario *, ActorBindings> &getActorBindings()
            {
                return actorBindings;
            };
            void setActorBindings(map<Scenario *, ActorBindings> &a)
            {
                actorBindings = a;
            };
            bool addActorBinding(Scenario *s, Actor *a, const Actor::ProcessorType &t);

            void removeActorBinding(Scenario *s, const Actor *a);
            bool hasActorBinding(Scenario *s, const Actor *a) const;

            // Channel bindings
            map<Scenario *, ChannelBindings> &getChannelBindings()
            {
                return channelBindings;
            };
            void setChannelBindings(map<Scenario *, ChannelBindings> &ch)
            {
                channelBindings = ch;
            };
            bool addChannelBinding(Scenario *s, Channel *c, BufferLocation l);
            void removeChannelBinding(Scenario *s, const Channel *c);
            bool hasChannelBinding(Scenario *s, const Channel *c) const;

            // Tile binding
            TileBinding *getTileBinding() const
            {
                return (TileBinding *)(getParent());
            };

            // Resource occupancy
            Size getAvailableMemorySize(Scenario *s, Memory *m) const;
            Size getAllocatedMemorySize(Scenario *s, Memory *m) const;

            // Resource occupancy of application graph
            Size getMemorySizeUsedForGraph(Memory *mem) const;

            // Initial resource occupancy
            // Function uses find instead of [] so it can be const.
            Size getMemorySizeUsedForOtherGraphs(Memory *m) const
            {
                map<Memory *, Size>::const_iterator i = memorySizeUsedForOtherGraphs.find(m);
                if (i == memorySizeUsedForOtherGraphs.end())
                    throw CException("MemorySizeUsedForOtherGraphs not found");
                return i->second;
            };
            void setMemorySizeUsedForOtherGraphs(Memory *m, Size sz)
            {
                memorySizeUsedForOtherGraphs[m] = sz;
            };

        private:
            // List of memories on the tile
            Memories *memories;

            // Actors
            map<Scenario *, ActorBindings> actorBindings;

            // Channels
            map<Scenario *, ChannelBindings> channelBindings;

            // Memory size used for other graphs
            map<Memory *, Size> memorySizeUsedForOtherGraphs;
    };

} // End namespace FSMSADF

#endif
