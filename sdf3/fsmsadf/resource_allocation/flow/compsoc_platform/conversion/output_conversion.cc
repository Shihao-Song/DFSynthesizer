/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   conversion.h
 *
 *  Author          :   Martijn Koedam (m.l.p.j.koedam@tue.nl)
 *
 *  Date            :   Mei 16, 2011
 *
 *  Function        :   Output format conversion
 *
 *  History         :
 *      16-05-11    :   Initial version.
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

/**
 * @todo C&E prob: We need to know network latency for memory mapping
 */
#include "output_conversion.h"
#include <assert.h>
#include "base/base.h"
#include "../../../platform_graph/memory.h"

#include "helper.h"

namespace FSMSADF
{
    /**
     * Constructs a new OutputConversionCompSoCPlatform.
     *
     * @param filename The filename of the input file.
     *
     * This sets default value of 1 on #timeUnit and #dataUnit
     * Default clock set at 50Mhz
     */
    OutputConversionCompSoCPlatform::OutputConversionCompSoCPlatform(CString filename) :
        OutputConversionBase(filename),
        application_document(NULL),
        timeUnit(1.0),
        dataUnit(1),
        cycleTime(20)
    {

    }
    /**
     * destructor
     */
    OutputConversionCompSoCPlatform::~OutputConversionCompSoCPlatform()
    {
        releaseDoc(application_document);
    }



