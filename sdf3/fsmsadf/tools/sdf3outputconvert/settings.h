/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   settings.h
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
 * $Id: settings.h,v 1.1.2.4 2010-08-01 17:26:44 mgeilen Exp $
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

#ifndef FSMSADF_TOOLS_SDF3FLOW_SETTINGS_H_INCLUDED
#define FSMSADF_TOOLS_SDF3FLOW_SETTINGS_H_INCLUDED

#include "../../fsmsadf.h"
#include "../base/tools.h"

namespace FSMSADF
{

    /**
     * Settings
     * Object containing all settings for the tool.
     */
    class ConvertSettings: public Settings
    {
        public:
            enum ConvType
            {
                Input,
                Output
            };
            // Constructor
            ConvertSettings(CString module, CString type);

            // Destructor
            ~ConvertSettings();

            // Initialize settings
            void init(int argc, char **argv);
            void init(CStrings args);

            // Output stream
            void initOutputStream();

        private:
            // Parsing of settings
            void parseArguments(CStrings args);
            void parseSettingsFile();

        public:
            // MoC supported by the tool
            CString module;

            // Settings type used by the tool
            CString type;

            // Usage information of the tool requested
            bool helpFlag;

            // Settings file
            CString settingsFile;

            // Output stream
            ofstream outputStream;

            // Run flow step-by-step
            bool stepFlag;

            // Output mapping in HTML format
            bool outputHTML;

            // Application graphs
            CString inputFile;
            // Communication output file
            CString communicationFile;
            // mapping output file
            CString mappingFile;
            int mapping;

            // The Application file
            CString applicationFile;
            // The architectureFile
            CString architectureFile;

            // PlatformType
            PlatformType platformType;

    };

} // End namespace FSMSADF

#endif
