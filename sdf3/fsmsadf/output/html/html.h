/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   html.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   December 6, 2009
 *
 *  Function        :   Output FSM-based SADF in HTML format
 *
 *  History         :
 *      06-12-09    :   Initial version.
 *
 * $Id: html.h,v 1.2 2010-02-08 08:04:42 sander Exp $
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

#ifndef FSMSADF_OUTPUT_HTML_HTML_H_INCLUDED
#define FSMSADF_OUTPUT_HTML_HTML_H_INCLUDED

#include "../../resource_allocation/flow/base_platform/base/flow.h"

namespace FSMSADF
{

    /**
     * OutputHTML
     * Output FSM-based SADF in HTML format.
     */
    class OutputHTML
    {
        public:
            // Constructor
            OutputHTML();

            // Destructor
            ~OutputHTML();

            // Settings
            void setDirname(const CString &name)
            {
                dirname = name;
            };
            CString getDirname() const
            {
                return dirname;
            };

            // Output
            void outputAsHTML(SDF3FlowBase *flow);

        private:
            // Application graph
            void printApplicationGraph(SDF3FlowBase *flow);

            // Platform graph
            void printPlatformGraph(SDF3FlowBase *flow);

            // Platform bindings
            void printPlatformBindings(SDF3FlowBase *flow);
            void printPlatformBinding(PlatformBinding *pb, SDF3FlowBase *flow);

            // Create directory for HTML output
            void createOutputDirectory();

            // Create output file
            void createOutputFileApplicationGraph(Graph *applicationGraph,
                                                  ofstream &out);
            void createOutputFilePlatformGraph(PlatformGraph *platformGraph,
                                               ofstream &out);
            void createOutputFilePlatformBinding(PlatformBinding *platformBinding,
                                                 ofstream &out);
            void createOutputFilePlatformBindings(ofstream &out);
            void createOutputFile(const CString &name, ofstream &out);

            // Header and footer
            void printHeader(ostream &ofile, SDF3FlowBase *flow);
            void printFooter(ostream &ofile, SDF3FlowBase *flow);
            void printStylesheet(ostream &ofile);

            // Application graph
            void printScenario(Scenario *s, ostream &ofile);

            // FSM
            void printFSM(FSM *fsm, ostream &ofile);

            // URLs
            CString getApplicationGraphURL(Graph *applicationGraph);
            CString getScenarioURL(Scenario *s, bool onlyName = false);
            CString getActorURL(Scenario *s, Actor *a, bool onlyName = false);
            CString getChannelURL(Scenario *s, Channel *c, bool onlyName = false);
            CString getPlatformGraphURL(PlatformGraph *platformGraph);
            CString getPlatformBindingURL(PlatformBinding *platformBinding);
            CString getPlatformBindingsURL();
            CString getScenarioGraphPicURL(Scenario *s);
            CString getFSMPicURL();
            CString getTileURL(Tile *t, bool onlyName = false);
            CString getConnectionURL(Connection *c, bool onlyName = false);
            CString getPlatformGraphPicURL(PlatformGraph *pg);
            CString getNetworkInterfaceURL(NetworkInterface *n, bool onlyName = false);
            CString getProcessorURL(Processor *p, bool onlyName = false);
            CString getMemoryURL(Memory *m, bool onlyName = false);
            CString getScenarioPlatformBindingURL(Scenario *s, PlatformBinding *pb,
                                                  bool onlyName = false);
            CString getPlatformBindingPicURL(Scenario *s, PlatformBinding *pb);

            // Pictures
            bool convertGraphToPNG(CString &graph, CString &filename, CString &map);
            void convertScenarioGraphToPNG(Scenario *s, ScenarioGraph *sg,
                                           CString filename, CString &map);
            void convertFSMToPNG(FSM *fsm, CString filename, CString &map);
            void convertPlatformGraphToPNG(PlatformGraph *pg, CString filename,
                                           CString &map);
            void convertPlatformBindingToPNG(Scenario *s, PlatformBinding *pb,
                                             CString filename, CString &map);

        private:
            CString dirname;
    };

} // End namespace FSMSADF

#endif

