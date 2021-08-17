/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   tools.h
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


#ifndef FSMSADF_TOOLS_BASIS_H_INCLUDED
#define FSMSADF_TOOLS_BASIS_H_INCLUDED

#include "base/string/cstring.h"
#include "base/xml/xml.h"

namespace FSMSADF
{

    typedef struct _CPair
    {
        CString key;
        CString value;
    } CPair;

    typedef list<CPair>         CPairs;
    typedef CPairs::iterator    CPairsIter;

    class Settings
    {
        public:
            // Output file
            CString outputFile;
    };


    class Tool
    {
        public:
            virtual void parseCommandLine(int argc, char **argv);
            virtual bool parseCommand(int argc, char **argv, int *arg) = 0;
            virtual CPairs parseSwitchArgument(CString arguments);
            virtual void initSettings(const CString &modulename, int argc, char **argv);
            virtual void setDefaults(void);
            virtual bool checkRequiredSettings(void) = 0;
            virtual void helpMessage(ostream &out) = 0;
            virtual void cleanUp();
            virtual void postInitSettings() {}
        protected:
            Settings *_settings;

    };

}

#endif