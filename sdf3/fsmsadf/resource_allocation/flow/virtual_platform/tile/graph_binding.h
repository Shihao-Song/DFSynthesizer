/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   graph_binding.h
 *
 *  Author          :   Martijn Koedam (m.l.p.j.koedam@tue.nl)
 *
 *  Date            :   April 19, 2011
 *
 *  Function        :   Graph binding
 *
 *  History         :
 *      19-04-11    :   Initial version.
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

#ifndef FSMSADF_RESOURCE_ALLOCATION_FLOW_VIRTUAL_PLATFORM_TILE_GRAPH_BINDING_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_FLOW_VIRTUAL_PLATFORM_TILE_GRAPH_BINDING_H_INCLUDED

#include "../../../platform_binding/graph.h"
#include "../../base_platform/tile/graph_binding.h"

namespace FSMSADF
{

    /**
     * class GraphBinding
     * This class annotates the platform binding with information used by the
     * tile binding algorithm.
     */
    class GraphBindingVirtualPlatform : public GraphBindingBase
    {

            // Constructor
        public:
            GraphBindingVirtualPlatform(PlatformBinding *b)
                : GraphBindingBase(b)
            {

            }

            // Destructor
            ~GraphBindingVirtualPlatform()
            {

            }
            GraphBindingBase *clone() const;

    };

} // End namespace FSMSADF

#endif
