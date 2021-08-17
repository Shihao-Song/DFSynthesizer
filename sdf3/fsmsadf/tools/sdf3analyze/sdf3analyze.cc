/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf3analyze.cc
 *
 *  Author          :   Sander Stuijk (s.stuijk@tue.nl)
 *
 *  Date            :   21 April 2009
 *
 *  Function        :   FSM-based SADF Graph Analysis Functionality
 *
 *  History         :
 *      21-04-09    :   Initial version.
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

#include "sdf3analyze.h"
#include "../../../base/base.h"
#include "../../../sdf/sdf.h"
#include "../../fsmsadf.h"

using namespace FSMSADF::MPExplore;

namespace FSMSADF
{

    /**
     * initSettings ()
     * The function initializes the program settings.
     */
    void ToolAnalyze::initSettings(const CString &modulename, int argc, char **argv)
    {
        this->_settings = new AnalyzeSettings();
        Tool::initSettings(modulename, argc, argv);
    }

    /**
     * helpMessage ()
     * Function prints help message for the tool.
     */
    void ToolAnalyze::helpMessage(ostream &out)
    {
        out << "SDF3 " << TOOL << " (version " << DOTTED_VERSION ")" << endl;
        out << endl;
        out << "Usage: " << TOOL << " --graph <file> --check <property>";
        out << " [--output <file>]" << endl;
        out << "   --graph  <file>     input SADF graph" << endl;
        out << "   --output <file>     output file (default: stdout)" << endl;
        out << "   --algo <algorithm>  analyze the graph with requested algorithm:";
        out << endl;
        out << "       extractscenariographs" << endl;
        out << "       throughput" << endl;
        out << "       isolatescenarios" << endl;
        out << "       mpeigen" << endl;
        out << "       throughput-statespace" << endl;
        out << "       throughput-mpautomaton" << endl;
        out << "       throughput-mpautomaton-weakconsistency" << endl;
        out << "       criticalscenariosequence-weakconsistency" << endl;
    }

    bool ToolAnalyze::parseCommand(int argc, char **argv, int *arg)
    {
        // Configuration file
        if (argv[*arg] == CString("--graph") && (*arg) + 1 < argc)
        {
            (*arg)++;
            this->settings()->graphFile = argv[*arg];
        }
        else if (argv[*arg] == CString("--output") && (*arg) + 1 < argc)
        {
            (*arg)++;
            this->settings()->outputFile = argv[*arg];
        }
        else if (argv[*arg] == CString("--algo") && (*arg) + 1 < argc)
        {
            (*arg)++;
            this->settings()->arguments = this->parseSwitchArgument(argv[*arg]);
        }
        else return false;
        return true;
    }

    bool ToolAnalyze::checkRequiredSettings(void)
    {
        if (this->settings()->graphFile.empty()) return false;
        if (this->settings()->arguments.size() == 0) return false;;
        return true;
    }

    /**
     * loadApplicationGraphFromFile ()
     * The function returns a pointer to an XML data structures contained in the
     * supplied file that describes the SDFG.
     */
    void ToolAnalyze::loadApplicationGraphFromFile(CString &file, CString module, CNode **appGraphNode, CDoc **appGraphDoc)
    {
        CNode *sdf3Node;

        // Open file
        *appGraphDoc = CParseFile(file);
        if (*appGraphDoc == NULL)
            throw CException("Failed loading SADF Graph from '" + file + "'.");

        // Locate the sdf3 root element and check module type
        sdf3Node = CGetRootNode(*appGraphDoc);
        if (CGetAttribute(sdf3Node, "type") != module)
        {
            throw CException("Root element in file '" + file + "' is not "
                             "of type '" + module + "'.");
        }

        // Get application graph node
        *appGraphNode = CGetChildNode(sdf3Node, "applicationGraph");
        if (*appGraphNode == NULL)
            throw CException("No applicationGraph node in '" + file + "'.");
    }

    void ToolAnalyze::cleanUp()
    {
        Tool::cleanUp();
        // cleanup the xml document
        if (this->xmlAppGraphDoc != NULL)
        {
            releaseDoc(this->xmlAppGraphDoc);
        }
        // cleanup other xml resources
        xmlCleanup();
    }

