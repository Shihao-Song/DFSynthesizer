/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   graph_binding.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 25, 2009
 *
 *  Function        :   Application graph binding
 *
 *  History         :
 *      25-05-09    :   Initial version.
 *      19-04-11    :   Add accessors (M. Koedam)
 *                      Virtual functions, rename to SDF3FlowBase
 *
 * $Id: graph_binding.cc,v 1.1 2009-12-23 13:37:22 sander Exp $
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

#include "graph_binding.h"

namespace FSMSADF
{

    /**
     * dominates()
     * The function returns true when this Pareto point dominates the Pareto point
     * q. Otherwise the function returns false.
     */
    bool GraphBindingBase::ParetoQuantities::dominates(const ParetoQuantities &q) const
    {
        for (map<Processor *, double>::const_iterator i = processingLoad.begin();
             i != processingLoad.end(); i++)
        {
            map<Processor *, double>::const_iterator j;

            j = q.processingLoad.find(i->first);
            ASSERT(j != q.processingLoad.end(), "Unequal dimensions.");

            if (i->second > j->second)
                return false;
        }

        for (map<Memory *, double>::const_iterator i = memoryLoad.begin();
             i != memoryLoad.end(); i++)
        {
            map<Memory *, double>::const_iterator j;

            j = q.memoryLoad.find(i->first);
            ASSERT(j != q.memoryLoad.end(), "Unequal dimensions.");

            if (i->second > j->second)
                return false;
        }

        for (map<NetworkInterface *, double>::const_iterator
             i = communicationLoad.begin(); i != communicationLoad.end(); i++)
        {
            map<NetworkInterface *, double>::const_iterator j;

            j = q.communicationLoad.find(i->first);
            ASSERT(j != q.communicationLoad.end(), "Unequal dimensions.");

            if (i->second > j->second)
                return false;
        }

        return true;
    }

    /**
     * distance()
     * The function returns the euclidean distance between this Pareto point and q.
     */
    double GraphBindingBase::ParetoQuantities::distance(const ParetoQuantities &q) const
    {
        double d = 0;

        for (map<Processor *, double>::const_iterator i = processingLoad.begin();
             i != processingLoad.end(); i++)
        {
            map<Processor *, double>::const_iterator j;

            j = q.processingLoad.find(i->first);
            ASSERT(j != q.processingLoad.end(), "Unequal dimensions.");

            d = pow(i->second - j->second, 2);
        }

        for (map<Memory *, double>::const_iterator i = memoryLoad.begin();
             i != memoryLoad.end(); i++)
        {
            map<Memory *, double>::const_iterator j;

            j = q.memoryLoad.find(i->first);
            ASSERT(j != q.memoryLoad.end(), "Unequal dimensions.");

            d = pow(i->second - j->second, 2);
        }

        for (map<NetworkInterface *, double>::const_iterator
             i = communicationLoad.begin(); i != communicationLoad.end(); i++)
        {
            map<NetworkInterface *, double>::const_iterator j;

            j = q.communicationLoad.find(i->first);
            ASSERT(j != q.communicationLoad.end(), "Unequal dimensions.");

            d = pow(i->second - j->second, 2);
        }

        return d;
    }

    /**
     * print()
     * Print quantities to the output stream.
     */
    ostream &GraphBindingBase::ParetoQuantities::print(ostream &out) const
    {
        out << "processing:" << endl;
        for (map<Processor *, double>::const_iterator i = processingLoad.begin();
             i != processingLoad.end(); i++)
        {
            out << "  " << i->first->getTile()->getName() << ".";
            out << i->first->getName() << " -> " << i->second << endl;
        }

        out << "memory:" << endl;
        for (map<Memory *, double>::const_iterator i = memoryLoad.begin();
             i != memoryLoad.end(); i++)
        {
            out << "  " << i->first->getTile()->getName() << ".";
            out << i->first->getName() << " -> " << i->second << endl;
        }

        out << "communication:" << endl;
        for (map<NetworkInterface *, double>::const_iterator
             i = communicationLoad.begin(); i != communicationLoad.end(); i++)
        {
            out << "  " << i->first->getTile()->getName() << ".";
            out << i->first->getName() << " -> " << i->second << endl;
        }

        return out;
    }

