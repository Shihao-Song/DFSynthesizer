/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf3generate.h
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
 * $Id: sdf3generate.h,v 1.1.2.4 2010-05-11 03:27:08 mgeilen Exp $
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

#ifndef FSMSADF_TOOLS_SDF3GENERATE_SDF3GENERATE_H_INCLUDED
#define FSMSADF_TOOLS_SDF3GENERATE_SDF3GENERATE_H_INCLUDED

/**
 * Tool name and version
 */
#define TOOL               "sdf3generate-fsmsadf"
#define DOTTED_VERSION     "1.0"
#define VERSION             10000

/**
 * Module (MoC) supported by the tool
 */
#define MODULE             "fsmsadf"

/**
 * Settings type used by the tool
 */
#define SETTINGS_TYPE      "generate"

#include <iostream>
#include "../../fsmsadf.h"
#include "../base/tools.h"

namespace FSMSADF
{

    /**
     * Settings
     * Struct to store program settings.
     */
    class GenerateSettings: public Settings
    {
        public:
            // settings file
            CString settingsFile;

            // Random graph settings node inside the settings file
            CNode *settingsNode;
    };

    class ToolGenerate: public Tool
    {
        public:
            virtual void helpMessage(std::ostream &out);
            virtual void generateRandomGraph(CNode *settingsNode, ostream &out);
            virtual void initSettings(const CString &modulename, int argc, char **argv);
            virtual void postInitSettings(void);
            virtual void setDefaults(void);
            virtual bool parseCommand(int argc, char **argv, int *arg);
            virtual bool checkRequiredSettings(void);
            virtual void cleanUp();
            GenerateSettings *settings()
            {
                return (GenerateSettings *) this->_settings;
            }
        private:
            void parseSettingsFile(CString module, CString type);
    };

}
#endif
