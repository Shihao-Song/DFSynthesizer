/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   helper.cc
 *
 *  Author          :   Martijn Koedam (m.l.p.j.koedam@tue.nl)
 *
 *  Date            :   Aug 9, 2011
 *
 *  Function        :   Output format conversion
 *
 *  History         :
 *      09-08-11    :   Initial version.
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

#include "helper.h"
// regular expression support
#ifdef _MSC_VER
#include <regex>
using namespace std::tr1;
#else
#include <boost/regex.hpp>
using namespace boost;
#endif

namespace FSMSADF
{
    /**
     * Converts a string of {1,2,3,4} into a vector.
     * Strings in shorthand like {2x1,3,3x4} are turned in to a vector
     * {1,1,3,4,4,4}
     *
     * Throws an #CException if the input cannot be parsed.
     *
     * @param input a list of , separated values and/or shorthand.
     * @return a vector with CSting elements.
     */
    vector<CString> expand(CString input)
    {
        vector<CString> retv;
        CStrings elems = input.split(',');

        for (CStrings::iterator e = elems.begin(); e != elems.end(); e++)
        {
            CString element = *e;
            CStrings t = (element).split('x');

            if (t.size() > 1)
            {
                if (t.size() != 2)
                    throw CException("String is incorrectly formatted");

                CString times = t.front();
                CString value = t.back();

                for (int j = int(times); j > 0; j--)
                {
                    retv.push_back(value);
                }
            }

            else
            {
                CString value = element;
                retv.push_back(value);
            }
        }

        return retv;
    }

    /**
     * Find a node with name %name under parent where attribute %field == %value
     *
     * @param parent the parent #CNode
     * @param name the name of the node to find.
     * @param field the name of the attribute to mach
     * @param value the value of the attribute.
     * @return a CNode or an exception if not found.
     */
    CNode *CFindNodeWithAttribute(CNode *parent,
                                  CString name,
                                  CString field,
                                  CString value)
    {
        CNode *node = CFindNode(parent, name);

        for (; node != NULL; node = CNextNode(node, name))
        {
            if (CHasAttribute(node, field))
            {
                CString node_id = CGetAttribute(node, field);

                if (node_id == value)
                {
                    return node;
                }
            }
        }
        throw CException(CString(CString(__FUNCTION__) +
                                 ": No node '" +
                                 name + "' found where "
                                 + field + "==" + value));
    }

    /**
     * Helper function
     * @param child A child #CNode of the the document.
     *
     * This function returns the toplevel node of #child.
     * It does this by going down the tree using the parent pointer.
     *
     * @return the root node of #child
     */
    CNode *getRootNode(CNode *child)
    {
        ASSERT(child != NULL, CString(__FUNCTION__) + ": Child != FAILED");
        CNode *root, *temp = child;

        do
        {
            root = temp;
            temp = CGetParentNode(temp);
        }
        while (temp != NULL);

        return root;
    }

    /**
     * Helper function.
     * Gets the numeric id of the memory.
     * So cmemin4 -> 4, cmemout0->0
     *
     * @param id the name of the memory f.e. cmemin4 or cmemout0
     * @return -1 when not found. >= 0 when found
     */
    int HelperGetMemId(CString id)
    {
        int res_id = -1;
        cmatch what;
        regex expression("(cmemin|cmemout|dma)([0-9+])");

        if (regex_match((const char *) id, what, expression))
        {
            res_id = std::atoi(what[2].first);
        }

        return res_id;
    }
}