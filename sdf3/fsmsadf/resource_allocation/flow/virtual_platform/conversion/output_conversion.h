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

#ifndef FSMSADF_RESOURCE_ALLOCATION_FLOW_VIRTUAL_PLATFORM_OUTPUT_CONVERSION_CONVERSION_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_FLOW_VIRTUAL_PLATFORM_OUTPUT_CONVERSION_CONVERSION_H_INCLUDED
#include <iostream>
#include <ostream>

#include "../../../../../base/exception/exception.h"
#include "../../base_platform/conversion/output_conversion.h"
#include "../../../../../base/xml/xml.h"
namespace FSMSADF
{

    /**
     * Dummy class that should do exactly nothing.
     */
    class OutputConversionVirtualPlatform : public OutputConversionBase
    {
        private:
            CDoc *output_doc;
        public:
            OutputConversionVirtualPlatform(CString input_filename);
            virtual ~OutputConversionVirtualPlatform();

            void convert();
            // write the result
            void writeOutput(std::ostream &out);
            void writeOutput(const std::string outfile);

    };
}
#endif  /* CONVERSION_H */

