/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   flow.h
 *
 *  Author          :   Martijn Koedam (M.L.P.J.Koedam@tue.nl)
 *
 *  Date            :   April 14, 2011
 *
 *  Function        :   Base Implementation of SDF3Flow
 *
 *  History         :
 *       14-4-11    :   Initial version.
 *
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_FLOW_VIRTUAL_PLATFORM_BASE_FLOW_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_FLOW_VIRTUAL_PLATFORM_BASE_FLOW_H_INCLUDED

#include "../../base_platform/base/flow.h"

namespace FSMSADF
{

    /**
     * Graph mapping to MP-SoC flow
     *
     * This is the base implementation that just inherits everything from the
     * base_platform.
     */
    class SDF3FlowVirtualPlatform : public SDF3FlowBase
    {
        public:
            SDF3FlowVirtualPlatform();
            ~SDF3FlowVirtualPlatform();
    };

} // End namespace FSMSADF

#endif
