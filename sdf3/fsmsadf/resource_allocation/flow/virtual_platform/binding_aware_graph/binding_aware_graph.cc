/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   binding_aware_sdfg.cc
 *
 *  Author          :   Martijn Koedam (m.l.p.j.koedam@tue.nl)
 *
 *  Date            :   April 15, 2011
 *
 *  Function        :   Binding-aware SDFG
 *
 *  History         :
 *      15-04-11    :   Initial version.
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

#include "binding_aware_graph.h"

namespace FSMSADF
{

    /**
     * BindingAwareGraph()
     * Constructor.
     */
    BindingAwareGraphVirtualPlatform::BindingAwareGraphVirtualPlatform(PlatformBinding *b)
        :
        BindingAwareGraphBase(b)
    {

    }

    /**
     * ~BindingAwareGraph()
     * Destructor.
     */
    BindingAwareGraphVirtualPlatform::~BindingAwareGraphVirtualPlatform()
    {
    }


} // End namespace FSMSADF