    /**
     * AnalyzeGraph()
     * The function that calls the actual analysis functions
     */
    void ToolAnalyze::analyzeGraph(ostream &out)
    {
        Graph *fsmsadfGraph;
        CNode *applicationNode;
        CTimer timer;

        // Find sdf graph in XML structure
        applicationNode = this->xmlAppGraph;

        // Construction SDF graph model
        fsmsadfGraph = new Graph();
        fsmsadfGraph->constructFromXML(applicationNode);

        CPairs &analyze = this->settings()->arguments;

        // Select the correct algorithm
        if (analyze.front().key == "extractscenariographs")
        {
            // Print the graph which has just been loaded from file
            fsmsadfGraph->print(cout);

            // Create XML
            CNode *sdf3Node = CNewNode("sdf3");
            CAddAttribute(sdf3Node, "version", "1.0");
            CAddAttribute(sdf3Node, "type", "fsmsadf");
            CAddAttribute(sdf3Node, "xmlns:xsi",
                          "http://www.w3.org/2001/XMLSchema-instance");
            CAddAttribute(sdf3Node, "xsi:noNamespaceSchemaLocation",
                          "http://www.es.ele.tue.nl/sdf3/xsd/sdf3-fsmsadf.xsd");

            // Application graph node
            CNode *appNode = CAddNode(sdf3Node, "applicationGraph");

            // SDF node
            fsmsadfGraph->convertToXML(appNode);

            // Create document and save it
            CDoc *doc = CNewDoc(sdf3Node);
            CSaveFile(out, doc);

            Scenarios scenarios = fsmsadfGraph->getScenarios();
            for (Scenarios::const_iterator i = scenarios.begin();
                 i != scenarios.end(); i++)
            {
                TimedSDFgraph *g = fsmsadfGraph->extractSDFgraph(*i);
                outputSDFasXML(g, out);
            }
        }
        else if (analyze.front().key == "throughput")
        {
            Throughput thr;

            // Measure execution time
            startTimer(&timer);

            thr = throughputAnalysisGraph(fsmsadfGraph);
            cout << "Throughput (scenario graph):       " << thr << endl;

            // Measure execution time
            stopTimer(&timer);
            out << "analysis time: ";
            printTimer(out, &timer);
            out << endl;

            // Measure execution time
            startTimer(&timer);

            thr = throughputAnalysisUsingRefSchedule(fsmsadfGraph);
            cout << "Throughput (reference schedule):   " << thr << endl;

            // Measure execution time
            stopTimer(&timer);
            out << "analysis time: ";
            printTimer(out, &timer);
            out << endl;

            // Measure execution time
            startTimer(&timer);

            thr = throughputAnalysisWithScenarioTransitions(fsmsadfGraph);
            cout << "Throughput (scenario transitions): " << thr << endl;

            // Measure execution time
            stopTimer(&timer);
            out << "analysis time: ";
            printTimer(out, &timer);
            out << endl;
        }
        else if (analyze.front().key == "isolatescenarios")
        {
            Graph *newFSMsadfGraph = fsmsadfGraph->clone(GraphComponent(NULL, 0));

            // Create a cloned copy of the graph
            newFSMsadfGraph->isolateScenarios();

            // Create XML
            CNode *sdf3Node = CNewNode("sdf3");
            CAddAttribute(sdf3Node, "version", "1.0");
            CAddAttribute(sdf3Node, "type", "fsmsadf");
            CAddAttribute(sdf3Node, "xmlns:xsi",
                          "http://www.w3.org/2001/XMLSchema-instance");
            CAddAttribute(sdf3Node, "xsi:noNamespaceSchemaLocation",
                          "http://www.es.ele.tue.nl/sdf3/xsd/sdf3-fsmsadf.xsd");

            // Application graph node
            newFSMsadfGraph->convertToXML(CAddNode(sdf3Node, "applicationGraph"));

            // Create document and save it
            CDoc *doc = CNewDoc(sdf3Node);
            CSaveFile(out, doc);

            delete newFSMsadfGraph;
        }
        else if (analyze.front().key == "mpeigen")
        {
            map<Scenario *, MaxPlus::Vector *> eigenvector;
            map<Scenario *, MPTime> period;
            map<FSMtransition *, MPTime> delay;
            map<Scenario *, MPExplore::Exploration *> mpe;

            // The set of scenarios...
            Scenarios scenarios = fsmsadfGraph->getScenarios();

            // Create Max-Plus exploration object for each scenario
            for (Scenarios::const_iterator i = scenarios.begin();
                 i != scenarios.end(); i++)
            {
                Scenario *s = *i;
                mpe[s] = new MPExplore::Exploration;
                mpe[s]->G = new SGraph(s->getScenarioGraph(), s);
            }

            // Compute eigenvector and period for each scenario
            for (Scenarios::const_iterator i = scenarios.begin();
                 i != scenarios.end(); i++)
            {
                Scenario *s = *i;
                MaxPlus::Vector *v_s = NULL;
                MPTime T_s;

                // Compute eigenvector and period of this scenario
                mpe[s]->exploreEigen(&v_s, &T_s);

                // Store results
                eigenvector[s] = v_s;
                period[s] = T_s;

                // Output result
                cout << "eigenvalue: " << T_s << endl;
                for (uint x = 0; x < v_s->getSize(); x++)
                    cout << v_s->get(x) << " ; ";
                cout << endl;
            }

            // Cleanup
            for (map<Scenario *, MaxPlus::Vector *>::iterator i = eigenvector.begin();
                 i != eigenvector.end(); i++)
            {
                delete i->second;
            }
            for (map<Scenario *, MPExplore::Exploration *>::iterator i = mpe.begin();
                 i != mpe.end(); i++)
            {
                delete i->second->G;
                delete i->second;
            }
        }
        else if (analyze.front().key == "throughput-statespace")
        {
            Throughput thr;

            // Measure execution time
            startTimer(&timer);

            thr = throughputAnalysisGraphUsingStateSpace(fsmsadfGraph);
            cout << "Throughput (state-space):          " << thr << endl;

            // Measure execution time
            stopTimer(&timer);
            out << "analysis time: ";
            printTimer(out, &timer);
            out << endl;
        }
        else if (analyze.front().key == "throughput-mpautomaton")
        {
            Throughput thr;

            // Measure execution time
            startTimer(&timer);

            thr = throughputAnalysisGraphUsingMaxPlusAutomaton(fsmsadfGraph);
            cout << "Throughput (max-plus automaton):          " << thr << endl;

            // Measure execution time
            stopTimer(&timer);
            out << "analysis time: ";
            printTimer(out, &timer);
            out << endl;
        }
        else if (analyze.front().key == "throughput-mpautomaton-weakconsistency")
        {
            Throughput thr;

            // Measure execution time
            startTimer(&timer);

            thr = throughputAnalysisGraphUsingMaxPlusAutomatonWeaklyConsistent(fsmsadfGraph);
            cout << "Throughput (max-plus automaton):          " << thr << endl;

            // Measure execution time
            stopTimer(&timer);
            out << "analysis time: ";
            printTimer(out, &timer);
            out << endl;
        }
        else if (analyze.front().key == "criticalscenariosequence-weakconsistency")
        {
            // Measure execution time
            startTimer(&timer);
            ScenarioList *ls = criticalScenarioSequenceWeaklyConsistent(fsmsadfGraph);

            cout << "Critical scenario sequence:" << endl;

            ScenarioList::const_iterator i = ls->begin();
            for (; i != ls->end();)
            {
                const Scenario s = *(*i);
                cout << s.getName();
                if (++i != ls->end())
                    cout << ", ";
            }
            cout << endl;

            // Measure execution time
            stopTimer(&timer);
            out << "analysis time: ";
            printTimer(out, &timer);
            out << endl;
        }
        else
        {
            throw CException("Unknown algorithm.");
        }

        delete fsmsadfGraph;
    }

    void ToolAnalyze::loadApplicationGraph()
    {
        this->loadApplicationGraphFromFile(
            this->settings()->graphFile, MODULE, &(this->xmlAppGraph), &(this->xmlAppGraphDoc));
    }

} // end namespace

/**
 * main ()
 * It does none of the hard work, but it is very needed...
 */
int main(int argc, char **argv)
{
    FSMSADF::ToolAnalyze T;

    int exit_status = 0;
    ofstream out;

    try
    {
        // Initialize the program
        T.initSettings(MODULE, argc, argv);

        // Load application graph
        T.loadApplicationGraph();

        // Set output stream
        if (!T.settings()->outputFile.empty())
            out.open(T.settings()->outputFile.c_str());
        else
            ((ostream &)(out)).rdbuf(cout.rdbuf());

        // Perform the actual function
        T.analyzeGraph(out);
        T.cleanUp();
    }
    catch (CException &e)
    {
        cerr << e;
        exit_status = 1;
    }

    return exit_status;
}

