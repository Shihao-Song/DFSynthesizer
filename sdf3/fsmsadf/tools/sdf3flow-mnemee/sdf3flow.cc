/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf3flow.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 14, 2009
 *
 *  Function        :   sdf3flow
 *
 *  History         :
 *      14-05-09    :   Initial version.
 *
 * $Id: sdf3flow.cc,v 1.1.2.2 2010-04-22 07:07:18 mgeilen Exp $
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

#include "sdf3flow.h"
#include "settings.h"

using namespace FSMSADF;

namespace FSMSADF
{

    /**
     * settings
     * Program settings.
     */
    Settings settings(MODULE, SETTINGS_TYPE);

    /**
     * helpMessage ()
     * Function prints help message for the tool.
     */
    void helpMessage(ostream &out)
    {
        out << "SDF3 " << TOOL << " (version " << DOTTED_VERSION ")" << endl;
        out << endl;
        out << "Usage: " << TOOL << " [--settings <file> --output <file>";
        out << " --step]";
        out << endl;
        out << "   --settings <file>  settings for algorithms (default: sdf3.opt)";
        out << endl;
        out << "   --output <file>    output file (default:stdout)" << endl;
        out << "   --step             run flow step-by-step" << endl;
        out << endl;
    }

    /**
     * initSettings ()
     * The function initializes the program settings.
     */
    bool initSettings(int argc, char **argv)
    {
        // Initialize settings
        settings.init(argc, argv);

        // Help message needed?
        if (settings.helpFlag)
        {
            helpMessage(cerr);
            return false;
        }

        // Initialize output stream
        settings.initOutputStream();

        // Done
        return true;
    }

    /**
     * findAllScenariosOfActor()
     * The function returns the set of scenarios in which the actor appears.
     */
    Scenarios findAllScenariosOfActor(Actor *a)
    {
        ScenarioGraph *sg = a->getScenarioGraph();
        Graph *g = sg->getGraph();
        Scenarios scenarios;

        // Iterate over all scenarios of the application graph
        for (Scenarios::iterator i = g->getScenarios().begin();
             i != g->getScenarios().end(); i++)
        {
            Scenario *s = *i;

            // Does this scenario use the scenario graph to which actor a belongs?
            if (s->getScenarioGraph() == sg)
            {
                scenarios.insert(s);
            }
        }

        return scenarios;
    }

    /**
     * actorSupportProcessorType()
     * The function returns true when the actor can be mapped to this processor
     * type
     */
    bool actorSupportProcessorType(Actor *a, CString type)
    {
        for (Actor::ProcessorTypes::const_iterator
             i = a->getProcessorTypes().begin();
             i != a->getProcessorTypes().end(); i++)
        {
            if (i->type == type)
                return true;
        }

        return false;
    }

    /**
     * unbindThreadToTile()
     * Remobe binding of the set of actors to the processor and memory in all
     * scenarios in which the actors are used.
     */
    void unbindThreadToTile(set<Actor *> &actors, ProcessorBinding *pb,
                            MemoryBinding *mb)
    {
        // Iterate over all actors
        for (set<Actor *>::iterator i = actors.begin(); i != actors.end(); i++)
        {
            Actor *a = *i;

            // Create a list of all scenarios in which this actor appears
            Scenarios scenarios = findAllScenariosOfActor(a);
            for (Scenarios::iterator j = scenarios.begin();
                 j != scenarios.end(); j++)
            {
                Scenario *s = *j;

                // Remove processor binding
                pb->removeActorBinding(s, a);

                // Remove memory binding
                mb->removeActorBinding(s, a);
            }
        }
    }

    /**
     * bindThreadToTile()
     * Bind the set of actors to the processor and memory in all scenarios in
     * which the actors are used.
     */
    bool bindThreadToTile(set<Actor *> &actors, ProcessorBinding *pb,
                          MemoryBinding *mb)
    {
        // Iterate over all actors
        for (set<Actor *>::iterator i = actors.begin(); i != actors.end(); i++)
        {
            Actor *a = *i;

            // Does this actor not support this processor type?
            if (!actorSupportProcessorType(a, pb->getProcessor()->getType()))
            {
                // Remove all actor bindings
                unbindThreadToTile(actors, pb, mb);
                return false;
            }

            // Create a list of all scenarios in which this actor appears
            Scenarios scenarios = findAllScenariosOfActor(a);
            for (Scenarios::iterator j = scenarios.begin();
                 j != scenarios.end(); j++)
            {
                Scenario *s = *j;

                // Try binding actor in this scenario to the processor and memory
                if (!mb->addActorBinding(s, a, pb->getProcessor()->getType())
                    || !pb->addActorBinding(s, a))
                {
                    // Remove all actor bindings
                    unbindThreadToTile(actors, pb, mb);
                    return false;
                }
            }
        }

        return true;
    }

