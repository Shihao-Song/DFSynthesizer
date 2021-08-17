/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   helper.h
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_FLOW_COMPSOC_PLATFORM_CONVERSION_HELPER_H
#define FSMSADF_RESOURCE_ALLOCATION_FLOW_COMPSOC_PLATFORM_CONVERSION_HELPER_H
#include <vector>
#include "base/base.h"
namespace FSMSADF
{
    vector<CString> expand(CString input);

    CNode *CFindNodeWithAttribute(CNode *parent,
                                  CString name,
                                  CString field,
                                  CString value);
    CNode *getRootNode(CNode *child);

    int HelperGetMemId(CString id);

}
#endif  /* HELPER_H */

