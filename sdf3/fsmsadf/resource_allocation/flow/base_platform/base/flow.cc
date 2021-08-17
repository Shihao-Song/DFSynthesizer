/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   flow.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   February 7, 2007
 *
 *  Function        :   SDFG mapping to MP-SoC
 *
 *  History         :
 *      07-02-07    :   Initial version.
 *      19-04-11    :   Add accessors (M. Koedam)
 *                      Virtual functions, rename to SDF3FlowBase
 *
 * $Id: flow.cc,v 1.2 2010-02-08 08:04:42 sander Exp $
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

#include "flow.h"
#include "../../../../output/html/html.h"

namespace FSMSADF
{

    /**
     * SDF3FlowBase ()
     * Constructor.
     */
    SDF3FlowBase::SDF3FlowBase()
        :
        stepMode(false),
        applicationGraph(NULL),
        platformGraph(NULL),
        stateOfFlow(FlowStart),
        maxNrBindingsTileBindingAlgo(1),
        memoryDimAlgo(NULL),
        tileBindingAlgo(NULL)
    {
    }

    /**
     * ~SDF3FlowBase ()
     * Destructor.
     */
    SDF3FlowBase::~SDF3FlowBase()
    {
        delete applicationGraph;
        delete platformGraph;


        for (PlatformBindings::iterator i = platformBindings.begin();
             i != platformBindings.end(); i++)
        {
            delete(*i);
        }
    }

    void SDF3FlowBase::constructFromXML(const CNodePtr sdf3Node)
    {
        // Application graph
        if (!CHasChildNode(sdf3Node, "applicationGraph"))
            throw CException("No application graph in sdf3 document.");
        applicationGraph = new Graph(GraphComponent(NULL, 0));
        applicationGraph->constructFromXML(CGetChildNode(sdf3Node, "applicationGraph"));

        // Architecture graph
        if (!CHasChildNode(sdf3Node, "architectureGraph"))
            throw CException("No archtitecture graph in sdf3 document.");
        platformGraph = new PlatformGraph(GraphComponent(NULL, 0));
        platformGraph->constructFromXML(CGetChildNode(sdf3Node, "architectureGraph"));

        // Mappings
        for (CNode *mappingNode = CGetChildNode(sdf3Node, "mapping");
             mappingNode != NULL; mappingNode = CNextNode(mappingNode, "mapping"))
        {
            // Create a new platform binding
            PlatformBinding *platformBinding = new PlatformBinding(
                GraphComponent(NULL, 0),
                platformGraph, applicationGraph);

            // Load mapping
            platformBinding->constructFromXML(mappingNode);

            // Add to platform bindings
            platformBindings.push_back(platformBinding);
        }
    }

    /**
     * convertToXML ()
     * The function add the application graph, platform graph, mapping, and
     * resource usage to the sdf3Node..
     */
    void SDF3FlowBase::convertToXML(const CNodePtr sdf3Node)
    {
        // Attributes
        CAddAttribute(sdf3Node, "version", "1.0");
        CAddAttribute(sdf3Node, "type", "fsmsadf");
        CAddAttribute(sdf3Node, "xmlns:xsi",
                      "http://www.w3.org/2001/XMLSchema-instance");
        CAddAttribute(sdf3Node, "xmlns", "uri:sdf3");
        CAddAttribute(sdf3Node, "xsi:noNamespaceSchemaLocation",
                      "http://www.es.ele.tue.nl/sdf3/xsd/sdf3-fsmsadf.xsd");

        // Application graph
        applicationGraph->convertToXML(CAddNode(sdf3Node, "applicationGraph"));

        // Architecture graph
        platformGraph->convertToXML(CAddNode(sdf3Node, "architectureGraph"));

        // Mappings
        for (PlatformBindings::iterator i = platformBindings.begin();
             i != platformBindings.end(); i++)
        {
            PlatformBinding *platformBinding = *i;
            platformBinding->convertToXML(CAddNode(sdf3Node, "mapping"));
        }
    }

    /**
     * convertToHTML ()
     * The function outputs the application graph, platform graph, mapping, and
     * resource usage to a set of HTML files. These files are stored in the
     * directory with the supplied dirname.
     */
    void SDF3FlowBase::convertToHTML(const CString &dirname)
    {
        OutputHTML html;

        // Set the name of the output directory
        html.setDirname(dirname);

        // Output flow as HTML
        html.outputAsHTML(this);
    }