    /**
     * mapApplicationGraphToArchitectureGraph ()
     * The complete mapping flow. The function returns a pointer to an sdf3node
     * when a mapping has been found. Otherwise the function returns a NULL pointer.
     */
    CNode *mapApplicationGraphToArchitectureGraph(CNode *sdf3Node, ostream &out)
    {
        CNode *appGraph, *archGraph, *mapping;
        map<Processor *, CString> processorToThread;
        map<CString, set<Actor *> > threadToActors;
        CNode *newSDF3Node, *newMappingNode;
        PlatformBinding *pb;
        PlatformGraph *pg;
        Graph *ag;

        // Get the application, architecture and initial mapping
        appGraph = CGetChildNode(sdf3Node, "applicationGraph");
        archGraph = CGetChildNode(sdf3Node, "architectureGraph");
        mapping = CGetChildNode(sdf3Node, "mapping");

        // Create a new application graph
        ag = new Graph();
        ag->constructFromXML(appGraph);

        // Create a new platfrom graph
        pg = new PlatformGraph();
        pg->constructFromXML(archGraph);

        // Create a new platform binding
        pb = new PlatformBinding(GraphComponent(NULL, 0, "binding"), pg, ag);
        pb->constructFromXML(mapping);

        // Create a map of threads to actors
        for (ScenarioGraphs::iterator i = ag->getScenarioGraphs().begin();
             i != ag->getScenarioGraphs().end(); i++)
        {
            ScenarioGraph *sg = *i;

            for (Actors::iterator j = sg->getActors().begin();
                 j != sg->getActors().end(); j++)
            {
                Actor *a = *j;

                threadToActors[a->getType()].insert(a);
            }
        }

        // Map each thread to the first processor on which it fits
        for (map<CString, set<Actor *> >::iterator i = threadToActors.begin();
             i != threadToActors.end(); i++)
        {
            bool validThreadBinding = false;

            // Iterate over the tiles in the platform binding
            for (TileBindings::iterator j = pb->getTileBindings().begin();
                 j != pb->getTileBindings().end(); j++)
            {
                TileBinding *tb = *j;

                // Iterate over the processors in the tile
                for (ProcessorBindings::iterator
                     k = tb->getProcessorBindings().begin();
                     k != tb->getProcessorBindings().end(); k++)
                {
                    ProcessorBinding *p = *k;

                    // Is this processor not used for another thread?
                    if (processorToThread.find(p->getProcessor())
                        == processorToThread.end())
                    {
                        // Iterate over the memories in the tile
                        /* for (MemoryBindings::iterator
                                 l = tb->getMemoryBindings().begin();
                                 l != tb->getMemoryBindings().end(); l++)
                         */
                        {
                            /// @todo check this.
                            MemoryBinding *m = tb->getMemoryBinding();

                            // Try mapping thread to this processor and memory
                            if (bindThreadToTile(i->second, p, m))
                            {
                                validThreadBinding = true;
                                processorToThread[p->getProcessor()] = i->first;
                                break;
                            }
                        }
                    }

                    // Found a valid thread to processor binding?
                    if (validThreadBinding)
                        break;
                }

                // Found a valid thread to processor binding?
                if (validThreadBinding)
                    break;
            }

            // Found no valid thread to processor binding?
            if (!validThreadBinding)
            {
                cerr << "Failed binding thread '" << i->first << "' to processor.";
                cerr << endl;
                return NULL;
            }
        }

        // Output the mapping of threads to processors
        for (map<Processor *, CString>::iterator i = processorToThread.begin();
             i != processorToThread.end(); i++)
        {
            out << i->second << ";" << i->first->getName() << endl;
        }

        // Create SDF3 node
        newSDF3Node = CNewNode("sdf3");
        newMappingNode = CAddNode(newSDF3Node, "mapping");
        pb->convertToXML(newMappingNode);

        return newSDF3Node;
    }

