/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   input_conversion.cc
 *
 *  Author          :   Umar Waqas (u.waqas@student.tue.nl)
 *
 *  Date            :   November 16, 2011
 *
 *  Function        :   Convert the input files to a compatible format.
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

#include "input_conversion.h"
#include "../../compsoc_platform/conversion/helper.h"
#include <assert.h>
#include "base/base.h"

namespace FSMSADF
{

    /// constructor

    InputConversionMampsPlatform::InputConversionMampsPlatform()
        :
        InputConversionBase(),
        internal_doc(NULL)
    {
        CNode *root = CNewNode("sdf3");
        output_doc = CNewDoc(root);
        CSetAttribute(root, "type", "fsmsadf");
    }

    /// destructor

    InputConversionMampsPlatform::~InputConversionMampsPlatform()
    {
        delete internal_doc;
    }

    /**
     * Read the architcture file, parse it and
     * store it internally in a datastructure
     *
     * @param input_file The file to read in
     */
    void InputConversionMampsPlatform::readArchitectureFile(const std::string input_file)
    {
        internal_doc = CParseFile(input_file);
        if (internal_doc == NULL)
        {
            throw CException("Failed to read input file.");
        }

        CNode *root = CGetRootNode(internal_doc);
        CNode *application_graph_src = CFindNode(root, "architectureGraph");

        if (application_graph_src == NULL)
        {
            throw CException("No architecture graph found");
        }

        CNode *application_graph_dest = CNewNode("architectureGraph");

        addArchGraph(application_graph_src, application_graph_dest);

        CAddNode(CGetRootNode(output_doc), application_graph_dest);

        internal_doc = NULL;
    }
    /**
    * getNumScenarios()
    * @param arch_graph_tile_src  source node containing the tile information
    * @param arch_graph_tile_dest destination node in the compatible format
    *
    * Loads the tile for a specified architecture from the
    * input and adds to the compatible specification.
    */
    void InputConversionMampsPlatform::addArchGraphTileProcessorsSchedulersArbitration(
        CNode *arch_proc_sched_src,
        CNode *arch_proc_proc_dest)
    {
        CNode *arbitration_src = CFindNode(arch_proc_sched_src, "arbitration");

        /* load all arbitrations for this scheduler */
        for (; arbitration_src != NULL; arbitration_src = CNextNode(arbitration_src, "arbitration"))
        {
            CNode *arbitration_dst = CNewNode("arbitration");

            if (!CHasAttribute(arbitration_src, "type"))
                throw CException("Type fr arbitration is required");

            CSetAttribute(arbitration_dst, "type", CGetAttribute(arbitration_src, "type"));

            if (CHasAttribute(arbitration_src, "wheelsize"))
                CSetAttribute(arbitration_dst, "wheelsize", CGetAttribute(arbitration_src, "wheelsize"));

            /* set the default values */
            CSetAttribute(arbitration_dst, "contextSwitchOverhead", "0");
            CSetAttribute(arbitration_dst, "preemptive", "false");

            CAddNode(arch_proc_proc_dest, arbitration_dst);
        }

    }

