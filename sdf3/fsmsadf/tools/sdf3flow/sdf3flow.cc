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
 * $Id: sdf3flow.cc,v 1.1.2.3 2010-08-01 17:26:43 mgeilen Exp $
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
    FlowSettings settings(MODULE, SETTINGS_TYPE);

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
        out << "   --html             output mapping in HTML format" << endl;
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
     * mapApplicationGraphToArchitectureGraph ()
     * The complete mapping flow. The function returns a pointer to an sdf3node
     * when a mapping has been found. Otherwise the function returns a NULL pointer.
     */
    CNode *mapApplicationGraphToArchitectureGraph(CNode *sdf3Node, bool outputHTML)
    {
        SDF3FlowBase::FlowState status;
        CNode *newSDF3Node;
        SDF3FlowBase *flow;
        CTimer timer;

        // Create a new mapping flow
        if (settings.platformType == PlatformTypeVirtual)
        {
            flow = new SDF3FlowVirtualPlatform();
        }
        else if (settings.platformType == PlatformTypeCompSoC)
        {
            flow = new SDF3FlowCompSoCPlatform();
        }
        else if (settings.platformType == PlatformTypeMAMPS)
        {
            flow = new SDF3FlowMampsPlatform();
        }
        else
        {
            throw CException("Unsupported platform type.");
        }

        // Run flow step-by-step?
        if (settings.stepFlag)
            flow->setStepMode(true);

        // Create flow from sdf3 node
        flow->constructFromXML(sdf3Node);

        // Set maximum number of platform bindings created in tile binding algorithm
        flow->setMaxNrBindingsTileBindingAlgo(settings.maxNrBindingsTileBindingAlgo);

        // Measure execution time
        startTimer(&timer);

        // Execute the flow
        status = flow->run();

        // Measure execution time
        stopTimer(&timer);

        cerr << "Execution time: ";
        printTimer(cerr, &timer);
        cerr << endl;

        // Create a new SDF3 node
        newSDF3Node = CNewNode("sdf3");
        flow->convertToXML(newSDF3Node);

        // Output mapping in HTML?
        if (outputHTML)
            flow->convertToHTML("html");

        // Cleanup
        delete flow;

        // Flow failed?
        if (status != SDF3FlowBase::FlowCompleted)
        {
            logError("Failed to complete flow.");
            return NULL;
        }

        // Done
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
        CDoc *archDoc, *appDoc, *sdf3doc;
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
            CAddNode(sdf3node, CCopyNode(appGraph));
            CAddNode(sdf3node, CCopyNode(archGraph));
            CAddNode(sdf3node, CCopyNode(mapping));

            // Map the application to the architecture
            sdf3node = mapApplicationGraphToArchitectureGraph(sdf3node,
                       settings.outputHTML);

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
            sdf3doc = CNewDoc(sdf3node);
            if (!CValidate(sdf3doc))
            {
                logWarning("Failed to validate the output file.");
            }
            CSaveFile(out, sdf3doc, 1);
            releaseDoc(sdf3doc);

            releaseDoc(appDoc);
        }

        releaseDoc(archDoc);

        return true;
    }

    // End namespace FSMSADF
}

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