    /**
     * mappingFlow()
     * Perform the mapping flow on all application graphs.
     */
    bool mappingFlow(ostream &out)
    {
        CNode *archGraph, *resourceUsage, *appGraph, *mapping;
        CNode *archSdfNode, *appSdfNode, *sdf3node;
        CDoc *archDoc, *appDoc;
        ofstream outFile;

        // Open architecture file
        archDoc = CParseFile(settings.architectureGraph);
        if (archDoc == NULL)
            throw CException("Failed opening '" + settings.architectureGraph + "'.");

        // Locate the sdf3 root element and check module type
        archSdfNode = CGetRootNode(archDoc);
        if (CGetAttribute(archSdfNode, "type") != settings.module)
        {
            throw CException("Root element in file '" + settings.architectureGraph
                             + "' is not of type '" + settings.module + "'.");
        }

        // Architecture graph
        if (!CHasChildNode(archSdfNode, "architectureGraph"))
            throw CException("No architecture graph found.");
        archGraph = CGetChildNode(archSdfNode, "architectureGraph");

        // Resource usage
        resourceUsage = NULL;

        // Iterate over the application graphs
        for (CStrings::iterator i = settings.applicationGraphs.begin();
             i != settings.applicationGraphs.end(); i++)
        {
            CString file = *i;

            // Open application file
            appDoc = CParseFile(file);
            if (appDoc == NULL)
                throw CException("Failed opening '" + file + "'.");

            // Locate the sdf3 root element and check module type
            appSdfNode = CGetRootNode(appDoc);
            if (CGetAttribute(appSdfNode, "type") != settings.module)
            {
                throw CException("Root element in file '" + file
                                 + "' is not of type '" + settings.module + "'.");
            }

            // Application graph
            if (!CHasChildNode(appSdfNode, "applicationGraph"))
                throw CException("No application graph found.");
            appGraph = CGetChildNode(appSdfNode, "applicationGraph");

            // Mapping related to this application?
            mapping = NULL;
            for (CNode *n = CGetChildNode(appSdfNode, "mapping");
                 n != NULL; n = CNextNode(n, "mapping"))
            {
                // Is this the initial mapping?
                if (CHasAttribute(n, "name")
                    && CGetAttribute(n, "name") == "initial")
                {
                    // Does the mapping specify an application graph?
                    if (!CHasAttribute(n, "appGraph"))
                    {
                        throw CException("No application graph specified "
                                         "with the default mapping.");
                    }

                    // Is the application graph of this mapping the application
                    // graph which is going to be mapped
                    if (CGetAttribute(n, "appGraph")
                        == CGetAttribute(appGraph, "name"))
                    {
                        mapping = n;
                        break;
                    }
                }
            }

            // No mapping specified? Create an empty mapping
            if (mapping == NULL)
            {
                mapping = CNewNode("mapping");
                CAddAttribute(mapping, "name", "initial");
            }

            // Resource usage specified?
            if (resourceUsage != NULL)
            {
                // Does the mapping already specify some resource usage?
                if (CHasChildNode(mapping, "resourceUsage"))
                {
                    // Remove the existing resource usage
                    CRemoveNode(CGetChildNode(mapping, "resourceUsage"));
                    cerr << "[Warning] ignoring resource usage of application";
                    cerr << " mapping" << endl;
                }

                // Add resource usage to the mapping
                CAddNode(mapping, resourceUsage);
            }

            // Create a new SDF3 node
            sdf3node = CNewNode("sdf3");

            // Add elements to the SDF3 node
            CAddNode(sdf3node, appGraph);
            CAddNode(sdf3node, archGraph);
            CAddNode(sdf3node, mapping);

            // Map the application to the architecture
            sdf3node = mapApplicationGraphToArchitectureGraph(sdf3node, out);

            // Failed to find a mapping?
            if (sdf3node == NULL)
                return false;

            // Update the resource usage
            resourceUsage = CGetChildNode(CGetChildNode(sdf3node, "mapping"),
                                          "resourceUsage");

            // Output every mapping to a different file
            if (!settings.outputFile.empty()
                && settings.applicationGraphs.size() != 1)
            {
                outFile.open(file + "_" + settings.outputFile);
                out.rdbuf(outFile.rdbuf());
            }

            // Output the result of the flow
            CNewDoc(sdf3node);
            //CSaveFile(out, sdf3doc);
        }

        return true;
    }

} // End namespace FSMSADF

/**
 * main ()
 * It does none of the hard work, but it is very needed...
 */
int main(int argc, char **argv)
{
    int exit_status = 0;

    try
    {
        // Initialize the program
        if (!initSettings(argc, argv))
            return 1;

        // Run mapping flow
        if (mappingFlow(settings.outputStream))
            exit_status = 0;
        else
            exit_status = 1;
    }
    catch (CException &e)
    {
        cerr << e;
        exit_status = 1;
    }

    return exit_status;
}