    /**
     * GraphBinding()
     * Constructor. Note that the constructor creates a clode of the platform
     * binding. This clone is automatically destroyed when the GraphBinding
     * destructor is called. A pointer to the cloned platfrom binding can be
     * retrieved using the function getPlatformBinding.
     */
    GraphBindingBase::GraphBindingBase(PlatformBinding *b)
        :
        platformBinding(b->clone(*b))
    {
        // Extract existing application binding from the platform binding
        extractApplicationBindingFromPlatformBinding();
    }

    /**
     * ~GraphBinding()
     * Destructor.
     */
    GraphBindingBase::~GraphBindingBase()
    {
        delete platformBinding;
    }

    /**
     * copyInternals()
     * Copies the internal values from this to gb
     * @param gb the #GraphBindingBase that is going to get the copied values.
     */

    void GraphBindingBase::copyInternalsTo(GraphBindingBase *gb) const
    {
        // Actors bindings
        gb->actorBindings = actorBindings;

        // Channel bindings
        gb->channelBindings = channelBindings;

        // TDMA bounds
        gb->minTDMAslices = minTDMAslices;
        gb->maxTDMAslices = maxTDMAslices;

        // Pareto quantities
        gb->paretoQuantities = paretoQuantities;
    }
    /**
     * clone()
     * Create a clone of this graph binding. The cloned copy will contain a cloned
     * platfom binding object.
     */
    GraphBindingBase *GraphBindingBase::clone() const
    {
        GraphBindingBase *gb = new GraphBindingBase(platformBinding);

        this->copyInternalsTo(gb);
        return gb;
    }