    /**
     * Find the ApplicationGraph that matches this mapping.
     *
     * throws an CEXception() when failed.
     *
     * @param src_map_node The mapping node.
     * @return Node pointing to the correct ApplicationGraph.
     */
    CNode *OutputConversionCompSoCPlatform::getApplicationGraph(CNode *src_map_node)
    {
        // Check if we are actually in the mapping node.
#ifdef _MSC_VER
        ASSERT(CString((const char *)(src_map_node->getName())) == "mapping",
               "src_map_node is not a Mapping node");
#else
        ASSERT(CString((const char *)(src_map_node->name)) == "mapping",
               "src_map_node is not a Mapping node");
#endif

        // Get the name of the application graph.
        CString app_graph = CGetAttribute(src_map_node, "appGraph");

        // Get the root node.
        CNode *root = getRootNode(src_map_node);

        try
        {
            return CFindNodeWithAttribute(root, "applicationGraph", "name", app_graph);
        }
        catch (CException e)
        {
            throw("Failed to find applicationGraph: " + app_graph, e);
        }

    }
    /**
     * Find the ArchitectureGraph that matches this mapping.
     *
     * throws an CEXception() when failed.
     *
     * @param src_map_node The mapping node.
     * @return Node pointing to the correct architectureGraph.
     */
    CNode *OutputConversionCompSoCPlatform::getArchitectureGraph(CNode *src_map_node)
    {
        // Check if we are actually in the mapping node.
#ifdef _MSC_VER
        ASSERT(CString((const char *)(src_map_node->getName())) == "mapping",
               "src_map_node is not a Mapping node");
#else
        ASSERT(CString((const char *)(src_map_node->name)) == "mapping",
               "src_map_node is not a Mapping node");
#endif

        // Get the name of the application graph.
        CString arch_graph = CGetAttribute(src_map_node, "archGraph");

        // Get the root node.
        CNode *root = getRootNode(src_map_node);

        try
        {
            return CFindNodeWithAttribute(root, "architectureGraph", "name", arch_graph);
        }
        catch (CException e)
        {
            throw("Failed to find architectureGraph: " + arch_graph, e);
        }
    }
    /**
     * Get the node that represents the memory of channel #chan_name
     * at memory location #bloc (src/dst/mem)
     *
     * @param src_map_node The node of the selected mapping in the source file.
     * @param chan_name The unique name (name in sdf3, id in compsoc) of the channel.
     * @param bloc Location of the mmemory (src/dst)
     * @return a CNode or NULL if not found.
     */
    CNode *OutputConversionCompSoCPlatform::getChannelMemory(CNode *src_map_node,
            CString chan_name, CString bloc)
    {
        try
        {
            CNode *src_scen = CFindNode(src_map_node, "scenario");

            for (; src_scen != NULL; src_scen = CNextNode(src_scen, "scenario"))
            {
                CNode *src_tile = CFindNode(src_scen, "tile");

                for (; src_tile != NULL; src_tile = CNextNode(src_tile, "tile"))
                {
                    CNode *src_memory = CFindNode(src_tile, "memory");

                    for (;
                         src_memory != NULL;
                         src_memory = CNextNode(src_memory, "memory"))
                    {
                        CNode *src_channel = CFindNode(src_memory, "channel");

                        for (;
                             src_channel != NULL;
                             src_channel = CNextNode(src_channel, "channel"))
                        {
                            if (CHasAttribute(src_channel, "name") &&
                                CGetAttribute(src_channel, "name") == chan_name)
                            {
                                if (CHasAttribute(src_channel, "bufferType") &&
                                    CGetAttribute(src_channel, "bufferType") == bloc)
                                    return src_memory;
                            }
                        }
                    }
                }
            }
        }

        catch (CException &e)
        {
            throw CException(CString(__FUNCTION__) + ": Failed to get channel memory", e);
        }

        return NULL;
    }
    /**
     * Get the max transactionSize of a channel
     *
     * @todo Currently we use the tokenSize for this. If we want to send multiple tokens
     * at one time we want to change this.
     *
     * @param src_map_node The node of the selected mapping in the source file.
     * @param chan_name The unique name (name in sdf3, id in compsoc) of the channel.
     * @return
     */
    Size OutputConversionCompSoCPlatform::getChannelTransactionSize(
        CNode *src_map_node, CString chan_name)
    {
        Size token_size = 0;
        bool token_size_found = false;

        try
        {
            CNode *root = getRootNode(src_map_node);
            CNode *src_fp = CFindNode(root, "fsmsadfProperties");

            for (; src_fp != NULL; src_fp = CNextNode(src_fp, "fsmsadfProperties"))
            {
                // Itterate over the default properties, then over the propeties of
                // the individual scenarios.

                // defaultProperties
                // there should only be one, but itterate it anyway.
                CNode *src_defprop = CFindNode(src_fp, "defaultProperties");

                for (; src_defprop; src_defprop = CNextNode(src_defprop, "defaultProperties"))
                {
                    CNode *src_chanprop = CFindNode(src_defprop, "channelProperties");

                    for (; src_chanprop != NULL;
                         src_chanprop = CNextNode(src_chanprop, "channelProperties"))
                    {
                        if (CHasAttribute(src_chanprop, "channel") &&
                            CGetAttribute(src_chanprop, "channel") == chan_name)
                        {
                            CNode *token_sz_node = CGetChildNode(src_chanprop, "tokenSize");

                            if (token_sz_node)
                            {
                                // tokensize is in bytes.
                                Size e = Size(CGetAttribute(token_sz_node, "sz"));
                                token_size = MAX(token_size, e);
                                token_size_found = true;
                            }

                        }
                    }
                }

                // Scenarios.
                CNode *src_scens = CFindNode(src_fp, "scenarios");

                for (; src_scens != NULL; src_scens = CNextNode(src_scens, "scenarios"))
                {
                    CNode *src_scen = CFindNode(src_scens, "scenario");

                    for (; src_scen != NULL; src_scen = CNextNode(src_scen, "scenario"))
                    {
                        CNode *src_chanprop = CFindNode(src_scen, "channelProperties");

                        for (; src_chanprop != NULL;
                             src_chanprop = CNextNode(src_chanprop, "channelProperties"))
                        {
                            if (CHasAttribute(src_chanprop, "channel") &&
                                CGetAttribute(src_chanprop, "channel") == chan_name)
                            {
                                CNode *token_sz_node = CGetChildNode(src_chanprop, "tokenSize");

                                if (token_sz_node)
                                {
                                    // tokensize is in bytes.
                                    Size e = Size(CGetAttribute(token_sz_node, "sz"));
                                    token_size = MAX(token_size, e);
                                    token_size_found = true;
                                }

                            }
                        }
                    }
                }
            }
        }

        catch (CException &e)
        {
            throw CException(
                CString(__FUNCTION__) +
                ": Failed to get transaction size of the channel",
                e);
        }

        if (!token_size_found)
        {
            throw CException("Did not find tokensize for channel: " + chan_name);
        }

        return token_size;
    }
    /**
     * Get the channel bandwidth.
     *
     * CompSoC uses mbyte/sec as unit for bandwidth. SDF3 uses bytes/timeunit.
     * We need to get the #timeUnit from the application
     * (using #applicationInputFileReadUnits) file to do the conversion.
     *
     * @param src_map_node The node of the selected mapping in the source file.
     * @param chan_name The unique name (name in sdf3, id in compsoc) of the channel.
     * @return  ChannelBandwidth (in tokenSize)
     */
    Bandwidth OutputConversionCompSoCPlatform::getChannelBandwidth(CNode *src_map_node,
            CString chan_name)
    {
        Bandwidth bw = 0;
        bool bw_found = false;

        try
        {
            CNode *src_scen = CFindNode(src_map_node, "scenario");

            for (; src_scen != NULL; src_scen = CNextNode(src_scen, "scenario"))
            {
                CNode *src_constrs = CFindNode(src_scen, "constraints");

                for (; src_constrs != NULL;
                     src_constrs = CNextNode(src_constrs, "constraints"))
                {
                    CNode *src_cconstr = CFindNode(src_constrs, "channelConstraints");

                    for (; src_cconstr != NULL;
                         src_cconstr = CNextNode(src_cconstr, "channelConstraints"))
                    {
                        if (CHasAttribute(src_cconstr, "channel") &&
                            CGetAttribute(src_cconstr, "channel") == chan_name)
                        {
                            CNode *bw_node = CGetChildNode(src_cconstr, "bandwidth");

                            if (bw_node)
                            {
                                Bandwidth e = CGetAttribute(bw_node, "min");
                                bw = MAX(bw, e);
                                bw_found = true;
                            }
                        }
                    }
                }
            }
        }

        catch (CException &e)
        {
            throw CException(
                CString(__FUNCTION__) +
                ": Failed to get bandwidth of the channel",
                e);
        }

        if (!bw_found)
        {
            throw CException("Did not find bandwidth for channel: " + chan_name);
        }

        // bw is in 1bytes/time unit.
        // We want to go to mbyte/sec.
        //  bw *(#timeUnits per sec)*(1/mbyte)
        // timeunit it always ns. (converted in input)
        return bw * (1e9) * (1.0 / (1024.0 * 1024.0));
    }
    /**
     * Finds the maximum rate for channel #chan_name at located #loc
     *
     * @param src_map_node The node of the selected mapping in the source file.
     * @param chan_name The unique name (name in sdf3, id in compsoc) of the channel.
     * @param loc The place (src or dst).
     *
     * @return The rate.
     */
    int OutputConversionCompSoCPlatform::getMaxChannelRate(CNode *src_map_node,
            CString chan_name, CString loc)
    {
        int chan_rate = 0;
        bool chan_rate_found = false;

        try
        {
            CNode *src_app_graph = getApplicationGraph(src_map_node);
            CNode *src_fsmsadf = CFindNode(src_app_graph, "fsmsadf");
            CNode *src_scengraph = CFindNode(src_fsmsadf, "scenariograph");

            for (; src_scengraph != NULL; src_scengraph = CNextNode(src_scengraph, "scenariograph"))
            {
                CString actor_name;
                CString port_name;
                /* Get channel */
                CNode *src_channel = CFindNode(src_fsmsadf, "channel");

                for (; src_channel != NULL; src_channel = CNextNode(src_channel, "channel"))
                {
                    if (CHasAttribute(src_channel, "name") &&
                        CGetAttribute(src_channel, "name") == chan_name)
                    {
                        actor_name = CGetAttribute(src_channel, loc + "Actor");
                        port_name = CGetAttribute(src_channel, loc + "Port");
                    }
                }
                // if we failed to find the channel, skip.
                if (actor_name.empty() || port_name.empty())
                    break;
                CNode *src_actor = CFindNodeWithAttribute(src_fsmsadf, "actor", "name", actor_name);
                CNode *src_port = CFindNodeWithAttribute(src_actor, "port", "name", port_name);
                CString src_rate = CGetAttribute(src_port, "rate");
                chan_rate = MAX(chan_rate, int(src_rate));
                chan_rate_found = true;
            }
        }

        catch (CException &e)
        {
            throw CException(
                CString(__FUNCTION__) + ": Failed to find channel rate",
                e);
        }

        if (!chan_rate_found)
        {
            throw CException(CString(__FUNCTION__) + "Did not find rate for channel: " + chan_name);
        }

        return chan_rate;
    }
    /**
     * Get the size of the channel (in tokens?)
     *
     * SDF3 does not support channels(fifo's) that can hold X nodes, so
     * we give fifo depth the size of the dst, place it at the dst and then
     * this will (by compsoc src. rs) be merged into one buffer.
     *
     * @param src_map_node The node of the selected mapping in the source file.
     * @param chan_name The unique name (name in sdf3, id in compsoc) of the channel.
     *
     * @returns Size of the channel (in tokens)
     */
    Size OutputConversionCompSoCPlatform::getChannelSize(CNode *src_map_node,
            CString chan_name)
    {
        Size size = 0;
        bool size_found = false;

        try
        {
            CNode *src_scen = CFindNode(src_map_node, "scenario");

            for (; src_scen != NULL; src_scen = CNextNode(src_scen, "scenario"))
            {
                CNode *src_constrs = CFindNode(src_scen, "constraints");

                for (; src_constrs != NULL;
                     src_constrs = CNextNode(src_constrs, "constraints"))
                {
                    CNode *src_cconstr = CFindNode(src_constrs, "channelConstraints");

                    for (; src_cconstr != NULL;
                         src_cconstr = CNextNode(src_cconstr, "channelConstraints"))
                    {
                        if (CHasAttribute(src_cconstr, "channel") &&
                            CGetAttribute(src_cconstr, "channel") == chan_name)
                        {
                            CNode *bw_node = CGetChildNode(src_cconstr, "bufferSize");

                            if (bw_node)
                            {
                                Size e = Size(CGetAttribute(bw_node, "dst"));
                                Size me = Size(CGetAttribute(bw_node, "mem"));
                                size = MAX(size, e + me);
                                size_found = true;
                            }
                        }
                    }
                }
            }
        }

        catch (CException &e)
        {
            throw CException(
                CString(__FUNCTION__) + ": Failed to find channel size",
                e);
        }

        if (!size_found)
        {
            throw CException(CString(__FUNCTION__) + "Did not find size for channel: " + chan_name);
        }

        return size;
    }
    /**
     * Find the tile that is at the src/dst (#type) of this channel
     *
     * @todo it now looks it up by first checking the network interface the channel is in.
     * then the tile belonging to the ni is the tile.  For memory fifo's it looks if
     * the memory of the channel is of type 'mem'.  better way is to only look at memory
     * and check if memory is of type src/dst. This way we can do it in one loop.
     *
     * @param src_map_node The node of the selected mapping in the source file.
     * @param chan_name The unique name (name in sdf3, id in compsoc) of the channel.
     * @param type of the channel, this indicates where it is located. (val: in or out)
     * @return The root #CNode of the Tile in the source file.
     */
    CNode *OutputConversionCompSoCPlatform::findChannelTile(CNode *src_map_node,
            CString chan_name, CString type)
    {
        // Search each time, get network interface and check if it is as outgoing channel.
        try
        {
            CNode *src_scen = CFindNode(src_map_node, "scenario");

            for (; src_scen != NULL; src_scen = CNextNode(src_scen, "scenario"))
            {
                CNode *src_tile = CFindNode(src_scen, "tile");

                for (; src_tile != NULL; src_tile = CNextNode(src_tile, "tile"))
                {
                    CNode *src_ni = CFindNode(src_tile, "networkInterface");

                    for (; src_ni != NULL;
                         src_ni = CNextNode(src_ni, "networkInterface"))
                    {
                        CNode *src_chan = CFindNode(src_ni, "channel");

                        for (; src_chan != NULL;
                             src_chan = CNextNode(src_chan, "channel"))
                        {
                            CString name = CGetAttribute(src_chan, "name");

                            if (name != chan_name) continue;

                            if (CHasAttribute(src_chan, type) &&
                                CGetAttribute(src_chan, type) == "true")
                            {
                                return src_tile;
                            }
                        }
                    }
                }
            }
        }

        catch (CException &e)
        {
            throw CException(
                CString(__FUNCTION__) +
                ": Failed to find the tile belonging to the " + type + " of channel" + chan_name,
                e);
        }

        // We haven't found the tile connected to a network interface. this could mean
        // that the channel is in the memory. Check this!
        try
        {
            CNode *src_scen = CFindNode(src_map_node, "scenario");

            for (; src_scen != NULL; src_scen = CNextNode(src_scen, "scenario"))
            {
                CNode *src_tile = CFindNode(src_scen, "tile");

                for (; src_tile != NULL; src_tile = CNextNode(src_tile, "tile"))
                {
                    CNode *src_mem = CFindNode(src_tile, "memory");

                    for (; src_mem != NULL;
                         src_mem = CNextNode(src_mem, "memory"))
                    {
                        CNode *src_chan = CFindNode(src_mem, "channel");

                        for (; src_chan != NULL;
                             src_chan = CNextNode(src_chan, "channel"))
                        {
                            CString name = CGetAttribute(src_chan, "name");

                            if (name != chan_name) continue;

                            if (CHasAttribute(src_chan, "bufferType") &&
                                CGetAttribute(src_chan, "bufferType") == "mem")
                            {
                                return src_tile;
                            }
                        }
                    }
                }
            }
        }

        catch (CException &e)
        {
            throw CException(
                CString(__FUNCTION__) +
                ": Failed to find the tile belonging to the " + type + " of channel" + chan_name,
                e);
        }

        throw CException(CString(__FUNCTION__) +
                         ": Failed to find the tile belonging to the " + type + " of channel" + chan_name);

    }
    /**
     *
     * @param src_node the Node in the src file.
     * @param actor_name the name of the actor to get the number of fifo's.
     * @param type if it is incoming (dstActor) or outgoing (srcActor)
     *
     * Get the number of in/out going fifo's on actor #actor_name
     *
     * @return the number of in/out going fifo's
     */
    int OutputConversionCompSoCPlatform::getNumFifos(
        CNode *src_map_node, CString actor_name, CString type)
    {
        int nm_fifos = 0;

        try
        {
            CNode *src_app_graph = getApplicationGraph(src_map_node);
            CNode *src_scen = CFindNode(src_app_graph, "scenariograph");

            for (; src_scen != NULL; src_scen = CNextNode(src_scen, "scenariogrpah"))
            {
                int scen_num_fifos = 0;
                CNode *src_chan_prop = CFindNode(src_scen, "channel");

                for (; src_chan_prop != NULL;
                     src_chan_prop = CNextNode(src_chan_prop, "channel"))
                {
                    if (CHasAttribute(src_chan_prop, type + "Actor") &&
                        CGetAttribute(src_chan_prop, type + "Actor") == actor_name)
                    {
                        scen_num_fifos++;
                    }
                }

                nm_fifos = MAX(nm_fifos, scen_num_fifos);
            }
        }

        catch (CException &e)
        {
            throw CException(
                CString(__FUNCTION__) +
                ": Failed to find the number of fifo's " + type + " at actor" + actor_name,
                e);
        }

        return nm_fifos;
    }
    /**
     *
     * Get the memory (of type #mem_type) used by action #actor_name.
     *
     *
     * @param src_node A node in the source file.
     * @param actor_name The (unique) name of the actor.
     * @param mem_type The #MemoryType.
     * @return
     */
    Size OutputConversionCompSoCPlatform::getMemorySizeActor(
        CNode *src_map_node, CString actor_name, MemoryType mem_type)
    {
        Size mem_size = 0;

        try
        {
            CNode *src_app_graph = getApplicationGraph(src_map_node);
            CNode *src_fp = CFindNode(src_app_graph, "fsmsadfProperties");

            for (; src_fp != NULL; src_fp = CNextNode(src_fp, "fsmsadfProperties"))
            {
                /// @todo read default properties aswell
                CNode *src_scens = CFindNode(src_fp, "scenarios");

                for (; src_scens != NULL; src_scens = CNextNode(src_scens, "scenarios"))
                {
                    CNode *src_scen = CFindNode(src_scens, "scenario");

                    for (; src_scen != NULL; src_scen = CNextNode(src_scen, "scenario"))
                    {
                        CNode *src_actorprop = CFindNode(src_scen, "actorProperties");

                        for (; src_actorprop != NULL;
                             src_actorprop = CNextNode(src_actorprop, "actorProperties"))
                        {
                            if (CHasAttribute(src_actorprop, "actor") &&
                                CGetAttribute(src_actorprop, "actor") == actor_name)
                            {
                                CNode *src_mem_prop = CFindNode(src_actorprop, "memory");

                                for (; src_mem_prop; src_mem_prop = CNextNode(src_mem_prop, "memory"))
                                {
                                    if (CHasAttribute(src_mem_prop, "type") &&
                                        mem_type == CGetAttribute(src_mem_prop, "type"))
                                    {
                                        CString size = CGetAttribute(src_mem_prop, "size");
                                        return size;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        catch (CException &e)
        {
            throw CException(CString(__FUNCTION__) +
                             ": Failed to find the memory size of type " + mem_type + " from actor" + actor_name,
                             e);
        }

        return mem_size;
    }
    /**
     *
     * Iterate over all <scenario>/<connection*> and write out the channels that
     * are mapped between tiles as network.
     *
     * Hardcoded cmemin/cmemout/dma
     *
     * Connections on the same dma and between the same tiles need to be
     * grouped together into one connection block.
     *
     * @param src_map_node The node of the selected mapping in the source file.
     * @param dst_app_node
     */
    void OutputConversionCompSoCPlatform::writeCommunicationAppConnections(
        CNode *src_map_node, CNode *dst_app_node)
    {
        // A connection in CompSoC world is a group of Channels (between same tiles and dma's) in the
        // SDF3 world.
        // List over all <scenario>/<connection*> get what channels are on there.
        // Those channels are actual connections to be mapped on the network.
        set<CString> mapped_channels;

        try
        {
            CNode *src_scen = CFindNode(src_map_node, "scenario");

            for (; src_scen != NULL; src_scen = CNextNode(src_scen, "scenario"))
            {
                CNode *src_con = CFindNode(src_scen, "connection");

                for (; src_con != NULL; src_con = CNextNode(src_con, "connection"))
                {
                    CNode *src_chan = CFindNode(src_con, "channel");

                    for (; src_chan != NULL; src_chan = CNextNode(src_chan, "channel"))
                    {
                        CString src_chan_name = CGetAttribute(src_chan, "name");
                        mapped_channels.insert(src_chan_name);
                    }
                }
            }

            int conn_id = 0;

            //            for (set<CString>::iterator i = mapped_channels.begin();
            //                 i != mapped_channels.end(); i++)
            while (!mapped_channels.empty())
            {
                CString chan_name = *mapped_channels.begin();
                mapped_channels.erase(mapped_channels.begin());

                CNode *dst_conn_node = CNewNode("connection");
                CAddAttribute(dst_conn_node, "id", conn_id++);
                /// @todo hardcoded qos=GT
                CAddAttribute(dst_conn_node, "qos", "GT");

                // Find the initiator
                // Search each time, get network interface and check if it is as outgoing channel.
                CNode *src_src_tile_node = findChannelTile(src_map_node, chan_name, "out");

                if (src_src_tile_node == NULL)
                    throw CException(CString("src tile not found, channel has no initiator ip: " + chan_name));

                CNode *dst_init = CNewNode("initiator");
                CAddAttribute(dst_init, "ip", CGetAttribute(src_src_tile_node, "name"));

                CNode *src_dst_init_mem = getChannelMemory(src_map_node, chan_name, "src");

                if (src_dst_init_mem == NULL)
                    throw CException("No matching input memory found for channel");

                int mem_id = HelperGetMemId(CGetAttribute(src_dst_init_mem, "name"));
                ASSERT(mem_id >= 0, "Failed to get memory id for channel.");
                CAddAttribute(dst_init, "port", "dma" + CString(mem_id) + "_pi");
                CAddNode(dst_conn_node, dst_init);



                // /communication/application/connection/initiator
                CNode *src_dst_tile_node = findChannelTile(src_map_node, chan_name, "in");

                if (src_dst_tile_node == NULL)
                    throw CException(CString("src tile not found, channel has no target ip: " + chan_name));

                // /communication/application/connection/target
                CNode *dst_target_node = CNewNode("target");
                CAddAttribute(dst_target_node, "ip", CGetAttribute(src_dst_tile_node, "name"));

                CNode *src_dst_target_mem = getChannelMemory(src_map_node, chan_name, "dst");

                if (src_dst_target_mem == NULL)
                    throw CException("No matching input memory found for channel");

                // Get the channel memory id..  we want cmemin<id> for target.
                mem_id = HelperGetMemId(CGetAttribute(src_dst_target_mem, "name"));
                ASSERT(mem_id >= 0, "Failed to get memory id for channel.");
                CAddAttribute(dst_target_node, "port", "cmemin" + CString(mem_id) + "_pt");
                CAddNode(dst_conn_node, dst_target_node);

                // /communication/application/connection//read
                Size tsz = getChannelTransactionSize(src_map_node, chan_name);
                Bandwidth bw = getChannelBandwidth(src_map_node, chan_name);


                // Loop through all the other connections, and removes the ones
                // between the same tiles/memory.
                // Because we directly create connection for both direction
                // Merge these.
                /// @todo: Do not combine 2 directions. Use separate bw/ts req
                /// for this.

                for (set<CString>::iterator i = mapped_channels.begin();
                     i != mapped_channels.end(); i++)
                {
                    CString name = *i;
                    CNode *src_tile = findChannelTile(src_map_node, name, "in");
                    CNode *dst_tile = findChannelTile(src_map_node, name, "out");
                    if ((src_tile == src_src_tile_node && dst_tile == src_dst_tile_node)
                        ||
                        (dst_tile == src_src_tile_node && src_tile == src_dst_tile_node)
                       )
                    {
                        CNode *dst_mem = getChannelMemory(src_map_node, name, "dst");
                        CNode *src_mem = getChannelMemory(src_map_node, name, "src");
                        if ((HelperGetMemId(CGetAttribute(src_dst_target_mem, "name")) ==
                             HelperGetMemId(CGetAttribute(dst_mem, "name")) &&
                             HelperGetMemId(CGetAttribute(src_dst_init_mem, "name")) ==
                             HelperGetMemId(CGetAttribute(src_mem, "name"))
                            ) ||
                            (HelperGetMemId(CGetAttribute(src_dst_target_mem, "name")) ==
                             HelperGetMemId(CGetAttribute(src_mem, "name")) &&
                             HelperGetMemId(CGetAttribute(src_dst_init_mem, "name")) ==
                             HelperGetMemId(CGetAttribute(dst_mem, "name"))
                            ))
                        {
                            // debug output.
                            cout << "Connection between same tile" << name << endl;

                            Size chan_tsz = getChannelTransactionSize(src_map_node, name);
                            Bandwidth chan_bw = getChannelBandwidth(src_map_node, name);
                            // Take the peak Transaction Size
                            tsz = MAX(tsz, chan_tsz);
                            // Combine the bandwidth
                            bw += chan_bw;
                            // remove the combined channel from the list.
                            mapped_channels.erase(i);
                        }
                    }
                }

                CNode *dst_read_node = CNewNode("read");
                CAddAttribute(dst_read_node, "bw", bw);
                CAddAttribute(dst_read_node, "burstsize", tsz);
                CAddNode(dst_conn_node, dst_read_node);

                CNode *dst_write_node = CNewNode("write");
                CAddAttribute(dst_write_node, "bw", bw);
                CAddAttribute(dst_write_node, "burstsize", tsz);
                CAddNode(dst_conn_node, dst_write_node);
                CAddNode(dst_app_node, dst_conn_node);

                /// return path.

                dst_conn_node = CNewNode("connection");
                CAddAttribute(dst_conn_node, "id", conn_id++);

                /// @todo hardcoded qos=GT
                CAddAttribute(dst_conn_node, "qos", "GT");

                /* initiator */
                dst_init = CNewNode("initiator");
                CAddAttribute(dst_init, "ip", CGetAttribute(src_dst_tile_node, "name"));
                mem_id = HelperGetMemId(CGetAttribute(src_dst_target_mem, "name"));
                ASSERT(mem_id >= 0, "Failed to get memory id for channel.");
                CAddAttribute(dst_init, "port", "dma" + CString(mem_id) + "_pi");
                CAddNode(dst_conn_node, dst_init);

                /* Target */
                dst_target_node = CNewNode("target");
                CAddAttribute(dst_init, "port", "dma" + CString(mem_id) + "_pi");
                CAddAttribute(dst_target_node, "ip", CGetAttribute(src_src_tile_node, "name"));
                mem_id = HelperGetMemId(CGetAttribute(src_dst_init_mem, "name"));
                ASSERT(mem_id >= 0, "Failed to get memory id for channel.");
                CAddAttribute(dst_target_node, "port", "cmemin" + CString(mem_id) + "_pt");
                CAddNode(dst_conn_node, dst_target_node);

                // read/write bw
                dst_read_node = CNewNode("read");
                CAddAttribute(dst_read_node, "bw", bw);
                CAddAttribute(dst_read_node, "burstsize", tsz);
                CAddNode(dst_conn_node, dst_read_node);

                dst_write_node = CNewNode("write");
                CAddAttribute(dst_write_node, "bw", bw);
                CAddAttribute(dst_write_node, "burstsize", tsz);
                CAddNode(dst_conn_node, dst_write_node);

                CAddNode(dst_app_node, dst_conn_node);
            }
        }

        catch (CException &e)
        {
            throw CException(CString(__FUNCTION__) +
                             ": Failed write application connections",
                             e);
        }
    }
    /**
     * Add a hardcoded monitor block
     *
     * @param dst_app_node
     */
    void OutputConversionCompSoCPlatform::writeCommunicationMonitor(
        CNode *dst_root)
    {
        CNode *mon_node = CNewNode("application");
        CAddAttribute(mon_node, "id", "monitoring");

        /* Communication */
        /* connection */
        CNode *con_node = CNewNode("connection");
        CAddAttribute(con_node, "id", "0");
        CAddAttribute(con_node, "qos", "GT");

        /* initiator */
        CNode *init_node = CNewNode("initiator");
        CAddAttribute(init_node, "ip", "monitor");
        CAddAttribute(init_node, "port", "pi");
        CAddNode(con_node, init_node);

        /* target */
        CNode *target_node = CNewNode("target");
        CAddAttribute(target_node, "ip", "shared");
        CAddAttribute(target_node, "port", "pt");
        CAddNode(con_node, target_node);

        CNode *read_node = CNewNode("read");
        CAddAttribute(read_node, "bw", "8");
        CAddNode(con_node, read_node);

        CNode *write_node = CNewNode("write");
        CAddAttribute(write_node, "bw", "8");
        CAddNode(con_node, write_node);

        CAddNode(mon_node, con_node);


        CAddNode(dst_root, mon_node);
    }
    /**
     * Add a hardcoded fake app block  to make the flow work.
     *
     * @param dst_app_node
     *
     * @todo: This needs to be fixed in compsoc. For now generate network to satisfy
     * synthesis
     */
    void OutputConversionCompSoCPlatform::writeCommunicationFakeNetwork(
        CNode *src_map_node, CNode *dst_root)
    {
        MemoryType a = MemoryType::CommOut;
        CNode *mon_node = CNewNode("application");
        CAddAttribute(mon_node, "id", "fake");
        // Get the root node.
        CNode *root = getRootNode(src_map_node);

        CNode *src_arch_graph = CFindNode(root, "architectureGraph");
        ASSERT(src_arch_graph != NULL, "No architecture Graph found");
        /* Should only be one */
        int fifo_id = 0;
        for (CNode *src_conn_graph = CFindNode(src_arch_graph, "tile");
             src_conn_graph != NULL;
             src_conn_graph = CNextNode(src_conn_graph, "tile")
            )
        {
            CString tile_name = CGetAttribute(src_conn_graph, "name");

            for (CNode *src_mem_node = CFindNode(src_conn_graph, "memory");
                 src_mem_node != NULL;
                 src_mem_node = CNextNode(src_mem_node, "memory"))
            {
                if (a == CGetAttribute(src_mem_node, "type"))
                {
                    CNode *conn_node = CNewNode("connection");
                    CAddAttribute(conn_node, "id", fifo_id);
                    CAddAttribute(conn_node, "qos", "GT");

                    int memory_id = HelperGetMemId(CGetAttribute(src_mem_node, "name"));

                    CNode *init_node = CNewNode("initiator");
                    CAddAttribute(init_node, "ip", tile_name);
                    CAddAttribute(init_node, "port", "dma" + CString(memory_id) + "_pi");
                    CAddNode(conn_node, init_node);
                    CNode *target_node = CNewNode("target");
                    CAddAttribute(target_node, "ip", tile_name);
                    CAddAttribute(target_node, "port", "cmemin" + CString(memory_id) + "_pt");
                    CAddNode(conn_node, target_node);

                    CNode *read_node = CNewNode("read");
                    CAddAttribute(read_node, "bw", "0.01");
                    CAddNode(conn_node, read_node);

                    CNode *write_node = CNewNode("write");
                    CAddAttribute(write_node, "bw", "0.01");
                    CAddNode(conn_node, write_node);

                    CAddNode(mon_node, conn_node);
                    fifo_id++;
                }
            }
        }


        CAddNode(dst_root, mon_node);
    }
    /**
     * write coomunicationFile to #file
     *
     * @param file the file location.
     */
    void OutputConversionCompSoCPlatform::writeCommunicationFile(const CString &file, const int mapping)
    {
        logInfo("Creating communication file: '" + file + "'");
        CDoc *output_doc = CNewDoc();
        CNode *src_root = CGetRootNode(input_doc);
        CNode *dst_root = CNewNode("communication");
        CSetRootNode(output_doc, dst_root);

        CSetPrivateDTD(output_doc, "communication", "../../etc/dtd/communicationgrm.dtd");

        /// @todo:  Select a mapping based on input user or first if not spec.
        CNode *src_map_node = CFindNode(src_root, "mapping");

        try
        {
            // If no mapping is found, bail out.
            if (src_map_node == NULL)
            {
                throw CException("No usable mapping found");
            }

            if (mapping < 0)
            {
                while (CGetAttribute(src_map_node, "name") == "initial")
                {
                    src_map_node = CNextNode(src_map_node, "mapping");
                }

                src_map_node = CNextNode(src_map_node, "mapping");
            }

            else
            {
                while (CGetAttribute(src_map_node, "name") != CString(mapping))
                {
                    src_map_node = CNextNode(src_map_node, "mapping");
                }

                if (CGetAttribute(src_map_node, "name") != CString(mapping))
                {
                    throw CException("Failed to find mapping: " + CString(mapping));

                }
            }

            CString map_id = CGetAttribute(src_map_node, "name");

            logInfo("Using mapping: " + map_id);
            // Get application name.
            CString src_app = CGetAttribute(src_map_node, "appGraph");
            // Create /communication/appplication
            CNode *dst_app_node = CNewNode("application");
            CAddAttribute(dst_app_node, "id", src_app);

            // Lookup connections.
            writeCommunicationAppConnections(src_map_node, dst_app_node);

            CAddNode(dst_root, dst_app_node);
            // Write monitor block.
            writeCommunicationMonitor(dst_root);
            writeCommunicationFakeNetwork(src_map_node, dst_root);

            if (!CValidate(output_doc))
            {
                logWarning("Communication file does not validate");
            }

            // Save
            CSaveFile(file, output_doc, 1);
            logInfo("Saving communication file: '" + file + "'");
        }

        catch (CException &e)
        {
            throw CException(CString(__FUNCTION__) +
                             ": Failed write communication file.",
                             e);
        }

        delete output_doc;
    }
    /**
     * write out /application/processor/app_TDM_slots
     *
     * @param procid
     * @param src_proc_node
     * @param dst_proc_node
     */
    void OutputConversionCompSoCPlatform::writeApplicationProcessorTDMSlots(
        CString procid, CNode *src_map_node, CNode *src_proc_node, CNode *dst_proc_node)
    {
        try
        {
            /// @todo: Write ou the app_tdm_slots
            CNode *dst_app_tdm_slots = CNewNode("app_TDM_slots");

            /// @todo this is a hack, redo this propperly.

            int slotLength = 0;
            int overhead = 0;
            int wheelSize = 0;
            int slots = 0;
            // get missing info from  can be get from architectureGraph/tile
            CNode *src_arch_graph = getArchitectureGraph(src_map_node);

            if (src_arch_graph == NULL)
                throw CException("Could not find architectureGraph");

            CNode *src_tile = CFindNode(src_arch_graph, "tile");

            for (; src_tile; src_tile = CNextNode(src_tile, "tile"))
            {

                /* skip tiles withouth cpu's*/
                if (!CHasChildNode(src_tile, "processor"))
                    continue;

                // Copy id
                CString src_tile_id = CGetAttribute(src_tile, "name");

                if (src_tile_id != procid)
                    continue;

                // Get the arbitration node
                CNode *src_arb = CFindNode(src_tile, "arbitration");

                if (src_arb == NULL)
                    throw CException("Aribtration not found for this src_tile");

                // Get params needed to calculate the TDM
                slotLength = CGetAttribute(src_arb, "slotLength");
                overhead = CGetAttribute(src_arb, "contextSwitchOverhead");
                wheelSize = CGetAttribute(src_arb, "wheelsize");
                break;

            }

            // Get the arbitration node under the proc, a
            // and get the timeslice it is assigned.
            // We only have a number of X continuesly slots.
            // so no min/max value is needed.

            CNode *dst_app_tdm_slots_param = CNewNode("parameter");
            CAddAttribute(dst_app_tdm_slots_param, "id", "num_slots");
            CAddAttribute(dst_app_tdm_slots_param, "type", "int");
            CNode *src_arb_node = CFindNode(src_proc_node, "arbitration");
            CString src_time_slice = CGetAttribute(src_arb_node, "timeslice");
            slots = int(src_time_slice) / (slotLength + overhead);
            CAddAttribute(dst_app_tdm_slots_param, "value", slots);

            /// @todo better way of determining this?
            CNode *dst_app_tdm_min_slots_dist = CNewNode("parameter");
            CAddAttribute(dst_app_tdm_min_slots_dist, "id", "min_slots_dist");
            CAddAttribute(dst_app_tdm_min_slots_dist, "type", "int");
            CAddAttribute(dst_app_tdm_min_slots_dist, "value", 0);

            CNode *dst_app_tdm_max_slots_dist = CNewNode("parameter");
            CAddAttribute(dst_app_tdm_max_slots_dist, "id", "max_slots_dist");
            CAddAttribute(dst_app_tdm_max_slots_dist, "type", "int");
            CAddAttribute(dst_app_tdm_max_slots_dist, "value",
                          (wheelSize / (slotLength + overhead)));

            CAddNode(dst_app_tdm_slots, dst_app_tdm_slots_param);
            CAddNode(dst_app_tdm_slots, dst_app_tdm_min_slots_dist);
            CAddNode(dst_app_tdm_slots, dst_app_tdm_max_slots_dist);

            /* add the nodes */
            CAddNode(dst_proc_node, dst_app_tdm_slots);
        }

        catch (CException &e)
        {
            throw CException(CString(__FUNCTION__) +
                             ": Failed write Application processor TDM slots.",
                             e);
        }
    }
    /**
     * Get the schedule (CSDF only) for processor (tile) procid.
     *
     * @param procid
     * @param src_map_node The node of the selected mapping in the source file.
     * @return an ordered array.
     */
    CStrings OutputConversionCompSoCPlatform::getProcSchedule(
        CString procid, CNode *src_map_node)
    {
        /// Create the schedule we want to run.
        /// @todo only works for CSDF
        CStrings sched_order;
        bool sched_order_found = false;

        for (CNode *src_scen_node = CFindNode(src_map_node, "scenario");
             src_scen_node;
             src_scen_node = CNextNode(src_scen_node, "scenario"))
        {
            CNode *src_tile_node = CFindNode(src_scen_node, "tile");

            for (; src_tile_node; src_tile_node = CNextNode(src_tile_node, "tile"))
            {
                if (CGetAttribute(src_tile_node, "name") == procid)
                {
                    CNode *src_sched_node = CFindNode(src_tile_node, "schedule");

                    if (src_sched_node == NULL) throw CException("Could not find schedule.");

                    for (CNode *src_sched_state_node = CFindNode(src_sched_node, "state");
                         src_sched_state_node;
                         src_sched_state_node = CNextNode(src_sched_state_node, "state"))
                    {
                        /// @todo start at 'start of periodic regime?
                        CString a = CGetAttribute(src_sched_state_node, "actor");
                        sched_order.push_back(a);
                        sched_order_found = true;
                    }
                }
            }
        }

        if (!sched_order_found)
        {
            throw CException(CString(__FUNCTION__) + ": No processor task schedule found.");
        }

        return sched_order;
    }
    /**
     *
     * write Task Scheduler.
     *
     * @param procid
     * @param src_proc_node
     * @param dst_proc_node
     */
    void OutputConversionCompSoCPlatform::writeTaskScheduler(
        CString procid, CNode *src_map_node, CNode *src_proc_node, CNode *dst_proc_node)
    {
        /// Write out hte task _scheduler
        CNode *dst_task_sched_node = CNewNode("task_scheduler");
        // We use static ordering.
        CAddAttribute(dst_task_sched_node, "type", "SO");
        CNode *dst_task_sched_space_node = CNewNode("space");
        /// @todo hardcode the space used in task_scheduer is os_space
        CAddAttribute(dst_task_sched_space_node, "type", "OS_space");
        CAddNode(dst_task_sched_node, dst_task_sched_space_node);

        /// Create the schedule we want to run.
        /// @todo only works for CSDF
        CStrings sched_order = getProcSchedule(procid, src_map_node);
        CString sched = CString::join(sched_order, ',');
        CNode *dst_param_sched = CNewNode("parameter");
        CAddAttribute(dst_param_sched, "id", "task_order");
        CAddAttribute(dst_param_sched, "type", "list");
        CAddAttribute(dst_param_sched, "type", "list");
        CAddAttribute(dst_param_sched, "value", sched);
        CAddNode(dst_task_sched_node, dst_param_sched);


        CAddNode(dst_proc_node, dst_task_sched_node);
    }
    /**
     *  Writes out a slac manager. This is all hardcoded.
     *
     * @param procid
     * @param src_proc_node
     * @param dst_proc_node
     */
    void OutputConversionCompSoCPlatform::writeSlackManager(
        CString procid, CNode *src_proc_node, CNode *dst_proc_node)
    {
        /// Write out the slack_manager
        CNode *dst_slack_man = CNewNode("slack_manager");
        /// @todo hardcoded the tyope of slack manager.
        CAddAttribute(dst_slack_man, "type", "Std");
        CNode *dst_proc_node_space = CNewNode("space");
        /// @todo hardcoded the slack_manager/space::type to OS_Space
        CAddAttribute(dst_proc_node_space, "type", "OS_Space");
        CAddNode(dst_slack_man, dst_proc_node_space);

        CAddNode(dst_proc_node, dst_slack_man);
    }

    /**
     * getNumScenarios()
     * @param map node
     *
     * finds the number of scenarios.
     */
    int OutputConversionCompSoCPlatform::getNumScenarios(CNode *map_node)
    {
        CNode *app_graph = getApplicationGraph(map_node);

        /* Get actors*/
        CNode *scen_graph = CFindNode(app_graph, "scenariograph");
        int num_scen = 0;
        for (; scen_graph != NULL; scen_graph = CNextNode(scen_graph, "scenariograph"))
        {
            num_scen++;
        }
        return num_scen;
    }

    /**
     * This writes the system/application/processor
     *
     * @param procid (this is tile id in the source)
     * @param src_proc_node
     * @param dst_app
     */
    void OutputConversionCompSoCPlatform::writeMappingApplicationsProcessor(
        CString procid, CNode *src_map_node, CNode *src_proc_node, CNode *dst_app)
    {
        CNode *dst_proc_node = CNewNode("processor");
        CSetAttribute(dst_proc_node, "id", procid);

        try
        {
            CNode *src_actor_node = CFindNode(src_proc_node, "actor");

            if (src_actor_node == NULL)
            {
                // If there is no actor scheduled to be on this tile, we do not need to output anything.
                CRemoveNode(dst_proc_node);
                return;
            }

            // Write the TDM setup
            writeApplicationProcessorTDMSlots(procid, src_map_node, src_proc_node, dst_proc_node);
            // Write the task scheduler
            writeTaskScheduler(procid, src_map_node, src_proc_node, dst_proc_node);
            // Write basic slackmanager
            writeSlackManager(procid, src_proc_node, dst_proc_node);


            /// Write out the tasks that are mapped on this processor.
            /// loop through all the different scenarios and collect what tasks are there.
            ///
            CStrings actors;

            for (; src_actor_node; src_actor_node = CNextNode(src_actor_node, "actor"))
            {
                CNode *dst_task_node = CNewNode("task");
                CString src_task_name = CGetAttribute(src_actor_node, "name");
                CAddAttribute(dst_task_node, "id", src_task_name);

                /// @todo: get swapable
                // We do not support swapable tasks. (runtime migration) so always no.
                CNode *param = CNewNode("parameter");
                CAddAttribute(param, "id", "swapable");
                CAddAttribute(param, "type", "string");
                CAddAttribute(param, "value", "no");
                CAddNode(dst_task_node, param);

                /// @todo: stack size is hardcoded.
                /// This is unknown and a wild wild guess
                param = CNewNode("parameter");
                CAddAttribute(param, "id", "stack_size");
                CAddAttribute(param, "type", "int");
                CAddAttribute(param, "value", "512");
                CAddNode(dst_task_node, param);


                /// data size
                /// use the enum here to get data?
                Size sz = getMemorySizeActor(src_map_node, src_task_name, MemoryType::Data);
                param = CNewNode("parameter");
                CAddAttribute(param, "id", "data_size");
                CAddAttribute(param, "type", "int");
                CAddAttribute(param, "value", sz);
                CAddNode(dst_task_node, param);

                /// instr size
                sz = getMemorySizeActor(src_map_node, src_task_name, MemoryType::Instr);
                param = CNewNode("parameter");
                CAddAttribute(param, "id", "instr_size");
                CAddAttribute(param, "type", "int");
                CAddAttribute(param, "value", sz);
                CAddNode(dst_task_node, param);

                /// num consumer fifo
                sz = getNumFifos(src_map_node, src_task_name, "src");
                param = CNewNode("parameter");
                CAddAttribute(param, "id", "num_prod_FIFOs");
                CAddAttribute(param, "type", "int");
                CAddAttribute(param, "value", sz);
                CAddNode(dst_task_node, param);

                /// @todo this is probably going to be removed in the future.
                param = CNewNode("parameter");
                CAddAttribute(param, "id", "num_out_args");
                CAddAttribute(param, "type", "int");
                CAddAttribute(param, "value", sz);
                CAddNode(dst_task_node, param);

                /// num producer fifos
                sz = getNumFifos(src_map_node, src_task_name, "dst");
                param = CNewNode("parameter");
                CAddAttribute(param, "id", "num_cons_FIFOs");
                CAddAttribute(param, "type", "int");
                CAddAttribute(param, "value", sz);
                CAddNode(dst_task_node, param);

                /// @todo this is probably going to be removed in the future.
                param = CNewNode("parameter");
                CAddAttribute(param, "id", "num_in_args");
                CAddAttribute(param, "type", "int");
                CAddAttribute(param, "value", sz);
                CAddNode(dst_task_node, param);

                /// Create the schedule we want to run.
                /// @todo only works for CSDF

                int length = getNumScenarios(src_map_node);
                param = CNewNode("parameter");
                CAddAttribute(param, "id", "csdf_pattern_length");
                CAddAttribute(param, "type", "int");
                CAddAttribute(param, "value", length);
                CAddNode(dst_task_node, param);

                CAddNode(dst_proc_node, dst_task_node);
            }

            CAddNode(dst_app, dst_proc_node);
        }

        catch (CException &e)
        {
            throw CException(CString(__FUNCTION__) +
                             ": Failed write system/application/processor.",
                             e);
        }
    }
    /**
     * Get the src/dst port or actor connected to the channel %chan_name.
     *
     * @param src_map_node The node of the selected mapping in the source file.
     * @param chan_name The unique name (name in sdf3, id in compsoc) of the channel.
     * @param type (dstActor/srcActor/srcPort/dstPrort)
     * @return the name of the src/dst actor or port connected to the channel.
     */
    CString OutputConversionCompSoCPlatform::getChannelActorOrPort(CNode *src_map_node, CString chan_name, CString type)
    {
        try
        {
            CNode *src_app_graph = getApplicationGraph(src_map_node);
            CNode *src_scen = CFindNode(src_app_graph, "scenariograph");

            for (; src_scen != NULL; src_scen = CNextNode(src_scen, "scenariograph"))
            {
                CNode *src_chanprop = CFindNode(src_scen, "channel");

                for (; src_chanprop != NULL;
                     src_chanprop = CNextNode(src_chanprop, "channel"))
                {
                    if (CHasAttribute(src_chanprop, "name") &&
                        CGetAttribute(src_chanprop, "name") == chan_name)
                    {
                        if (CHasAttribute(src_chanprop, type))
                        {
                            return CGetAttribute(src_chanprop, type);
                        }
                    }
                }

            }
        }

        catch (CException &e)
        {
            throw CException(CString(__FUNCTION__) +
                             ": Failed to find the actor at the " +
                             type + " of channel " + chan_name,
                             e);
        }

        throw CException(CString(__FUNCTION__) +
                         ": Failed to find the actor at the " +
                         type + " of channel " + chan_name);
    }
    /**
     * Finds the DMA_id that is used by this channel (fifo)
     * look for the cmemout[0-9] at the src of the channel.
     * If it finds cmemout4 it will then return dma4
     * (s/cmemount/dma/)
     *
     * @param src_tile_out #CNode pointing to the tile the channel is mapped to.
     * @param src_fifo_name the name of the channel.
     *
     * @return the name of the dma, empty if not found.
     */
    CString OutputConversionCompSoCPlatform::getChannelDMA(CNode *src_tile_out, CString src_fifo_name)
    {
        CNode *src_mem_fifo_out = CFindNode(src_tile_out, "memory");

        for (; src_mem_fifo_out; src_mem_fifo_out = CNextNode(src_mem_fifo_out, "memory"))
        {
            if (CHasChildNode(src_mem_fifo_out, "channel"))
            {
                CNode *src_chan_node = CGetChildNode(src_mem_fifo_out, "channel");

                for (; src_chan_node; src_chan_node = CNextNode(src_chan_node, "channel"))
                {
                    if (CHasAttribute(src_chan_node, "name") &&
                        CGetAttribute(src_chan_node, "name") == src_fifo_name)
                    {
                        /* other wise the compare won't work.*/
                        BufferLocation loc = BufferLocation::Src;

                        if (CHasAttribute(src_chan_node, "bufferType") &&
                            loc == CGetAttribute(src_chan_node, "bufferType"))
                        {
                            CString src_mem_name = CGetAttribute(src_mem_fifo_out, "name");
                            int memory_id = HelperGetMemId(src_mem_name);
                            return "dma" + CString(memory_id);
                        }
                    }
                }
            }
        }

        throw CException(CString(__FUNCTION__) + ": Failed to find dma use by channel: " + src_fifo_name);
    }
    /**
     * Write the applications to the mapping file. <system><application>
     *
     * The list of applications are obtained from the mapping's applicationGraph
     *
     * @param src_root the root node of the source file.
     * @param src_map_node The node of the selected mapping in the source file.
     * @param dst_root the destination node.
     */
    void OutputConversionCompSoCPlatform::writeMappingApplications(
        CNode *src_root, CNode *src_map_node, CNode *dst_root)
    {
        try
        {
            CNode *dst_app_node = CNewNode("application");
            CString src_app_name = CGetAttribute(src_map_node, "appGraph");
            CSetAttribute(dst_app_node, "id", src_app_name);
            /// @todo write out Application/processor
            /// Look in a scenario and write that out?
            CNode *src_scen_node = CFindNode(src_map_node, "scenario");

            for (CNode *src_tile_node = CFindNode(src_scen_node, "tile");
                 src_tile_node; src_tile_node = CNextNode(src_tile_node, "tile"))
            {
                for (CNode *src_proc_node = CFindNode(src_tile_node, "processor");
                     src_proc_node;
                     src_proc_node = CNextNode(src_proc_node, "processor"))
                {
                    CString src_proc_name = CGetAttribute(src_tile_node, "name");
                    writeMappingApplicationsProcessor(src_proc_name, src_map_node, src_proc_node, dst_app_node);

                }
            }

            // Writing out the FIFO's

            CNode *src_chan_constr_node = CFindNode(src_map_node, "channelConstraints");

            for (; src_chan_constr_node;
                 src_chan_constr_node = CNextNode(src_chan_constr_node, "channelConstraints"))
            {
                CNode *dst_fifo_node = CNewNode("FIFO");
                CString src_fifo_name = CGetAttribute(src_chan_constr_node, "channel");
                CAddAttribute(dst_fifo_node, "id", src_fifo_name);

                /// Get the producer task.
                CString prod = getChannelActorOrPort(src_map_node, src_fifo_name, "srcActor");

                ///
                CNode *param = CNewNode("parameter");
                CAddAttribute(param, "id", "PTask");
                CAddAttribute(param, "type", "string");
                CAddAttribute(param, "value", prod);
                CAddNode(dst_fifo_node, param);

                /// Get the consumer task.
                CString consm = getChannelActorOrPort(src_map_node, src_fifo_name, "dstActor");

                param = CNewNode("parameter");
                CAddAttribute(param, "id", "CTask");
                CAddAttribute(param, "type", "string");
                CAddAttribute(param, "value", consm);
                CAddNode(dst_fifo_node, param);

                /// @todo is the tokensize equal to transaction size?
                Size trsz = getChannelTransactionSize(src_map_node, src_fifo_name);
                param = CNewNode("parameter");
                CAddAttribute(param, "id", "token_size");
                CAddAttribute(param, "type", "int");
                CAddAttribute(param, "value", trsz);
                CAddNode(dst_fifo_node, param);

                /// @todo fifo size. What exactly is this value? where to get it from
                Size fifosz = getChannelSize(src_map_node, src_fifo_name);
                param = CNewNode("parameter");
                CAddAttribute(param, "id", "fifo_size");
                CAddAttribute(param, "type", "int");
                CAddAttribute(param, "value", fifosz);
                CAddNode(dst_fifo_node, param);


                /// @todo Max_prod_rate
                int mpr = getMaxChannelRate(src_map_node, src_fifo_name, "src");
                param = CNewNode("parameter");
                CAddAttribute(param, "id", "Max_prod_rate");
                CAddAttribute(param, "type", "int");
                CAddAttribute(param, "value", mpr);
                CAddNode(dst_fifo_node, param);
                /// @todo Max_cons_rate
                mpr = getMaxChannelRate(src_map_node, src_fifo_name, "dst");
                param = CNewNode("parameter");
                CAddAttribute(param, "id", "Max_cons_rate");
                CAddAttribute(param, "type", "int");
                CAddAttribute(param, "value", mpr);
                CAddNode(dst_fifo_node, param);


                CString app_name = CGetAttribute(src_map_node, "appGraph");
                CString prod_port = getChannelActorOrPort(src_map_node, src_fifo_name, "srcPort");
                int num = applicationInputFileGetFifoArgument(app_name, prod, prod_port);
                param = CNewNode("parameter");
                CAddAttribute(param, "id", "PTask_outport");
                CAddAttribute(param, "type", "int");
                CAddAttribute(param, "value", num);


                CAddNode(dst_fifo_node, param);
                CString consm_port = getChannelActorOrPort(src_map_node, src_fifo_name, "dstPort");
                num = applicationInputFileGetFifoArgument(app_name, consm, consm_port);
                param = CNewNode("parameter");
                CAddAttribute(param, "id", "CTask_inport");
                CAddAttribute(param, "type", "int");
                CAddAttribute(param, "value", num);
                CAddNode(dst_fifo_node, param);

                //Write out the firing rule.
                CString firing_rule = applicationInputFileGetFiringRule(app_name, prod, prod_port);
                param = CNewNode("parameter");
                CAddAttribute(param, "id", "PTask_firing-rule");
                CAddAttribute(param, "type", "string");
                CAddAttribute(param, "value", firing_rule);
                CAddNode(dst_fifo_node, param);

                firing_rule = applicationInputFileGetFiringRule(app_name, consm, consm_port);
                param = CNewNode("parameter");
                CAddAttribute(param, "id", "CTask_firing-rule");
                CAddAttribute(param, "type", "string");
                CAddAttribute(param, "value", firing_rule);
                CAddNode(dst_fifo_node, param);

                CNode *dst_fifo_com_node = CNewNode("communication");

                if (findChannelTile(src_map_node, src_fifo_name, "in") !=
                    findChannelTile(src_map_node, src_fifo_name, "out"))
                {
                    CNode *src_tile_in = findChannelTile(src_map_node, src_fifo_name, "in");
                    CString src_tile_in_name = CGetAttribute(src_tile_in, "name");
                    CNode *src_tile_out = findChannelTile(src_map_node, src_fifo_name, "out");
                    CString src_tile_out_name = CGetAttribute(src_tile_out, "name");

                    CSetAttribute(dst_fifo_com_node, "type", "remote");
                    // Always use DMA for transfers over fifo.
                    CSetAttribute(dst_fifo_com_node, "DMA", "Yes");

                    /// get the appropiate DMA? lookup the cmemout that this channel uses.
                    param = CNewNode("parameter");
                    CAddAttribute(param, "id", "DMA_id");
                    CAddAttribute(param, "type", "string");
                    CString src_dma_name = getChannelDMA(src_tile_out, src_fifo_name);

                    CAddAttribute(param, "value", src_dma_name);
                    CAddNode(dst_fifo_com_node, param);

                    /// @todo the id's of the different parameters are not stable.
                    /// The values used are the ones I got from Radu.
                    param = CNewNode("parameter");
                    CAddAttribute(param, "id", "pRC");
                    CAddAttribute(param, "type", "string");
                    CAddAttribute(param, "value", src_tile_out_name);
                    CAddNode(dst_fifo_com_node, param);
                    param = CNewNode("parameter");
                    CAddAttribute(param, "id", "pWC");
                    CAddAttribute(param, "type", "string");
                    CAddAttribute(param, "value", src_tile_out_name);
                    CAddNode(dst_fifo_com_node, param);

                    param = CNewNode("parameter");
                    CAddAttribute(param, "id", "pbuf");
                    CAddAttribute(param, "type", "string");
                    CAddAttribute(param, "value", src_tile_out_name);
                    CAddNode(dst_fifo_com_node, param);


                    param = CNewNode("parameter");
                    CAddAttribute(param, "id", "cRC");
                    CAddAttribute(param, "type", "string");
                    CAddAttribute(param, "value", src_tile_in_name);
                    CAddNode(dst_fifo_com_node, param);
                    param = CNewNode("parameter");
                    CAddAttribute(param, "id", "cWC");
                    CAddAttribute(param, "type", "string");
                    CAddAttribute(param, "value", src_tile_in_name);
                    CAddNode(dst_fifo_com_node, param);

                    param = CNewNode("parameter");
                    CAddAttribute(param, "id", "cbuf");
                    CAddAttribute(param, "type", "string");
                    CAddAttribute(param, "value", src_tile_in_name);
                    CAddNode(dst_fifo_com_node, param);

                    param = CNewNode("parameter");
                    CAddAttribute(param, "id", "data");
                    CAddAttribute(param, "type", "string");
                    CAddAttribute(param, "value", src_tile_in_name);
                    CAddNode(dst_fifo_com_node, param);

                }

                else
                {
                    CSetAttribute(dst_fifo_com_node, "type", "local");
                }

                CAddNode(dst_fifo_node, dst_fifo_com_node);

                CAddNode(dst_app_node, dst_fifo_node);
            }

            CAddNode(dst_root, dst_app_node);
        }

        catch (CException &e)
        {
            throw CException(CString(__FUNCTION__) +
                             ": Failed write <system><application> + <fifo>.",
                             e);
        }
    }
    /**
     * This writes out all the processor tiles in the mapping file.
     *
     * The tiles are found by looking in the <architectureGraph><tile>
     *
     * \f$num\_slots = \frac{wheelsize}{contextSwitch+slotlength}\f$
     *
     * @param src_root The root node of the source file.
     * @param src_map_node The node of the selected mapping in the source file.
     * @param dst_root The destination file.
     */
    void OutputConversionCompSoCPlatform::writeMappingProcessorTiles(
        CNode *src_root, CNode *src_map_node, CNode *dst_root)
    {
        try
        {
            CNode *src_arch_graph = getArchitectureGraph(src_map_node);
            // Data can be get from architectureGraph/tile
            CNode *src_tile = CFindNode(src_arch_graph, "tile");

            for (; src_tile; src_tile = CNextNode(src_tile, "tile"))
            {

                /* skip tiles withouth cpu's*/
                if (!CHasChildNode(src_tile, "processor"))
                    continue;

                // Add Tile node.
                CNode *dst_tile = CNewNode("processor_tile");
                // Copy id
                CString src_tile_id = CGetAttribute(src_tile, "name");
                CSetAttribute(dst_tile, "id", src_tile_id);

                // Get the clock this tile runs at.
                double clock = architectureInputFileReadTileClockSpeed(src_tile_id);

                // Get the arbitration node
                CNode *src_arb = CFindNode(src_tile, "arbitration");

                if (src_arb == NULL)
                    throw CException("Aribtration not found for this src_tile");

                // Get params needed to calculate the TDM
                int slotLength = CGetAttribute(src_arb, "slotLength");
                int overhead = CGetAttribute(src_arb, "contextSwitchOverhead");
                int wheelSize = CGetAttribute(src_arb, "wheelsize");
                int slots = (wheelSize) / (slotLength + overhead);


                // Write out the application scheduler.
                CNode *dst_app_scheduler = CNewNode("app_scheduler");

                /// @todo We always assume that the app_scheduler type is TDM
                CSetAttribute(dst_app_scheduler, "type", "TDM");

                // Set the length of the OS slot.
                // This is context switch time (+-)
                CNode *param = CNewNode("parameter");
                CAddAttribute(param, "id", "OS_slot_len");
                CAddAttribute(param, "type", "int");
                CAddAttribute(param, "value", overhead / clock);
                CAddNode(dst_app_scheduler, param);

                // set task slot length (slotLength
                param = CNewNode("parameter");
                CAddAttribute(param, "id", "task_slot_len");
                CAddAttribute(param, "type", "int");
                CAddAttribute(param, "value", slotLength / clock);
                CAddNode(dst_app_scheduler, param);

                /// @todo Hardcode the number of slots.
                param = CNewNode("parameter");
                CAddAttribute(param, "id", "num_slots");
                CAddAttribute(param, "type", "int");
                CAddAttribute(param, "value", slots);
                CAddNode(dst_app_scheduler, param);


                CAddNode(dst_tile, dst_app_scheduler);
                CAddNode(dst_root, dst_tile);

            }
        }

        catch (CException &e)
        {
            throw CException(CString(__FUNCTION__) +
                             ": Failed write processor tile.",
                             e);
        }
    }
    /**
     * This is the root function that will write out the mapping file to %file.
     *
     * If  #mapping is specified and larger or equal to 0 then this mapping
     * will be used in the output.
     * By default it will take the 2nd mapping after the initial one.
     * This is done because the first mapping result is often all the tasks mapped
     * to one node.
     *
     *
     * @param file to write the mapping graph to.
     * @param mapping what mapping to pick. (default -1)
     */
    void OutputConversionCompSoCPlatform::writeMappingFile(CString output_file, int mapping)
    {
        logInfo("Creating mapping file: " + output_file);
        CDoc *output_doc = CNewDoc();
        CNode *src_root = CGetRootNode(input_doc);
        CNode *dst_root = CNewNode("system");

        CSetPrivateDTD(output_doc, "system", "etc/dtd/composegrm.dtd");


        CSetRootNode(output_doc, dst_root);
        /// @todo what should the id here be? Now I hardcode it to template.
        CSetAttribute(dst_root, "id", "template");

        try
        {
            /// @todo:  Select a mapping based on input user or first if not spec.
            CNode *src_map_node = CFindNode(src_root, "mapping");

            // If no mapping is found, bail out.
            if (src_map_node == NULL)
            {
                throw CException("No usable mapping found");
            }


            if (mapping < 0)
            {
                while (CGetAttribute(src_map_node, "name") == "initial")
                {
                    src_map_node = CNextNode(src_map_node, "mapping");
                }

                src_map_node = CNextNode(src_map_node, "mapping");

            }

            else
            {
                while (CGetAttribute(src_map_node, "name") != CString(mapping))
                {
                    src_map_node = CNextNode(src_map_node, "mapping");
                }

                if (CGetAttribute(src_map_node, "name") != CString(mapping))
                {
                    throw CException("Failed to find mapping: " + CString(mapping));

                }
            }

            CString map_id = CGetAttribute(src_map_node, "name");
            logInfo("Using mapping " + map_id);

            /// @todo write out the slack, ignored for now.

            // Write processor tile
            writeMappingProcessorTiles(src_root, src_map_node, dst_root);

            // write application.
            writeMappingApplications(src_root, src_map_node, dst_root);


            // Save
            if (!CValidate(output_doc))
            {
                logWarning("Mapping file does not validate");
            }

            CSaveFile(output_file, output_doc, 1);
            logInfo("Saving mapping file: " + output_file);
        }

        catch (CException &e)
        {
            throw CException(CString(__FUNCTION__) +
                             ": Failed write mapping file.",
                             e);
        }

        delete output_doc;
    }

    /**
     * Tell the output converter where the application file is.
     * This file is needed to get the clock.
     *
     * @param input_file
     */
    void OutputConversionCompSoCPlatform::setArchitectureInputFile(const CString &input_file)
    {
        architecture_document  = CParseFile(input_file);

        if (architecture_document  == NULL)
        {
            // Failed to open document. Throw error.
            throw CException(CString(__FUNCTION__) + "Failed to application file: " + input_file);
        }

        architectureInputFileReadDefaultClockSpeed();
    }

    /**
     * Parses the architecture file to find the clock.
     * This is done by walking through the parameters, finding the clk entry.
     * Then looking up the clock period (in ns) off this clock in the separate
     * <clk /> tags.
     *
     * This is the NocSpeeed. This is the default speed for tiles.
     * Each time however can have a different clockspeed.
      */
    void OutputConversionCompSoCPlatform::architectureInputFileReadDefaultClockSpeed()
    {
        ASSERT(architecture_document != NULL, "No application file has been set");
        CNode *root = CGetRootNode(architecture_document);


        // Lookup the name of the clock.
        CString clk_name;
        try
        {
            CNode *src_param = CFindNodeWithAttribute(root, "parameter", "id", "clk");
            clk_name = CGetAttribute(src_param, "value");

            CNode *src_clk = CFindNodeWithAttribute(root, "clk", "id", clk_name);
            cycleTime = CGetAttribute(src_clk, "period");

            logInfo("We found a clock: " + clk_name + " (" +
                    CString(1e3 / cycleTime) + " MHz)");
        }
        catch (CException e)
        {
            throw CException("Failed to find the default clock for the"
                             " current architecture", e);
        }
    }
    /**
     * Parses the architecture file to find the clock for a specific file..
     * This is done by walking through the parameters inside the tile, finding the clk entry.
     * Then looking up the clock period (in ns) off this clock in the separate
     * <clk /> tags.
     *
     * If no clock is specified, the default (NoC clock is used.
     *
     * @param tile_name the name of the tile to get the clock for.
     * @returns the number of ns per cycle.
     */
    double OutputConversionCompSoCPlatform::architectureInputFileReadTileClockSpeed(
        const CString tile_name)
    {
        ASSERT(architecture_document != NULL, "No application file has been set");
        CNode *root = CGetRootNode(architecture_document);
        CNode *src_ip = CFindNodeWithAttribute(root, "ip", "id", tile_name);

        CString clk_name;

        /* This is the right tile */
        for (CNode *src_param = CFindNode(src_ip, "parameter");
             clk_name.empty() && src_param != NULL;
             src_param = CNextNode(src_param, "parameter"))
        {
            if (CHasAttribute(src_param, "id") &&
                CGetAttribute(src_param, "id") == "clk")
            {
                clk_name = CGetAttribute(src_param, "value");
            }
        }
        if (clk_name.empty())
        {
            logInfo("Using default clock for tile: " + tile_name);
            return cycleTime;
        }

        /* lookup the clock. */
        CNode *src_clk = CFindNodeWithAttribute(root, "clk", "id", clk_name);
        double retv = CGetAttribute(src_clk, "period");
        logInfo("We found a clock: " + clk_name + " (" + CString(1e3 / retv) +
                " MHz) for tile: " + tile_name);
        return retv;
    }
    /**
     * Tell the output converter where the application file is.
     * This file is needed to get data/time unit and to figure out what argument
     * maps to what channel.
     *
     * @param input_file pointer to the application input file.
     */
    void OutputConversionCompSoCPlatform::setApplicationInputFile(const CString &input_file)
    {
        application_document = CParseFile(input_file);

        if (application_document == NULL)
        {
            // Failed to open document. Throw error.
            throw CException(CString(__FUNCTION__) + "Failed to application file: " + input_file);
        }

        applicationInputFileReadUnits();
    }
    /**
     * Parse the application file, and look for the time/dataUnit.
     *
     * This function fills in #timeUnit and #dataUnit.
     * It does this by looking in <sdf><applicationGraph><csdfProperties><units>
     *
     *
     * <sdf><applicationGraph><csdfProperties><units><dataUnits>\n
     * #dataUnit:
     *
     * It supports parsing:
     * -              b  - bit
     * -              B  - Byte
     * -              kb - 1024 bit
     * -              kB - 1024 byte
     * -              Mb - 1024*1024 bit
     * -              MB - 1024*1024 byte
     *
     * <sdf><applicationGraph><csdfProperties><units><timeUnits>\n
     * #timeUnit:
     *
     * It supports parsing:
     * -              us - micro second
     * -              ns - nano second
     *
     * It will throw an exception on:
     * -              cc - clock cycle
     *
     *
     */
    void OutputConversionCompSoCPlatform::applicationInputFileReadUnits()
    {
        ASSERT(application_document != NULL, "No application file has been set");

        CNode *root = CGetRootNode(application_document);

        CNode *src_appgraph = CFindNode(root, "applicationGraph");

        if (src_appgraph == NULL)
            throw CException("Could not find applicationGraph");

        // <sdf><applicationGraph><csdfProperties>
        CNode *src_csdfprop = CFindNode(src_appgraph, "csdfProperties");

        if (src_csdfprop == NULL)
            throw CException("Could not find applicationGraph/csdfProperties");

        // <sdf><applicationGraph><csdfProperties><units>
        CNode *src_units = CFindNode(src_csdfprop, "units");

        if (src_units == NULL)
            throw CException("Could not find applicationGraph/csdfProperties/units");

        // <sdf><applicationGraph><csdfProperties><units><dataUnits>
        CNode *src_dataUnit = CGetChildNode(src_units, "dataUnit");

        if (src_dataUnit != NULL)
        {
            CString du = CGetNodeContent(src_dataUnit);
            // Remote white spaces
            du.trim();

            if (du.length() < 2)
                throw CException("Invalid data unit field: " + du);

            switch (du[du.length() - 1])
            {
                case 'b':
                    dataUnit /= 8;
                    break;
                case 'B':
                    dataUnit *= 1;
                    break;
                default:
                    throw CException("Unknown data unit type: " + du);
            }

            switch (du[du.length() - 2])
            {
                case 'M':
                    dataUnit *= 1024;
                case 'k':
                    dataUnit *= 1024;
                    break;
                default:
                    break;
            }

            dataUnit *= double(du);
        }

        // <sdf><applicationGraph><csdfProperties><timeUnit>
        CNode *src_timeUnit = CGetChildNode(src_units, "timeUnit");

        if (src_timeUnit != NULL)
        {
            CString tu = CGetNodeContent(src_timeUnit);
            // Remote white spaces
            tu.trim();

            if (tu.length() < 2)
                throw CException("Invalid time unit field: " + tu);

            CString type = tu.substr(tu.length() - 2, 2);

            if (type == "cc")
            {
                // I would need a clock to convert this.
                throw(CException("Clock units in cycles not supported"));
            }

            else if (type == "us")
            {
                timeUnit = 1e3;
            }

            else if (type == "ns")
            {
                timeUnit = 1;
            }

            else
            {
                throw CException("Unknown time unit type: " + tu);
            }

            timeUnit *= double(tu);
        }

        logInfo("Reading unit conversion values: time unit: " +
                CString(timeUnit) + "ns, data unit: " + CString(dataUnit) + "bytes");
    }

    /**
     * Get the firing rule for the application %app_name, actor: %actor_name and
     * %portname.
     *
     * It does this by looking up the rates. The rates are expanded from the compact
     * for for easy parsing in the next step. (2x3,1 -> 3,3,1)
     *
     * @param app_name The application name.
     * @param actor_name The name of the actor.
     * @param port_name The name of the port.
     * @return returns the firing pattern in for 1,2,3,4
     */
    CString OutputConversionCompSoCPlatform::applicationInputFileGetFiringRule(
        CString app_name, CString actor_name, CString port_name)
    {
        try
        {
            CNode *app_root = CGetRootNode(application_document);
            // from channel you can get actorname/port_name, this function
            // should return the argument number it is in the code implementation.
            CNode *app_graph = CFindNodeWithAttribute(app_root, "applicationGraph",
                               "name", app_name);
            CNode *csdf_graph = CFindNode(app_graph, "csdfgraph");
            ASSERT(csdf_graph != NULL, "No csdfgraph found");

            // Get the actor */
            CNode *actor_node = CFindNodeWithAttribute(csdf_graph, "actor",
                                "name", actor_name);
            // Get the port
            CNode *port_node = CFindNodeWithAttribute(actor_node, "port",
                               "name", port_name);
            // Expand the string so it is easy to parse for genapp
            // no more 2x1,3x2 etc.
            CString rate_string = CGetAttribute(port_node, "rate");
            vector<CString> rates = expand(rate_string);
            // lovely C++ voodoo
            CStrings result(rates.begin(), rates.end());

            return CString::join(result, ',');
        }
        catch (CException e)
        {
            throw("Failed to find firing rules for application: " + app_name +
                  "actor: " + actor_name + " port: " + port_name, e);
        }
    }
    /**
     * @todo: comment
     *
     * @param app_name The application name.
     * @param actor_name The name of the actor.
     * @param port_name The name of the port.
     * @return returns  "in" or "out"
     */
    CString OutputConversionCompSoCPlatform::applicationInputFileGetPortDirection(
        CString app_name, CString actor_name, CString port_name)
    {
        try
        {
            CNode *app_root = CGetRootNode(application_document);
            // from channel you can get actorname/port_name, this function
            // should return the argument number it is in the code implementation.
            CNode *app_graph = CFindNodeWithAttribute(app_root, "applicationGraph",
                               "name", app_name);
            CNode *csdf_graph = CFindNode(app_graph, "csdfgraph");
            ASSERT(csdf_graph != NULL, "No csdfgraph found");

            // Get the actor */
            CNode *actor_node = CFindNodeWithAttribute(csdf_graph, "actor",
                                "name", actor_name);
            // Get the port
            CNode *port_node = CFindNodeWithAttribute(actor_node, "port",
                               "name", port_name);
            // Expand the string so it is easy to parse for genapp
            // no more 2x1,3x2 etc.
            CString type_string = CGetAttribute(port_node, "type");
            return type_string;
        }
        catch (CException e)
        {
            throw("Failed to find port direction:: " + app_name +
                  "actor: " + actor_name + " port: " + port_name, e);
        }
    }


    /**
     * Quick sort function for list.
     * @param a
     * @param b
     * @return
     */
    static bool arguments_sort(std::pair<int, CString> a, std::pair<int, CString> b)
    {
        return a.first == b.first;
    }
    /**
     * Gets the argument number for the application %app_name, actor: %actor_name and
     * %portname.
     *
     * This argument number will match up the fifo number in the actual code.
     * So e.g. port p1 equals fifo 0 in the C code.
     *
     * @param app_name The application name.
     * @param actor_name The name of the actor.
     * @param port_name The name of the port.
     * @return the fifo number (integer)
     */
    int OutputConversionCompSoCPlatform::applicationInputFileGetFifoArgument(
        CString app_name, CString actor_name, CString port_name)
    {
        try
        {
            list<std::pair<int, CString> >  input;
            list<std::pair<int, CString> >  output;

            CNode *app_root = CGetRootNode(application_document);
            // from channel you can get actorname/port_name, this function
            // should return the argument number it is in the code implementation.
            CNode *app_graph = CFindNodeWithAttribute(app_root, "applicationGraph",
                               "name", app_name);

            CNode *csdf_prop = CFindNode(app_graph, "csdfProperties");
            ASSERT(csdf_prop != NULL, "No csdfProperties found");

            CNode *actor_prop = CFindNodeWithAttribute(csdf_prop, "actorProperties",
                                "actor", actor_name);

            CNode *impl_node = CFindNode(actor_prop, "implementation");
            ASSERT(impl_node != NULL, "Failed to find implementation node");

            CNode *func_node = CFindNode(impl_node, "function");
            ASSERT(func_node != NULL, "Failed to find function node");

            CNode *argmap_node = CFindNode(func_node, "argumentMapping");
            ASSERT(argmap_node != NULL, "Failed to find argumentMapping node");

            for (CNode *arg_node = CFindNode(argmap_node, "argument"); arg_node != NULL;
                 arg_node = CNextNode(arg_node, "argument"))
            {
                CString port = CGetAttribute(arg_node, "port");
                int number = CGetAttribute(arg_node, "number");
                CString dir =  applicationInputFileGetPortDirection(app_name, actor_name, port);

                if (dir == "in")
                {
                    input.push_back(std::pair<int, CString>(number, port));
                }
                else
                {
                    output.push_back(std::pair<int, CString>(number, port));
                }
            }
            // Sort the list by there argument number.
            input.sort(arguments_sort);
            output.sort(arguments_sort);

            CString dir =  applicationInputFileGetPortDirection(app_name, actor_name, port_name);
            if (dir == "in")
            {
                // Get the what number in the list is our port.
                int num = 0;
                for (list<std::pair<int, CString> >::iterator a = input.begin(); a != input.end(); a++)
                {
                    if (a->second == port_name) return num;
                    num++;
                }
            }
            else
            {
                // Get the what number in the list is our port.
                int num = 0;
                for (list<std::pair<int, CString> >::iterator a = output.begin(); a != output.end(); a++)
                {
                    if (a->second == port_name) return num;
                    num++;
                }
            }

            throw CException(CString(__FUNCTION__) + ": Failed to find the argument number"
                             "for actor: " + actor_name + " port: " + port_name);

        }
        catch (CException e)
        {
            throw CException(CString(__FUNCTION__) + ": Failed to find the argument number"
                             "for actor: " + actor_name + " port: " + port_name, e);
        }
    }

    // end namespace
}

