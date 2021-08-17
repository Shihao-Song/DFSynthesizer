/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf3transform.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 21, 2009
 *
 *  Function        :   FSM-based SADF transformation functionality
 *
 *  History         :
 *      21-07-09    :   Initial version.
 *
 * $Id: sdf3transform.cc,v 1.1.2.3 2010-05-11 03:27:08 mgeilen Exp $
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

#include "sdf3transform.h"
#include "../../fsmsadf.h"

using namespace FSMSADF;


/**
 * initSettings ()
 * The function initializes the program settings.
 */
void ToolTransform::initSettings(const CString &modulename, int argc, char **argv)
{
    this->_settings = new TransformSettings();
    Tool::initSettings(modulename, argc, argv);
}

/**
 * helpMessage ()
 * Function prints help message for the tool.
 */
void ToolTransform::helpMessage(ostream &out)
{
    out << "SDF3 " << TOOL << " (version " << DOTTED_VERSION ")" << endl;
    out << endl;
    out << "Usage: " << TOOL << " --graph <file> --transform <list>";
    out << " [--scenario <name>] [--output <file>]" << endl;
    out << "   --graph  <file>     input SDF graph" << endl;
    out << "   --scenario <name>   select a scenario" << endl;
    out << "   --output <file>     output file (default: stdout)" << endl;
    out << "   --transform <list>  comma separated list with graph";
    out << " transformations:" << endl;
    out << "       wc_sdfg" << endl;
    out << "       model_autoconc(<max>)" << endl;
    out << "       to_sdfg" << endl;
}

bool ToolTransform::parseCommand(int argc, char **argv, int *arg)
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
    else if (argv[*arg] == CString("--scenario") && (*arg) + 1 < argc)
    {
        (*arg)++;
        this->settings()->scenario = argv[*arg];
    }
    else if (argv[*arg] == CString("--transform") && (*arg) + 1 < argc)
    {
        (*arg)++;
        this->settings()->arguments = this->parseSwitchArgument(argv[*arg]);
    }
    else return false;
    return true;
}

bool ToolTransform::checkRequiredSettings(void)
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
void ToolTransform::loadApplicationGraphFromFile(CString &file, CString module, CNode **appGraphNode, CDoc **appGraphDoc)
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

void ToolTransform::cleanUp()
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

void ToolTransform::loadApplicationGraph()
{
    this->loadApplicationGraphFromFile(
        this->settings()->graphFile, MODULE, &(this->xmlAppGraph), &(this->xmlAppGraphDoc));
}



/**
 * transformGraph ()
 * The function transforms the SDF graph and the resulting (H)SDF graph
 * is outputted in XML format to the out stream.
 */
Graph *ToolTransform::transformGraph(Graph *g, CPairs &transforms, ostream &out)
{
    Graph *h_in = g->clone(GraphComponent(g->getParent(), g->getId(), g->getName()));
    Graph *h_out;

    for (CPairsIter iter = transforms.begin();
         h_in != NULL && iter != transforms.end(); iter++)
    {
        CPair &transform = *iter;
        TimedSDFgraph *sdfg;

        // Perform transformation
        if (transform.key == "wc_sdfg")
        {
            h_out = transformToWorstCaseGraph(h_in);
        }
        else if (transform.key == "to_sdfg")
        {
            sdfg = h_in->extractSDFgraph(h_in->getScenario(this->settings()->scenario));
            outputSDFasXML(sdfg, out);
            h_out = NULL;
        }
        else if (transform.key == "model_autoconc")
        {
            if (transform.value == "")
                throw CException("Missing maximum degree of auto-concurrency.");

            h_out = modelAutoConcurrencyInGraph(h_in, transform.value);
        }
        else
        {
            throw CException("Unknown transformation.");
        }

        // Output graph becomes next input
        delete h_in;
        h_in = h_out;
    }

    return h_in;
}

/**
 * transformGraph ()
 * The function transforms the graph.
 */
void ToolTransform::transformGraph(ostream &out)
{
    Graph *g, *gTransformed;
    CNode *applicationNode;

    // Find sdf graph in XML structure
    applicationNode = this->xmlAppGraph;

    // Construction graph model
    g = new Graph();
    g->constructFromXML(applicationNode);

    // The actual transformation...
    gTransformed = transformGraph(g, this->settings()->arguments, out);

    // Graph returned
    if (gTransformed != NULL)
    {
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
        gTransformed->convertToXML(appNode);

        // Create document and save it
        CDoc *doc = CNewDoc(sdf3Node);
        CSaveFile(out, doc);

        // Cleanup
        delete gTransformed;
    }

    // Cleanup
    delete g;
}

/**
 * main ()
 * It does none of the hard work, but it is very needed...
 */
int main(int argc, char **argv)
{
    FSMSADF::ToolTransform T;

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

        // Perform requested actions
        T.transformGraph(out);
        T.cleanUp();
    }
    catch (CException &e)
    {
        cerr << e;
        exit_status = 1;
    }

    return exit_status;
}
