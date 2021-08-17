/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   output_conversion.cc
 *
 *  Author          :   Umar Waqas (u.waqas@student.tue.nl)
 *
 *  Date            :   November 16, 2011
 *
 *  Function        :   Convert the output files to a compatible format.
 *
 *  History         :
 *      16-11-11    :   Initial version.
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

#include "output_conversion.h"
#include "base/log/log.h"
namespace FSMSADF
{
    /**
     *
     * Write the converted structure out to file outfile.
     *
     * @param outfile Path to the file to write the result too.
     */
    void OutputConversionMampsPlatform::writeOutput(const std::string outfile)
    {
        ASSERT(output_doc != NULL, "No output document exists.");

        if (!CValidate(output_doc))
        {
            logWarning("Failed to validate output document: '" + outfile + "'");
        }
        // write the result out.
        CSaveFile(outfile, output_doc, 1);
    }
    /**
     * Convert the processor information from the
     * source processor and source tile to the destination
     * processor.
     *
     * @param arch_src source architecture graph
     * @param tile_src source tile
     * @param processor_dst destination processor
     */
    void OutputConversionMampsPlatform::convertProcessor(CNode *arch_src,
            CNode *tile_src,
            CNode *processor_dst)
    {
        CNode *processor_src = NULL;

        try
        {
            processor_src = CFindNode(tile_src, "processor");
        }
        catch (CException e)
        {
            throw CException("Processor not found in tile '" + CGetAttribute(tile_src, "name") + "'.");
        }
        if (!CHasAttribute(tile_src, "name"))
            throw CException("Tile name not found for the tile in mapping specification.");

        CNode *archTile_src;
        try
        {
            archTile_src = CFindNodeWithAttribute(arch_src, "tile", "name", CGetAttribute(tile_src, "name"));
        }
        catch (CException e)
        {
            throw CException("Tile '" + CGetAttribute(tile_src, "name") +
                             "' not found in the architecture graph specification.");
        }

        CNode *arch_processor_src;
        try
        {
            arch_processor_src = CFindNodeWithAttribute(archTile_src, "processor", "name",
                                 CGetAttribute(processor_src, "name"));
        }
        catch (CException e)
        {
            throw CException("Processor '" + CGetAttribute(processor_src, "name") +
                             "' not found in the architecture specification of tile '" +
                             CGetAttribute(tile_src, "name") + "'.");
        }

        /* extracrt the required information from the architecture specification of processor */
        CSetAttribute(processor_dst, "name", CGetAttribute(arch_processor_src, "name"));

        if (!CHasAttribute(arch_processor_src, "type"))
            throw CException("Processor type not specified for '" +
                             CGetAttribute(arch_processor_src, "name") + "' in tile '" + CGetAttribute(tile_src, "name")
                             + "'.");

        CSetAttribute(processor_dst, "type", CGetAttribute(arch_processor_src, "type"));

        /* copy the actors mapped to this processor */
        CNode *actor_src = CFindNode(processor_src, "actor");

        for (; actor_src != NULL ; actor_src = CNextNode(actor_src, "actor"))
        {
            CNode *actor_dst = CNewNode("actor");
            if (CHasAttribute(actor_src, "name"))
                CSetAttribute(actor_dst, "name", CGetAttribute(actor_src, "name"));
            CAddNode(processor_dst, actor_dst);
        }
    }
    /**
     * Convert the actor information from the
     * source actor and source tile to the destination
     * actor.
     *
     * @param appGraph source architecture graph
     * @param actor_src source actor
     * @param memory_dst memory destination
     */
    void OutputConversionMampsPlatform::convertActor(CNode *appGraph, CNode *actor_src, CNode *memory_dst)
    {

        if (!CHasAttribute(actor_src, "name"))
            throw CException("No name found for actor in memory specification.");

        CString actor_name = CGetAttribute(actor_src, "name");


        /* has the actor already added to the memory_dst
           All memory mappings of each actor are added at once. The
           mapping consists redundent specifications for each actor even for the memories with
           size = 0.
           However, in the required mapping only memories having non-zero size are specifed.
         */

        CNode *memory_actor;

        try
        {

            memory_actor = CFindNodeWithAttribute(memory_dst, "actor", "name", actor_name);
        }
        catch (CException e)
        {
            memory_actor = NULL;
        }

        if (memory_actor == NULL)
        {
            CNode *default_prop = CFindNode(appGraph, "defaultProperties");

            if (default_prop == NULL)
                throw CException("Default properties not found in the application graph.");

            CNode *actor_prop_src;
            try
            {
                actor_prop_src = CFindNodeWithAttribute(default_prop, "actorProperties",
                                                        "actor", actor_name);
            }
            catch (CException e)
            {
                throw CException("Actor properties not found for actor '" + actor_name + "'.");
            }



            CNode *actor_dst = CNewNode("actor");
            CSetAttribute(actor_dst, "name", actor_name);


            CNode *code;

            try
            {
                code = CFindNodeWithAttribute(actor_prop_src, "memory", "name", ".code");
            }
            catch (CException e)
            {
                code = NULL;
            }

            CNode *data;
            try
            {
                data = CFindNodeWithAttribute(actor_prop_src, "memory", "name", ".data");
            }
            catch (CException e)
            {
                data = NULL;
            }

            CNode *sharedVar;

            try
            {
                sharedVar = CFindNodeWithAttribute(actor_prop_src, "memory", "name", ".sharedVar");
            }
            catch (CException e)
            {
                sharedVar = NULL;
            }

            int size = 0;
            if (code != NULL)
            {
                if (!CHasAttribute(code, "size"))
                    throw CException("Size is required for memory of type .code for actor '"
                                     + actor_name + "'");

                size += atoi(CGetAttribute(code, "size"));
            }

            if (data != NULL)
            {
                if (!CHasAttribute(data, "size"))
                    throw CException("Size is required for memory of type .data for actor '"
                                     + actor_name + "'");

                size += atoi(CGetAttribute(data, "size"));
            }

            if (sharedVar != NULL)
            {
                if (!CHasAttribute(sharedVar, "size"))
                    throw CException("Size is required for memory of type .sharedVar for actor '"
                                     + actor_name + "'");

                size += atoi(CGetAttribute(sharedVar, "size"));
            }

            char buffer[20];

            /* add the calculated size to the actor specification */

            sprintf(buffer, "%d", size);

            CSetAttribute(actor_dst, "size", buffer);

            /* add the actor to the memory specification */
            CAddNode(memory_dst, actor_dst);

        }
    }
    /**
     * load the channel information to the destination format.
     *
     * @param appGraph source architecture graph
     * @param sdf3_src source graph
     * @param channel_src source channel
     * @param memory_dst destination memory
     */
    void OutputConversionMampsPlatform::convertChannel(CNode *appGraph, CNode *sdf3_src,
            CNode *channel_src, CNode *memory_dst)
    {

        if (!CHasAttribute(channel_src, "name"))
            throw CException("No name found for actor in memory specification.");

        CString channel_name = CGetAttribute(channel_src, "name");


        CNode *default_prop = CFindNode(appGraph, "defaultProperties");

        if (default_prop == NULL)
            throw CException("Default properties not found in the application graph.");

        CNode *channel_prop_src;
        try
        {
            channel_prop_src = CFindNodeWithAttribute(default_prop, "channelProperties",
                               "channel", channel_name);
        }
        catch (CException e)
        {
            throw CException("Channel properties not found for channel '" + channel_name + "'.");
        }

        CNode *channel_dst = CNewNode("channel");

        CNode *token_size = CFindNode(channel_prop_src, "tokenSize");

        if (token_size == NULL)
            throw CException("Token size not specified for channel '" +
                             channel_name + "' in default properties.");

        if (!CHasAttribute(token_size, "sz"))
            throw CException("Token size attribute 'sz 'not specified for channel '" + channel_name
                             + "' in default properties of the application graph.");

        int size1 = atoi(CGetAttribute(token_size, "sz"));

        CNode *channel_const;

        try
        {
            channel_const = CFindNodeWithAttribute(sdf3_src, "channelConstraints",
                                                   "channel", channel_name);

        }
        catch (CException e)
        {
            throw CException("Channel constraints not found for channel '" + channel_name + "'");
        }

        CNode *buffer_size = CFindNode(channel_const, "bufferSize");

        if (buffer_size == NULL)
            throw CException("Buffer size not found in the channel constraints for channel '" +
                             channel_name + "'.");
        if (!CHasAttribute(buffer_size, "mem"))
            throw CException("Mem not specified in buffer size in channel constraints for channel'" +
                             channel_name + "'.");

        int size2 = atoi(CGetAttribute(buffer_size, "mem"));

        char buffer[20];
        sprintf(buffer, "%d", size1 * size2);

        CSetAttribute(channel_dst, "size", buffer);
        CSetAttribute(channel_dst, "name", channel_name);

        CAddNode(memory_dst, channel_dst);


    }
    /**
     * load the network interface channel information to the destination format.
     *
     * @param resourceUsage The resource usage information present in the mapping
     * @param ni_channel_src source channel
     * @param ni_dst destination network interface
     * @param hasInAttribute Boolean specifiying whether in bandwidth == 0
     * @param ni_name name of the network interface
     * @param tileName name of tile
     */
    void OutputConversionMampsPlatform::convertNetworkInterfaceChannel(CNode *resourceUsage,
            CNode *ni_channel_src,
            CNode *ni_dst,
            bool hasInAttribute,
            CString ni_name,
            CString tileName)
    {
        CNode *channel_dst = CNewNode("channel");

        if (!CHasAttribute(ni_channel_src, "name"))
            throw CException("Channel name not found.");

        CString channel_name = CGetAttribute(ni_channel_src, "name");
        CSetAttribute(channel_dst, "name", channel_name);
        CSetAttribute(channel_dst, "nrConnections", "1");

        CNode *tile_resourceUsage;
        try
        {
            tile_resourceUsage = CFindNodeWithAttribute(resourceUsage, "tile", "name", tileName);
        }
        catch (CException e)
        {
            throw CException("No resource usage found for tile '" + tileName + "'.");
        }

        CNode *ni_resource_usage;

        try
        {
            ni_resource_usage = CFindNodeWithAttribute(tile_resourceUsage, "networkInterface",
                                "name", ni_name);
        }
        catch (CException e)
        {
            throw CException("No resource usage found for network interface '" + ni_name + "'.");
        }

        if (hasInAttribute)
        {
            CSetAttribute(channel_dst, "inBandwidth", CGetAttribute(ni_resource_usage, "inBandwidth"));
            CSetAttribute(channel_dst, "outBandwidth", "0");
        }
        else
        {
            CSetAttribute(channel_dst, "outBandwidth", CGetAttribute(ni_resource_usage, "outBandwidth"));
            CSetAttribute(channel_dst, "inBandwidth", "0");
        }

        CAddNode(ni_dst, channel_dst);
    }
    /**
     * convert the network interface
     *
     * @param appGraph application graph
     * @param sdf3_src source graph
     * @param ni_src source network interface
     * @param tile_dst destination tile
     * @param tileName name of tile
     */
    void OutputConversionMampsPlatform::convertNetworkInterface(CNode *appGraph, CNode *sdf3_src,
            CNode *ni_src, CNode *tile_dst,
            CString tileName)
    {
        CNode *mapping;

        try
        {
            mapping = CFindNodeWithAttribute(sdf3_src, "mapping", "name", "0");
        }
        catch (CException e)
        {
            throw CException("Mapping '0' not found in the mapping specification.");
        }

        CNode *resourceUsage = CFindNode(mapping, "resourceUsage");

        if (resourceUsage == NULL)
            throw CException("Resource usage not found in the mapping specification.");

        if (!CHasAttribute(ni_src, "name"))
            throw CException("No name found for the network interface defined in tile '" +
                             tileName + "'.");

        CString ni_name = CGetAttribute(ni_src, "name");

        CNode *ni_dst = CNewNode("networkInterface");
        CSetAttribute(ni_dst, "name", ni_name);

        CNode *ni_channel_src = CFindNode(ni_src, "channel");
        for (; ni_channel_src != NULL; ni_channel_src = CNextNode(ni_channel_src, "channel"))
            convertNetworkInterfaceChannel(resourceUsage, ni_channel_src,
                                           ni_dst, CHasAttribute(ni_channel_src, "in"),
                                           ni_name, tileName);

        CAddNode(tile_dst, ni_dst);

    }
    /**
     * convert memory.
     *
     * @param appGraph application graph
     * @param sdf3_src source graph
     * @param tile_src source tile
     * @param memory_dst destination memory
     */
    void OutputConversionMampsPlatform::convertMemory(CNode *appGraph, CNode *sdf3_src,
            CNode *tile_src, CNode *memory_dst)
    {
        CNode *memory_src = CFindNode(tile_src, "memory");
        CString tileName;

        if (memory_src == NULL)
            throw CException("No memory specification found in tile '" +
                             CGetAttribute(tile_src, "name") + "'.");

        if (CHasAttribute(memory_src, "name"))
        {
            tileName = CGetAttribute(memory_src, "name");
            CSetAttribute(memory_dst, "name", tileName);
        }
        else
            throw CException("Tile name not found.");

        /* load actor mapings */
        CNode *actor_src = CFindNode(memory_src, "actor");
        for (; actor_src != NULL; actor_src = CNextNode(actor_src, "actor"))
            convertActor(appGraph, actor_src, memory_dst);

        /* load channel mapping */
        CNode *channel_src = CFindNode(memory_src, "channel");
        for (; channel_src != NULL; channel_src = CNextNode(channel_src, "channel"))
            convertChannel(appGraph, sdf3_src, channel_src, memory_dst);
    }
    /**
     * convert tile.
     *
     * @param arch_src source architecure graph
     * @param appGraph application graph
     * @param sdf3_src source graph
     * @param tile_src source tile
     * @param tile_dst destination tile
     */
    void OutputConversionMampsPlatform::convertTile(CNode *arch_src, CNode *appGraph, CNode *sdf3_src,
            CNode *tile_src, CNode *tile_dst)
    {
        /* load the processor specification for current tile */
        CNode *processor_dst = CNewNode("processor");
        convertProcessor(arch_src, tile_src, processor_dst);
        CAddNode(tile_dst, processor_dst);

        /* load memory specifications */
        CNode *memory_dst = CNewNode("memory");
        convertMemory(appGraph, sdf3_src, tile_src, memory_dst);
        CAddNode(tile_dst, memory_dst);

        /* load the network interface mapping */
        if (!CHasAttribute(tile_src, "name"))
            throw CException("Tile name not found in the mapping.");

        CString tileName = CGetAttribute(tile_src, "name");
        CNode *ni_src = CFindNode(tile_src, "networkInterface");
        for (; ni_src != NULL; ni_src = CNextNode(ni_src, "networkInterface"))
            convertNetworkInterface(appGraph, sdf3_src, ni_src, tile_dst, tileName);

    }
    /**
     * convert usecase schedule tile.
     *
     * @param tileSrc source tile
     * @param applicationSchedule application schedule present in the mapping
     * @param applicationName name of the application
     */
    void OutputConversionMampsPlatform::convertUsecaseScheduleTile(CNode *tileSrc, CNode *applicationSchedule,
            CString applicationName)
    {
        if (!CHasAttribute(tileSrc, "name"))
            throw CException("Tile name not found in the mapping specification.");

        CString tileName = CGetAttribute(tileSrc, "name");

        CNode *processor_src = CFindNode(tileSrc, "processor");

        if (processor_src == NULL)
            throw CException("No processor specification found in tile '" + tileName + "'.");

        if (!CHasAttribute(processor_src, "name"))
            throw CException("No name found for procesor in tile '" + tileName + "'.");

        CString processorName = CGetAttribute(processor_src, "name");



        CNode *schedule = CNewNode("schedule");
        CSetAttribute(schedule, "tile", tileName);
        CSetAttribute(schedule, "proc", processorName);


        CNode *state_src = CFindNode(processor_src, "state");
        for (; state_src != NULL; state_src = CNextNode(state_src, "state"))
        {
            CNode *state_dst = CNewNode("state");
            CSetAttribute(state_dst, "appl", applicationName);
            CSetAttribute(state_dst, "numberOfRuns", "1");

            if (!CHasAttribute(state_src, "actor"))
                throw CException("Actor not found in the schedule specification of procesor '" +
                                 processorName + "' in tile '" + tileName + "'.");

            CSetAttribute(state_dst, "actor", CGetAttribute(state_src, "actor"));

            if (CHasAttribute(state_src, "startOfPeriodicRegime"))
                CSetAttribute(state_dst, "startOfPeriodicRegime", "true");
            CAddNode(schedule, state_dst);
        }

        CAddNode(applicationSchedule, schedule);
    }

