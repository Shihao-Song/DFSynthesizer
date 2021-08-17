/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf3transform.h
 *
 *  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
 *
 *  Date            :   August 9, 2012
 *
 *  Function        :   CSDFG transformation functionality
 *
 *  History         :
 *      09-08-12    :   Initial version.
 *
 * $Id:  $
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
#include "../../csdf.h"
#include "../../transform/to_wc_fsm_sadf/csdftowcfsmsadf.h"

using namespace CSDF;
namespace CSDF
{


    typedef struct _CPair
    {
        CString key;
        CString value;
    } CPair;

    typedef list<CPair>         CPairs;
    typedef CPairs::iterator    CPairsIter;

    /**
     * Settings
     * Struct to store program settings.
     */
    typedef struct _Settings
    {
        // Input file with graph
        CString graphFile;

        // Output file
        CString outputFile;

        // Switch argument(s) given to algorithm
        CPairs arguments;

        // Application graph
        CNode *xmlAppGraph;
    } Settings;

    /**
     * settings
     * Program settings.
     */
    Settings settings;

    /**
     * helpMessage ()
     * Function prints help message for the tool.
     */
    void helpMessage(ostream &out)
    {
        out << "SDF3 " << TOOL << " (version " << DOTTED_VERSION ")" << endl;
        out << endl;
        out << "Usage: " << TOOL << " --graph <file> --transform <list>";
        out << " [--output <file>]" << endl;
        out << "   --graph  <file>     input CSDF graph" << endl;
        out << "   --output <file>     output file (default: stdout)" << endl;
        out << "   --transform <list>  comma separated list with graph";
        out << " transformations:" << endl;
        out << "       to_wc-fsmsadf" << endl;
    }

    /**
     * parseSwitchArgument ()
     * The function parses the string 'arguments' into a sequence of (arg, value)
     * pairs. The syntax as as follows:
     *
     * pair := key(value)
     * arg := pair,pair,...
     *
     * Note: value may be a pair itself, but this is not expanded into a set of
     * pairs (i.e. nested pairs are not supported).
     */
    CPairs parseSwitchArgument(CString arguments)
    {
        CPairs pairs;
        CPair p;

        while (arguments.size() != 0)
        {
            char c;
            p.key = "";
            p.value = "";

            // Get key from argument string
            do
            {
                c = arguments[0];
                arguments = arguments.substr(1);
                if (c == ',' || c == '(')
                    break;
                p.key += c;
            }
            while (arguments.size() != 0);

            // Is next part of argument a value?
            if (c == '(')
            {
                CString::size_type ePos = 0;
                int level = 1;

                // Find the matching closing brace
                while (level != 0 && arguments.size() != 0)
                {
                    if (arguments.operator[](ePos) == ')')
                        level--;
                    else if (arguments.operator[](ePos) == '(')
                        level++;

                    // Next
                    ePos++;
                }

                // Closing brace found?
                if (level != 0)
                    throw CException("Missing closing brace in value of argument.");

                // Get value
                p.value = arguments.substr(0, ePos - 1);

                // More arguments left?
                if (arguments.size() > ePos)
                    arguments = arguments.substr(ePos + 1);
                else
                    arguments = "";
            }

            // Add pair to list of pairs
            pairs.push_back(p);
        }

        return pairs;
    }

    /**
     * parseCommandLine ()
     * The function parses the command line arguments and add info to the
     * supplied settings structure.
     */
    void parseCommandLine(int argc, char **argv)
    {
        int arg = 1;

        while (arg < argc)
        {
            // Configuration file
            if (argv[arg] == CString("--graph") && arg + 1 < argc)
            {
                arg++;
                settings.graphFile = argv[arg];
            }
            else if (argv[arg] == CString("--output") && arg + 1 < argc)
            {
                arg++;
                settings.outputFile = argv[arg];
            }
            else if (argv[arg] == CString("--transform") && arg + 1 < argc)
            {
                arg++;
                settings.arguments = parseSwitchArgument(argv[arg]);
            }
            else
            {
                helpMessage(cerr);
                throw CException("");
            }

            // Next argument
            arg++;
        }
    }

