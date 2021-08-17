/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   graph_binding.cc
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

#include "graph_binding.h"

namespace FSMSADF
{

    /**
     * clone()
     * Create a clone of this graph binding. The cloned copy will contain a cloned
     * platfom binding object.
     */
    GraphBindingBase *
    GraphBindingCompSoCPlatform::clone() const
    {
        GraphBindingCompSoCPlatform *gb = new GraphBindingCompSoCPlatform(
            getPlatformBinding());

        // Copy internal values to the new instance.
        this->copyInternalsTo(gb);

        return gb;
    }

} // End namespace FSMSADF

