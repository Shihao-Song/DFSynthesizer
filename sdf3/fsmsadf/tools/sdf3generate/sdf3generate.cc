/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf3generate.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 15, 2009
 *
 *  Function        :   Random FSM-SADF graphs
 *
 *  History         :
 *      15-07-09    :   Initial version.
 *
 * $Id: sdf3generate.cc,v 1.1.2.4 2010-05-11 03:27:08 mgeilen Exp $
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

#include "sdf3generate.h"
#include "../../fsmsadf.h"

using namespace FSMSADF;

/**
 * initSettings ()
 * The function initializes the program settings.
 */
void ToolGenerate::initSettings(const CString &modulename, int argc, char **argv)
{
    this->_settings = new GenerateSettings();
    Tool::initSettings(modulename, argc, argv);
}


/**
 * helpMessage ()
 * Function prints help message for the tool.
 */
void ToolGenerate::helpMessage(ostream &out)
{
    out << "SDF3 " << TOOL << " (version " << DOTTED_VERSION ")" << endl;
    out << endl;
    out << "Usage: " << TOOL << " [--settings <file> --output <file>]" << endl;
    out << "   --settings  <file>  settings for the graph generator (default: ";
    out << "sdf3.opt)" << endl;
    out << "   --output <file>     output file (default: stdout)" << endl;
}

bool ToolGenerate::parseCommand(int argc, char **argv, int *arg)
{
    if (argv[*arg] == CString("--output") && (*arg) + 1 < argc)
    {
        (*arg)++;
        this->settings()->outputFile = argv[*arg];
    }
    else if (argv[*arg] == CString("--settings") && (*arg) + 1 < argc)
    {
        (*arg)++;
        this->settings()->settingsFile = argv[*arg];
    }
    else return false;
    return true;
}


/**
 * setDefaults ()
 * Set all settings at their default value.
 */
void ToolGenerate::setDefaults()
{
    this->settings()->settingsFile = "sdf3.opt";
}

/**
 * parseSettingsFile ()
 * The function parses all settings from the file.
 */
void ToolGenerate::parseSettingsFile(CString module, CString type)
{
    CNode *sdf3Node, *settingsNode;
    CString name, file;
    CDoc *settingsDoc;

    // Open settings file and get root node
    settingsDoc = CParseFile(this->settings()->settingsFile);
    sdf3Node = CGetRootNode(settingsDoc);
    if (sdf3Node == NULL)
    {
        throw CException("Failed opening '" + this->settings()->settingsFile + "'.");
    }

    // Is the node of the correct type?
    if (CGetAttribute(sdf3Node, "type") != module)
    {
        throw CException("Root element in file '" + this->settings()->settingsFile
                         + "' is not of type '" + module + "'.");
    }

    // Get the settings element of the tool
    for (settingsNode = CGetChildNode(sdf3Node, "settings");
         settingsNode != NULL;
         settingsNode = CNextNode(settingsNode, "settings"))
    {
        if (CGetAttribute(settingsNode, "type") == type)
        {
            break;
        }
    }

    // Found the correct settings element?
    if (settingsNode == NULL)
    {
        throw CException("File '" + this->settings()->settingsFile + "' contains no "
                         " settings of type '" + type + "'.");
    }

    this->settings()->settingsNode = settingsNode;
}

bool ToolGenerate::checkRequiredSettings()
{
    return true;
}

/**
 * postInitSettings ()
 *
 */
void ToolGenerate::postInitSettings()
{
    // Parse settings
    this->parseSettingsFile(MODULE, SETTINGS_TYPE);
}

void ToolGenerate::cleanUp()
{
    Tool::cleanUp();
    // cleanup other xml resources
    xmlCleanup();
}


/**
 * generateRandomGraph ()
 * Generate a random graph which is connected, consistent and deadlock-free.
 */
void ToolGenerate::generateRandomGraph(CNode *settingsNode, ostream &out)
{
    RandomGraph randomGraph;

    // Load settings from the settings file
    randomGraph.initializeFromXML(settingsNode);

    // Generate a random graph
    Graph *g = randomGraph.generateGraph();

    // Create XML
    CNode *sdf3Node = CNewNode("sdf3");
    CAddAttribute(sdf3Node, "version", "1.0");
    CAddAttribute(sdf3Node, "type", "fsmsadf");
    CAddAttribute(sdf3Node, "xmlns:xsi",
                  "http://www.w3.org/2001/XMLSchema-instance");
    CAddAttribute(sdf3Node, "xsi:noNamespaceSchemaLocation",
                  "http://www.es.ele.tue.nl/sdf3/xsd/sdf3-fsmsadf.xsd");

    // Application graph node
    g->convertToXML(CAddNode(sdf3Node, "applicationGraph"));

    // Create document and save it
    CDoc *doc = CNewDoc(sdf3Node);
    CSaveFile(out, doc);

    delete g;
}

/**
 * main ()
 * It does none of the hard work, but it is very needed...
 */
int main(int argc, char **argv)
{
    FSMSADF::ToolGenerate T;

    int exit_status = 0;
    ofstream out;

    try
    {
        // Initialize the program
        T.initSettings(MODULE, argc, argv);

        // Set output stream
        if (!T.settings()->outputFile.empty())
            out.open(T.settings()->outputFile.c_str());
        else
            ((ostream &)(out)).rdbuf(cout.rdbuf());

        // Generate random graph
        T.generateRandomGraph(T.settings()->settingsNode, out);

        // cleanup
        T.cleanUp();
    }
    catch (CException &e)
    {
        cerr << e;
        exit_status = 1;
    }

    return exit_status;
}