    /**
     * extractApplicationBindingFromPlatformBinding()
     * The function extracts the application to platform binding from the platform
     * binding object.
     */
    void GraphBindingBase::extractApplicationBindingFromPlatformBinding()
    {
        // Iterate over tiles in the platform
        for (TileBindings::iterator i = platformBinding->getTileBindings().begin();
             i != platformBinding->getTileBindings().end(); i++)
        {
            TileBinding *tb = *i;

            // Iterate over the processor bindings in the tile binding
            for (ProcessorBindings::iterator j = tb->getProcessorBindings().begin();
                 j != tb->getProcessorBindings().end(); j++)
            {
                ProcessorBinding *pb = *j;

                // Iterate over the actors bound to this processor
                for (map<Scenario *, Actors>::iterator k =
                         pb->getActorBindings().begin();
                     k != pb->getActorBindings().end(); k++)
                {
                    Scenario *s = k->first;

                    // Iterate over the actor
                    for (Actors::iterator l = k->second.begin();
                         l != k->second.end(); l++)
                    {
                        Actor *a = *l;

                        actorBindings[s][a].processor = pb->getProcessor();
                    }
                }
            }

            // Iterate over the memory bindings in the tile binding
            //        for (MemoryBindings::iterator j = tb->getMemoryBindings().begin();
            //                j != tb->getMemoryBindings().end(); j++)
            {
                MemoryBinding *mb = tb->getMemoryBinding();

                // Iterate over the actors bound to this memory
                for (map<Scenario *, MemoryBinding::ActorBindings>::iterator
                     k = mb->getActorBindings().begin();
                     k != mb->getActorBindings().end(); k++)
                {
                    Scenario *s = k->first;

                    for (MemoryBinding::ActorBindings::iterator
                         l = k->second.begin(); l != k->second.end(); l++)
                    {
                        Actor *a = l->actor;


                        actorBindings[s][a].actor_memory = l->actor_memory;
                        actorBindings[s][a].tile_memory = l->tile_memory;
                    }
                }

                // Iterate over the channels bound to this memory
                for (map<Scenario *, MemoryBinding::ChannelBindings>::iterator
                     k = mb->getChannelBindings().begin();
                     k != mb->getChannelBindings().end(); k++)
                {
                    Scenario *s = k->first;

                    for (MemoryBinding::ChannelBindings::iterator
                         l = k->second.begin(); l != k->second.end(); l++)
                    {
                        Channel *c = l->channel;

                        if (l->bufferLocation == BufferLocation::Mem)
                            channelBindings[s][c].memoryMem = l->memory;//mb->getMemory();
                        else if (l->bufferLocation == BufferLocation::Src)
                            channelBindings[s][c].memorySrc = l->memory;//mb->getMemory();
                        else if (l->bufferLocation == BufferLocation::Dst)
                            channelBindings[s][c].memoryDst = l->memory;//mb->getMemory();
                    }
                }
            }

            // Iterate over the network interface bindings in the tile
            for (NetworkInterfaceBindings::iterator
                 j = tb->getNetworkInterfaceBindings().begin();
                 j != tb->getNetworkInterfaceBindings().end(); j++)
            {
                NetworkInterfaceBinding *nb = *j;

                // Iterate over the input channel bindings
                for (map<Scenario *, Channels>::iterator
                     k = nb->getInChannelBindings().begin();
                     k != nb->getInChannelBindings().end(); k++)
                {
                    Scenario *s = k->first;

                    // Iterate over the channels
                    for (Channels::iterator l = k->second.begin();
                         l != k->second.end(); l++)
                    {
                        Channel *c = *l;

                        channelBindings[s][c].networkInterfaceIn
                            = nb->getNetworkInterface();
                    }
                }

                // Iterate over the output channel bindings
                for (map<Scenario *, Channels>::iterator
                     k = nb->getOutChannelBindings().begin();
                     k != nb->getOutChannelBindings().end(); k++)
                {
                    Scenario *s = k->first;

                    // Iterate over the channels
                    for (Channels::iterator l = k->second.begin();
                         l != k->second.end(); l++)
                    {
                        Channel *c = *l;

                        channelBindings[s][c].networkInterfaceOut
                            = nb->getNetworkInterface();
                    }
                }
            }
        }

        // Iterate over the connections in the platform
        for (ConnectionBindings::iterator
             i = platformBinding->getConnectionBindings().begin();
             i != platformBinding->getConnectionBindings().end(); i++)
        {
            ConnectionBinding *cb = *i;

            // Iterate over the channel bindings
            for (map<Scenario *, Channels>::iterator
                 j = cb->getChannelBindings().begin();
                 j != cb->getChannelBindings().end(); j++)
            {
                Scenario *s = j->first;

                // Iterate over the channels
                for (Channels::iterator k = j->second.begin();
                     k != j->second.end(); k++)
                {
                    Channel *c = *k;

                    channelBindings[s][c].connection = cb->getConnection();
                }
            }
        }
    }

    /**
     * bindActorToTile()
     * The function binds actor a in scenario s to a processor and memory. The
     * function returns true when a binding has been made or false when the binding
     * fails.
     */
    bool GraphBindingBase::bindActorToTile(Scenario *s, Actor *a, ActorBinding &b)
    {
        ProcessorBinding *pb = getProcessorBinding(b.processor);
        MemoryBinding *mb = getMemoryBinding(b.tile_memory);

        // Processor and memory do not belong to the same tile?
        if (b.processor->getTile() != b.tile_memory->getTile())
            throw CException("Not all elements belong to the same tile.");

        // Bind actor to resources
        if (!pb->addActorBinding(s, a)
            || !mb->addActorBinding(s, a, b.processor->getType()))
        {
            // Binding failed, remove partial binding
            pb->removeActorBinding(s, a);
            mb->removeActorBinding(s, a);

            return false;
        }

        // Update the binding state
        actorBindings[s][a] = b;

        return true;
    }

    /**
     * removeActorToTileBinding()
     * The function removes the binding of actor a in scenario s.
     */
    void GraphBindingBase::removeActorToTileBinding(Scenario *s, Actor *a)
    {
        // Remove binding of actor to processor
        getProcessorBinding(actorBindings[s][a].processor)->removeActorBinding(s, a);

        // Remove binding of actor to memory
        getMemoryBinding(actorBindings[s][a].tile_memory)->removeActorBinding(s, a);

        // Update the binding state
        actorBindings[s].erase(a);
    }