    /**
     * convert usecase schedule.
     *
     * @param sdf3_src source graph
     * @param sdf3_des destination graph
     */
    void OutputConversionMampsPlatform::convertUsecaseSchedule(CNode *sdf3_src, CNode *sdf3_des)
    {
        CNode *mapping;
        try
        {
            mapping = CFindNodeWithAttribute(sdf3_src, "mapping", "name", "0");
        }
        catch (CException e)
        {
            throw CException("Mapping 'Mapping 0' not found in the specification.");
        }

        if (!CHasAttribute(mapping, "appGraph"))
            throw CException("Name of application graph not found in the mapping specification.");

        CString name = CGetAttribute(mapping, "appGraph");

        CNode *usecase_dst = CNewNode("usecaseSchedule");
        CSetAttribute(usecase_dst, "name", "usecase3");

        CNode *applicationSchedule = CNewNode("applicationSchedule");
        CSetAttribute(applicationSchedule, "name", name);
        CSetAttribute(applicationSchedule, "weight", "1");

        CNode *tile_src = CFindNode(mapping, "tile");
        for (; tile_src != NULL; tile_src = CNextNode(tile_src, "tile"))
            convertUsecaseScheduleTile(tile_src, applicationSchedule, name);

        CAddNode(usecase_dst, applicationSchedule);
        CAddNode(sdf3_des, usecase_dst);
    }
    /**
     * convert mapping information.
     *
     * @param sdf3_src source graph
     * @param sdf3_des destination graph
     */
    void OutputConversionMampsPlatform::convertMapping(CNode *sdf3_src, CNode
            *sdf3_des)
    {
        /* mapping is converted according to the format accepted by the Mamps
        platform*/

        CNode *mapping_src = NULL;
        CNode *mapping_dst = NULL;
        CNode *scenario = NULL;
        CNode *archGraph = NULL;
        CNode *appGraph = NULL;

        try
        {
            mapping_src = CFindNodeWithAttribute(sdf3_src, "mapping", "name", "0");
        }
        catch (CException e)
        {
            throw CException("'Mapping 0' is not found in the mapping specification.");
        }

        mapping_dst = CNewNode("mapping");

        if (CHasAttribute(mapping_src, "appGraph"))
        {
            CSetAttribute(mapping_dst, "appGraph", CGetAttribute(mapping_src, "appGraph"));
            CSetAttribute(mapping_dst, "name", CGetAttribute(mapping_src, "appGraph"));
        }

        appGraph = CFindNode(sdf3_src, "applicationGraph");

        if (CHasAttribute(mapping_src, "archGraph"))
            CSetAttribute(mapping_dst, "archGraph", CGetAttribute(mapping_src, "archGraph"));



        try
        {
            scenario = CFindNode(mapping_src, "scenario");
        }
        catch (CException e)
        {
            throw CException("'Scenario' not found in the mapping specification");
        }

        archGraph = CFindNode(sdf3_src, "architectureGraph");

        if (archGraph == NULL)
            throw CException("Architecture graph not found in mapping specification.");

        CNode *currentTile = CFindNode(scenario, "tile");

        /* add all tiles to the mapping */
        for (; currentTile != NULL; currentTile = CNextNode(currentTile, "tile"))
        {
            CNode *tile_dst = CNewNode("tile");

            if (CHasAttribute(currentTile, "name"))
                CSetAttribute(tile_dst, "name", CGetAttribute(currentTile, "name"));
            else
                logInfo("Tile name not specified.");

            convertTile(archGraph, appGraph, sdf3_src, currentTile, tile_dst);

            CAddNode(mapping_dst, tile_dst);
        }

        /* convert the connections */
        convertConnections(mapping_src, mapping_dst);

        CAddNode(sdf3_des, mapping_dst);

        /* convert the usecase schedule */
        convertUsecaseSchedule(sdf3_src, sdf3_des);
    }
    /**
     * convert connections.
     *
     * @param mapping_src source mapping
     * @param mapping_dst destination mapping
     */
    void OutputConversionMampsPlatform::convertConnections(CNode *mapping_src, CNode *mapping_dst)
    {
        CNode *connection = CFindNode(mapping_src, "connection");
        for (; connection != NULL; connection = CNextNode(connection, "connection"))
            CAddNode(mapping_dst, CCopyNode(connection));
    }

