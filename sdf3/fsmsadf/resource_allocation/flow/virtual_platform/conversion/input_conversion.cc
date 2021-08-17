/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   conversion.h
 *
 *  Author          :   Martijn Koedam (m.l.p.j.koedam@tue.nl)
 *
 *  Date            :   April 20, 2011
 *
 *  Function        :   Input/Output format conversion
 *
 *  History         :
 *      20-04-11    :   Initial version.
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
#include <assert.h>
namespace FSMSADF
{

    // constructor

    InputConversionVirtualPlatform::InputConversionVirtualPlatform()
        :
        InputConversionBase(),
        internal_doc(NULL)
    {
        CNode *root = CNewNode("sdf3");
        output_doc = CNewDoc(root);
        CSetAttribute(root, "type", "fsmsadf");
    }

    //destructor

    InputConversionVirtualPlatform::~InputConversionVirtualPlatform()
    {
        delete internal_doc;
    }

    void InputConversionVirtualPlatform::readApplicationFile(const std::string input_file)
    {
        internal_doc = CParseFile(input_file);
        if (internal_doc == NULL)
        {
            throw CException("Failed to read input file.");
        }
        CNode *root = CGetRootNode(internal_doc);
        CNode *application_graph_src = CFindNode(root, "applicationGraph");

        CNode *application_graph_dest = CCopyNode(application_graph_src);

        CAddNode(CGetRootNode(output_doc), application_graph_dest);

        internal_doc = NULL;
    }

    void InputConversionVirtualPlatform::readArchitectureFile(const std::string input_file)
    {
        internal_doc = CParseFile(input_file);
        if (internal_doc == NULL)
        {
            throw CException("Failed to read input file.");
        }
        CNode *root = CGetRootNode(internal_doc);
        CNode *application_graph_src = CFindNode(root, "architectureGraph");

        CNode *application_graph_dest = CCopyNode(application_graph_src);

        CAddNode(CGetRootNode(output_doc), application_graph_dest);

        internal_doc = NULL;
    }
}