    /**
     * loadApplicationGraphFromFile ()
     * The function returns a pointer to an XML data structures contained in the
     * supplied file that describes the SDFG.
     */
    CNode *loadApplicationGraphFromFile(CString &file, CString module)
    {
        CNode *appGraphNode, *sdf3Node;
        CDoc *appGraphDoc;

        // Open file
        appGraphDoc = CParseFile(file);
        if (appGraphDoc == NULL)
            throw CException("Failed loading application from '" + file + "'.");

        // Locate the sdf3 root element and check module type
        sdf3Node = CGetRootNode(appGraphDoc);
        if (CGetAttribute(sdf3Node, "type") != module)
        {
            throw CException("Root element in file '" + file + "' is not "
                             "of type '" + module + "'.");
        }

        // Get application graph node
        appGraphNode = CGetChildNode(sdf3Node, "applicationGraph");
        if (appGraphNode == NULL)
            throw CException("No application graph in '" + file + "'.");

        return appGraphNode;
    }

    /**
     * initSettings ()
     * The function initializes the program settings.
     */
    void initSettings(int argc, char **argv)
    {
        // Parse the command line
        parseCommandLine(argc, argv);

        // Check required settings
        if (settings.graphFile.empty() || settings.arguments.size() == 0)
        {
            helpMessage(cerr);
            throw CException("");
        }

        // Load application graph
        settings.xmlAppGraph = loadApplicationGraphFromFile(settings.graphFile,
                               MODULE);
    }

    /**
     * transformGraph ()
     * The function transforms the SDF graph and the resulting (H)SDF graph
     * is outputted in XML format to the out stream.
     * TODO(MG) tool is still based on a sequence of transformations, but this is not compatible with transformations
     * to different graph types.
     */
    void transformGraph(TimedCSDFgraph *g, CPairs &transforms, ostream &out)
    {
        CSDFcomponent component = CSDFcomponent(g->getParent(), g->getId());
        TimedCSDFgraph *h_in = g->clone(component);
        FSMSADF::Graph *h_out = NULL;

        for (CPairsIter iter = transforms.begin(); iter != transforms.end(); iter++)
        {
            CPair &transform = *iter;

            // Perform transformation
            if (transform.key == "to_wc-fsmsadf")
            {
                h_out = (FSMSADF::Graph *)convertCSDFGtoWCFSMSADFG(h_in);
            }
            else
            {
                throw CException("Unknown transformation.");
            }

        }

        // Output resulting SDF graph
        if (h_out != NULL)
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
            h_out->convertToXML(appNode);

            // Create document and save it
            CDoc *doc = CNewDoc(sdf3Node);
            CSaveFile(out, doc);

            // Cleanup
            delete h_out;
        }

        // Cleanup
        delete h_in;
    }

    /**
     * transformGraph ()
     * The function transforms the SDF graph.
     */
    void transformGraph(ostream &out)
    {
        TimedCSDFgraph *csdfGraph;
        CNode *csdfNode, *csdfPropertiesNode;

        // Find sdf graph in XML structure
        csdfNode = CGetChildNode(settings.xmlAppGraph, "csdf");
        if (csdfNode == NULL)
            throw CException("Invalid xml file - missing 'csdf' node");
        csdfPropertiesNode = CGetChildNode(settings.xmlAppGraph, "csdfProperties");

        // Construction CSDF graph model
        csdfGraph = constructTimedCSDFgraph(csdfNode, csdfPropertiesNode);

        // The actual transformation...
        transformGraph(csdfGraph, settings.arguments, out);

        // Cleanup
        delete csdfGraph;
    }

}//namespace CSDF
/**
 * main ()
 * It does none of the hard work, but it is very needed...
 */
int main(int argc, char **argv)
{
    int exit_status = 0;
    ofstream out;

    try
    {
        // Initialize the program
        initSettings(argc, argv);

        // Set output stream
        if (!settings.outputFile.empty())
            out.open(settings.outputFile.c_str());
        else
            ((ostream &)(out)).rdbuf(cout.rdbuf());

        // Perform requested actions
        transformGraph(out);
    }
    catch (CException &e)
    {
        cerr << e;
        exit_status = 1;
    }

    return exit_status;
}