    /**
    * @param arch_tile_proc_src The source node of the processor information
    * @param arch_tile_proc_dest The destination node of the compatible format
    *
    * Load the processor scheduler in the compatible format.
    */
    void InputConversionMampsPlatform::addArchGraphTileProcessorsSchedulers(CNode *arch_tile_proc_src,
            CNode *arch_tile_proc_dest)
    {


        CNode *scheduler_src = CFindNode(arch_tile_proc_src, "scheduler");

        if (scheduler_src == NULL)
            throw CException("No scheduler found");

        /* load all schedulers for this processor */
        for (; scheduler_src != NULL; scheduler_src = CNextNode(scheduler_src, "scheduler"))
        {
            /* One arbitration is required for a scheduler */
            if (!CHasChildNode(scheduler_src, "arbitration"))
                throw CException("No arbitration found for scheduler");

            addArchGraphTileProcessorsSchedulersArbitration(scheduler_src, arch_tile_proc_dest);

        }


    }
    /**
    * @param arch_tile_mem_src The source node containing the information of the tile
    * @param arch_tile_mem_dest The destination node to add the tile
    *
    * Load the network interfaces to the compatible format.
    */
    void InputConversionMampsPlatform::addArchGraphTileNetworkInterfaces(CNode *arch_tile_mem_src,
            CNode *arch_tile_mem_dest)
    {
        CNode *network_interface_src = CFindNode(arch_tile_mem_src, "networkinterface");

        for (; network_interface_src != NULL; network_interface_src = CNextNode(network_interface_src, "networkInterface"))
        {
            /* Verify the correctness of the input xml */
            if (!CHasAttribute(network_interface_src, "name"))
                throw CException("Name is required for a Network Interface");

            if (!CHasAttribute(network_interface_src, "model"))
                throw CException("Model is required for a Network Interface");

            CString model = CGetAttribute(network_interface_src, "model");

            if (model != SUPPORTED_MODEL)
                throw CException("Network interface model '" + model + "' is not supported");

            if (!CHasAttribute(network_interface_src, "nrInputConnections"))
                throw CException("Number of input connections  is required for a Network Interface");

            if (!CHasAttribute(network_interface_src, "nrOutputConnections"))
                throw CException("Number of output connections  is required for a Network Interface");

            if (!CHasAttribute(network_interface_src, "inBandwidth"))
                throw CException("In bandwidth is required for a Network Interface");

            if (!CHasAttribute(network_interface_src, "outBandwidth"))
                throw CException("In bandwidth is required for a Network Interface");

            CNode *network_interface_dst = CNewNode("networkInterface");

            /* Extract the  network interface attributes */
            CSetAttribute(network_interface_dst, "name", CGetAttribute(network_interface_src, "name"));
            CSetAttribute(network_interface_dst, "model", CGetAttribute(network_interface_src, "model"));
            CSetAttribute(network_interface_dst, "nrInConnections", CGetAttribute(network_interface_src, "nrInputConnections"));
            CSetAttribute(network_interface_dst, "nrOutConnections", CGetAttribute(network_interface_src, "nrOutputConnections"));
            CSetAttribute(network_interface_dst, "inBandwidth", CGetAttribute(network_interface_src, "inBandwidth"));
            CSetAttribute(network_interface_dst, "outBandwidth", CGetAttribute(network_interface_src, "outBandwidth"));

            /* add the nework interface in the destination */
            CAddNode(arch_tile_mem_dest, network_interface_dst);

        }

    }

