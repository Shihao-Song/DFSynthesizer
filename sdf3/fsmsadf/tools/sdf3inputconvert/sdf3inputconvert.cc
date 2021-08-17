/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf3convert.h
 *
 *  Author          :   Martijn Koedam (m.l.p.j.koedam@tue.nl)
 *
 *  Date            :   6 April 2011
 *
 *  Function        :   Input file conversion
 *
 *  History         :
 *      19-04-11    :   Initial version.
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

/**
 * <settings type="convert">
 *  <!-- 3 different platforms are supported: CompSoC, MAMPS, and Virtual.
 *      The virtual platform will just copy the subgraphs.         -->
 *  <platformType type="CompSoC"/>
 *  <!-- File from witch the applicationGraph should be generated. -->
 *  <applicationGraph file="../../Specification/app_test0.xml"/>
 *  <!-- File from the architecture graph should be generated      -->
 *  <architectureGraph file="../../Specification/architecture.xml"/>
 *  <output file="example.xml"/>
 *    <!-- stuff used to fix missing data SHOULD BE REMOVED -->
 *    <!-- these are in ns -->
 *    <missing>
 *      <wheelsize value="215000"/>
 *      <contextSwitchOverhead value="1500"/>
 *      <slotLength value="20000"/>
 *      <preemptive value="true"/>
 *  </missing>
 * </settings>
 */
#include "sdf3inputconvert.h"
#include "settings.h"

using namespace FSMSADF;
namespace FSMSADF
{
    /**
     * settings
     * Program settings.
     */
    ConvertSettings settings(MODULE, SETTINGS_TYPE);
    /**
     * helpMessage ()
     * Function prints help message for the tool.
     */
    void helpMessage(ostream &out)
    {
        out << "SDF3 " << TOOL << " (version " << DOTTED_VERSION ")" << endl;
        out << endl;
        out << "Usage: " << TOOL << " [--settings <file> --output <file>]";
        out << endl;
        out << "   --settings <file>  settings for algorithms (default: sdf3.opt)";
        out << endl;
        out << "   --output <file>    output file (default:stdout)" << endl;
        out << endl;
    }

    /**
     * convertInputFiles()
     *
     * Start the conversion process
     */
    void convertInputFiles(void)
    {
        InputConversionBase *conv = NULL;
        // Create a new mapping flow
        if (settings.platformType == PlatformTypeCompSoC)
        {
            conv = new InputConversionCompSoCPlatform();
            /// @todo remove!
            dynamic_cast<InputConversionCompSoCPlatform *>(conv)->preemptive = settings.preemptive;
            dynamic_cast<InputConversionCompSoCPlatform *>(conv)->wheelsize = settings.wheelsize;
            dynamic_cast<InputConversionCompSoCPlatform *>(conv)->contextSwitchOverhead = settings.contextSwitchOverhead;
            dynamic_cast<InputConversionCompSoCPlatform *>(conv)->slotLength = settings.slotLength;

        }
        else if (settings.platformType == PlatformTypeVirtual)
        {
            conv = new InputConversionVirtualPlatform();
        }
        else if (settings.platformType == PlatformTypeMAMPS)
        {
            conv = new InputConversionMampsPlatform();
        }
        else
        {
            throw CException("Unsupported platform type.");
        }

        conv->readApplicationFile(settings.applicationGraph);

        conv->readArchitectureFile(settings.architectureGraph);

        if (settings.outputFile.empty())
        {
            conv->writeOutput(std::cout);
        }
        else
        {
            conv->writeOutput(settings.outputFile);
        }
        delete conv;
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
} // end namespace
/**
 * main ()
 * It does none of the hard work, but it is very needed...
 */
int main(int argc, char **argv)
{
    int exit_status = EXIT_SUCCESS;

    try
    {
        // Initialize the program
        if (!initSettings(argc, argv))
            return EXIT_FAILURE;
        try
        {
            convertInputFiles();
        }
        catch (CException &e)
        {
            throw e;
        }
        // Run mapping flow

    }
    catch (CException &e)
    {
        cerr << e << endl;
        cerr << "Cause: " << e.getCause() << endl;
        exit_status = EXIT_FAILURE;
    }

    xmlCleanup();
    return exit_status;
}


