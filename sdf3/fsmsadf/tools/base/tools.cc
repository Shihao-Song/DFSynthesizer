/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   tools.cc
 *
 *  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
 *
 *  Date            :   6 April 2009
 *
 *  Function        :   Generic tool functionality
 *
 *  History         :
 *      06-04-09    :   Initial version.
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


#include "tools.h"
#include "base/exception/exception.h"

namespace FSMSADF
{


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
    CPairs Tool::parseSwitchArgument(CString arguments)
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
                    if (arguments.operator []((int) ePos) == ')')
                        level--;
                    else if (arguments.operator []((int) ePos) == '(')
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
    void Tool::parseCommandLine(int argc, char **argv)
    {
        int arg = 1;

        while (arg < argc)
        {
            if (! this->parseCommand(argc, argv, &arg))
            {
                helpMessage(cerr);
                throw CException("");
            }
            // Next argument
            arg++;
        }
    }


    /**
     * initSettings ()
     * The function initializes the program settings.
     */
    void Tool::initSettings(const CString &modulename, int argc, char **argv)
    {
        // Defaults
        this->setDefaults();

        // Parse the command line
        this->parseCommandLine(argc, argv);

        // Check required settings
        if (!this->checkRequiredSettings())
        {
            helpMessage(cerr);
            throw CException("");
        }

        // any post processing based on the settings
        this->postInitSettings();

    }

    void Tool::setDefaults()
    {
    }

    void Tool::cleanUp()
    {
    }


}