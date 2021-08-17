/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   conversion.h
 *
 *  Author          :   Martijn Koedam (m.l.p.j.koedam@tue.nl)
 *
 *  Date            :   April 19, 2011
 *
 *  Function        :   Input/Output format conversion
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

#include "input_conversion.h"
#include "base/exception/exception.h"
#include "base/log/log.h"

namespace FSMSADF
{

    // constructor

    InputConversionBase::InputConversionBase()
        :
        output_doc(NULL)
    {
        CNode *root = CNewNode("sdf3");
        output_doc = CNewDoc(root);
        CSetAttribute(root, "type", "fsmsadf");
    }

    //destructor

    InputConversionBase::~InputConversionBase()
    {
        if (output_doc != NULL)
            releaseDoc(output_doc);
    }

    /**
     * writeOutput()
     * Write the converted structure out to #ostream out.
     *
     * @param out The ostream to output the result.
     */
    void InputConversionBase::writeOutput(std::ostream &out)
    {
        ASSERT(output_doc != NULL, "No output document exists.");

        if (!CValidate(output_doc))
        {
            logWarning("Failed to validate output document");
        }
        logInfo("Write output document to stream");
        // write the result out.
        CSaveFile(out, output_doc, 1);
    }

    /**
     * writeOutput()
     * Write the converted structure out to file outfile.
     *
     * @param outfile Path to the file to write the result too.
     */
    void InputConversionBase::writeOutput(const std::string outfile)
    {
        ASSERT(output_doc != NULL, "No output document exists.");

        if (!CValidate(output_doc))
        {
            logWarning("Failed to validate output document: '" + outfile + "'");
        }
        logInfo("Write output document to file: '" + outfile + "'");
        // write the result out.
        CSaveFile(outfile, output_doc, 1);
    }

}