    /**
    * @param arch_tile_mem_src The source tile containing the memory information
    * @param arch_tile_mem_dest The destination tile of the compatible format
    *
    * Load the memory information in the compatible format.
    */
    void InputConversionMampsPlatform::addArchGraphTileMemories(CNode *arch_tile_mem_src,
            CNode *arch_tile_mem_dest)
    {
        CNode *memory_src = CFindNode(arch_tile_mem_src, "memory");
        int memoryCount = 1;

        /* load all processors for this tile */

        for (; memory_src != NULL; memory_src = CNextNode(memory_src, "memory"))
        {
            CNode *memory_dst = CNewNode("memory");
            if (CHasAttribute(memory_src, "name"))
                CSetAttribute(memory_dst, "type", CGetAttribute(memory_src, "name"));

            if (!CHasChildNode(memory_src, "address"))
                throw CException("No address found for memory");

            CNode *address_src = CGetChildNode(memory_src, "address");

            if (!CHasAttribute(address_src, "base"))
                throw CException("No base address found for memory");

            if (!CHasAttribute(address_src, "high"))
                throw CException("No high address found for memory");

            CString base_str = CGetAttribute(address_src, "base");
            CString high_str = CGetAttribute(address_src, "high");

            /* Convert the addresses */
            int base = strtol(base_str.c_str(), NULL, 16);
            int high = strtol(high_str.c_str(), NULL, 16);

            int size = high - base;

            // Negative sizes are not supported
            if (size < 0)
                throw CException("Invalid base and high specified");

            CSetAttribute(memory_dst, "size", size);

            CString new_name = "mem";
            new_name.append(CString(memoryCount));
            memoryCount++;

            CSetAttribute(memory_dst, "name", new_name);


            CAddNode(arch_tile_mem_dest, memory_dst);

        }

    }
    /**
    * @param arch_tile_src The source processor node
    * @param arch_tile_dest The destination node
    *
    * Load the processors present in the tile to the compatible format.
    */
    void InputConversionMampsPlatform::addArchGraphTileProcessors(CNode *arch_tile_src,
            CNode *arch_tile_dest)
    {
        CNode *processor_src = CFindNode(arch_tile_src, "processor");

        /* load all processors for this tile */
        for (; processor_src != NULL; processor_src = CNextNode(processor_src, "processor"))
        {
            CNode *processor_dst = CNewNode("processor");

            if (CHasAttribute(processor_src, "name"))
                CSetAttribute(processor_dst, "name", CGetAttribute(processor_src, "name"));
            else
                throw CException("Name of processor is required");

            if (CHasAttribute(processor_src, "type"))
                CSetAttribute(processor_dst, "type", CGetAttribute(processor_src, "type"));
            else
                throw CException("Type of processor is required");

            addArchGraphTileProcessorsSchedulers(processor_src, processor_dst);



            CAddNode(arch_tile_dest, processor_dst);
        }
    }
    /**
    * @param arch_graph_src The Graph connection source
    * @param arch_graph_dest The destination node
    *
    * Load the Graph connection into the compatible format.
    */
    void InputConversionMampsPlatform::addArchGraphConnections(CNode *arch_graph_src,
            CNode *arch_graph_dest)
    {
        CNode *connection_src = CFindNode(arch_graph_src, "connection");

        for (; connection_src != NULL; connection_src = CNextNode(connection_src, "connection"))
        {
            CNode *connection_dest = CNewNode("connection");
            CString name = CString("");

            if (CHasAttribute(connection_src, "name"))
            {
                name = CGetAttribute(connection_src, "name");
                CSetAttribute(connection_dest, "name", name);
            }

            if (CHasAttribute(connection_src, "src"))
            {
                CString src = CGetAttribute(connection_src, "src");
                CSetAttribute(connection_dest, "srcTile", src);

                // Find the tile with the same name
                try
                {
                    CNode *src_tile = CFindNodeWithAttribute(arch_graph_src, "tile", "name", src);
                    CNode *tile_ni = CFindNode(src_tile, "networkinterface");

                    if (tile_ni != NULL)
                    {
                        CSetAttribute(connection_dest, "srcNetworkInterface", CGetAttribute(tile_ni, "name"));
                    }

                }
                catch (CException e)
                {
                    throw CException("Invalid source specified in connection '" + name + "'");
                }
            }

            if (CHasAttribute(connection_src, "dst"))
            {
                CString dst = CGetAttribute(connection_src, "dst");
                CSetAttribute(connection_dest, "dstTile", dst);

                // Find the tile with the same name
                try
                {
                    CNode *dst_tile = CFindNodeWithAttribute(arch_graph_src, "tile", "name", dst);
                    CNode *tile_ni = CFindNode(dst_tile, "networkinterface");

                    if (tile_ni != NULL)
                    {
                        CSetAttribute(connection_dest, "dstNetworkInterface", CGetAttribute(tile_ni, "name"));
                    }

                }
                catch (CException e)
                {
                    throw CException("Invalid destination specified in connection '" + name + "'");
                }
            }

            if (CHasAttribute(connection_src, "delay"))
            {
                CString latency = CGetAttribute(connection_src, "delay");
                CSetAttribute(connection_dest, "latency", latency);
            }

            // Add the connection to the architecture graph

            CAddNode(arch_graph_dest, connection_dest);
        }
    }
    /**
    * @param arch_graph_src The source graph tile
    * @param arch_graph_dest The destination graph tile
    *
    * Load the graph tile into the compatible format.
    */
    void InputConversionMampsPlatform::addArchGraphTiles(CNode *arch_graph_src,
            CNode *arch_graph_dest)
    {
        CNode *tile_src = CFindNode(arch_graph_src, "tile");

        if (tile_src == NULL)
        {
            throw CException("No tile specified");
        }

        for (; tile_src != NULL; tile_src = CNextNode(tile_src, "tile"))
        {
            CNode *tile_dest = CNewNode("tile");

            try
            {

                CString name = CGetAttribute(tile_src, "name");
                CSetAttribute(tile_dest, "name", name);

            }
            catch (CException e)
            {
                throw CException(" No name specified for tile");
            }

            // load the processsors
            addArchGraphTileProcessors(tile_src, tile_dest);

            //load memories
            addArchGraphTileMemories(tile_src, tile_dest);

            // load network interfaces
            addArchGraphTileNetworkInterfaces(tile_src, tile_dest);

            // add the loaded tile in the architecture graph
            CAddNode(arch_graph_dest, tile_dest);
        }

    }
    /**
     * @param arch_graph_src The source architecture graph
     * @param arch_graph_dest The destination architecture graph
     *
     * Loads the architecture graph.
     */
    void InputConversionMampsPlatform::addArchGraph(CNode *arch_graph_src,
            CNode *arch_graph_dest)
    {
        if (CHasAttribute(arch_graph_src, "name"))
            CSetAttribute(arch_graph_dest, "name", CGetAttribute(arch_graph_src, "name"));
        else
            throw CException("Name of architcture graph is required");


        // load the tile
        addArchGraphTiles(arch_graph_src, arch_graph_dest);

        // load the connections
        addArchGraphConnections(arch_graph_src, arch_graph_dest);


    }



