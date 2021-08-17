/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   settings.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 14, 2009
 *
 *  Function        :   Settings for sdf3flow
 *
 *  History         :
 *      28-04-11    :   Copy for SDF3Convert (M.L.P.J. Koedam)
 *      14-05-09    :   Initial version.
 *
 * $Id: settings.cc,v 1.1.2.3 2010-08-01 17:26:44 mgeilen Exp $
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

#include "settings.h"

namespace FSMSADF
{
    /**
     * Settings ()
     * Constructor. Create settings object with default values.
     */
    ConvertSettings::ConvertSettings(CString module, CString type)
        : module(module), type(type)
    {



        // No help requested
        helpFlag = false;

        // Settings file
        settingsFile = "sdf3.opt";


        // Output (stdout)
        outputFile = "";

        // PlatformType
        platformType = PlatformTypeUnknown;

        // file containing the application graph
        applicationGraph = "applications.xml";
        // file containing the application graph
        architectureGraph = "architecture.xml";

        /** @todo Hardcoded values: these has to be removed*/
        /* These are in ns */
        wheelsize = 215000;
        contextSwitchOverhead = 1500;
        slotLength = 20000;
        preemptive = true;


    }
    /**
     * ~FlowSettings ()
     * Destructor.
     */
    ConvertSettings::~ConvertSettings()
    {
    }
    /**
     * init ()
     * Initialize the settings using the supplied arguments. Note that the first
     * argument (argv[0]) is skipped.
     */
    void ConvertSettings::init(int argc, char **argv)
    {
        CStrings args;

        for (int i = 1; i < argc; i++)
            args.push_back(argv[i]);

        init(args);
    }
    /**
     * init ()
     * Initialize the settings using the supplied arguments. The function
     * loads also external files (e.g. settings, graph, etc) when required.
     */
    void ConvertSettings::init(CStrings args)
    {
        // Parse list of arguments
        parseArguments(args);

        // Only continue when no help is needed
        if (helpFlag)
            return;

        // Parse settings file
        parseSettingsFile();
    }
    /**
     * parseArguments ()
     * Parse the supplied arguments.
     */
    void ConvertSettings::parseArguments(CStrings args)
    {
        CStringsIter argIter = args.begin();

        // Parse list of arguments
        while (argIter != args.end())
        {
            CString arg, argNext;

            // Current argument
            arg = *argIter;

            // Argument left in list of arguments?
            if (argIter != args.end())
            {
                argIter++;
                if (argIter != args.end())
                    argNext = *argIter;
            }

            // Parse argument
            if (arg == "--settings")
            {
                settingsFile = argNext;
                argIter++;
            }
            else if (arg == "--output")
            {
                outputFile = argNext;
                argIter++;
            }
            else
            {
                helpFlag = true;
            }
        }
    }
    /**
     * parseSettingsFile ()
     * Load all settings from the settings file.
     */
    void ConvertSettings::parseSettingsFile()
    {
        CNode *settingsNode, *sdf3Node, *archGraphNode, *appGraphNode;
        CNode *platformTypeNode;
        CString name, file, platformTypeString;
        CDoc *settingsDoc;

        // Open settings file and get root node
        settingsDoc = CParseFile(settingsFile);
        sdf3Node = CGetRootNode(settingsDoc);
        if (sdf3Node == NULL)
        {
            throw CException("Failed opening '" + settingsFile + "'.");
        }

        // Is the node of the correct type?
        if (CGetAttribute(sdf3Node, "type") != module)
        {
            throw CException("Root element in file '" + settingsFile + "' is not "
                             "of type '" + module + "'.");
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
            throw CException("File '" + settingsFile + "' contains no settings " +
                             "of type '" + type + "'.");
        }


        // Platform type
        platformTypeNode = CGetChildNode(settingsNode, "platformType");
        if (platformTypeNode != NULL)
        {
            if (!CHasAttribute(platformTypeNode, "type"))
                throw CException("Missing type attribute on platformType.");
            platformTypeString = CGetAttribute(platformTypeNode, "type");
            if (platformTypeString == "Virtual")
            {
                platformType = PlatformTypeVirtual;
            }
            else if (platformTypeString == "CompSoC")
            {
                platformType = PlatformTypeCompSoC;
            }
            else if (platformTypeString == "MAMPS")
            {
                platformType = PlatformTypeMAMPS;
            }
            else
            {
                throw CException("Platform type '" + platformTypeString
                                 + "'is not supported.");
            }
        }

        // Architecture graph
        if (!CHasChildNode(settingsNode, "architectureGraph"))
        {
            throw CException("File '" + settingsFile
                             + "' does not contains architectureGraph.");
        }
        archGraphNode = CGetChildNode(settingsNode, "architectureGraph");
        architectureGraph = CGetAttribute(archGraphNode, "file");

        // Application graphs
        if (!CHasChildNode(settingsNode, "applicationGraph"))
        {
            throw CException("File '" + settingsFile
                             + "' does not contains applicationGraph.");
        }
        appGraphNode = CGetChildNode(settingsNode, "applicationGraph");
        applicationGraph = CGetAttribute(appGraphNode, "file");

        if (CHasChildNode(settingsNode, "output"))
        {
            CNode *outputNode = CGetChildNode(settingsNode, "output");
            outputFile = CGetAttribute(outputNode, "file");
        }

        if (CHasChildNode(settingsNode, "missing"))
        {
            logWarning("Architecture values specified in sdf3.opt: TO BE REMOVED!");
            CNode *missing = CGetChildNode(settingsNode, "missing");
            if (CHasChildNode(missing, "wheelsize"))
            {
                CNode *n = CGetChildNode(missing, "wheelsize");
                if (CHasAttribute(n, "value"))
                {
                    wheelsize = int(CGetAttribute(n, "value"));
                }
            }
            if (CHasChildNode(missing, "contextSwitchOverhead"))
            {
                CNode *n = CGetChildNode(missing, "contextSwitchOverhead");
                if (CHasAttribute(n, "value"))
                {
                    contextSwitchOverhead = int(CGetAttribute(n, "value"));
                }
            }
            if (CHasChildNode(missing, "slotLength"))
            {
                CNode *n = CGetChildNode(missing, "slotLength");
                if (CHasAttribute(n, "value"))
                {
                    slotLength = int(CGetAttribute(n, "value"));
                }
            }
            if (CHasChildNode(missing, "preemptive"))
            {
                CNode *n = CGetChildNode(missing, "preemptive");
                if (CHasAttribute(n, "value"))
                {
                    if (CGetAttribute(n, "value") == "true")
                        preemptive = true;
                    else
                        preemptive = false;
                }
            }
        }
        releaseDoc(settingsDoc);
    }
    /**
     * initOutputStream ()
     * Initialize the output stream.
     */
    void ConvertSettings::initOutputStream()
    {
        // Set output stream
        if (!outputFile.empty())
            outputStream.open(outputFile.c_str());
        else
            ((ostream &)(outputStream)).rdbuf(cout.rdbuf());
    }

} // End namespace FSMSADF