    /**
     * checkInputDesignFlow ()
     * The function checks that all required inputs are present and it initializes
     * the used data-structures.
     */
    void SDF3FlowBase::checkInputDesignFlow()
    {
        // Output current state of the flow
        logInfo("Check input design flow.");

        // Memory dimensioning algorithm specified?
        if (memoryDimAlgo == NULL)
            throw CException("[ERROR] No memory dimensioning algorithm specified.");

        // Tile biding and scheduling algorithm specified?
        if (tileBindingAlgo == NULL)
            throw CException("[ERROR] No tile binding algorithm specified.");

        // Application graph has no isolated scenarios?
        if (!getApplicationGraph()->hasIsolatedScenarios())
        {
            // Output current state of the flow
            logInfo("Isolate scenarios.");

            // Isolate scenarios
            getApplicationGraph()->isolateScenarios();
        }

        // Link application to memory dimensioning algorithm
        memoryDimAlgo->setApplicationGraph(getApplicationGraph());
        memoryDimAlgo->setPlatformGraph(getPlatformGraph());
        memoryDimAlgo->setPlatformBindings(&platformBindings);

        // Initialize memory dimensioning algorithm
        memoryDimAlgo->init();

        // Link application, platform and binding to tile binding algorithm
        tileBindingAlgo->setApplicationGraph(getApplicationGraph());
        tileBindingAlgo->setPlatformGraph(getPlatformGraph());
        tileBindingAlgo->setPlatformBindings(&platformBindings);

        // Initialize tile binding algorithm
        tileBindingAlgo->init();

        // Advance to next state in the flow
        setNextStateOfFlow(FlowComputeStorageDist);
    }

    /**
     * run ()
     * Execute the design flow. The function returns the last state reached by
     * the design flow. Its value is equal to 'FlowCompleted' when all phases
     * have been executed succesfully or else it is 'FlowFailed' which indicates
     * that a step of the flow could not be completed succesfully.
     */
    SDF3FlowBase::FlowState SDF3FlowBase::run()
    {
        CTimer timer;

        do
        {
            // Measure execution time
            startTimer(&timer);

            switch (getStateOfFlow())
            {
                case FlowStart:
                    checkInputDesignFlow();
                    break;

                case FlowComputeStorageDist:
                    computeStorageDist();
                    break;

                case FlowSelectStorageDist:
                    selectStorageDist();
                    break;

                case FlowEstimateStorageDist:
                    estimateStorageDist();
                    break;

                case FlowEstimateBandwidthConstraint:
                    estimateBandwidthConstraint();
                    break;

                case FlowBindGraphtoTile:
                    bindGraphtoTiles();
                    break;

                case FlowEstimateConnectionDelay:
                    estimateConnectionDelay();
                    break;

                case FlowConstructTileSchedules:
                    constructTileSchedules();
                    break;

                default:
                    break;
            }

            // Measure execution time
            stopTimer(&timer);

            cout << "[INFO   ] Step took: ";
            printTimer(cout, &timer);
            cout << endl;

            if (getStepMode())
            {
                handleUserInteraction();
            }

        }
        while (getStateOfFlow() != FlowCompleted
               && getStateOfFlow() != FlowFailed);

        return getStateOfFlow();
    }

    /**
     * handleUserInteraction ()
     * The function request the user for the next action to perform. Possible
     * actions include continuing with the next step of the flow, completing the
     * remaining flow or printing the current result of the flow to the terminal.
     */
    void SDF3FlowBase::handleUserInteraction()
    {
        bool done = false;
        char cmd, c;

        do
        {
            // Print message
            cout << "Command: (n)ext step / (c)ontinue flow / print (s)tate? ";

            // First character determines command
            cmd = cin.get();

            // Command is end-of-line (default action: step)
            if (cmd == '\n')
                cmd = 'n';
            else
            {
                // Remove remaining characters on current line from the input
                do
                {
                    c = cin.get();
                }
                while (c != '\n');
            }

            switch (cmd)
            {
                case 'n':
                    done = true;
                    break;

                case 'c':
                    setStepMode(false);
                    done = true;
                    break;

                case 's':
                    outputAsXML(cout);
                    break;

                default:
                    break;
            }
        }
        while (!done);
    }

    /**
     * outputAsXML ()
     * The function ouputs an sdf3 node to the supplied stream.
     */
    void SDF3FlowBase::outputAsXML(ostream &out)
    {
        CNode *sdf3Node;
        CDoc *doc;

        // SDF mapping node
        sdf3Node = CNewNode("sdf3");
        convertToXML(sdf3Node);

        // Create document and save it
        doc = CNewDoc(sdf3Node);
        CSaveFile(out, doc);
    }

} // End namespace FSMSADF