    /**
     * bindChannelToTile()
     * The function binds channel c in scenario s to memory m. The function returns
     * true when a binding has been made or false when the binding fails.
     */
    bool GraphBindingBase::bindChannelToTile(Scenario *s, Channel *c, ChannelBinding &b)
    {
        MemoryBinding *mb = getMemoryBinding(b.memoryMem);

        // Bind channel to memory
        if (!mb->addChannelBinding(s, c, BufferLocation::Mem))
            return false;

        // Update the binding state
        channelBindings[s][c] = b;

        return true;
    }

    /**
     * bindChannelToConnection()
     * The function binds channel c in scenario s to the connection binding cb. The
     * function allocates also resources in the source and destination tile (i.e.
     * on the memory and network interfaces). The function returns true when a
     * binding has been made or false when the binding fails.
     */
    bool GraphBindingBase::bindChannelToConnection(Scenario *s, Channel *c,
            ChannelBinding &b)
    {
        NetworkInterfaceBinding *nbSrc, *nbDst;
        MemoryBinding *mbSrc, *mbDst;
        ConnectionBinding *cb;

        mbSrc = getMemoryBinding(b.memorySrc);
        nbSrc = getNetworkInterfaceBinding(b.networkInterfaceOut);
        mbDst = getMemoryBinding(b.memoryDst);
        nbDst = getNetworkInterfaceBinding(b.networkInterfaceIn);
        cb = getConnectionBinding(b.connection);

        // Memory and network interface do not belong to the same tile?
        if (b.memorySrc->getTile()
            != nbSrc->getNetworkInterface()->getTile()
            || b.memoryDst->getTile()
            != nbDst->getNetworkInterface()->getTile())
        {
            throw CException("Memory and network interface do not belong to the "
                             "same tile.");
        }

        // Bind channel to resources
        if (!cb->addChannelBinding(s, c)
            || !mbSrc->addChannelBinding(s, c, BufferLocation::Src)
            || !nbSrc->addOutChannelBinding(s, c)
            || !mbDst->addChannelBinding(s, c, BufferLocation::Dst)
            || !nbDst->addInChannelBinding(s, c))
        {
            // Binding failed, remove partial binding
            cb->removeChannelBinding(s, c);
            mbSrc->removeChannelBinding(s, c);
            nbSrc->removeOutChannelBinding(s, c);
            mbDst->removeChannelBinding(s, c);
            nbDst->removeInChannelBinding(s, c);

            return false;
        }

        // Update binding state
        channelBindings[s][c] = b;

        return true;
    }

    /**
     * removeChannelBinding()
     * The function removes the binding of channel c in scenario s.
     */
    void GraphBindingBase::removeChannelBinding(Scenario *s, Channel *c)
    {
        // Remove binding of channel to connection (if exists)
        if (channelBindings[s][c].connection != NULL)
        {
            // Connection binding
            getConnectionBinding(channelBindings[s][c].connection)
            ->removeChannelBinding(s, c);

            // Remove binding of channel to network interface input
            getNetworkInterfaceBinding(channelBindings[s][c].networkInterfaceIn)
            ->removeInChannelBinding(s, c);

            // Remove binding of channel to network interface outputs
            getNetworkInterfaceBinding(channelBindings[s][c].networkInterfaceOut)
            ->removeInChannelBinding(s, c);
        }

        // Remove binding of channel to memories
        if (channelBindings[s][c].memoryMem != NULL)
        {
            getMemoryBinding(channelBindings[s][c].memoryMem)
            ->removeChannelBinding(s, c);
        }
        if (channelBindings[s][c].memorySrc != NULL)
        {
            getMemoryBinding(channelBindings[s][c].memorySrc)
            ->removeChannelBinding(s, c);
        }
        if (channelBindings[s][c].memoryDst != NULL)
        {
            getMemoryBinding(channelBindings[s][c].memoryDst)
            ->removeChannelBinding(s, c);
        }

        // Update binding state
        channelBindings[s].erase(c);
    }