    /**
     * @param app_graph The application graph
     *
     * Return the number of scenarios present in the application graph.
     * Application specified as SDFGs are equal to FSMSADF with only one scenerio
     * present in it.
     */
    int InputConversionMampsPlatform::getNumScenarios(CNode *app_graph)
    {
        return 1;
    }
    /**
    *
    * @param input_file The input file
    * Read, parse and store the input file.
    */
    void InputConversionMampsPlatform::readApplicationFile(
        const std::string input_file)
    {
        logInfo("Parsing application file: " + input_file);
        CDoc *application_doc = CParseFile(input_file);

        if (application_doc == NULL)
        {
            throw CException("Failed to read input file.");
        }

        //appReadUnits(application_doc); Not required as it loads CSDF dependant properties.
        createApplicationGraph(application_doc);


        releaseDoc(application_doc);
    }
    /**
    *
    * @param application_doc The input file
    *
    * Provided an application document, create an application graph.
    */
    void InputConversionMampsPlatform::createApplicationGraph(
        CDoc *application_doc)
    {
        // Copy the application graph name.
        CNode *app_graph_src = CFindNode(CGetRootNode(application_doc),
                                         "applicationGraph");

        // Check if name exists
        if (app_graph_src == NULL)
            throw CException("No applicationGraph found");

        for (; app_graph_src != NULL; app_graph_src = CNextNode(app_graph_src,
                "applicationGraph"))
        {
            CNode *app_graph_dest = CNewNode("applicationGraph");
            CString name;
            name = CGetAttribute(app_graph_src, "name");

            logInfo("Create application graph for application: " + name);

            CSetAttribute(app_graph_dest, "name", name);
            CAddNode(CGetRootNode(output_doc), app_graph_dest);

            // Add FSMSADF Graph
            addApplicationFsmsadf(app_graph_src, app_graph_dest);

            // ADd FSMSADF Properties
            addAppFsmsadfProp(app_graph_src, app_graph_dest);
            // Add FSM
            addApplicationFsm(app_graph_src, app_graph_dest);
        }
    }
    /**
    * Create a FSM machine for the given sdf graph which only contains one scenario
    * @param app_graph_src The application graph
    * @param app_graph_dest The destination node of compatible format
    */
    void InputConversionMampsPlatform::addApplicationFsm(
        CNode *app_graph_src,
        CNode *app_graph_dest)
    {
        // Determine the number of scenarios to create.
        int num_scenarios = 0;
        num_scenarios = getNumScenarios(app_graph_src);

        // No scenarios found? do nothing.
        if (num_scenarios != 1)
            throw(CException("Invalid number of scenarios"));

        // FSM is just a loop with only one state:
        //    ___
        //  ->|1|--------
        //     /\       |
        //     |________|

        CNode *fsm = CNewNode("fsm");
        // Always start in the first state.
        CSetAttribute(fsm, "initialstate", "q1");


        CString state_name = "q1";
        CString scenario_name = DEFAULT_SCENARIO;
        CNode *state = CNewNode("state");
        CAddAttribute(state, "name", state_name);
        CAddAttribute(state, "scenario", scenario_name);
        CNode *transition = CNewNode("transition");
        CString dest = "q1" ;
        CAddAttribute(transition, "destination", dest);

        CAddNode(state, transition);
        CAddNode(fsm, state);

        CAddNode(app_graph_dest, fsm);
    }
    /**
     * Create a scenarioGraph for each scenario.
     *
     * @param app_src_graph The source application graph
     * @param fsmsadf_dest_node The destination node
     */
    void InputConversionMampsPlatform::addAppFsmsadfScenarioGraph(
        CNode *app_src_graph,
        CNode *fsmsadf_dest_node)
    {
        // Determine the number of scenarios to create.
        int num_scenarios = 0;

        try
        {
            num_scenarios = getNumScenarios(app_src_graph);

            // Validate the number of scenerios.
            if (num_scenarios != 1)
            {
                throw(CException("Invalid number of scenarios"));
            }

            CString scenario_name = DEFAULT_SCENARIO;
            CNode *scenario_node_dest = CNewNode("scenariograph");
            CSetAttribute(scenario_node_dest, "name", scenario_name);
            CSetAttribute(scenario_node_dest, "type", scenario_name.toUpper());

            // Iterate over the actors, and add them to each scenariograph.
            CNode *src_actor = CFindNode(app_src_graph, "actor");

            for (;
                 src_actor != NULL;
                 src_actor = CNextNode(src_actor, "actor"))
            {
                addAppFsmsadfScenarioGraphActor(1,
                                                src_actor, scenario_node_dest);
            }

            // Iterate over the channels, and add them to each scenariograph.
            CNode *src_channel = CFindNode(app_src_graph, "channel");

            for (;
                 src_channel != NULL;
                 src_channel = CNextNode(src_channel, "channel"))
            {
                addAppFsmsadfScenarioGraphChannel(1,
                                                  app_src_graph, src_channel, scenario_node_dest);
            }

            CAddNode(fsmsadf_dest_node, scenario_node_dest);

        }

        catch (CException &e)
        {
            throw CException(CString(__FUNCTION__) + ": Failed to add scenario graph",
                             e);
        }
    }
    /**
     * Add the channel from the source scenario graph to the destination graph.
     *
     * @param scenario_num Number of scenario
     * @param app_src_graph Source application graph
     * @param src_channel Source channel
     * @param scenario_dest_graph Destination graph
     */
    void InputConversionMampsPlatform::addAppFsmsadfScenarioGraphChannel(
        int scenario_num,
        CNode *app_src_graph,
        CNode *src_channel,
        CNode *scenario_dest_graph)
    {
        CNode *dest_channel = CNewNode("channel");

        try
        {
            CString name = CGetAttribute(src_channel, "name");
            CSetAttribute(dest_channel, "name", name);

            CString srcActor = CGetAttribute(src_channel, "srcActor");
            CSetAttribute(dest_channel, "srcActor", srcActor);

            CString srcPort = CGetAttribute(src_channel, "srcPort");
            CSetAttribute(dest_channel, "srcPort", srcPort);

            CString dstActor = CGetAttribute(src_channel, "dstActor");
            CSetAttribute(dest_channel, "dstActor", dstActor);

            CString dstPort = CGetAttribute(src_channel, "dstPort");
            CSetAttribute(dest_channel, "dstPort", dstPort);

        }

        catch (CException &e)
        {
            throw CException(CString(__FILE__ + CString(":") + __FUNCTION__), e);
        }

        // Not required
        if (CHasAttribute(src_channel, "initialTokens"))
        {
            CString intialTokens = CGetAttribute(src_channel, "initialTokens");
            CSetAttribute(dest_channel, "initialTokens", intialTokens);
        }

        CAddNode(scenario_dest_graph, dest_channel);
    }
    /**
     * Load the scenario graph actor from the source graph to destination graph.
     *
     * @param scenario_num Number of scenario
     * @param src_actor Source actor
     * @param scenario_dest_graph Destination graph
     */
    void InputConversionMampsPlatform::addAppFsmsadfScenarioGraphActor(
        int scenario_num,
        CNode *src_actor,
        CNode *scenario_dest_graph)
    {
        CString name;
        name = CGetAttribute(src_actor, "name");

        CNode *dest_actor = CNewNode("actor");
        CSetAttribute(dest_actor, "name", name);
        CSetAttribute(dest_actor, "type", name.toUpper());

        CNode *src_port = CFindNode(src_actor, "port");

        for (; src_port != NULL; src_port = CNextNode(src_port, "port"))
        {
            CNode *dest_port = CNewNode("port");
            // Name
            CString name = CGetAttribute(src_port, "name");
            CSetAttribute(dest_port, "name", name);
            // Type
            CString type = CGetAttribute(src_port, "type");
            CSetAttribute(dest_port, "type", type);
            // Rate
            CString rate = CGetAttribute(src_port, "rate");
            CSetAttribute(dest_port, "rate", rate);

            CAddNode(dest_actor, dest_port);
        }

        CAddNode(scenario_dest_graph, dest_actor);
    }
    /**
    * Converts the Fsmsadf graph from the application document to the internal
    * format.
    *
    * @param application_doc The #CDoc representing the application document.
    * @param src The root #CNode of the applicationGraph
    * @param dest The #CNode to add the new fsmadf graph under.
    *
    */
    void InputConversionMampsPlatform::addApplicationFsmsadf(
        CNode *app_src_graph,
        CNode *dest)
    {
        // Create fsmadf root node
        CNode *fsmsadf_dest_node = CNewNode("fsmsadf");
        addAppFsmsadfScenarioGraph(app_src_graph, fsmsadf_dest_node);
        CAddNode(dest, fsmsadf_dest_node);
    }
    /**
     * Load the memory information from the source actor
     * properties to the destination actor properties.
     *
     * @param actor_prop_src  source actor properties
     * @param actor_prop_dest destination actor properties
     */
    void InputConversionMampsPlatform::addAppFsmsadfPropScenarioActorPropProcessorMemory(
        CNode *actor_prop_src,
        CNode *actor_prop_dest)
    {
        CNode *memory_element_src = CFindNode(actor_prop_src, "memoryElement");
        int memoryCount = 0;

        for (; memory_element_src != NULL; memory_element_src = CNextNode(memory_element_src, "memoryElement"))
        {
            memoryCount = memoryCount + 1;
            if (memoryCount > 3)
                throw CException("Too many memory elements");

            try
            {
                CString name = CGetAttribute(memory_element_src, "name");
                CNode *size_src = CFindNode(memory_element_src, "size");

                CNode *memory_dest = CNewNode("memory");
                CSetAttribute(memory_dest, "name", name);
                CSetAttribute(memory_dest, "size", CGetNodeContent(size_src));

                if (name == ".code")
                {
                    CSetAttribute(memory_dest, "type", "imem");
                }
                else if (name == ".data")
                {
                    CSetAttribute(memory_dest, "type", "dmem");
                }
                else if (name == "sharedVar")
                {
                    CSetAttribute(memory_dest, "type", "shared");
                }
                else
                {
                    throw CException("Unknown memory type '" + name + "'");
                }


                CAddNode(actor_prop_dest, memory_dest);
            }

            catch (CException &e)
            {
                throw CException("Failed to parse memoryElement", e);
            }

        }
    }
    /**
     * Load the processor information from the source actor
     * properties to the destination actor properties.
     *
     * @param actor_name string containing actor name
     * @param app_graph_src node pointing ti the graph source
     * @param actor_prop_src source actor properties
     * @param actor_prop_dest destination actor properties
     *
     */
    void InputConversionMampsPlatform::addAppFsmsadfPropScenarioActorPropProcessor(
        CString actor_name,
        CNode *app_graph_src,
        CNode *actor_prop_src,
        CNode *actor_prop_dest)
    {
        CNode *processor_src = CFindNode(actor_prop_src, "processor");

        for (; processor_src != NULL;
             processor_src = CNextNode(processor_src, "processor"))
        {
            CNode *processor_dest = CNewNode("processor");
            CString type = CGetAttribute(processor_src, "type");
            CSetAttribute(processor_dest, "type", type);
            CString def = CGetAttribute(processor_src, "default");
            CSetAttribute(processor_dest, "default", def);
            CNode *exe_time_src = CFindNode(processor_src, "executionTime");

            if (exe_time_src == NULL)
            {
                throw CException("No execution time found");
            }

            CNode *exe_time_dst = CNewNode("executionTime");
            CSetAttribute(exe_time_dst, "time", CGetNodeContent(exe_time_src));
            CAddNode(processor_dest, exe_time_dst);

            addAppFsmsadfPropScenarioActorPropProcessorMemory(actor_prop_src,
                    processor_dest);
            CAddNode(actor_prop_dest, processor_dest);
        }
    }
    /**
     * Load the actor channel properties from the source actor to the
     * destination actor properties.
     *
     * @param app_graph_src source application graph
     * @param scenario_node scenario node
     */
    void InputConversionMampsPlatform::addAppFsmsadfPropScenarioChannelProp(
        CNode *app_graph_src,
        CNode *scenario_node)
    {
        CNode *channel_prop_src = CFindNode(app_graph_src, "channelProperties");

        for (; channel_prop_src != NULL;
             channel_prop_src = CNextNode(channel_prop_src, "channelProperties"))
        {
            CNode *channel_properties_dest = CNewNode("channelProperties");
            CString channel = CGetAttribute(channel_prop_src, "channel");
            CSetAttribute(channel_properties_dest, "channel", channel);

            CNode *token_size_src = CFindNode(channel_prop_src, "tokenSize");

            if (token_size_src == NULL)
                throw CException("No tokenSize found");

            CNode *token_size_dest = CNewNode("tokenSize");
            CString size = CGetAttribute(token_size_src, "sz");
            CSetAttribute(token_size_dest, "sz", size);
            CAddNode(channel_properties_dest, token_size_dest);
            CAddNode(scenario_node, channel_properties_dest);
        }
    }
    /**
     * Load the source actor properties.
     *
     * @param app_graph_src source application graph
     * @param scenario_node scenario node
     */
    void InputConversionMampsPlatform::addAppFsmsadfPropScenarioActorProperties(
        CNode *app_graph_src,
        CNode *scenario_node)
    {
        /* Get actors*/
        CNode *actor_prop_src = CFindNode(app_graph_src, "actorProperties");

        for (; actor_prop_src != NULL;
             actor_prop_src = CNextNode(actor_prop_src, "actorProperties"))
        {
            CNode *actor_prop_dest = CNewNode("actorProperties");
            CString actor_name;

            actor_name = CGetAttribute(actor_prop_src, "actor");
            CSetAttribute(actor_prop_dest, "actor", actor_name);
            addAppFsmsadfPropScenarioActorPropProcessor(
                actor_name, app_graph_src, actor_prop_src, actor_prop_dest);
            CAddNode(scenario_node, actor_prop_dest);
        }
    }
    /**
    * Create the FsmsadfPropertiesScenarios from the application document to the internal
    * format.
    *
    * @param app_graph_src source application graph
    * @param fsmsadfprop_dest_node destination properties
    */
    void InputConversionMampsPlatform::addAppFsmsadfPropScenarios(
        CNode *app_graph_src,
        CNode *fsmsadfprop_dest_node)
    {
        CNode *scenarios = CNewNode("scenarios");
        CNode *scenario  = CNewNode("scenario");

        CSetAttribute(scenario, "name", DEFAULT_SCENARIO);
        CSetAttribute(scenario, "graph", DEFAULT_SCENARIO);

        CAddNode(scenarios, scenario);
        CAddNode(fsmsadfprop_dest_node, scenarios);


    }
    /**
    * Create the FsmsadfProperties from the application document to the internal
    * format.
    *
    * @param app_graph_src source application graph.
    * @param dest destination application graph
    *
    */
    void InputConversionMampsPlatform::addAppFsmsadfProp(
        CNode *app_graph_src,
        CNode *dest)
    {
        // Determine the number of scenarios to create.
        int num_scenarios = 0;
        num_scenarios = getNumScenarios(app_graph_src);

        // No scenarios found? do nothing.
        if (num_scenarios != 1)
            throw(CException("Invalid number of scenarios"));

        CNode *fsmsadfprop_dest_node = CNewNode("fsmsadfProperties");
        CNode *default_prop = CNewNode("defaultProperties");

        addAppFsmsadfPropScenarioActorProperties(app_graph_src, default_prop);

        // Add channel properties
        addAppFsmsadfPropScenarioChannelProp(app_graph_src, default_prop);

        CAddNode(fsmsadfprop_dest_node, default_prop);

        // Add ApplicationGraph
        CNode *graph_properties_src = CFindNode(app_graph_src, "graphProperties");

        if (graph_properties_src == NULL)
            throw CException("graphProperties not found");

        CNode *throughput_src = CFindNode(graph_properties_src, "throughput");

        // Add the default scenario
        addAppFsmsadfPropScenarios(app_graph_src, fsmsadfprop_dest_node);

        // check.
        if (throughput_src == NULL)
            throw CException("throughput not found");

        CNode *graph_properties_dest = CNewNode("graphProperties");
        CNode *time_constraints_dest = CNewNode("timeConstraints");
        CNode *throughput_dest = CNewNode("throughput");

        // Copy node content
        CDouble thr = CGetNodeContent(throughput_src);

        CSetContent(throughput_dest, thr);

        // Add throughput to the timeingConstraint
        CAddNode(time_constraints_dest, throughput_dest);
        // Add timingConstraints to the graphPRoperties
        CAddNode(graph_properties_dest, time_constraints_dest);
        // Add graphProperties to tree
        CAddNode(fsmsadfprop_dest_node, graph_properties_dest);

        // Add fsmsadfProperites to the tree
        CAddNode(dest, fsmsadfprop_dest_node);
    }
}
