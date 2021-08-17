/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   conversion.h
 *
 *  Author          :   Martijn Koedam (m.l.p.j.koedam@tue.nl)
 *
 *  Date            :   April 20, 2011
 *
 *  Function        :   Input/Output format conversion
 *
 *  History         :
 *      20-04-11    :   Initial version.
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
#include <assert.h>
#include "base/base.h"
#include "../../../platform_graph/memory.h"
#include "helper.h"

namespace FSMSADF
{
    /**
     * Constructs a new InputConversionCompSoCPlatform.
     *
     * Hardcoded values:
     * set wheelsize to 10
     * contextSWitchOverhead 1
     * preemptive true
     * Default dataUnit is 1 byte.
     */
    InputConversionCompSoCPlatform::InputConversionCompSoCPlatform()
        :
        InputConversionBase(),
        wheelsize(10),
        contextSwitchOverhead(1),
        slotLength(1),
        preemptive(true),
        dataUnit(1),
        timeUnit(1.0)
    {

    }
    /**
     * destructor
     */
    InputConversionCompSoCPlatform::~InputConversionCompSoCPlatform()
    {

    }

    /**
     * getNumScenarios()
     * @param doc a #CDoc
     *
     * Parses the application file and finds the number of scenarios.
     * It does this by finding all the cycles and finding the LCM.
     */
    int InputConversionCompSoCPlatform::getNumScenarios(CNode *app_graph)
    {
        std::set<int> rep_length;

        /* Get actors*/
        CNode *actor = CFindNode(app_graph, "actor");

        for (; actor != NULL; actor = CNextNode(actor, "actor"))
        {
            CString rep_size = CGetAttribute(actor, "size");
            rep_length.insert(int(rep_size));
        }

        /* Calculate length*/
        int _lcm = 1;

        for (set<int>::iterator iter = rep_length.begin();
             iter != rep_length.end(); iter++)
        {
            _lcm = lcm(*iter, _lcm);
        }

        return _lcm;
    }

