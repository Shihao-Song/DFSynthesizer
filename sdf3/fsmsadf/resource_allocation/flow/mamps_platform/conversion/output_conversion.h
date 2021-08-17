/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   output_conversion.h
 *
 *  Author          :   Umar Waqas (u.waqas@student.tue.nl)
 *
 *  Date            :   November 16, 2011
 *
 *  Function        :   Convert the output files to a compatible format.
 *
 *  History         :
 *      16-11-11    :   Initial version.
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_FLOW_MAMPS_PLATFORM_OUTPUT_CONVERSION_CONVERSION_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_FLOW_MAMPS_PLATFORM_OUTPUT_CONVERSION_CONVERSION_H_INCLUDED
#include <iostream>
#include <ostream>
#include <cstdlib>

#include "../../../../../base/exception/exception.h"
#include "../../base_platform/conversion/output_conversion.h"
#include "../../../../../base/xml/xml.h"
#include "../../compsoc_platform/conversion/helper.h"

namespace FSMSADF
{

    /**
     * OutputConversionMampsPlatform provides implementation
     * of the output conversion layer to make MAMPS-SDF3 compatible
     * with each other.
     *
     */
    class OutputConversionMampsPlatform : public OutputConversionBase
    {
        private:
            CDoc *output_doc;
        public:
            OutputConversionMampsPlatform(CString input_filename);
            virtual ~OutputConversionMampsPlatform();

            void convert();
            // write the result
            void writeOutput(std::ostream &out);
            void writeOutput(const std::string outfile);

        private:
            /* helper methods supporting the conversion */
            void convertMapping(CNode *sdf3_src, CNode *sdf3_des);
            void convertRoot(CNode *sdf3_src, CNode *sdf3_des);
            void convertTile(CNode *arch_src, CNode *appGraph,
                             CNode *sdf3_src, CNode *tile_src,
                             CNode *tile_dst);
            void convertProcessor(CNode *arch_src, CNode *tile_src,
                                  CNode *processor_dst);
            void convertMemory(CNode *appGraph, CNode *sdf3_src,
                               CNode *memory_src, CNode *memory_dst);
            void convertActor(CNode *appGraph, CNode *actor_src,
                              CNode *memory_dst);
            void convertChannel(CNode *appGraph, CNode *sdf3_src,
                                CNode *channel_src, CNode *memory_dst);
            void convertNetworkInterface(CNode *appGraph, CNode *sdf3_src,
                                         CNode *ni_src, CNode *memory_dst,
                                         CString tileName);
            void convertNetworkInterfaceChannel(CNode *resourceUsage, CNode *ni_channel_src,
                                                CNode *ni_dst, bool hasInAttribute,
                                                CString ni_name, CString tileName);
            void convertUsecaseSchedule(CNode *sdf3_src, CNode *sdf3_des);
            void convertUsecaseScheduleTile(CNode *tileSrc, CNode *applicationSchedule,
                                            CString name);
            void convertConnections(CNode *mapping_src, CNode *mapping_dst);



    };
}
#endif  /* CONVERSION_H */