    /**
     * getProcessorBinding()
     * The function returns a pointer to the processor binding in the platform
     * binding which is associated with processor p.
     */
    ProcessorBinding *GraphBindingBase::getProcessorBinding(Processor *p) const
    {
        return platformBinding->getTileBinding(p->getTile())
               ->getProcessorBinding(p);
    }

    /**
     * getMemoryBinding()
     * The function returns a pointer to the memory binding in the platform
     * binding which is associated with memory m.
     */
    MemoryBinding *GraphBindingBase::getMemoryBinding(Memory *m) const
    {
        return platformBinding->getTileBinding(m->getTile())->getMemoryBinding();
    }

    /**
     * getNetworkInterfaceBinding()
     * The function returns a pointer to the network interface binding in the
     * platform binding which is associated with network interface n.
     */
    NetworkInterfaceBinding *GraphBindingBase::getNetworkInterfaceBinding(
        NetworkInterface *n) const
    {
        return platformBinding->getTileBinding(n->getTile())
               ->getNetworkInterfaceBinding(n);

    }

    /**
     * getConnectionBinding()
     * The function returns a pointer to the connection binding in the platform
     * binding which is associated with connection c.
     */
    ConnectionBinding *GraphBindingBase::getConnectionBinding(Connection *c) const
    {
        return platformBinding->getConnectionBinding(c);
    }

    /**
     * getMinTDMAslice()
     * The function returns the minimum time slice for processor p in scenario s.
     * When no minimum has been set, the function returns zero.
     */
    Time GraphBindingBase::getMinTDMAslice(Scenario *s, Processor *p) const
    {
        map<Processor *, map<Scenario *, Time> >::const_iterator i;

        // Processor known?
        i = minTDMAslices.find(p);
        if (i != minTDMAslices.end())
        {
            map<Scenario *, Time>::const_iterator j = i->second.find(s);

            // Scenario known?
            if (j != i->second.end())
                return j->second;
        }

        return 0;
    }

    /**
     * setMinTDMAslice()
     * Set the minimum TDMA slice on procssor p in scenario s to t time units.
     */
    void GraphBindingBase::setMinTDMAslice(Scenario *s, Processor *p, Time t)
    {
        minTDMAslices[p][s] = t;
    }

    /**
     * getMaxTDMAslice()
     * The function returns the maximum time slice for processor p in scenario s.
     * When no maximum has been set, the function returns the available
     * time wheel size on processor p.
     */
    Time GraphBindingBase::getMaxTDMAslice(Scenario *s, Processor *p) const
    {
        map<Processor *, map<Scenario *, Time> >::const_iterator i;

        // Processor known?
        i  = maxTDMAslices.find(p);
        if (i != maxTDMAslices.end())
        {
            map<Scenario *, Time>::const_iterator j = i->second.find(s);

            // Scenario known?
            if (j != i->second.end())
                return j->second;
        }

        return getProcessorBinding(p)->getAvailableWheelsize(s);
    }

    /**
     * setMaxTDMAslice()
     * Set the maximum TDMA slice on processor p in scenario s to t time units.
     */
    void GraphBindingBase::setMaxTDMAslice(Scenario *s, Processor *p, Time t)
    {
        maxTDMAslices[p][s] = t;
    }

    /**
     * allocateTDMAslice()
     * Allocate TDMA time slice t on processor p in scenario s.
     */
    bool GraphBindingBase::allocateTDMAslice(Scenario *s, Processor *p, Time t)
    {
        return getProcessorBinding(p)->allocateWheelsize(s, t);
    }

    /**
     * getAllocatedTDMAslice()
     * Get the allocated TDMA time slice on processor p in scenario s.
     */
    Time GraphBindingBase::getAllocatedTDMAslice(Scenario *s, Processor *p)
    {
        return getProcessorBinding(p)->getAllocatedWheelsize(s);
    }

