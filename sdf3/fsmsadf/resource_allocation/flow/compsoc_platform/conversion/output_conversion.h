/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   conversion.h
 *
 *  Author          :   Martijn Koedam (m.l.p.j.koedam@tue.nl)
 *
 *  Date            :   Mei 16, 2011
 *
 *  Function        :   Output format conversion
 *
 *  History         :
 *      16-05-11    :   Initial version.
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_FLOW_COMPSOC_PLATFORM_OUTPUT_CONVERSION_CONVERSION_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_FLOW_COMPSOC_PLATFORM_OUTPUT_CONVERSION_CONVERSION_H_INCLUDED
#include <iostream>
#include <ostream>

#include "../../../../../base/exception/exception.h"
#include "../../base_platform/conversion/output_conversion.h"
#include "../../../../../base/xml/xml.h"
#include "../../../../base/type.h"

namespace FSMSADF
{

    /**
     * Class that takes care to convert from the platform specific input file
     * to the generic format supported by sdf3.
     */
    class OutputConversionCompSoCPlatform : public OutputConversionBase
    {
        public:
            OutputConversionCompSoCPlatform(CString filename);
            virtual ~OutputConversionCompSoCPlatform();

            // See the __FILE__.c file to see help for each funcion.

            // write communication file.
            void writeCommunicationFile(const CString &file, const int mapping = -1);
            // Write mapping file.
            void writeMappingFile(CString output_file, int mapping = -1);
            void setApplicationInputFile(const CString &input_file);
            void setArchitectureInputFile(const CString &input_file);

        private:
            void architectureInputFileReadDefaultClockSpeed();
            double architectureInputFileReadTileClockSpeed(const CString tile_name);
            CString applicationInputFileGetPortDirection(CString app_name, CString actor_name, CString port_name);

            int applicationInputFileGetFifoArgument(CString app_name, CString actor_name, CString port_name);
            CString applicationInputFileGetFiringRule(CString app_name, CString actor_name, CString port_name);
            // Read the input file and get the dataUnit and timeUnit.
            void applicationInputFileReadUnits();
            // <communication>/<application>/<connection*>
            void writeCommunicationAppConnections(CNode *src_app_node,
                                                  CNode *dst_app_node);
            // <communication>/<application id="monitoring">
            void writeCommunicationMonitor(CNode *dst_root);
            // <communication>/<application id="fake">
            void writeCommunicationFakeNetwork(CNode *src_map_node, CNode *dst_root);

            CNode *getApplicationGraph(CNode *src_map_node);
            CNode *getArchitectureGraph(CNode *src_map_node);


            void writeMappingApplications(CNode *src_root, CNode *src_map_node, CNode *dst_root);
            void writeMappingApplicationsProcessor(CString procid, CNode *src_map_node, CNode *src_proc_node, CNode *dst_app);
            void writeApplicationProcessorTDMSlots(CString procid, CNode *src_map_node,
                                                   CNode *src_proc_node, CNode *dst_proc_node);
            void writeTaskScheduler(CString procid, CNode *src_map_node, CNode *src_proc_node, CNode *dst_proc_node);
            void writeSlackManager(CString procid, CNode *src_proc_node, CNode *dst_proc_node);
            void writeMappingProcessorTiles(CNode *src_root, CNode *src_map_node, CNode *dst_root);

            CStrings getProcSchedule(CString procid, CNode *src_map_node);
            Size getMemorySizeActor(CNode *src_map_node, CString actor_name, MemoryType mem_type);
            int getNumFifos(CNode *src_map_node, CString actor_name, CString type);
            CString getChannelActorOrPort(CNode *src_map_node, CString chan_name, CString type);

            Size getChannelSize(CNode *src_map_node, CString chan_name);
            CString getChannelDMA(CNode *src_tile_out, CString src_fifo_name);

            Size getChannelTransactionSize(CNode *src_map_node, CString chan_name);
            CNode *getChannelMemory(CNode *src_map_node, CString chan_name, CString bloc);
            Bandwidth getChannelBandwidth(CNode *src_map_node, CString chan_name);
            CNode *findChannelTile(CNode *src_map_node, CString chan_name, CString type);

            int getMaxChannelRate(CNode *src_map_node, CString chan_name, CString loc);

            int getNumScenarios(CNode *app_graph);

        private:

            /// Application document. This one is needed as input document.
            CDoc *application_document;
            CDoc *architecture_document;

            /// The time unit size (in seconds)
            double timeUnit;
            /// The data unit (in bytes)
            double dataUnit;
            /// The number of time-units in a cycle
            /// for the NoC. This is the default 'clock'
            double cycleTime;

    };
} // end namespace
#endif  /* CONVERSION_H */

