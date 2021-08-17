/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   graph_binding.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 25, 2009
 *
 *  Function        :   Graph binding
 *
 *  History         :
 *      25-05-09    :   Initial version.
 *      19-04-11    :   Add accessors (M. Koedam)
 *                      Virtual functions, rename to SDF3FlowBase
 *
 * $Id: graph_binding.h,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_FLOW_TILE_GRAPH_BINDING_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_FLOW_TILE_GRAPH_BINDING_H_INCLUDED

#include "../../../platform_binding/graph.h"

namespace FSMSADF
{

    // Forward class definitions
    class ActorBinding;
    class ChannelBinding;
    class GraphBindingBase;

    // List of actor bindings
    typedef list<ActorBinding> ActorBindings;

    // List of channel bindings
    typedef list<ChannelBinding> ChannelBindings;

    // List of graph bindings
    typedef list<GraphBindingBase *> GraphBindings;

    /**
     * Actor binding
     */
    class ActorBinding
    {
        public:
            // Constructor
            ActorBinding(Processor *p = NULL, Memory *m = NULL, Actor::Memory *am = NULL)
            {
                processor = p;
                tile_memory = m;
                actor_memory = am;
            };

        public:
            Processor *processor;
            // We bind Actor::Memory to a Memory
            Memory *tile_memory;
            Actor::Memory *actor_memory;
    };

    /**
     * Channel binding
     */
    class ChannelBinding
    {
        public:
            // Constructor
            ChannelBinding(Memory *m = NULL)
            {
                memoryMem = m;
                memorySrc = NULL;
                memoryDst = NULL;
                networkInterfaceIn = NULL;
                networkInterfaceOut = NULL;
                connection = NULL;
            };

            // Constructor
            ChannelBinding(Memory *mSrc, NetworkInterface *nSrc,
                           Memory *mDst, NetworkInterface *nDst, Connection *c)
            {
                memoryMem = NULL;
                memorySrc = mSrc;
                memoryDst = mDst;
                networkInterfaceOut = nSrc;
                networkInterfaceIn = nDst;
                connection = c;
            };

        public:
            Memory *memoryMem;
            Memory *memorySrc;
            Memory *memoryDst;
            NetworkInterface *networkInterfaceIn;
            NetworkInterface *networkInterfaceOut;
            Connection *connection;
    };

    /**
     * class GraphBinding
     * This class annotates the platform binding with information used by the
     * tile binding algorithm.
     */
    class GraphBindingBase
    {
        public:
            class ParetoQuantities
            {
                public:
                    // Pareto dominance
                    bool dominates(const ParetoQuantities &q) const;

                    // Euclidean distance
                    double distance(const ParetoQuantities &q) const;

                    // Print quantities
                    ostream &print(ostream &out) const;

                public:
                    // Quantities
                    map<Processor *, double> processingLoad;
                    map<Memory *, double> memoryLoad;
                    map<NetworkInterface *, double> communicationLoad;
            };

        public:
            // Constructor
            GraphBindingBase(PlatformBinding *b);

            // Destructor
            virtual ~GraphBindingBase();

            // Constructor
            virtual GraphBindingBase *clone() const;

            // Platform binding
            PlatformBinding *getPlatformBinding() const
            {
                return platformBinding;
            };

            // Actors bindings
            map<Scenario *, map<Actor *, ActorBinding> > &getActorBindings()
            {
                return actorBindings;
            };

            // Channel bindings
            map<Scenario *, map<Channel *, ChannelBinding> > &getChannelBindings()
            {
                return channelBindings;
            } ;

            // Bind actor to tile resources
            virtual bool bindActorToTile(Scenario *s, Actor *a, ActorBinding &b);

            // Remove binding of actor to tile resources
            virtual void removeActorToTileBinding(Scenario *s, Actor *a);

            // Bind channel to tile resources
            virtual bool bindChannelToTile(Scenario *s, Channel *c, ChannelBinding &b);

            // Bind channel to connection resources
            virtual bool bindChannelToConnection(Scenario *s, Channel *c, ChannelBinding &b);

            // Remove binding of channel to platfrom resources
            virtual void removeChannelBinding(Scenario *s, Channel *c);

            // Minimum TDMA time slices on processors
            map<Processor *, map<Scenario *, Time> > &getMinTDMAslices()
            {
                return minTDMAslices;
            };
            Time getMinTDMAslice(Scenario *s, Processor *p) const;
            void setMinTDMAslice(Scenario *s, Processor *p, Time t);

            // Maximum TDMA time slices on processors
            map<Processor *, map<Scenario *, Time> > &getMaxTDMAslices()
            {
                return maxTDMAslices;
            };

            Time getMaxTDMAslice(Scenario *s, Processor *p) const;
            void setMaxTDMAslice(Scenario *s, Processor *p, Time t);

            // Allocate TDMA time slice on processor
            bool allocateTDMAslice(Scenario *s, Processor *p, Time t);

            // Get allocated TDMA time slice on processor
            Time getAllocatedTDMAslice(Scenario *s, Processor *p);

            // Pareto quantities
            void computeParetoQuantities(
                const map<Scenario *, RepetitionVector> &repetitionVectors,
                const bool processingLoadBasedOnTDMAslices);
            const ParetoQuantities &getParetoQuantities() const
            {
                return paretoQuantities;
            };
            bool paretoDominates(const GraphBindingBase *g) const
            {
                return paretoQuantities.dominates(g->getParetoQuantities());
            };

        private:
            // Extract application binding from platform binding
            void extractApplicationBindingFromPlatformBinding();

            // Get platform binding object associated with platform object
            ProcessorBinding *getProcessorBinding(Processor *p) const;
            MemoryBinding *getMemoryBinding(Memory *m) const;
            NetworkInterfaceBinding *getNetworkInterfaceBinding(
                NetworkInterface *n) const;
            ConnectionBinding *getConnectionBinding(Connection *c) const;

        protected:
            void copyInternalsTo(GraphBindingBase *gb) const;
        private:

            // Platform binding
            PlatformBinding *platformBinding;

            // Actors bindings
            map<Scenario *, map<Actor *, ActorBinding> > actorBindings;

            // Channel bindings
            map<Scenario *, map<Channel *, ChannelBinding> > channelBindings;

            // TDMA bounds
            map<Processor *, map<Scenario *, Time> > minTDMAslices;
            map<Processor *, map<Scenario *, Time> > maxTDMAslices;

            // Pareto quantities
            ParetoQuantities paretoQuantities;
    };

} // End namespace FSMSADF

#endif