    /// constructor
    OutputConversionMampsPlatform::OutputConversionMampsPlatform(CString input_filename)
        :
        OutputConversionBase(input_filename),
        output_doc(NULL)
    {
    }

    /// destructor
    OutputConversionMampsPlatform::~OutputConversionMampsPlatform()
    {
        delete output_doc;
    }
    /**
     * convert root node.
     *
     * @param sdf3_src source graph
     * @param sdf3_dst destination graph
     */
    void OutputConversionMampsPlatform::convertRoot(CNode *sdf3_src, CNode *sdf3_dst)
    {
        if (CHasAttribute(sdf3_src, "version"))
            CSetAttribute(sdf3_dst, "version", CGetAttribute(sdf3_src, "version"));
        CSetAttribute(sdf3_dst, "type", "sdf");

        /* copy the application and architecture file */
        CNode *appGraph = CFindNode(sdf3_src, "applicationGraph");

        if (appGraph == NULL)
            logInfo("Application graph not specified.");
        else
        {
            CNode *app_graph_dst = CCopyNode(appGraph);
            CAddNode(sdf3_dst, app_graph_dst);
        }

        /* copy architecture graph */
        CNode *archGraph = CFindNode(sdf3_src, "architectureGraph");

        if (archGraph == NULL)
            logInfo("Architecture graph not specified.");
        else
        {
            CNode *arch_graph_dst = CCopyNode(archGraph);
            CAddNode(sdf3_dst, arch_graph_dst);
        }
    }
    /// converts the input to a format compatible with MAMPS
    void OutputConversionMampsPlatform::convert()
    {
        CNode *root = CGetRootNode(input_doc);
        CNode *sdf3_src = CCopyNode(root);
        CNode *sdf3_dst = CNewNode("premadona");

        /* create the root node, copy application and architecture specification */
        convertRoot(sdf3_src, sdf3_dst);

        /* convert the mapping */
        convertMapping(sdf3_src, sdf3_dst);

        /* transform the output to the required format */
        output_doc = CNewDoc(sdf3_dst);

    }
    /**
     * writeOutput()
     * Write the converted structure out to #ostream out.
     *
     * @param out The ostream to output the result.
     */
    void OutputConversionMampsPlatform::writeOutput(std::ostream &out)
    {
        ASSERT(output_doc != NULL, "No output document exists.");

        if (!CValidate(output_doc))
        {
            logWarning("Failed to validate output document");
        }
        // write the result out.
        CSaveFile(out, output_doc, 1);
    }

}
