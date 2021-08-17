/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   binding.h
 *
 *  Author          :   Umar Waqas (u.waqas@student.tue.nl)
 *
 *  Date            :   November 16, 2011
 *
 *  Function        :   Memory dimensioning
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


#ifndef FSMSADF_RESOURCE_ALLOCATION_FLOW_MAMPS_PLATFORM_TILE_BINDING_H_INCLUDED
#define FSMSADF_RESOURCE_ALLOCATION_FLOW_MAMPS_PLATFORM_TILE_BINDING_H_INCLUDED

#include "../../base_platform/tile/binding.h"
#include "../../base_platform/tile/graph_binding.h"
#include "graph_binding.h"

namespace FSMSADF
{

    /**
     * Tile binding and scheduling
     */
    class TileBindingAlgoMampsPlatform : public TileBindingAlgoBase
    {
        public:
            // Constructor
            TileBindingAlgoMampsPlatform();

            // Destructor
            ~TileBindingAlgoMampsPlatform();

            GraphBindingBase *createGraphBinding(PlatformBinding *pb);
            bool constructTileSchedules(const uint maxNrAppBindings);

        protected:
            // Throughput
            Throughput analyzeThroughput(PlatformBinding *pb);
    };

} // End namespace FSMSADF

#endif