    /**
     * computeParetoQuantities()
     * This function computes the values of the pareto quantanties of this
     * graph binding.
     */
    void GraphBindingBase::computeParetoQuantities(
        const map<Scenario *, RepetitionVector> &repetitionVectors,
        const bool processingLoadBasedOnTDMAslices)
    {
        double sumProcessingLoad = 0;

        // Clear existing Pareto quantities
        paretoQuantities.processingLoad.clear();
        paretoQuantities.memoryLoad.clear();
        paretoQuantities.communicationLoad.clear();

        // Iterate over all tiles in the platform binding
        for (TileBindings::iterator i = platformBinding->getTileBindings().begin();
             i != platformBinding->getTileBindings().end(); i++)
        {
            TileBinding *tb = *i;

            // Iterate over the processor bindings
            for (ProcessorBindings::iterator j = tb->getProcessorBindings().begin();
                 j != tb->getProcessorBindings().end(); j++)
            {
                ProcessorBinding *pb = *j;
                Processor *p = pb->getProcessor();

                // Initialize processing load at zero
                paretoQuantities.processingLoad[p] = 0;

                // Compute processing load using TDMA slice allocations?
                if (processingLoadBasedOnTDMAslices)
                {
                    // Iterate over the TDMA slice allocations of this processor
                    for (map<Scenario *, Time>::iterator
                         k = pb->getWheelsizeAllocations().begin();
                         k != pb->getWheelsizeAllocations().end(); k++)
                    {
                        Time sz = k->second;

                        // Add slice size of this scenario to the Pareto quantity
                        paretoQuantities.processingLoad[p] += sz;
                    }

                    // Normalize the Pareto quantity over the product of the wheel
                    // size and number of scenarios in wich it is used
                    paretoQuantities.processingLoad[p] =
                        paretoQuantities.processingLoad[p]
                        / (pb->getWheelsizeAllocations().size()
                           * p->getWheelsize());
                }
                else
                {
                    // Iterate over the actors bound to this processor
                    for (map<Scenario *, Actors>::iterator
                         k = pb->getActorBindings().begin();
                         k != pb->getActorBindings().end(); k++)
                    {
                        Scenario *s = k->first;

                        // Iterate over the actors
                        for (Actors::iterator l = k->second.begin();
                             l != k->second.end(); l++)
                        {
                            Actor *a = *l;

                            paretoQuantities.processingLoad[p] =
                                paretoQuantities.processingLoad[p]
                                + (a->getExecutionTimeOfScenario(s, p->getType())
                                   * repetitionVectors.at(s).at(a->getId()));
                        }
                    }
                }

                sumProcessingLoad = sumProcessingLoad
                                    + paretoQuantities.processingLoad[p];
            }

            // Iterate over the memory bindings
            //        for (MemoryBindings::iterator j = tb->getMemoryBindings().begin();
            //               j != tb->getMemoryBindings().end(); j++)
            {
                MemoryBinding *mb = tb->getMemoryBinding();
                for (Memories::iterator j = mb->getMemories()->begin(); j != mb->getMemories()->end(); j++)
                {
                    Memory *m = *j;//mb->getMemory();

                    paretoQuantities.memoryLoad[m] = mb->getMemorySizeUsedForGraph(m) / (double)(m->getSize());
                }
            }

            // Iterate over the network interface bindings
            for (NetworkInterfaceBindings::iterator
                 j = tb->getNetworkInterfaceBindings().begin();
                 j != tb->getNetworkInterfaceBindings().end(); j++)
            {
                NetworkInterfaceBinding *nb = *j;
                NetworkInterface *n = nb->getNetworkInterface();

                paretoQuantities.communicationLoad[n] =
                    (nb->getInBandwidthUsedForGraph()
                     + nb->getOutBandwidthUsedForGraph())
                    / (n->getInBandwidth() + n->getOutBandwidth());
            }
        }

        // Normalize the processing load
        for (map<Processor *, double>::iterator
             i =  paretoQuantities.processingLoad.begin();
             i !=  paretoQuantities.processingLoad.end(); i++)
        {
            Processor *p = i->first;

            paretoQuantities.processingLoad[p] = paretoQuantities.processingLoad[p]
                                                 / sumProcessingLoad;
        }
    }

} // End namespace FSMSADF