    /**
     *
     * @param scenario_num
     * @param src_channel
     * @param scenario_dest_graph
     */
    void InputConversionCompSoCPlatform::addAppFsmsadfScenarioGraphChannel(
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

            // Validate channel
            // Get matching Actor.
            CNode *src_actor_node = CFindNodeWithAttribute(app_src_graph,
                                    "actor", "name", srcActor);
            CNode *dst_actor_node = CFindNodeWithAttribute(app_src_graph,
                                    "actor", "name", dstActor);

            // Find maching port
            CNode *src_actor_node_port = CFindNodeWithAttribute(src_actor_node,
                                         "port", "name", srcPort);
            CNode *dst_actor_node_port = CFindNodeWithAttribute(dst_actor_node,
                                         "port", "name", dstPort);

            // Get rates
            CString src_actor_rates = CGetAttribute(src_actor_node_port, "rate");
            CString dst_actor_rates = CGetAttribute(dst_actor_node_port, "rate");
            vector <CString> src_rates = expand(src_actor_rates);
            vector <CString> dst_rates = expand(dst_actor_rates);

            // 0 rate situation
            if (int(src_rates[scenario_num % src_rates.size()]) == 0 ||
                int(dst_rates[scenario_num % dst_rates.size()]) == 0)
            {
                if (int(src_rates[scenario_num % src_rates.size()]) ==
                    int(dst_rates[scenario_num % dst_rates.size()]))
                {
                    // Both zero, do not create channel.
                    CRemoveNode(dest_channel);
                    return;
                }

                else
                {
                    /// @todo We do not support  (a){2,0}-->{1,1}(b)
                    throw CException("Connections between actors where "
                                     "one actor has firing rate 0 is not supported");
                }
            }
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
     *
     * @param scenario_num
     * @param src_actor
     * @param scenario_dest_graph
     */
    void InputConversionCompSoCPlatform::addAppFsmsadfScenarioGraphActor(
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
            vector<CString> rates = expand(rate);
            int required_index = scenario_num % (rates.size());
            CSetAttribute(dest_port, "rate", rates[required_index]);

            if (int(rates[required_index]) == 0)
            {
                /* skip */
                CRemoveNode(dest_port);
                continue;
            }

            CAddNode(dest_actor, dest_port);
        }

        CAddNode(scenario_dest_graph, dest_actor);
    }
    /**
     * Create a scenarioGraph for each scenario.
     *
     * @param app_src_graph
     * @param fsmsadf_dest_node
     */
    void InputConversionCompSoCPlatform::addAppFsmsadfScenarioGraph(
        CNode *app_src_graph,
        CNode *fsmsadf_dest_node)
    {
        // Determine the number of scenarios to create.
        int num_scenarios = 0;

        try
        {
            num_scenarios = getNumScenarios(app_src_graph);

            // No scenarios found? do nothing.
            if (num_scenarios == 0)
                throw(CException("No scenario found"));

            for (int i = 0; i < num_scenarios; i++)
            {
                CString scenario_name = "scenario-s" + CString(i + 1);
                CNode *scenario_node_dest = CNewNode("scenariograph");
                CSetAttribute(scenario_node_dest, "name", scenario_name);
                CSetAttribute(scenario_node_dest, "type", scenario_name.toUpper());

                // Iterate over the actors, and add them to each scenariograph.
                CNode *src_actor = CFindNode(app_src_graph, "actor");

                for (;
                     src_actor != NULL;
                     src_actor = CNextNode(src_actor, "actor"))
                {
                    addAppFsmsadfScenarioGraphActor(i,
                                                    src_actor, scenario_node_dest);
                }

                // Iterate over the channels, and add them to each scenariograph.
                CNode *src_channel = CFindNode(app_src_graph, "channel");

                for (;
                     src_channel != NULL;
                     src_channel = CNextNode(src_channel, "channel"))
                {
                    addAppFsmsadfScenarioGraphChannel(i,
                                                      app_src_graph, src_channel, scenario_node_dest);
                }

                CAddNode(fsmsadf_dest_node, scenario_node_dest);
            }
        }

        catch (CException &e)
        {
            throw CException(CString(__FUNCTION__) + ": Failed to add scenario graph",
                             e);
        }
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
    void InputConversionCompSoCPlatform::addApplicationFsmsadf(
        CNode *app_src_graph,
        CNode *dest)
    {
        // Create fsmadf root node
        CNode *fsmsadf_dest_node = CNewNode("fsmsadf");
        addAppFsmsadfScenarioGraph(app_src_graph, fsmsadf_dest_node);
        CAddNode(dest, fsmsadf_dest_node);
    }
    /**
     * addActorPropertiesProcessorMemory()
     *
     *
     * @param actor_prop_src
     * @param actor_prop_dest
     */
    void InputConversionCompSoCPlatform::addAppFsmsadfPropScenarioActorPropProcessorMemory(
        CNode *actor_prop_src,
        CNode *actor_prop_dest)
    {
        CNode *memory_src = CFindNode(actor_prop_src, "memory");

        for (; memory_src != NULL; memory_src = CNextNode(memory_src, "memory"))
        {
            for (CNode *mem_elm_src = CFindNode(memory_src, "memoryElement");
                 mem_elm_src != NULL;
                 mem_elm_src = CNextNode(mem_elm_src, "memoryElement"))
            {
                try
                {
                    CString name = CGetAttribute(mem_elm_src, "name");
                    CNode *size_src = CFindNode(mem_elm_src, "size");
                    CNode *access_type_src = CFindNode(mem_elm_src, "accessType");
                    // Access type can be IFetch,DWrite,DRead
                    /// @todo: Note Assume IFetch can never be combined with D* (xml suggest it can)
                    CStrings access_types = CGetNodeContent(access_type_src).split(',');

                    CNode *memory_dest = CNewNode("memory");
                    CSetAttribute(memory_dest, "name", name);
                    CSetAttribute(memory_dest, "size",
                                  Size(CGetNodeContent(size_src)) * dataUnit);

                    // Check type of memory
                    if (access_types.size() == 0)
                    {
                        throw CException("MemoryElement has no access_type");
                    }

                    if (access_types.front() == "DRead" ||
                        access_types.front() == "DWrite")
                    {
                        // Type is Data memory
                        CSetAttribute(memory_dest,
                                      "type", MemoryType(MemoryType::Data));
                    }

                    else if (access_types.front() == "IFetch")
                    {
                        // Type is Ifetch
                        CSetAttribute(memory_dest,
                                      "type", MemoryType(MemoryType::Instr));
                    }

                    else
                    {
                        throw CException(
                            CString("MemoryElement has unknown access_type: " +
                                    access_types.front())
                        );
                    }

                    CAddNode(actor_prop_dest, memory_dest);
                }

                catch (CException &e)
                {
                    throw CException("Failed to parse memoryElement", e);
                }
            }
        }
    }
    /**
     *
     * @param i
     * @param actor_name
     * @param app_graph_src
     * @param actor_prop_src
     * @param actor_prop_dest
     */
    void InputConversionCompSoCPlatform::addAppFsmsadfPropScenarioActorPropProcessor(
        int scenario_num,
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

            // Lookup the actor in the original csdf graph that is right type.
            // This is where the execution time is specified.
            CNode *actor_src = CFindNode(app_graph_src, "actor");

            for (; actor_src != NULL; actor_src = CNextNode(actor_src, "actor"))
            {
                CString name = CGetAttribute(actor_src, "name");

                if (name != actor_name)
                    continue;

                CNode *exec_times_dest = CFindNode(actor_src, "executionTime");
                ASSERT(exec_times_dest != NULL, "Failed to find executionTime for actor: " + actor_name);

                CString time = CGetAttribute(exec_times_dest, "time");
                vector<CString> times = expand(time);
                CNode *exec_time_dst = CNewNode("executionTime");
                int index = scenario_num % times.size();
                CAddAttribute(exec_time_dst, "time", (Time)((Time)(times[index]) * timeUnit));
                CAddNode(processor_dest, exec_time_dst);
            }

            addAppFsmsadfPropScenarioActorPropProcessorMemory(actor_prop_src,
                    processor_dest);
            CAddNode(actor_prop_dest, processor_dest);
        }
    }
    /**
     *
     * @param i
     * @param app_graph_src
     * @param scenario_node
     */
    void InputConversionCompSoCPlatform::addAppFsmsadfPropScenarioChannelProp(
        int i,
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
            Size size = CGetAttribute(token_size_src, "size");
            CSetAttribute(token_size_dest, "sz", size * dataUnit);
            CAddNode(channel_properties_dest, token_size_dest);
            CAddNode(scenario_node, channel_properties_dest);
        }
    }
    /**
     *
     * @param i the active scenario.
     * @param app_graph_src
     * @param scenario_node
     */
    void InputConversionCompSoCPlatform::addAppFsmsadfPropScenarioActorProperties(
        int scenario_num,
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
            addAppFsmsadfPropScenarioActorPropProcessor(scenario_num,
                    actor_name, app_graph_src, actor_prop_src, actor_prop_dest);
            CAddNode(scenario_node, actor_prop_dest);
        }
    }
    /**
     * Create the FsmsadfProperties from the application document to the internal
     * format.
     *
     * @param application_doc The #CDoc representing the application document.
     * @param src The root #CNode of the applicationGraph
     * @param dest The #CNode to add the new fsmadf graph under.
     *
     * @todo should we validate the values we read? this should be done in the schema validation?
     */
    void InputConversionCompSoCPlatform::addAppFsmsadfProp(
        CNode *app_graph_src,
        CNode *dest)
    {
        // Determine the number of scenarios to create.
        int num_scenarios = 0;
        num_scenarios = getNumScenarios(app_graph_src);

        // No scenarios found? do nothing.
        if (num_scenarios == 0)
            throw(CException("No scenario found"));

        CNode *fsmsadfprop_dest_node = CNewNode("fsmsadfProperties");
        CNode *scenarios_node = CNewNode("scenarios");

        // Add actorproperties (under scenarios)
        for (int i = 0; i < num_scenarios; i++)
        {
            CNode *scenario_node = CNewNode("scenario");
            // Name
            CString scenario_name = "s" + CString(i + 1);
            CAddAttribute(scenario_node, "name", scenario_name);
            // Graph name. (known)
            CString graph_name = "scenario-s" + CString(i + 1);
            CAddAttribute(scenario_node, "graph", graph_name);
            // Add actor properties
            // Add actor properties
            addAppFsmsadfPropScenarioActorProperties(i,
                    app_graph_src, scenario_node);
            // Add channel properties
            addAppFsmsadfPropScenarioChannelProp(i,
                                                 app_graph_src, scenario_node);
            CAddNode(scenarios_node, scenario_node);
        }

        CAddNode(fsmsadfprop_dest_node, scenarios_node);
        // Add ApplicationGraph
        CNode *graph_properties_src = CFindNode(app_graph_src, "graphProperties");

        if (graph_properties_src == NULL)
            throw CException("graphProperties not found");

        CNode *throughput_src = CFindNode(graph_properties_src, "throughput");

        // check.
        if (throughput_src == NULL)
            throw CException("throughput not found");

        CNode *graph_properties_dest = CNewNode("graphProperties");
        CNode *time_constraints_dest = CNewNode("timeConstraints");
        CNode *throughput_dest = CNewNode("throughput");

        // Copy node content
        CDouble thr = CGetNodeContent(throughput_src);
        /// @todo validatie throughput?
        /// fire/timeunit.
        CSetContent(throughput_dest, thr / timeUnit);

        // Add throughput to the timeingConstraint
        CAddNode(time_constraints_dest, throughput_dest);
        // Add timingConstraints to the graphPRoperties
        CAddNode(graph_properties_dest, time_constraints_dest);
        // Add graphProperties to tree
        CAddNode(fsmsadfprop_dest_node, graph_properties_dest);

        // Add fsmsadfProperites to the tree
        CAddNode(dest, fsmsadfprop_dest_node);
    }
    /**
     * Create a FSM machine for the given csdf graph
     * @param application_doc
     * @param app_graph_dest
     */
    void InputConversionCompSoCPlatform::addApplicationFsm(
        CNode *app_graph_src,
        CNode *app_graph_dest)
    {
        // Determine the number of scenarios to create.
        int num_scenarios = 0;
        num_scenarios = getNumScenarios(app_graph_src);

        // No scenarios found? do nothing.
        if (num_scenarios == 0)
            throw(CException("No scenario found"));

        // no need to create an FSM graph for one scenario.
        // if (num_scenarios == 1)
        //     return;

        // FSM is just a loop:
        //    ___    _____   ____
        //  ->|1| -->| 2 |-->|3 |-->..--> num_scenarios
        //    /\                            |
        //     |____________________________|
        //

        CNode *fsm = CNewNode("fsm");
        // Always start in the first state.
        CSetAttribute(fsm, "initialstate", "q1");

        for (int i = 0; i < num_scenarios; i++)
        {
            CString state_name = "q" + CString(i + 1);
            CString scenario_name = "s" + CString(i + 1);
            CNode *state = CNewNode("state");
            CAddAttribute(state, "name", state_name);
            CAddAttribute(state, "scenario", scenario_name);
            CNode *transition = CNewNode("transition");
            CString dest = "q" + CString((i + 1) % num_scenarios + 1);
            CAddAttribute(transition, "destination", dest);

            CAddNode(state, transition);
            CAddNode(fsm, state);

        }

        CAddNode(app_graph_dest, fsm);
    }
    /**
     *
     * @param application_doc
     */
    void InputConversionCompSoCPlatform::createApplicationGraph(
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
     *
     * @param input_file
     */
    void InputConversionCompSoCPlatform::readApplicationFile(
        const std::string input_file)
    {
        logInfo("Parsing application file: " + input_file);
        CDoc *application_doc = CParseFile(input_file);

        if (application_doc == NULL)
        {
            throw CException("Failed to read input file.");
        }

        appReadUnits(application_doc);
        createApplicationGraph(application_doc);
        releaseDoc(application_doc);
    }
    /**
     * Memory on each time has support for different types of memory.
     * f.e. cmem, dmem, imem, etc.
     *
     * @param src
     * @param dest
     */
    void InputConversionCompSoCPlatform::addArchitectureTileMemory(
        CNode *src,
        CNode *dest)
    {
        int num_imem = 0;
        int num_dmem = 0;
        int num_shared = 0;

        try
        {
            for (CNode *parameter_src = CFindNode(src, "parameter");
                 parameter_src != NULL;
                 parameter_src = CNextNode(parameter_src, "parameter"))
            {

                CString id_src = CGetAttribute(parameter_src, "id");

                if (id_src == "cmemin")
                {
                    int mem_elemns = int(CGetAttribute(parameter_src, "value"));

                    for (int i = 0; i < mem_elemns; i++)
                    {
                        CString mem_name = CString("cmemin" + CString(i) + "_size");

                        for (CNode *mem_src = parameter_src;
                             mem_src != NULL;
                             mem_src = CNextNode(mem_src, "parameter"))
                        {
                            if (CGetAttribute(mem_src, "id") != mem_name)
                                continue;

                            if (mem_src == NULL)
                                throw CException("Failed to find node: " + mem_name);

                            CNode *memory_dest = CNewNode("memory");
                            CString mem_size_src = CGetAttribute(mem_src, "value");
                            CSetAttribute(memory_dest, "name", mem_name.replace("_size", ""));
                            // This size in BYTES
                            CSetAttribute(memory_dest, "size", Size(mem_size_src));
                            CSetAttribute(memory_dest, "type",
                                          MemoryType(MemoryType::CommIn));
                            CAddNode(dest, memory_dest);
                        }
                    }
                }

                else if (id_src == "dma")
                {
                    int mem_elemns = int(CGetAttribute(parameter_src, "value"));

                    for (int i = 0; i < mem_elemns; i++)
                    {
                        /// dma memory is cmemout.
                        CString mem_name = CString("cmemout" + CString(i) + "_size");

                        for (CNode *mem_src = parameter_src;
                             mem_src != NULL;
                             mem_src = CNextNode(mem_src, "parameter"))
                        {
                            if (CGetAttribute(mem_src, "id") != mem_name)
                                continue;

                            if (mem_src == NULL)
                                throw CException("Failed to find node: " + mem_name);

                            CNode *memory_dest = CNewNode("memory");
                            CString mem_size_src = CGetAttribute(mem_src, "value");
                            CSetAttribute(memory_dest, "name", mem_name.replace("_size", ""));
                            // This size in BYTES
                            CSetAttribute(memory_dest, "size", Size(mem_size_src));
                            CSetAttribute(memory_dest, "type",
                                          MemoryType(MemoryType::CommOut));
                            CAddNode(dest, memory_dest);
                        }
                    }

                }

                else if (id_src == "imem_size")
                {
                    CNode *memory_dest = CNewNode("memory");
                    CString mem_size_src = CGetAttribute(parameter_src, "value");
                    // name
                    CSetAttribute(memory_dest, "name", id_src.replace("_size", ""));
                    // size in bytes
                    CSetAttribute(memory_dest, "size", Size(mem_size_src));
                    // type
                    CSetAttribute(memory_dest, "type", MemoryType(MemoryType::Instr));
                    CAddNode(dest, memory_dest);
                    num_imem++;
                }

                else if (id_src == "dmem_size")
                {
                    CNode *memory_dest = CNewNode("memory");
                    CString mem_size_src = CGetAttribute(parameter_src, "value");
                    // name
                    CSetAttribute(memory_dest, "name", id_src.replace("_size", ""));
                    // size in bytes
                    CSetAttribute(memory_dest, "size", Size(mem_size_src));
                    // type
                    CSetAttribute(memory_dest, "type", MemoryType(MemoryType::Data));
                    CAddNode(dest, memory_dest);
                    num_dmem++;
                }

                else if (id_src == "mem_size" || id_src == "size")
                {
                    // mem_size or size are shared memories, latter is shared mem tile.
                    CString mem_size_src = CGetAttribute(parameter_src, "value");
                    CNode *memory_dest = CNewNode("memory");
                    CSetAttribute(memory_dest, "name", "memory");
                    // In bytes
                    CSetAttribute(memory_dest, "size", Size(mem_size_src));
                    CSetAttribute(memory_dest, "type",
                                  MemoryType(MemoryType::Shared));
                    CAddNode(dest, memory_dest);
                    num_shared++;
                }
            }

            // We either have a tile with only shared memory and no i/dmem
            // or we have 1 imem and 1 dmem.
            if (!(
                    (num_imem == 1 && num_dmem == 1) ||
                    (num_shared > 0 && num_imem == 0 && num_dmem == 0)
                )
               )
            {
                char error[256];
#ifndef _MSC_VER
                snprintf(error, 256, "Invalid memory configuration found: "
                         "imem: %i, dmem: %i, shared: %i", num_imem, num_dmem, num_shared);
#endif
                throw CException(error);

            }
        }

        catch (CException &e)
        {
            throw CException(CString(__FUNCTION__) + ": Failed adding memory to tile", e);
        }
    }
    /**
     * @todo Processor information is not provided. We take some sensible values now.
     *
     * @param src
     * @param dest
     */
    void InputConversionCompSoCPlatform::addArchitectureTileProcessorArbitration(
        CNode *dest)
    {
        CNode *arbitration_dest = CNewNode("arbitration");
        logWarning("Using hardcoded values for TDMA");
        // Set the type
        CSetAttribute(arbitration_dest, "type", "TDMA");
        // set wheelsize  10*(20000+1500) (slots * (slot_len + overhead))
        CSetAttribute(arbitration_dest, "wheelsize", wheelsize * timeUnit);
        // overhead 1500
        CSetAttribute(arbitration_dest, "contextSwitchOverhead",
                      contextSwitchOverhead * timeUnit);
        // overhead 1500
        CSetAttribute(arbitration_dest, "slotLength",
                      slotLength * timeUnit);

        // preemptive
        if (preemptive)
            CSetAttribute(arbitration_dest, "preemptive", "true");

        else
            CSetAttribute(arbitration_dest, "preemptive", "false");

        CAddNode(dest, arbitration_dest);
    }
    /**
     * Output the processor tile architecture.
     *
     * @param src
     * @param dest
     */
    void InputConversionCompSoCPlatform::addArchitectureTileProcessor(CNode *src,
            CNode *dest)
    {
        CString type_src;
        type_src = CGetAttribute(src, "type");

        // Make this less hardcoded? lookup table?
        CNode *processor_dest = CNewNode("processor");

        if (type_src == "MBtile")
        {
            CSetAttribute(processor_dest, "name", "proc");
            CSetAttribute(processor_dest, "type", "microblaze");
            addArchitectureTileProcessorArbitration(processor_dest);

        }

        else if (type_src == "Memory")
        {
            /** @todo What todo with memory tile? No size specified in architecture*/
            // Memory tile has no processor?
            CRemoveNode(processor_dest);
            return;
        }

        else if (type_src == "Host")
        {
            /** @todo What must we do host tiles? */
            // Host tile does nothing useful for sdf3.
            CRemoveNode(processor_dest);
            return;
        }

        else
        {
            throw CException(CString("Unknown tile type" + type_src));
        }

        CAddNode(dest, processor_dest);
    }
    /**
     * Generate the tiles in the architecture graph.
     *
     * @todo When generating networkInterface, the parameters
     * ((nr|)(in|out)(Connections|Bandwidth)) should be part of the arch. file.
     * @param src
     * @param dest
     */
    void InputConversionCompSoCPlatform::addArchitectureTiles(CNode *src,
            CNode *dest)
    {
        // Find ip block.
        try
        {
            CNode *tile_src = CFindNode(src, "ip");

            if (tile_src == NULL)
                throw CException("No tile (ip) block found");

            for (; tile_src != NULL; tile_src = CNextNode(tile_src, "ip"))
            {
                CNode *tile_dest = CNewNode("tile");
                CString name = CGetAttribute(tile_src, "id");
                CString type = CGetAttribute(tile_src, "type");

                // Ignore monitor tile
                if (!(type == "MBtile" || type == "Memory"))
                {
                    CRemoveNode(tile_dest);
                    continue;
                }

                CSetAttribute(tile_dest, "name", name);

                // Look up constraint matching the ip block
                // Needed for generating NI
                CNode *constraint_ni = NULL;

                for (CNode *temp = CFindNode(src, "constraint");
                     temp != NULL;
                     temp = CNextNode(temp, "constraint"))
                {
                    CString constraint_id = CGetAttribute(temp, "id");

                    if (constraint_id == name)
                    {
                        constraint_ni = temp;
                        break;
                    }
                }

                if (constraint_ni == NULL)
                {
                    throw CException("No contraint found for this tile. "
                                     "This is required for creating NetworkInterfaces.");
                }

                // Add network interface
                CNode *network_interface_dest = CNewNode("networkInterface");
                /// @todo hardcoded values 100 connections and  5p/sec bandwidth
                CString ni_src_name = CGetAttribute(constraint_ni, "ni");
                CSetAttribute(network_interface_dest, "name", ni_src_name);

                CSetAttribute(network_interface_dest, "nrInConnections", "100");
                CSetAttribute(network_interface_dest, "nrOutConnections", "100");
                CSetAttribute(network_interface_dest, "inBandwidth", "5");
                CSetAttribute(network_interface_dest, "outBandwidth", "5");

                CAddNode(tile_dest, network_interface_dest);

                // Processor
                addArchitectureTileProcessor(tile_src, tile_dest);

                // Memory
                addArchitectureTileMemory(tile_src, tile_dest);

                CAddNode(dest, tile_dest);
            }
        }

        catch (CException &e)
        {
            throw CException(CString(__FUNCTION__) + ": Failed to create tile", e);
        }
    }
    /**
     * Generate a fully connected network.
     *
     * @todo currently a fully connected network is generated using constraints.
     * values (latency) are hard-coded.
     *
     * @param src
     * @param dest
     * @todo latency of a connection should defined in architecture file.
     */
    void InputConversionCompSoCPlatform::addArchitectureConnections(CNode *src,
            CNode *dest)
    {
        try
        {
            int num_con = 0;
            // We generate the connection matrix from the contraints.
            CNode *constraint_src = CFindNode(src, "constraint");

            if (constraint_src == NULL)
                throw CException("Did not find any contraints.");

            for (; constraint_src != NULL;
                 constraint_src = CNextNode(constraint_src, "constraint"))
            {
                CString constraint_id_src = CGetAttribute(constraint_src, "id");
                CString constraint_ni_src = CGetAttribute(constraint_src, "ni");

                /* We ignore monitor tiles. */
                if (constraint_id_src == "monitor")
                    continue;

                for (CNode *constraint_src2 = CFindNode(src, "constraint");
                     constraint_src2 != NULL;
                     constraint_src2 = CNextNode(constraint_src2, "constraint"))
                {
                    CString cstr_id_src2 = CGetAttribute(constraint_src2, "id");

                    if (cstr_id_src2 != "monitor" &&
                        constraint_id_src != cstr_id_src2)
                    {
                        CString cstr_ni_src2 = CGetAttribute(constraint_src2, "ni");
                        CNode *connection_dest = CNewNode("connection");
                        CSetAttribute(connection_dest,
                                      "name", "con_" + CString(++num_con));
                        CSetAttribute(connection_dest,
                                      "srcTile", constraint_id_src);
                        CSetAttribute(connection_dest,
                                      "srcNetworkInterface", constraint_ni_src);
                        CSetAttribute(connection_dest,
                                      "dstTile", cstr_id_src2);
                        CSetAttribute(connection_dest,
                                      "dstNetworkInterface", cstr_ni_src2);

                        /** @todo latency is now hardcoded to 1200 ns*/
                        logWarning("Using hardcoded value for latency: 1200 timeUnits");
                        CSetAttribute(connection_dest, "latency", "1200");
                        CAddNode(dest, connection_dest);
                    }
                }

                /* Each network_interfaces can talk with itself */
                CNode *feedback_dest = CNewNode("connection");
                CSetAttribute(feedback_dest,
                              "name", "con_" + CString(++num_con));
                CSetAttribute(feedback_dest,
                              "srcTile", constraint_id_src);
                CSetAttribute(feedback_dest,
                              "srcNetworkInterface", constraint_ni_src);
                CSetAttribute(feedback_dest,
                              "dstTile", constraint_id_src);
                CSetAttribute(feedback_dest,
                              "dstNetworkInterface", constraint_ni_src);
                /** @todo latency is now hardcoded to 2*/
                logWarning("Using hardcoded value for latency: 1200 timeUnits");
                CSetAttribute(feedback_dest, "latency", "1200");
                CAddNode(dest, feedback_dest);
            }
        }

        catch (CException &e)
        {
            // catch one or multiple exceptions
            throw CException(CString(__FUNCTION__) + ": Failed to add connection to "
                             "the architecture graph.", e);
        }
    }
    /**
     * createArchitectureGraph()
     *
     * @param app_doc
     */
    void InputConversionCompSoCPlatform::createArchitectureGraph(CDoc *app_doc)
    {
        logInfo("Create architecture graph");

        try
        {
            CNode *arch_graph_src = CGetRootNode(app_doc);

            // Check if it is the right document. (check root name)
            if (!CIsNode(arch_graph_src, "architecture"))
                throw CException("No architectureGraph found");

            CNode *arch_graph_dest = CNewNode("architectureGraph");
            CString name;

            name = CGetAttribute(arch_graph_src, "id");
            CSetAttribute(arch_graph_dest, "name", name);
            // Add the different tiles.
            addArchitectureTiles(arch_graph_src, arch_graph_dest);
            // Add connections
            addArchitectureConnections(arch_graph_src, arch_graph_dest);
            CAddNode(CGetRootNode(output_doc), arch_graph_dest);
        }

        catch (CException &e)
        {
            throw CException(CString(__FUNCTION__) + "Failed to create architecture "
                             "graph", e);
        }
    }

    /*************************/
    /* Architecture Graph   */
    /************************/
    void InputConversionCompSoCPlatform::readArchitectureFile(
        const std::string input_file)
    {
        logInfo("Parsing architecture file: " + input_file);
        CDoc *architecture_doc = CParseFile(input_file);

        if (architecture_doc == NULL)
        {
            throw CException("Failed to read input file.");
        }

        createArchitectureGraph(architecture_doc);
        releaseDoc(architecture_doc);
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
     * @param input_file
     */
    void InputConversionCompSoCPlatform::appReadUnits(CDoc *app_doc)
    {
        CNode *root = CGetRootNode(app_doc);

        // <sdf><applicationGraph><csdfProperties>
        CNode *src_csdfprop = CFindNode(root, "csdfProperties");

        if (src_csdfprop == NULL)
            return;

        // <sdf><applicationGraph><csdfProperties><units>
        CNode *src_units = CFindNode(src_csdfprop, "units");

        if (src_units == NULL)
            return;

        // <sdf><applicationGraph><csdfProperties><dataUnits>
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

        // <sdf><applicationGraph><csdfProperties><dataUnits>
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
    // end namespace
}

